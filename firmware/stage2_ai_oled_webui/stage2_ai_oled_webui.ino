/*
 * ============================================================
 *  AI Smart Glasses — Stage 2 Firmware (Web Control Pro)
 *  Fully asynchronous: Shows image instantly while processing!
 * ============================================================
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

// ─────────────────────────────────────────────
// ⚙️  EDIT THESE 2 LINES ONLY
// ─────────────────────────────────────────────
#define WIFI_SSID      "YOUR_WIFI_NAME"
#define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"
// ─────────────────────────────────────────────
#define SERVER_IP      "10.252.137.51"
#define SERVER_PORT    5000

// OLED
#define PIN_SDA  13
#define PIN_SCL  14
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

String lastResult    = "No result yet";
String lastMode      = "-";
float  lastRespTime  = 0;
String lastImageBase64 = "";  

String serverBase = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT);
WebServer webServer(80);

// ─────────────────────────────────────────────
// OLED Helpers
// ─────────────────────────────────────────────

void oledClear() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void oledHome() {
  oledClear();
  display.fillRect(0, 0, 128, 13, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 3); display.print("AI SMART GLASSES");
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 24); display.print("Ready for command!");
  display.setCursor(0, 54); display.print("http://");
  display.print(WiFi.localIP().toString());
  display.display();
}

void oledStatus(String msg) {
  oledClear();
  display.setCursor(0, 26);
  display.println(msg);
  display.display();
}

void oledShowResponse(String modeName, String result, float t) {
  oledClear();
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print(modeName + " RESULT");
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 15);
  if (result.length() > 100) result = result.substring(0, 97) + "...";
  display.print(result);
  display.drawLine(0, 55, 128, 55, SSD1306_WHITE);
  display.setCursor(0, 57);
  display.print(String(t, 1) + "s");
  display.display();
}

void oledError(String msg) {
  oledClear();
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2); display.print("ERROR");
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 16);
  display.println(msg);
  display.display();
}

// ─────────────────────────────────────────────
// Camera Init & Base64
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
  config.frame_size   = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count     = 1;
  return esp_camera_init(&config) == ESP_OK;
}

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
// Web Endpoints
// ─────────────────────────────────────────────

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>AI Smart Glasses</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { background: #0a0a0f; color: #eee; font-family: Arial, sans-serif; padding: 16px; }
    h1 { color: #4ade80; text-align: center; margin-bottom: 6px; font-size: 22px; }
    .subtitle { text-align: center; color: #888; font-size: 13px; margin-bottom: 20px; }
    
    .img-container { text-align: center; margin-bottom: 15px; background: #1a1a2e; border-radius: 10px; padding: 10px; }
    #camImage { max-width: 100%; border-radius: 8px; border: 1px solid #333; min-height: 120px; object-fit: contain; display: none; }
    
    .btn { display: block; width: 100%; padding: 16px; margin: 10px 0;
           border: none; border-radius: 12px; font-size: 18px;
           font-weight: bold; cursor: pointer; text-align: center; }
    .btn-describe { background: #4ade80; color: #000; }
    .btn-ocr      { background: #60a5fa; color: #000; }
    .btn:active   { opacity: 0.7; }
    .btn:disabled { opacity: 0.5; cursor: not-allowed; }
    
    .result-box { background: #1a1a2e; border: 1px solid #333; border-radius: 12px;
                  padding: 14px; margin-top: 15px; min-height: 80px; }
    .result-box .r-label { color: #888; font-size: 12px; margin-bottom: 6px; }
    .result-box .r-text  { color: #fff; font-size: 15px; line-height: 1.5; }
    .result-box .r-time  { color: #4ade80; font-size: 12px; margin-top: 8px; }
    #procMsg { color: #f59e0b; text-align: center; font-size: 14px; margin: 10px 0; min-height: 20px; }
  </style>
</head>
<body>
  <h1>🤖 AI Smart Glasses</h1>
  <p class="subtitle">Live View Control Panel</p>

  <div class="img-container">
    <div style="color:#888; font-size:12px; margin-bottom:5px;">CAPTURED IMAGE</div>
    <img id="camImage" src="" alt="Image">
  </div>

  <div id="procMsg">Ready</div>

  <button class="btn btn-describe" id="btnDesc" onclick="triggerFlow('description')">📷 DESCRIBE SCENE</button>
  <button class="btn btn-ocr" id="btnOcr" onclick="triggerFlow('ocr')">🔤 READ TEXT (OCR)</button>

  <div class="result-box">
    <div class="r-label">LAST AI RESULT</div>
    <div class="r-text" id="resultText">)rawliteral" + lastResult + R"rawliteral(</div>
    <div class="r-time" id="resultTime">)rawliteral" +
      (lastMode != "-" ? ("Mode: " + lastMode + " | " + String(lastRespTime, 1) + "s") : "") +
    R"rawliteral(</div>
  </div>

  <script>
    async function triggerFlow(type) {
      // 1. Disable buttons & show status
      document.getElementById('btnDesc').disabled = true;
      document.getElementById('btnOcr').disabled = true;
      let msg = document.getElementById('procMsg');
      msg.innerText = "📸 Capturing image...";
      
      try {
        // 2. Tell ESP32 to capture image
        let capRes = await fetch('/capture');
        if (!capRes.ok) throw new Error("Capture failed");
        
        // 3. Immediately download and show the image!
        msg.innerText = "🖼️ Loading image to screen...";
        let imgRes = await fetch('/image');
        let b64 = await imgRes.text();
        let imgTag = document.getElementById('camImage');
        imgTag.src = 'data:image/jpeg;base64,' + b64;
        imgTag.style.display = 'inline-block';
        
        // 4. Tell ESP32 to send it to the PC server (Ollama/OCR)
        msg.innerText = "🧠 Analyzing with AI on PC... (this may take a minute)";
        let ansRes = await fetch('/analyze?type=' + type);
        let ansData = await ansRes.json();
        
        // 5. Show results!
        document.getElementById('resultText').innerText = ansData.result;
        document.getElementById('resultTime').innerText = 'Mode: ' + ansData.mode + ' | ' + ansData.time + 's';
        msg.innerText = "✅ Done!";
        
      } catch (e) {
        msg.innerText = "❌ Error: " + e.message;
      }
      
      // Re-enable buttons
      document.getElementById('btnDesc').disabled = false;
      document.getElementById('btnOcr').disabled = false;
    }
    
    // Show last image on load if exists
    window.onload = async () => {
       let r = await fetch('/image');
       if(r.ok) {
           let b64 = await r.text();
           if(b64.length > 10) {
               let img = document.getElementById('camImage');
               img.src = 'data:image/jpeg;base64,' + b64;
               img.style.display = 'inline-block';
           }
       }
    };
  </script>
</body>
</html>
)rawliteral";

  webServer.send(200, "text/html", html);
}

void handleCapture() {
  oledStatus("Capturing...");
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) { webServer.send(500, "text/plain", "Camera error"); return; }
  
  String b64 = base64Encode(fb->buf, fb->len);
  esp_camera_fb_return(fb);
  
  if (b64.isEmpty()) { webServer.send(500, "text/plain", "Memory error"); return; }
  
  lastImageBase64 = b64;
  oledStatus("Captured! Sending to browser...");
  webServer.send(200, "text/plain", "OK");
}

void handleImage() {
  if (lastImageBase64.length() > 0) {
    webServer.send(200, "text/plain", lastImageBase64);
  } else {
    webServer.send(404, "text/plain", "");
  }
}

String escapeJson(String s) {
  s.replace("\\", "\\\\");
  s.replace("\"", "\\\"");
  s.replace("\n", " ");
  s.replace("\r", "");
  return s;
}

void handleAnalyze() {
  String type = webServer.arg("type");
  String modeName = (type == "description") ? "DESCRIBE" : "OCR";
  
  oledClear();
  display.setCursor(0, 0); display.println("[" + modeName + "]");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  display.setCursor(0, 20); display.println("Processing on PC...");
  display.display();

  if (lastImageBase64.isEmpty()) { 
    webServer.send(400, "application/json", "{\"result\":\"No image captured\"}"); 
    return; 
  }

  DynamicJsonDocument doc(lastImageBase64.length() + 200);
  doc["image_base64"]  = lastImageBase64;
  doc["analysis_type"] = type;
  String payload;
  serializeJson(doc, payload);

  HTTPClient http;
  http.begin(serverBase + "/analyze-image");
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(180000); // 3 minutes for local Ollama

  unsigned long t0 = millis();
  int httpCode = http.POST(payload);
  lastRespTime = (millis() - t0) / 1000.0;
  lastMode = modeName;

  if (httpCode == 200) {
    String body = http.getString();
    DynamicJsonDocument resp(4096);
    if (!deserializeJson(resp, body)) {
      if      (resp.containsKey("result"))  lastResult = resp["result"].as<String>();
      else if (resp.containsKey("text"))    lastResult = resp["text"].as<String>();
      else if (resp.containsKey("answer"))  lastResult = resp["answer"].as<String>();
      else                                  lastResult = body.substring(0, 100);
    } else { lastResult = body.substring(0, 100); }
    lastResult.trim();
    oledShowResponse(modeName, lastResult, lastRespTime);
  } else {
    lastResult = "HTTP error: " + String(httpCode);
    if (httpCode == -1) lastResult = "Timeout or Server Offline";
    oledError(lastResult);
  }
  http.end();

  String respJson = "{\"result\":\"" + escapeJson(lastResult) + "\",\"mode\":\"" + modeName + "\",\"time\":\"" + String(lastRespTime, 1) + "\"}";
  webServer.send(200, "application/json", respJson);
}

// ─────────────────────────────────────────────
// WiFi
// ─────────────────────────────────────────────

bool connectWiFi() {
  oledStatus("Connecting WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) { delay(500); }
  if (WiFi.status() == WL_CONNECTED) {
    oledHome();
    delay(3000);
    return true;
  }
  oledError("WiFi FAILED!");
  return false;
}

// ─────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────

void setup() {
  Serial.begin(115200);

  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true) delay(1000);
  }

  oledClear();
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  display.setCursor(12, 8);  display.print("AI SMART GLASSES");
  display.setCursor(22, 22); display.print("Stage 2 + WebUI");
  display.setCursor(18, 52); display.print("Initializing...");
  display.display();
  delay(2000);

  if (!initCamera()) { oledError("Camera FAILED!"); while (true) delay(1000); }
  if (!connectWiFi()) while (true) delay(1000);

  webServer.on("/",        handleRoot);
  webServer.on("/capture", handleCapture);
  webServer.on("/image",   handleImage);
  webServer.on("/analyze", handleAnalyze);
  webServer.begin();

  oledHome();
}

void loop() {
  webServer.handleClient();
  delay(10);
}
