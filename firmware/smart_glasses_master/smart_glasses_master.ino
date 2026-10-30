/*
 * =========================================================================
 *  AI SMART GLASSES — MASTER FIRMWARE (Assistive Vision Edition)
 * =========================================================================
 *  Architectural Model: Companion Device Architecture
 *    - Optical Sensing & HUD: Onboard ESP32-CAM + SSD1306 OLED
 *    - Speech Synthesis (TTS): User's Wireless Bluetooth Earbuds via Web Companion
 *
 *  Hardware Connections:
 *    - OLED:          SDA -> GPIO 13 (Pin 4) | SCL -> GPIO 14 (Pin 6) [3.3V power]
 *    - Smart Button:  GPIO 15 (Pin 5) <-> GND (Internal pull-up enabled)
 *                     - Short Press (< 1.2 sec) = Take Photo & Ask AI
 *                     - Long Press  (>= 1.2 sec) = Switch Mode (VISION -> OCR -> TRANSLATE)
 *    - Camera:        AI-Thinker OV2640 (Built-in)
 *    - Microphone:    REMOVED (Hands-free button control used instead)
 *    - Speaker:       REMOVED from frame (Audio routed directly to Earbuds)
 * =========================================================================
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "mbedtls/base64.h"
#include "soc/soc.h"             // Brownout protection control
#include "soc/rtc_cntl_reg.h"    // Brownout register

// ─────────────────────────────────────────────────────────
// ⚙️ EDIT YOUR WIFI DETAILS HERE
// ─────────────────────────────────────────────────────────
#define WIFI_SSID       "Niggaa"        // <-- Replace with your WiFi SSID
#define WIFI_PASSWORD   "012345678"    // <-- Replace with your WiFi Password
#define SERVER_IP       "10.111.103.51" // Your PC's active IP address
#define SERVER_PORT     5000

// ─────────────────────────────────────────────────────────
// Pin Definitions
// ─────────────────────────────────────────────────────────
// OLED Display (I2C)
#define PIN_SDA         13
#define PIN_SCL         14
#define OLED_ADDR       0x3C
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Single Multifunction Button
#define PIN_BUTTON      15    // GPIO 15 to one leg, GND to the other!

// Camera pins (AI-Thinker OV2640)
#define PWDN_GPIO_NUM   32
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM   26
#define SIOC_GPIO_NUM   27
#define Y9_GPIO_NUM     35
#define Y8_GPIO_NUM     34
#define Y7_GPIO_NUM     39
#define Y6_GPIO_NUM     36
#define Y5_GPIO_NUM     21
#define Y4_GPIO_NUM     19
#define Y3_GPIO_NUM     18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM  25
#define HREF_GPIO_NUM   23
#define PCLK_GPIO_NUM   22

// ─────────────────────────────────────────────────────────
// Global State
// ─────────────────────────────────────────────────────────
enum GlassesMode {
  MODE_DESCRIBE  = 0,
  MODE_OCR       = 1,
  MODE_TRANSLATE = 2
};

GlassesMode currentMode = MODE_DESCRIBE;
String lastResult       = "Ready for command!";
String lastModeName     = "DESCRIBE";
float  lastRespTime     = 0.0;
String lastImageBase64  = "";

WebServer webServer(80);
String serverBase = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT);

// Button Timing Tracking
unsigned long btnPressStart   = 0;
bool isButtonPressed          = false;
bool longPressTriggered       = false;
const unsigned long LONG_PRESS_MS = 1200;

// ─────────────────────────────────────────────────────────
// Audio Feedback (Disabled to guarantee 100% board stability)
// ─────────────────────────────────────────────────────────
inline void soundStartup()    {}
inline void soundCapture()    {}
inline void soundModeChange() {}
inline void soundSuccess()    {}
inline void soundError()      {}

// ─────────────────────────────────────────────────────────
// OLED Display Helpers
// ─────────────────────────────────────────────────────────
void oledClear() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void oledHome() {
  oledClear();
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(6, 2);
  display.print("CORTEX HUD v2.5");
  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 18);
  display.print("STATUS: ");
  display.print("ONLINE [READY]");

  display.setCursor(0, 30);
  display.print("MODE:   ");
  if (currentMode == MODE_DESCRIBE)      display.print("NEURAL VISION");
  else if (currentMode == MODE_OCR)     display.print("TEXT OCR");
  else                                  display.print("TRANSLATE");

  display.setCursor(0, 42);
  display.print("INPUT:  [TAP BUTTON]");

  display.drawLine(0, 53, 128, 53, SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print("WiFi: ");
  display.print(WiFi.localIP().toString());
  display.display();
}

void oledStatus(String title, String msg) {
  oledClear();
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(4, 2);
  display.print(title);
  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(msg);
  
  display.drawRect(0, 53, 128, 8, SSD1306_WHITE);
  display.fillRect(2, 55, 80, 4, SSD1306_WHITE);
  display.display();
}

void oledShowResult(String modeName, String result, float t) {
  oledClear();
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(4, 2);
  display.print("NEURAL HUD | ");
  display.print(String(t, 1));
  display.print("s");
  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 16);
  if (result.length() > 95) result = result.substring(0, 92) + "...";
  display.print(result);

  display.drawLine(0, 54, 128, 54, SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print("CORTEX VISION AI");
  display.display();
}

// ─────────────────────────────────────────────────────────
// Camera & Capture
// ─────────────────────────────────────────────────────────
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
  config.frame_size   = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count     = 1;
#if defined(CAMERA_GRAB_LATEST)
  config.grab_mode    = CAMERA_GRAB_LATEST;
#endif

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) return false;

  sensor_t* s = esp_camera_sensor_get();
  if (s) {
    s->set_whitebal(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_hmirror(s, 1); // Fix hardware horizontal mirroring so text is readable
    s->set_vflip(s, 0);
  }
  return true;
}

String captureBase64() {
  // Flush stale DMA buffer frames so we capture the true live scene NOW
  for (int i = 0; i < 2; i++) {
    camera_fb_t* oldFb = esp_camera_fb_get();
    if (oldFb) {
      esp_camera_fb_return(oldFb);
      delay(25);
    }
  }

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) return "";
  
  size_t encodedLen = ((fb->len + 2) / 3) * 4 + 1;
  uint8_t* encoded = (uint8_t*)malloc(encodedLen);
  if (!encoded) { esp_camera_fb_return(fb); return ""; }
  
  size_t outLen = 0;
  mbedtls_base64_encode(encoded, encodedLen, &outLen, fb->buf, fb->len);
  String result = String((char*)encoded).substring(0, outLen);
  free(encoded);
  esp_camera_fb_return(fb);
  return result;
}

// ─────────────────────────────────────────────────────────
// AI Processing Pipeline
// ─────────────────────────────────────────────────────────
void runAIPipeline(String type) {
  oledStatus("OPTICAL SCAN", "Capturing frame...");
  soundCapture();  // Click sound when photo is taken
  
  String b64 = captureBase64();
  if (b64.isEmpty()) {
    oledStatus("SYSTEM ERROR", "Camera frame error!");
    soundError();
    delay(1500);
    oledHome();
    return;
  }
  lastImageBase64 = b64;

  String modeName = "NEURAL VISION";
  if (type == "ocr") modeName = "TEXT OCR";
  else if (type == "translate") modeName = "TRANSLATE";

  oledStatus("NEURAL CORE", (type == "translate") ? "Translating..." : "Analyzing scene...");

  DynamicJsonDocument doc(b64.length() + 200);
  doc["image_base64"]  = b64;
  doc["analysis_type"] = type;
  String payload;
  serializeJson(doc, payload);

  HTTPClient http;
  http.begin(serverBase + "/analyze-image");
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(25000); // 25s timeout for AI cloud inference

  unsigned long t0 = millis();
  int httpCode = http.POST(payload);
  lastRespTime = (millis() - t0) / 1000.0;
  lastModeName = modeName;

  if (httpCode == 200) {
    String body = http.getString();
    DynamicJsonDocument resp(4096);
    if (!deserializeJson(resp, body)) {
      if (resp.containsKey("oled_text"))   lastResult = resp["oled_text"].as<String>();
      else if (resp.containsKey("result")) lastResult = resp["result"].as<String>();
      else if (resp.containsKey("text"))   lastResult = resp["text"].as<String>();
      else                                 lastResult = body.substring(0, 95);
    } else {
      lastResult = body.substring(0, 95);
    }
    lastResult.trim();
    oledShowResult(modeName, lastResult, lastRespTime);
    soundSuccess();  // Rising beep = result arrived!
  } else {
    lastResult = "HTTP Error: " + String(httpCode);
    oledStatus("SERVER ERROR", lastResult);
    soundError();    // Descending beep = something went wrong
  }
  http.end();
}

bool pendingWebTrigger = false;
String pendingWebType   = "";

String escapeJsonString(String input) {
  input.replace("\\", "\\\\");
  input.replace("\"", "\\\"");
  input.replace("\n", " ");
  input.replace("\r", " ");
  return input;
}

// ─────────────────────────────────────────────────────────
// Web Endpoints (Allows phone/browser control too!)
// ─────────────────────────────────────────────────────────
void handleRoot() {
  String safeResult = escapeJsonString(lastResult);
  String html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>AI Glasses Pro</title><style>
body{background:#0d1117;color:#c9d1d9;font-family:sans-serif;padding:16px;text-align:center}
h1{color:#58a6ff;font-size:20px}
.btn{display:block;width:100%;padding:14px;margin:8px 0;border:none;border-radius:10px;font-size:16px;font-weight:bold;cursor:pointer}
.btn-desc{background:#238636;color:#fff}.btn-ocr{background:#1f6feb;color:#fff}.btn-trans{background:#8957e5;color:#fff}
.btn-hud{background:linear-gradient(135deg,#00f2fe,#4facfe);color:#040711;margin-top:14px}
.box{background:#161b22;border:1px solid #30363d;border-radius:10px;padding:12px;margin-top:12px;text-align:left}
</style></head><body>
<h1>👓 AI Smart Glasses</h1>
<button class="btn btn-desc" onclick="fetch('/action?type=description')">📷 SCENE VISION</button>
<button class="btn btn-ocr" onclick="fetch('/action?type=ocr')">🔤 READ TEXT (OCR)</button>
<button class="btn btn-trans" onclick="fetch('/action?type=translate')">🌐 TRANSLATE TEXT</button>
<a href=")rawliteral" + serverBase + R"rawliteral(" target="_blank" style="text-decoration:none;">
  <button class="btn btn-hud">⚡ OPEN FULL HUD</button>
</a>
<div class="box"><b>Latest Result:</b><p id="r">)rawliteral" + safeResult + R"rawliteral(</p></div>
<script>
let polling = false;
setInterval(async()=>{
  if (polling) return;
  polling = true;
  try {
    let res = await fetch('/status');
    let d = await res.json();
    if (d && d.result) document.getElementById('r').innerText = d.result;
  } catch(e) {}
  polling = false;
}, 3000);
</script>
</body></html>)rawliteral";
  webServer.send(200, "text/html", html);
}

void handleWebAction() {
  String type = webServer.arg("type");
  if (type.isEmpty()) {
    if (currentMode == MODE_DESCRIBE) type = "description";
    else if (currentMode == MODE_OCR) type = "ocr";
    else type = "translate";
  }
  pendingWebType = type;
  pendingWebTrigger = true;
  webServer.send(200, "text/plain", "OK");
}

void handleStatus() {
  String json = "{\"result\":\"" + escapeJsonString(lastResult) + "\",\"mode\":\"" + lastModeName + "\"}";
  webServer.send(200, "application/json", json);
}

// ─────────────────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────────────────
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector to prevent voltage drop resets
  Serial.begin(115200);
  delay(200);
  Serial.println("\n\n========================================");
  Serial.println(">>> AI SMART GLASSES BOOT SEQUENCE <<<");
  Serial.println("========================================");

  // Smart Button on GPIO 15 (Internal pull-up enabled!)
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // OLED Init (GPIO 13 SDA, GPIO 14 SCL)
  pinMode(PIN_SDA, INPUT_PULLUP);
  pinMode(PIN_SCL, INPUT_PULLUP);
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000); // 100kHz standard speed

  // Automatic I2C Scanner & Address Auto-Detection (0x3C or 0x3D)
  uint8_t detectedAddr = 0;
  Serial.println("\n[I2C] Scanning I2C bus on GPIO 13 (SDA) and GPIO 14 (SCL)...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("[I2C] -> Device found at 0x%02X\n", addr);
      if (addr == 0x3C || addr == 0x3D) {
        detectedAddr = addr;
      }
    }
  }

  if (detectedAddr == 0) {
    Serial.println("[I2C] Warning: No OLED responded. Falling back to default 0x3C.");
    detectedAddr = OLED_ADDR;
  }

  if (display.begin(SSD1306_SWITCHCAPVCC, detectedAddr)) {
    Serial.printf("[OLED] Display initialized successfully at 0x%02X!\n", detectedAddr);
    oledClear();
    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
    display.setCursor(14, 14); display.print("CORTEX VISION");
    display.setCursor(20, 30); display.print("OS Booting...");
    display.display();
  } else {
    Serial.println("[OLED] Failed to initialize display.");
  }

  // Camera Init (OV2640 built-in)
  if (!initCamera()) {
    oledStatus("ERROR", "Camera Init FAILED!");
    soundError();
    while (true) delay(1000);
  }

  // WiFi Connection
  oledStatus("WIFI", "Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_15dBm); // Reduce Wi-Fi current spikes on FTDI/USB power
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    webServer.on("/", handleRoot);
    webServer.on("/action", handleWebAction);
    webServer.on("/status", handleStatus);
    webServer.begin();
    oledHome();
    soundStartup();  // 3-note ascending chime = all systems ready!
  } else {
    oledStatus("WIFI ERROR", "Could not connect!");
    soundError();
  }
}

// ─────────────────────────────────────────────────────────
// MAIN LOOP
// ─────────────────────────────────────────────────────────
void loop() {
  webServer.handleClient();

  // Asynchronous web action trigger (prevents web server freeze)
  if (pendingWebTrigger) {
    pendingWebTrigger = false;
    runAIPipeline(pendingWebType);
  }

  // ── Smart Multifunction Button Handler ──
  // - Short Press (< 1.2 sec)  = Take Photo in Current Mode
  // - Long Press  (>= 1.2 sec) = Cycle Mode (VISION -> OCR -> TRANSLATE)
  if (digitalRead(PIN_BUTTON) == LOW) {
    unsigned long pressStart = millis();
    delay(40); // Clean debounce filter
    
    if (digitalRead(PIN_BUTTON) == LOW) {
      bool modeSwitched = false;

      // Monitor button hold duration
      while (digitalRead(PIN_BUTTON) == LOW) {
        if (!modeSwitched && (millis() - pressStart >= 1200)) {
          // Long press detected! Cycle through all 3 modes
          modeSwitched = true;
          if (currentMode == MODE_DESCRIBE)      currentMode = MODE_OCR;
          else if (currentMode == MODE_OCR)     currentMode = MODE_TRANSLATE;
          else                                  currentMode = MODE_DESCRIBE;
          
          String newMode = (currentMode == MODE_DESCRIBE) ? "NEURAL VISION" :
                           (currentMode == MODE_OCR) ? "TEXT OCR" : "TRANSLATE";
          Serial.println("[BUTTON] Mode switched to: " + newMode);
          oledStatus("MODE CHANGED", "Active: " + newMode);
          soundModeChange();  // Two-tone blip = mode switched!
          delay(1000);
          oledHome();
        }
        delay(20);
      }

      // If released before 1.2 seconds, it was a short press -> Take Photo!
      if (!modeSwitched) {
        Serial.println("[BUTTON] Short tap detected! Snapping photo...");
        String modeStr = "description";
        if (currentMode == MODE_OCR) modeStr = "ocr";
        else if (currentMode == MODE_TRANSLATE) modeStr = "translate";
        runAIPipeline(modeStr);
      }

      delay(300); // 300ms cooldown after button release
    }
  }

  delay(10);
}
