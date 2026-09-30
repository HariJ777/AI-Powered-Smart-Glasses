/*
 * ============================================================
 *  AI Smart Glasses — Stage 1 Test Firmware
 *  OLED Display + Wi-Fi + Flask AI Backend
 * ============================================================
 *  Tests:
 *    ✅ OLED display (SSD1306 via I2C on GPIO 13/14)
 *    ✅ Wi-Fi connection
 *    ✅ Camera capture (OV2640)
 *    ✅ Flask server /health ping
 *    ✅ Flask server /analyze-image (scene description)
 *
 *  Hardware needed for this test:
 *    - ESP32-CAM (AI-Thinker)
 *    - SSD1306 OLED (SDA=GPIO13, SCL=GPIO14)
 *    - FTDI adapter (for upload + Serial Monitor)
 *
 *  Upload steps:
 *    1. Wire GPIO 0 → GND
 *    2. Upload this sketch (Board: AI Thinker ESP32-CAM)
 *    3. Remove GPIO 0 → GND wire
 *    4. Press RESET button
 *    5. Watch Serial Monitor (115200 baud)
 * ============================================================
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_camera.h>
#include "base64.h"

// ─────────────────────────────────────────────
// ⚙️  CONFIGURE THESE BEFORE UPLOADING
// ─────────────────────────────────────────────
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define SERVER_IP       "192.168.1.100"   // Your PC's local IP (run ipconfig)
#define SERVER_PORT     5000
// ─────────────────────────────────────────────

// OLED — I2C pins for ESP32-CAM
#define PIN_SDA   13
#define PIN_SCL   14
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// AI-Thinker ESP32-CAM pin definitions
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

String serverBase = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT);

// ─────────────────────────────────────────────
// OLED Helper Functions
// ─────────────────────────────────────────────

void oledClear() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void oledPrint(String line1, String line2 = "", String line3 = "", String line4 = "") {
  oledClear();
  if (line1 != "") { display.setCursor(0, 0);  display.println(line1); }
  if (line2 != "") { display.setCursor(0, 16); display.println(line2); }
  if (line3 != "") { display.setCursor(0, 32); display.println(line3); }
  if (line4 != "") { display.setCursor(0, 48); display.println(line4); }
  display.display();
}

void oledResponse(String title, String response) {
  oledClear();
  // Title bar
  display.drawRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setCursor(2, 2);
  display.print(title);
  // Response text (word wrapped)
  display.setCursor(0, 14);
  // Trim to fit ~5 lines of 21 chars
  if (response.length() > 105) response = response.substring(0, 102) + "...";
  display.print(response);
  display.display();
}

void oledStatus(String msg) {
  oledClear();
  display.setTextSize(1);
  display.setCursor(0, 24);
  display.println(msg);
  display.display();
  Serial.println("[STATUS] " + msg);
}

// ─────────────────────────────────────────────
// Camera Init
// ─────────────────────────────────────────────

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
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
  config.frame_size   = FRAMESIZE_QVGA;  // 320x240 — small for fast upload
  config.jpeg_quality = 15;
  config.fb_count     = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Camera init failed: 0x%x\n", err);
    return false;
  }
  Serial.println("[OK] Camera initialized");
  return true;
}

// ─────────────────────────────────────────────
// Wi-Fi Connect
// ─────────────────────────────────────────────

bool connectWiFi() {
  oledStatus("Connecting WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    String ip = WiFi.localIP().toString();
    Serial.println("\n[OK] WiFi connected: " + ip);
    oledPrint("WiFi Connected!", ip, "");
    delay(2000);
    return true;
  } else {
    Serial.println("\n[ERROR] WiFi FAILED");
    oledPrint("WiFi FAILED!", "Check SSID/Pass", "in config");
    return false;
  }
}

// ─────────────────────────────────────────────
// Check Flask Server
// ─────────────────────────────────────────────

bool checkServer() {
  oledStatus("Pinging server...");
  HTTPClient http;
  http.begin(serverBase + "/health");
  http.setTimeout(5000);
  int code = http.GET();

  if (code == 200) {
    String body = http.getString();
    Serial.println("[OK] Server alive: " + body);
    oledPrint("Server: ONLINE", SERVER_IP, "Port: " + String(SERVER_PORT));
    delay(2000);
    http.end();
    return true;
  } else {
    Serial.printf("[ERROR] Server response: %d\n", code);
    oledPrint("Server: OFFLINE", "Start Flask:", "python main.py");
    http.end();
    return false;
  }
}

// ─────────────────────────────────────────────
// Capture + Send to AI
// ─────────────────────────────────────────────

void captureAndDescribe() {
  oledStatus("Capturing image...");
  Serial.println("[CAM] Capturing frame...");

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[ERROR] Camera capture failed");
    oledPrint("Camera Error!", "Capture failed");
    return;
  }

  Serial.printf("[CAM] Captured: %d bytes\n", fb->len);
  oledStatus("Encoding image...");

  // Base64 encode
  String b64 = base64::encode(fb->buf, fb->len);
  esp_camera_fb_return(fb);

  Serial.println("[NET] Sending to Flask /analyze-image...");
  oledStatus("Sending to AI...");

  // Build JSON
  String payload = "{\"image\":\"" + b64 + "\",\"mode\":\"describe\"}";

  HTTPClient http;
  http.begin(serverBase + "/analyze-image");
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(30000);  // 30s timeout for AI processing

  int code = http.POST(payload);

  if (code == 200) {
    String response = http.getString();
    Serial.println("[AI] Response: " + response);

    // Parse the result field from JSON (simple extraction)
    int idx = response.indexOf("\"result\":");
    if (idx != -1) {
      String result = response.substring(idx + 10);
      result.replace("\"}", "");
      result.replace("\"", "");
      result.trim();
      Serial.println("[AI] Scene: " + result);
      oledResponse("AI: DESCRIBE", result);
    } else {
      oledResponse("AI Response:", response.substring(0, 100));
    }
  } else {
    Serial.printf("[ERROR] HTTP %d\n", code);
    oledPrint("AI Error!", "HTTP: " + String(code), "Check server log");
  }

  http.end();
}

// ─────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("============================================");
  Serial.println("  AI Smart Glasses — Stage 1 Test Firmware");
  Serial.println("============================================");

  // Init OLED
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[ERROR] OLED not found! Check SDA=13 SCL=14 and 4.7kΩ pull-ups");
    while (true) delay(1000);
  }
  Serial.println("[OK] OLED initialized");

  // Splash screen
  oledClear();
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(12, 10);
  display.println("AI SMART GLASSES");
  display.setCursor(22, 24);
  display.println("Stage 1 Test");
  display.setCursor(10, 40);
  display.println("OLED + WiFi + AI");
  display.display();
  delay(2000);

  // Init Camera
  oledStatus("Init camera...");
  if (!initCamera()) {
    oledPrint("Camera Error!", "Check ribbon", "cable connection");
    Serial.println("[ERROR] Camera failed — continuing without it");
  }

  // Connect WiFi
  if (!connectWiFi()) {
    Serial.println("[ERROR] WiFi failed — halting");
    while (true) delay(1000);
  }

  // Check Flask server
  if (!checkServer()) {
    Serial.println("[WARN] Server offline — retrying every 5s");
    oledPrint("Waiting for", "Flask server...", "python main.py");
    while (!checkServer()) delay(5000);
  }

  // All good!
  Serial.println();
  Serial.println("[READY] Everything working! Auto-capturing in 3s...");
  oledPrint("All Systems OK!", "Auto-capture", "in 3 seconds...");
  delay(3000);

  // First capture
  captureAndDescribe();
}

// ─────────────────────────────────────────────
// LOOP — captures every 10 seconds
// ─────────────────────────────────────────────

void loop() {
  Serial.println("\n[LOOP] Waiting 10 seconds before next capture...");
  oledPrint("AI Response shown", "Next capture in", "10 seconds...");
  delay(10000);

  captureAndDescribe();
}
