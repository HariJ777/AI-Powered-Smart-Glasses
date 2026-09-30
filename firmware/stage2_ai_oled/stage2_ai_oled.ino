/*
 * ============================================================
 *  AI Smart Glasses — Stage 2 Firmware
 *  Real AI Responses on OLED via Flask Server
 * ============================================================
 *
 *  Hardware needed: ESP32-CAM + SSD1306 OLED (GPIO 13/14)
 *  No buttons required — uses Serial commands + auto-timer
 *
 *  MODES:
 *    0 = DESCRIBE  → Camera → Groq Llama 4 → Scene description
 *    1 = READ TEXT → Camera → EasyOCR     → Extract text
 *
 *  SERIAL COMMANDS (type in Serial Monitor):
 *    d → Capture + Describe scene now
 *    r → Capture + Read text (OCR) now
 *    m → Switch mode
 *    s → Show server status
 *
 *  AUTO: Captures every 20 seconds automatically
 *
 *  Flask request formats (confirmed from main.py):
 *    POST /analyze-image
 *    Body: { "image_base64": "...", "analysis_type": "description"|"ocr" }
 *    Response: { "result": "...", "response_time_seconds": ... }
 *
 *  SETUP:
 *    1. Edit WIFI_SSID, WIFI_PASSWORD, SERVER_IP below
 *    2. Start Flask: cd test_multi_model_project && python main.py
 *    3. Upload (GPIO 0 → GND during upload, remove after)
 *    4. Open Serial Monitor at 115200 baud
 * ============================================================
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "mbedtls/base64.h"

// ─────────────────────────────────────────────
// ⚙️  CONFIGURE THESE 3 LINES
// ─────────────────────────────────────────────
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define SERVER_IP       "192.168.1.100"    // Your PC IP (run ipconfig)
// ─────────────────────────────────────────────

#define SERVER_PORT     5000
#define AUTO_CAPTURE_MS 20000   // Auto capture every 20 seconds

// OLED
#define PIN_SDA   13
#define PIN_SCL   14
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Camera pins (AI-Thinker)
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM   0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27
#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM     5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

// Modes
enum Mode { MODE_DESCRIBE = 0, MODE_OCR = 1 };
const char* MODE_NAMES[]   = { "DESCRIBE", "READ TEXT" };
const char* MODE_ICONS[]   = { "CAM+AI", "CAM+OCR" };
const char* MODE_DESC[]    = { "Scene description", "Extract text/OCR" };

int   currentMode    = MODE_DESCRIBE;
bool  actionPending  = false;
unsigned long lastCapture = 0;

String serverBase = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT);

// ─────────────────────────────────────────────
// OLED Helpers
// ─────────────────────────────────────────────

void oledClear() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void oledShowMode() {
  oledClear();
  // Header bar
  display.fillRect(0, 0, 128, 13, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 3);
  display.print("AI SMART GLASSES");
  display.setTextColor(SSD1306_WHITE);

  // Current mode
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.print(MODE_NAMES[currentMode]);

  // Description
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print(MODE_DESC[currentMode]);

  // Footer
  display.setCursor(0, 54);
  display.print("Auto: 20s | Cmds: d r m");

  display.display();
}

void oledStatus(String msg, bool clearFirst = true) {
  if (clearFirst) oledClear();
  display.setTextSize(1);
  display.setCursor(0, 26);
  display.println(msg);
  display.display();
  Serial.println("[OLED] " + msg);
}

void oledProcessing(String modeName) {
  oledClear();
  display.setCursor(0, 0);
  display.println("[" + modeName + "]");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  if (currentMode == MODE_DESCRIBE) {
    display.setCursor(0, 18); display.println("Capturing image...");
    display.setCursor(0, 30); display.println("Sending to Groq AI...");
    display.setCursor(0, 42); display.println("Llama 4 Scout processing");
  } else {
    display.setCursor(0, 18); display.println("Capturing image...");
    display.setCursor(0, 30); display.println("Running EasyOCR...");
    display.setCursor(0, 42); display.println("Extracting text...");
  }

  // Animated dots progress
  for (int i = 0; i < 3; i++) {
    display.fillRect(40 + (i * 20), 54, 14, 8, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(44 + (i * 20), 55);
    display.print(".");
    display.setTextColor(SSD1306_WHITE);
    display.display();
    delay(300);
  }
}

void oledShowResponse(String modeName, String response, float respTime) {
  oledClear();

  // Header
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print(modeName + " RESULT");
  display.setTextColor(SSD1306_WHITE);

  // Response text
  display.setCursor(0, 15);
  // Fit ~5 lines × 21 chars = 105 chars max
  if (response.length() > 100) {
    response = response.substring(0, 97) + "...";
  }
  display.print(response);

  // Footer with response time
  display.drawLine(0, 55, 128, 55, SSD1306_WHITE);
  display.setCursor(0, 57);
  display.print(String(respTime, 1) + "s | any key=next");

  display.display();
}

void oledError(String error) {
  oledClear();
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2); display.print("ERROR");
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 16);
  display.println(error);
  display.display();
  Serial.println("[ERROR] " + error);
}

// ─────────────────────────────────────────────
// Camera Init
// ─────────────────────────────────────────────

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM; config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_QVGA;  // 320x240 — best for speed
  config.jpeg_quality = 12;
  config.fb_count     = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAM] Init failed: 0x%x\n", err);
    return false;
  }
  Serial.println("[CAM] OV2640 initialized OK");
  return true;
}

// ─────────────────────────────────────────────
// Base64 Encode
// ─────────────────────────────────────────────

String base64Encode(uint8_t* data, size_t len) {
  size_t encodedLen = ((len + 2) / 3) * 4 + 1;
  uint8_t* encoded = (uint8_t*)malloc(encodedLen);
  if (!encoded) return "";

  size_t outLen = 0;
  mbedtls_base64_encode(encoded, encodedLen, &outLen, data, len);
  String result = String((char*)encoded).substring(0, outLen);
  free(encoded);
  return result;
}

// ─────────────────────────────────────────────
// Core: Capture + Send to Flask + Show Result
// ─────────────────────────────────────────────

void captureAndProcess() {
  String modeName = String(MODE_NAMES[currentMode]);
  String analysisType = (currentMode == MODE_DESCRIBE) ? "description" : "ocr";

  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("[ACTION] Mode: " + modeName);

  // Show processing screen
  oledProcessing(modeName);

  // 1. Capture frame
  Serial.println("[CAM] Capturing frame...");
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    oledError("Camera capture\nfailed!\nCheck ribbon cable");
    Serial.println("[ERROR] Camera capture failed");
    delay(3000);
    oledShowMode();
    return;
  }
  Serial.printf("[CAM] Frame: %dx%d | %d bytes\n", fb->width, fb->height, fb->len);

  // 2. Base64 encode
  Serial.println("[NET] Encoding image...");
  String b64 = base64Encode(fb->buf, fb->len);
  esp_camera_fb_return(fb);

  if (b64.isEmpty()) {
    oledError("Memory error\nduring encoding");
    delay(3000);
    oledShowMode();
    return;
  }
  Serial.printf("[NET] Encoded: %d chars\n", b64.length());

  // 3. Build JSON payload
  // Using ArduinoJson to build properly escaped JSON
  DynamicJsonDocument doc(b64.length() + 200);
  doc["image_base64"]  = b64;
  doc["analysis_type"] = analysisType;
  String payload;
  serializeJson(doc, payload);

  // 4. POST to Flask
  Serial.println("[NET] POSTing to " + serverBase + "/analyze-image ...");
  HTTPClient http;
  http.begin(serverBase + "/analyze-image");
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(30000);  // 30s for AI processing

  unsigned long t0 = millis();
  int httpCode = http.POST(payload);
  float respTime = (millis() - t0) / 1000.0;

  Serial.printf("[NET] HTTP %d | %.1fs\n", httpCode, respTime);

  if (httpCode == 200) {
    String body = http.getString();
    Serial.println("[AI] Raw response: " + body.substring(0, 200));

    // Parse JSON response
    DynamicJsonDocument resp(4096);
    DeserializationError err = deserializeJson(resp, body);

    String result;
    if (!err) {
      // Try common response fields
      if (resp.containsKey("result"))      result = resp["result"].as<String>();
      else if (resp.containsKey("text"))   result = resp["text"].as<String>();
      else if (resp.containsKey("answer")) result = resp["answer"].as<String>();
      else                                 result = body.substring(0, 100);
    } else {
      result = body.substring(0, 100);
    }

    result.trim();
    Serial.println("[AI] Result: " + result);

    // Show on OLED
    oledShowResponse(modeName, result, respTime);

    // Keep showing for 8 seconds, then back to mode screen
    delay(8000);

  } else {
    String errBody = http.getString();
    Serial.println("[ERROR] Server: " + errBody.substring(0, 100));

    String errMsg = "HTTP " + String(httpCode);
    if (httpCode == -1)      errMsg = "Server offline\nStart python main.py";
    else if (httpCode == 500) errMsg = "Server error\nCheck Flask logs";
    else if (httpCode == 429) errMsg = "API rate limit\nTry again later";

    oledError(errMsg);
    delay(4000);
  }

  http.end();
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  oledShowMode();
  lastCapture = millis();
}

// ─────────────────────────────────────────────
// WiFi + Server Setup
// ─────────────────────────────────────────────

bool connectWiFi() {
  oledStatus("Connecting WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    String ip = WiFi.localIP().toString();
    Serial.println("\n[OK] WiFi: " + ip);
    oledClear();
    display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(2, 2); display.print("WiFi Connected!");
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 18); display.println("IP: " + ip);
    display.setCursor(0, 32); display.println("SSID: " + String(WIFI_SSID));
    display.display();
    delay(2000);
    return true;
  }

  oledError("WiFi FAILED!\nCheck SSID+Pass\nin firmware");
  return false;
}

bool checkServer() {
  oledStatus("Pinging server...");
  HTTPClient http;
  http.begin(serverBase + "/health");
  http.setTimeout(5000);
  int code = http.GET();

  if (code == 200) {
    String body = http.getString();
    Serial.println("[OK] Server: " + body.substring(0, 80));
    oledClear();
    display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(2, 2); display.print("Server: ONLINE");
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 18); display.println(SERVER_IP + String(":") + String(SERVER_PORT));
    display.setCursor(0, 32); display.println("Flask AI Backend OK");
    display.setCursor(0, 46); display.println("Models: Groq + EasyOCR");
    display.display();
    delay(2500);
    http.end();
    return true;
  }

  Serial.printf("[WARN] Server returned %d\n", code);
  oledError("Server OFFLINE!\nRun on PC:\npython main.py");
  http.end();
  return false;
}

// ─────────────────────────────────────────────
// Handle Serial Commands
// ─────────────────────────────────────────────

void handleSerialCommand(char cmd) {
  switch (cmd) {
    case 'd': case 'D':
      Serial.println("[CMD] DESCRIBE mode → capturing...");
      currentMode = MODE_DESCRIBE;
      oledShowMode();
      delay(500);
      actionPending = true;
      break;

    case 'r': case 'R':
      Serial.println("[CMD] READ TEXT (OCR) mode → capturing...");
      currentMode = MODE_OCR;
      oledShowMode();
      delay(500);
      actionPending = true;
      break;

    case 'm': case 'M':
      currentMode = (currentMode + 1) % 2;
      Serial.println("[CMD] Mode → " + String(MODE_NAMES[currentMode]));
      oledShowMode();
      break;

    case 's': case 'S':
      Serial.println("[CMD] Checking server status...");
      checkServer();
      oledShowMode();
      break;

    default:
      // Any other key = trigger current mode
      Serial.println("[CMD] Triggering current mode: " + String(MODE_NAMES[currentMode]));
      actionPending = true;
      break;
  }
}

// ─────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("════════════════════════════════════════════");
  Serial.println("  AI Smart Glasses — Stage 2 Firmware");
  Serial.println("  Real AI Responses via Flask Server");
  Serial.println("════════════════════════════════════════════");
  Serial.println("  Commands: d=Describe  r=ReadText  m=Mode  s=Status");
  Serial.println("════════════════════════════════════════════\n");

  // OLED
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[ERROR] OLED not found! Check GPIO13/14 + 4.7kΩ resistors");
    while (true) delay(1000);
  }
  Serial.println("[OK] OLED initialized");

  // Splash
  oledClear();
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(12, 6);  display.print("AI SMART GLASSES");
  display.setCursor(22, 20); display.print("Stage 2: Live AI");
  display.drawLine(10, 32, 118, 32, SSD1306_WHITE);
  display.setCursor(8, 38);  display.print("Camera + Flask + OLED");
  display.setCursor(18, 52); display.print("Initializing...");
  display.display();
  delay(2000);

  // Camera
  oledStatus("Init camera...");
  if (!initCamera()) {
    oledError("Camera FAILED!\n0x20002 = ribbon\ncable loose");
    Serial.println("[ERROR] Camera init failed");
    while (true) delay(1000);
  }

  // WiFi
  if (!connectWiFi()) {
    Serial.println("[ERROR] WiFi failed — halting");
    while (true) delay(1000);
  }

  // Server check (retry until online)
  while (!checkServer()) {
    Serial.println("[WAIT] Retrying server in 5s...");
    delay(5000);
  }

  // Ready!
  Serial.println("\n[READY] All systems GO!");
  Serial.println("  First capture in 3 seconds...");
  Serial.println("  Commands: d=Describe  r=OCR  m=Mode  s=Status\n");

  oledShowMode();
  delay(3000);

  // First auto capture on boot
  actionPending = true;
}

// ─────────────────────────────────────────────
// LOOP
// ─────────────────────────────────────────────

void loop() {
  // Handle serial input
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd != '\n' && cmd != '\r') {
      handleSerialCommand(cmd);
    }
  }

  // Auto-capture timer
  if (!actionPending && (millis() - lastCapture >= AUTO_CAPTURE_MS)) {
    Serial.println("[AUTO] Auto-capture triggered");
    actionPending = true;
  }

  // Process pending action
  if (actionPending) {
    actionPending = false;
    captureAndProcess();
  }

  delay(50);
}
