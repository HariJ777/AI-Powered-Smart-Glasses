/*
 * ============================================================
 *  ESP32-CAM Camera Test Script
 *  AI Smart Glasses — Hardware Verification
 * ============================================================
 *  Tests:
 *    ✅ Camera initialization (OV2640)
 *    ✅ Frame capture (JPEG)
 *    ✅ Live stream via browser over Wi-Fi
 *    ✅ Reports frame size + timing on Serial Monitor
 *
 *  HOW TO USE:
 *    1. Set your Wi-Fi credentials below
 *    2. Wire GPIO 0 → GND (flash mode)
 *    3. Upload to ESP32-CAM (Board: AI Thinker ESP32-CAM)
 *    4. Remove GPIO 0 → GND wire, press RESET
 *    5. Open Serial Monitor at 115200 baud
 *    6. Copy the IP address shown and open in your browser
 *    7. You should see a live camera stream!
 *
 *  No libraries needed beyond ESP32 board package.
 * ============================================================
 */

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// ─────────────────────────────────────────────
// ⚙️  SET YOUR WI-FI HERE
// ─────────────────────────────────────────────
#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"
// ─────────────────────────────────────────────

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

// Flash LED
#define LED_GPIO_NUM       4

httpd_handle_t stream_httpd = NULL;

// ─────────────────────────────────────────────
// MJPEG Stream Handler
// ─────────────────────────────────────────────
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* STREAM_CONTENT_TYPE = 
  "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART = 
  "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  char part_buf[64];
  int frame_count = 0;
  unsigned long start_time = millis();

  res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  Serial.println("[STREAM] Client connected — streaming started");

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("[ERROR] Camera capture failed");
      res = ESP_FAIL;
      break;
    }

    // Print frame info every 30 frames
    frame_count++;
    if (frame_count % 30 == 0) {
      float fps = frame_count / ((millis() - start_time) / 1000.0);
      Serial.printf("[STREAM] Frame %d | Size: %d bytes | ~%.1f FPS\n",
                    frame_count, fb->len, fps);
    }

    res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
    if (res == ESP_OK) {
      size_t hlen = snprintf(part_buf, sizeof(part_buf), STREAM_PART, fb->len);
      res = httpd_resp_send_chunk(req, part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char*)fb->buf, fb->len);
    }

    esp_camera_fb_return(fb);

    if (res != ESP_OK) {
      Serial.println("[STREAM] Client disconnected");
      break;
    }
  }
  return res;
}

// ─────────────────────────────────────────────
// Snapshot Handler — single JPEG at /snapshot
// ─────────────────────────────────────────────
esp_err_t snapshot_handler(httpd_req_t *req) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
  httpd_resp_send(req, (const char*)fb->buf, fb->len);
  Serial.printf("[SNAP] Snapshot served: %d bytes\n", fb->len);
  esp_camera_fb_return(fb);
  return ESP_OK;
}

// ─────────────────────────────────────────────
// Info page at /
// ─────────────────────────────────────────────
esp_err_t index_handler(httpd_req_t *req) {
  String html = R"rawliteral(
<!DOCTYPE html><html>
<head>
  <title>ESP32-CAM Test</title>
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <style>
    body { background:#111; color:#eee; font-family:Arial; text-align:center; padding:20px; }
    h1 { color:#4CAF50; }
    img { max-width:100%; border:3px solid #4CAF50; border-radius:8px; margin:10px; }
    .btn { display:inline-block; padding:12px 24px; margin:8px; border-radius:6px;
           background:#4CAF50; color:white; text-decoration:none; font-size:16px; }
    .btn:hover { background:#45a049; }
    .info { background:#222; padding:12px; border-radius:8px; margin:10px; font-size:14px; }
  </style>
</head>
<body>
  <h1>🎥 ESP32-CAM Test</h1>
  <p class="info">AI Smart Glasses — Camera Hardware Verification</p>
  <br>
  <img src="/stream" id="stream" alt="Camera Stream"><br>
  <a class="btn" href="/snapshot" target="_blank">📸 Save Snapshot</a>
  <a class="btn" href="/stream" target="_blank">🔴 Full Stream</a>
  <br><br>
  <div class="info">
    ✅ If you can see the video above, your camera is working correctly!<br>
    📸 Click "Save Snapshot" to download a single JPEG frame.
  </div>
</body>
</html>
)rawliteral";
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, html.c_str(), html.length());
  return ESP_OK;
}

// ─────────────────────────────────────────────
// Start Web Server
// ─────────────────────────────────────────────
void startServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t snapshot_uri = {
    .uri       = "/snapshot",
    .method    = HTTP_GET,
    .handler   = snapshot_handler,
    .user_ctx  = NULL
  };

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &index_uri);
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    httpd_register_uri_handler(stream_httpd, &snapshot_uri);
    Serial.println("[OK] Web server started");
  }
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

  // Use UXGA if PSRAM available, else SVGA
  if (psramFound()) {
    config.frame_size   = FRAMESIZE_UXGA;  // 1600x1200
    config.jpeg_quality = 10;
    config.fb_count     = 2;
    Serial.println("[CAM] PSRAM found — using UXGA (1600x1200)");
  } else {
    config.frame_size   = FRAMESIZE_SVGA;  // 800x600
    config.jpeg_quality = 12;
    config.fb_count     = 1;
    Serial.println("[CAM] No PSRAM — using SVGA (800x600)");
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Camera init FAILED: 0x%x\n", err);
    Serial.println("        → Check ribbon cable is firmly seated");
    Serial.println("        → Try pressing on camera module gently");
    return false;
  }

  // Test: take one frame and report
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[ERROR] Test capture failed — camera may be faulty");
    return false;
  }
  Serial.printf("[OK] Camera initialized!\n");
  Serial.printf("     Test frame: %dx%d | %d bytes | format: JPEG\n",
                fb->width, fb->height, fb->len);
  esp_camera_fb_return(fb);
  return true;
}

// ─────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("============================================");
  Serial.println("  ESP32-CAM Camera Test");
  Serial.println("  AI Smart Glasses — Hardware Verification");
  Serial.println("============================================");

  // Turn off flash LED
  pinMode(LED_GPIO_NUM, OUTPUT);
  digitalWrite(LED_GPIO_NUM, LOW);

  // Init camera
  Serial.println("\n[1/3] Initializing camera...");
  if (!initCamera()) {
    Serial.println("\n❌ CAMERA TEST FAILED");
    Serial.println("   Possible causes:");
    Serial.println("   • Ribbon cable not seated properly");
    Serial.println("   • Wrong board selected (use AI Thinker ESP32-CAM)");
    Serial.println("   • Defective camera module");
    // Blink LED fast to indicate error
    while (true) {
      digitalWrite(LED_GPIO_NUM, HIGH); delay(100);
      digitalWrite(LED_GPIO_NUM, LOW);  delay(100);
    }
  }

  // Connect Wi-Fi
  Serial.printf("\n[2/3] Connecting to Wi-Fi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WARN] Wi-Fi failed — running offline mode");
    Serial.println("       Camera is still initialized, but no stream available");
    // Still show camera captures on serial
    while (true) {
      camera_fb_t *fb = esp_camera_fb_get();
      if (fb) {
        Serial.printf("[CAM] Frame captured: %dx%d | %d bytes ✅\n",
                      fb->width, fb->height, fb->len);
        esp_camera_fb_return(fb);
      } else {
        Serial.println("[ERROR] Frame capture failed ❌");
      }
      delay(2000);
    }
  }

  Serial.printf("\n[OK] WiFi connected!\n");
  Serial.printf("     IP Address: %s\n", WiFi.localIP().toString().c_str());

  // Start web server
  Serial.println("\n[3/3] Starting web server...");
  startServer();

  // Done!
  Serial.println("\n============================================");
  Serial.println("  ✅ CAMERA TEST READY!");
  Serial.println("============================================");
  Serial.printf("  📺 Live Stream:  http://%s/\n",       WiFi.localIP().toString().c_str());
  Serial.printf("  📸 Snapshot:     http://%s/snapshot\n", WiFi.localIP().toString().c_str());
  Serial.println("============================================");
  Serial.println("  Open the IP address above in your browser");
  Serial.println("  on the SAME Wi-Fi network as the ESP32");
  Serial.println("============================================\n");
}

// ─────────────────────────────────────────────
// LOOP — just keep alive + periodic status
// ─────────────────────────────────────────────
void loop() {
  delay(10000);
  Serial.printf("[STATUS] Uptime: %lus | IP: %s | Heap: %d bytes free\n",
                millis() / 1000,
                WiFi.localIP().toString().c_str(),
                ESP.getFreeHeap());
}
