/*
 * ================================================================
 *  AI Smart Glasses — ESP32-CAM Main Firmware
 * ================================================================
 *  
 *  Hardware:
 *    - ESP32-CAM (AI-Thinker) with OV2640 camera
 *    - SSD1306 OLED Display (I2C, 128x64)
 *    - INMP441 I2S Microphone
 *    - MAX98357A I2S Amplifier + Speaker
 *    - 2x Push Buttons (Mode + Action)
 *
 *  Backend:
 *    - Flask server with Smart Router on local PC
 *    - Endpoints: /analyze-image, /ask, /transcribe, /health
 *
 *  Modes:
 *    0 = Describe Scene  (camera → Llama 4 Scout via Groq)
 *    1 = Read Text / OCR  (camera → EasyOCR local)
 *    2 = Speech-to-Text   (mic → Whisper tiny local)
 *    3 = Question & Answer (mic → Llama 3.3 via Groq)
 *    4 = Translation       (mic → Llama 3.3 via Groq)
 *
 *  How to Use:
 *    1. Press BTN1 (GPIO 0) to cycle through modes
 *    2. Press BTN2 (GPIO 16) to capture/record and send
 *    3. Response is shown on the OLED display
 *
 *  Authors: Arya B Shetty, Harinand J, Karthik P, Pavan SN
 *  Guide:   Prof. Jyothibha R Chinchankar
 *  Dept:    CS (IoT Cybersecurity Including Blockchain)
 *           Alva's Institute of Engineering & Technology
 * ================================================================
 */

#include "config.h"

// ─── Core Libraries ─────────────────────────────────────
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "base64.h"

// ─── I2C / OLED Libraries ───────────────────────────────
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ─── I2S Library (Mic + Speaker) ────────────────────────
#include <driver/i2s.h>

// ─────────────────────────────────────────────────────────
//  ESP32-CAM (AI-Thinker) Camera Pin Definitions
//  These are FIXED by the board — do NOT change them.
// ─────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────
//  Global Objects
// ─────────────────────────────────────────────────────────

// OLED Display (I2C on custom pins)
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

// Current operating mode
volatile int currentMode = MODE_DESCRIBE;

// Button state tracking
volatile unsigned long lastBtnModePress   = 0;
volatile unsigned long lastBtnActionPress = 0;
volatile bool actionTriggered = false;

// Status flags
bool wifiConnected   = false;
bool cameraReady     = false;
bool displayReady    = false;
bool serverReachable = false;

// ─────────────────────────────────────────────────────────
//  INTERRUPT: Button Handlers
// ─────────────────────────────────────────────────────────

/**
 * ISR for Button 1 (Mode Select) — GPIO 0
 * Cycles through modes: DESCRIBE → OCR → SPEECH → QA → TRANSLATE → DESCRIBE...
 */
void IRAM_ATTR onBtnModePress() {
  unsigned long now = millis();
  if (now - lastBtnModePress > DEBOUNCE_MS) {
    currentMode = (currentMode + 1) % MODE_COUNT;
    lastBtnModePress = now;
  }
}

/**
 * ISR for Button 2 (Action/Capture) — GPIO 16
 * Sets a flag to trigger capture in the main loop
 */
void IRAM_ATTR onBtnActionPress() {
  unsigned long now = millis();
  if (now - lastBtnActionPress > DEBOUNCE_MS) {
    actionTriggered = true;
    lastBtnActionPress = now;
  }
}

// ─────────────────────────────────────────────────────────
//  OLED Display Functions
// ─────────────────────────────────────────────────────────

/**
 * Initialize the OLED display on custom I2C pins (GPIO 13, 14)
 */
bool initDisplay() {
  // Set custom I2C pins (not default 21/22 which are used by camera)
  Wire.begin(PIN_SDA, PIN_SCL);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[ERROR] OLED display not found at address 0x" + String(OLED_ADDR, HEX));
    return false;
  }
  
  display.clearDisplay();
  display.setTextSize(1);          // 6x8 pixels per character
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("AI Smart Glasses");
  display.println("Initializing...");
  display.display();
  
  Serial.println("[OK] OLED display initialized");
  return true;
}

/**
 * Show the current mode on the OLED display
 */
void showMode() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  // Header bar
  display.println("--- AI GLASSES ---");
  display.println();
  
  // Show current mode with indicator arrow
  display.setTextSize(1);
  for (int i = 0; i < MODE_COUNT; i++) {
    if (i == currentMode) {
      display.print("> ");
      display.println(MODE_NAMES[i]);
    }
  }
  
  display.println();
  display.println("BTN1:Mode BTN2:Go");
  display.display();
}

/**
 * Show a short status message on the OLED
 */
void showStatus(const char* line1, const char* line2 = "", const char* line3 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("--- AI GLASSES ---");
  display.println();
  display.println(line1);
  if (strlen(line2) > 0) display.println(line2);
  if (strlen(line3) > 0) display.println(line3);
  display.display();
}

/**
 * Display the AI response text on the OLED with word wrapping.
 * The SSD1306 (128x64) can show about 21 chars x 8 lines at text size 1.
 */
void showResponse(const String& response) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  // Header
  display.print("[");
  display.print(MODE_NAMES[currentMode]);
  display.println("]");
  display.println("----------------");
  
  // Word-wrap the response text to fit the 21-char wide OLED
  // Available lines after header: ~6 lines (48 pixels)
  int maxChars = 21 * 6;  // ~126 chars fit on screen
  String truncated = response.substring(0, maxChars);
  display.print(truncated);
  
  // Show "..." if text was truncated
  if (response.length() > maxChars) {
    display.println();
    display.print("...(more)");
  }
  
  display.display();
}

/**
 * Show a progress animation while waiting for server response
 */
void showProcessing() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("--- AI GLASSES ---");
  display.println();
  display.print("[");
  display.print(MODE_NAMES[currentMode]);
  display.println("]");
  display.println();
  display.println("  Processing...");
  display.println();
  
  // Simple progress bar
  display.drawRect(14, 48, 100, 10, SSD1306_WHITE);
  for (int i = 0; i < 96; i += 8) {
    display.fillRect(16, 50, i, 6, SSD1306_WHITE);
    display.display();
    delay(30);
  }
}

// ─────────────────────────────────────────────────────────
//  Camera Functions
// ─────────────────────────────────────────────────────────

/**
 * Initialize the OV2640 camera on the ESP32-CAM
 */
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
  
  config.xclk_freq_hz = 20000000;        // 20 MHz clock
  config.pixel_format = PIXFORMAT_JPEG;   // JPEG output
  config.grab_mode    = CAMERA_GRAB_LATEST;
  
  // Use higher quality settings if PSRAM is available
  if (psramFound()) {
    config.frame_size   = CAMERA_RESOLUTION;  // From config.h (default: VGA 640x480)
    config.jpeg_quality = CAMERA_QUALITY;     // From config.h (default: 12)
    config.fb_count     = 2;                  // Double buffer for smooth capture
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    Serial.println("[INFO] PSRAM found — using high quality settings");
  } else {
    config.frame_size   = FRAMESIZE_QVGA;    // Fallback: 320x240
    config.jpeg_quality = 15;
    config.fb_count     = 1;
    config.fb_location  = CAMERA_FB_IN_DRAM;
    Serial.println("[WARN] No PSRAM — using reduced quality");
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Camera init failed: 0x%x\n", err);
    return false;
  }
  
  // Fine-tune sensor settings
  sensor_t *s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_brightness(s, 1);    // Slightly brighter
    s->set_contrast(s, 1);      // Slightly more contrast
    s->set_saturation(s, 0);    // Normal saturation
    s->set_whitebal(s, 1);      // Auto white balance ON
    s->set_awb_gain(s, 1);      // AWB gain ON
    s->set_exposure_ctrl(s, 1); // Auto exposure ON
  }
  
  Serial.println("[OK] Camera initialized");
  return true;
}

/**
 * Capture a JPEG image and return it as a Base64-encoded string.
 * Returns empty string on failure.
 */
String captureImageBase64() {
  // Take a picture
  camera_fb_t *fb = esp_camera_fb_get();
  
  if (!fb) {
    Serial.println("[ERROR] Camera capture failed");
    return "";
  }
  
  Serial.printf("[INFO] Captured image: %dx%d, %u bytes\n", 
                fb->width, fb->height, fb->len);
  
  // Encode the JPEG buffer to Base64
  String base64Image = base64::encode(fb->buf, fb->len);
  
  // Release the frame buffer
  esp_camera_fb_return(fb);
  
  Serial.printf("[INFO] Base64 encoded: %u chars\n", base64Image.length());
  return base64Image;
}

// ─────────────────────────────────────────────────────────
//  I2S Microphone Functions (INMP441)
// ─────────────────────────────────────────────────────────

/**
 * Initialize the I2S peripheral for the INMP441 microphone.
 * Must be called before recording. Call deinitMic() after to free resources.
 */
bool initMic() {
  i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = AUDIO_SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = I2S_BUFFER_SIZE,
    .use_apll             = false,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num   = PIN_I2S_SCK,     // GPIO 12 — Bit clock
    .ws_io_num    = PIN_I2S_WS,      // GPIO 4  — Word select
    .data_out_num = I2S_PIN_NO_CHANGE,  // Not used for mic input
    .data_in_num  = PIN_I2S_SD_IN    // GPIO 15 — Data from mic
  };
  
  esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] I2S mic driver install failed: %d\n", err);
    return false;
  }
  
  err = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] I2S mic pin config failed: %d\n", err);
    i2s_driver_uninstall(I2S_NUM_0);
    return false;
  }
  
  // Clear any noise in the DMA buffer
  i2s_zero_dma_buffer(I2S_NUM_0);
  
  Serial.println("[OK] I2S microphone initialized");
  return true;
}

/**
 * Uninstall the I2S driver to free the peripheral for speaker use
 */
void deinitMic() {
  i2s_driver_uninstall(I2S_NUM_0);
  Serial.println("[INFO] I2S mic driver uninstalled");
}

/**
 * Record audio from the INMP441 microphone and return as Base64.
 * Records for AUDIO_RECORD_SECS seconds (default: 5s).
 * Returns empty string on failure.
 */
String recordAudioBase64() {
  Serial.println("[INFO] Starting audio recording...");
  showStatus("Recording...", "Speak now!", "");
  
  // Initialize the mic
  if (!initMic()) {
    return "";
  }
  
  // Calculate buffer size: sample_rate * bytes_per_sample * channels * seconds
  int totalBytes = AUDIO_SAMPLE_RATE * (AUDIO_BITS / 8) * AUDIO_CHANNELS * AUDIO_RECORD_SECS;
  
  // Allocate buffer in PSRAM if available, otherwise heap
  uint8_t *audioBuffer;
  if (psramFound()) {
    audioBuffer = (uint8_t *)ps_malloc(totalBytes);
  } else {
    // Without PSRAM, limit recording to 2 seconds to avoid OOM
    totalBytes = AUDIO_SAMPLE_RATE * (AUDIO_BITS / 8) * AUDIO_CHANNELS * 2;
    audioBuffer = (uint8_t *)malloc(totalBytes);
  }
  
  if (!audioBuffer) {
    Serial.println("[ERROR] Failed to allocate audio buffer");
    deinitMic();
    return "";
  }
  
  // Read audio data from I2S
  size_t bytesRead = 0;
  size_t totalRead = 0;
  int readChunkSize = I2S_BUFFER_SIZE;
  
  while (totalRead < totalBytes) {
    int remaining = totalBytes - totalRead;
    int toRead = (remaining < readChunkSize) ? remaining : readChunkSize;
    
    esp_err_t err = i2s_read(I2S_NUM_0, audioBuffer + totalRead, toRead, &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
      Serial.printf("[ERROR] I2S read error: %d\n", err);
      break;
    }
    totalRead += bytesRead;
    
    // Show progress on OLED
    int progress = (totalRead * 100) / totalBytes;
    if (progress % 20 == 0) {
      char progressStr[32];
      snprintf(progressStr, sizeof(progressStr), "Recording: %d%%", progress);
      showStatus("Recording...", progressStr, "");
    }
  }
  
  Serial.printf("[INFO] Recorded %u bytes of audio\n", totalRead);
  
  // Encode to Base64
  String base64Audio = base64::encode(audioBuffer, totalRead);
  
  // Clean up
  free(audioBuffer);
  deinitMic();
  
  Serial.printf("[INFO] Audio Base64 encoded: %u chars\n", base64Audio.length());
  return base64Audio;
}

// ─────────────────────────────────────────────────────────
//  Wi-Fi Functions
// ─────────────────────────────────────────────────────────

/**
 * Connect to the Wi-Fi network defined in config.h
 */
bool connectWiFi() {
  Serial.printf("[INFO] Connecting to Wi-Fi: %s\n", WIFI_SSID);
  showStatus("Connecting WiFi...", WIFI_SSID, "");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startAttempt = millis();
  int dotCount = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    dotCount++;
    
    // Update OLED with dots
    if (dotCount % 5 == 0) {
      char dots[16];
      snprintf(dots, sizeof(dots), "Attempt %d...", dotCount / 5);
      showStatus("Connecting WiFi...", WIFI_SSID, dots);
    }
    
    // Timeout check
    if (millis() - startAttempt > WIFI_TIMEOUT_MS) {
      Serial.println("\n[ERROR] Wi-Fi connection timed out!");
      showStatus("WiFi FAILED!", "Check SSID/Pass", "Restarting...");
      delay(3000);
      return false;
    }
  }
  
  Serial.println();
  Serial.printf("[OK] Wi-Fi connected! IP: %s\n", WiFi.localIP().toString().c_str());
  
  char ipStr[20];
  snprintf(ipStr, sizeof(ipStr), "IP:%s", WiFi.localIP().toString().c_str());
  showStatus("WiFi Connected!", ipStr, "");
  delay(1500);
  
  return true;
}

/**
 * Check if the Flask backend server is reachable
 */
bool checkServer() {
  HTTPClient http;
  String url = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + ENDPOINT_HEALTH;
  
  Serial.printf("[INFO] Checking server: %s\n", url.c_str());
  
  http.begin(url);
  http.setTimeout(5000);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.printf("[OK] Server is alive: %s\n", response.c_str());
    http.end();
    return true;
  } else {
    Serial.printf("[WARN] Server returned code: %d\n", httpCode);
    http.end();
    return false;
  }
}

// ─────────────────────────────────────────────────────────
//  HTTP Communication with Flask Backend
// ─────────────────────────────────────────────────────────

/**
 * Send a captured image to the Flask backend for analysis.
 * 
 * @param imageBase64   Base64-encoded JPEG image
 * @param analysisType  "description" or "ocr"
 * @param prompt        Optional prompt for description mode
 * @return              AI-generated response text
 */
String sendImageToServer(const String& imageBase64, const char* analysisType, const char* prompt = "") {
  HTTPClient http;
  String url = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + ENDPOINT_DESCRIBE;
  
  Serial.printf("[INFO] Sending image to: %s (type: %s)\n", url.c_str(), analysisType);
  
  http.begin(url);
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.addHeader("Content-Type", "application/json");
  
  // Build JSON payload
  // Using DynamicJsonDocument for large base64 payloads
  size_t jsonCapacity = imageBase64.length() + 512;
  DynamicJsonDocument doc(jsonCapacity);
  doc["image_base64"] = imageBase64;
  doc["analysis_type"] = analysisType;
  if (strlen(prompt) > 0) {
    doc["prompt"] = prompt;
  }
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.printf("[INFO] Payload size: %u bytes\n", payload.length());
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.printf("[OK] Server response: %s\n", response.c_str());
    
    // Parse JSON response
    DynamicJsonDocument responseDoc(4096);
    DeserializationError error = deserializeJson(responseDoc, response);
    
    if (!error) {
      // Extract the result text based on analysis type
      String result = "";
      if (strcmp(analysisType, "description") == 0) {
        result = responseDoc["description"].as<String>();
      } else if (strcmp(analysisType, "ocr") == 0) {
        result = responseDoc["extracted_text"].as<String>();
      }
      
      // Fallback to generic "result" field
      if (result == "null" || result.length() == 0) {
        result = responseDoc["result"].as<String>();
      }
      
      float responseTime = responseDoc["response_time_seconds"].as<float>();
      Serial.printf("[INFO] Response time: %.2fs\n", responseTime);
      
      http.end();
      return result;
    }
    
    http.end();
    return response;  // Return raw response if JSON parsing fails
  } else {
    Serial.printf("[ERROR] HTTP POST failed, code: %d\n", httpCode);
    http.end();
    return "Error: Server returned " + String(httpCode);
  }
}

/**
 * Send a text question to the Flask backend for Q&A or translation.
 * 
 * @param question  The question or text to process
 * @param mode      MODE_QA or MODE_TRANSLATE
 * @return          AI-generated response text
 */
String sendQuestionToServer(const String& question, int mode) {
  HTTPClient http;
  String url = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + ENDPOINT_ASK;
  
  Serial.printf("[INFO] Sending question to: %s\n", url.c_str());
  
  http.begin(url);
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.addHeader("Content-Type", "application/json");
  
  // Build JSON payload
  DynamicJsonDocument doc(2048);
  doc["question"] = question;
  
  // Add context for translation mode
  if (mode == MODE_TRANSLATE) {
    doc["context"] = "Translate the following text to English. Only provide the translation, no explanation.";
  }
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    
    DynamicJsonDocument responseDoc(4096);
    DeserializationError error = deserializeJson(responseDoc, response);
    
    if (!error) {
      String result = responseDoc["answer"].as<String>();
      if (result == "null" || result.length() == 0) {
        result = responseDoc["result"].as<String>();
      }
      http.end();
      return result;
    }
    
    http.end();
    return response;
  } else {
    Serial.printf("[ERROR] HTTP POST failed, code: %d\n", httpCode);
    http.end();
    return "Error: Server returned " + String(httpCode);
  }
}

/**
 * Send recorded audio to the Flask backend for speech-to-text.
 * 
 * @param audioBase64  Base64-encoded PCM audio data
 * @return             Transcribed text
 */
String sendAudioToServer(const String& audioBase64) {
  HTTPClient http;
  String url = "http://" + String(SERVER_IP) + ":" + String(SERVER_PORT) + ENDPOINT_TRANSCRIBE;
  
  Serial.printf("[INFO] Sending audio to: %s\n", url.c_str());
  
  http.begin(url);
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.addHeader("Content-Type", "application/json");
  
  // Build JSON payload
  size_t jsonCapacity = audioBase64.length() + 256;
  DynamicJsonDocument doc(jsonCapacity);
  doc["audio_base64"] = audioBase64;
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.printf("[INFO] Audio payload size: %u bytes\n", payload.length());
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    
    DynamicJsonDocument responseDoc(4096);
    DeserializationError error = deserializeJson(responseDoc, response);
    
    if (!error) {
      String result = responseDoc["transcription"].as<String>();
      if (result == "null" || result.length() == 0) {
        result = responseDoc["text"].as<String>();
      }
      if (result == "null" || result.length() == 0) {
        result = responseDoc["result"].as<String>();
      }
      http.end();
      return result;
    }
    
    http.end();
    return response;
  } else {
    Serial.printf("[ERROR] HTTP POST failed, code: %d\n", httpCode);
    http.end();
    return "Error: Server returned " + String(httpCode);
  }
}

// ─────────────────────────────────────────────────────────
//  Main Action Handler
// ─────────────────────────────────────────────────────────

/**
 * Execute the action for the currently selected mode.
 * Called when the user presses Button 2 (Action).
 */
void handleAction() {
  Serial.printf("\n[ACTION] Mode: %s (%d)\n", MODE_NAMES[currentMode], currentMode);
  
  String response = "";
  
  switch (currentMode) {
    
    // ──────────────────────────────────────────
    // MODE 0: Describe Scene (Camera → Groq Cloud)
    // ──────────────────────────────────────────
    case MODE_DESCRIBE: {
      showStatus("Capturing image...", "", "");
      
      // Briefly flash the LED to indicate capture
      digitalWrite(PIN_STATUS_LED, LOW);  // LED ON (active LOW)
      
      String imageB64 = captureImageBase64();
      
      digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF
      
      if (imageB64.length() == 0) {
        showStatus("ERROR", "Camera capture", "failed!");
        delay(2000);
        break;
      }
      
      showProcessing();
      response = sendImageToServer(imageB64, "description", "Describe what you see in this image in 2-3 sentences. Be concise and helpful for a visually impaired person.");
      break;
    }
    
    // ──────────────────────────────────────────
    // MODE 1: Read Text / OCR (Camera → EasyOCR Local)
    // ──────────────────────────────────────────
    case MODE_OCR: {
      showStatus("Capturing text...", "Hold steady!", "");
      
      digitalWrite(PIN_STATUS_LED, LOW);
      String imageB64 = captureImageBase64();
      digitalWrite(PIN_STATUS_LED, HIGH);
      
      if (imageB64.length() == 0) {
        showStatus("ERROR", "Camera capture", "failed!");
        delay(2000);
        break;
      }
      
      showProcessing();
      response = sendImageToServer(imageB64, "ocr");
      break;
    }
    
    // ──────────────────────────────────────────
    // MODE 2: Speech-to-Text (Mic → Whisper Local)
    // ──────────────────────────────────────────
    case MODE_SPEECH: {
      String audioB64 = recordAudioBase64();
      
      if (audioB64.length() == 0) {
        showStatus("ERROR", "Audio recording", "failed!");
        delay(2000);
        break;
      }
      
      showProcessing();
      response = sendAudioToServer(audioB64);
      break;
    }
    
    // ──────────────────────────────────────────
    // MODE 3: Q&A (Mic → Groq Cloud)
    // Record question, transcribe it, then ask Llama
    // ──────────────────────────────────────────
    case MODE_QA: {
      // Step 1: Record the question via microphone
      String audioB64 = recordAudioBase64();
      
      if (audioB64.length() == 0) {
        showStatus("ERROR", "Audio recording", "failed!");
        delay(2000);
        break;
      }
      
      // Step 2: Transcribe the audio to text
      showStatus("Transcribing...", "", "");
      String question = sendAudioToServer(audioB64);
      
      if (question.length() == 0 || question.startsWith("Error")) {
        showStatus("ERROR", "Transcription", "failed!");
        delay(2000);
        break;
      }
      
      Serial.printf("[INFO] Transcribed question: %s\n", question.c_str());
      showStatus("Question:", question.substring(0, 20).c_str(), "Asking AI...");
      
      // Step 3: Send the question to Llama for an answer
      showProcessing();
      response = sendQuestionToServer(question, MODE_QA);
      break;
    }
    
    // ──────────────────────────────────────────
    // MODE 4: Translation (Mic → Groq Cloud)
    // Record speech, transcribe, then translate
    // ──────────────────────────────────────────
    case MODE_TRANSLATE: {
      // Step 1: Record the speech
      String audioB64 = recordAudioBase64();
      
      if (audioB64.length() == 0) {
        showStatus("ERROR", "Audio recording", "failed!");
        delay(2000);
        break;
      }
      
      // Step 2: Transcribe the audio
      showStatus("Transcribing...", "", "");
      String text = sendAudioToServer(audioB64);
      
      if (text.length() == 0 || text.startsWith("Error")) {
        showStatus("ERROR", "Transcription", "failed!");
        delay(2000);
        break;
      }
      
      Serial.printf("[INFO] Text to translate: %s\n", text.c_str());
      showStatus("Translating...", text.substring(0, 20).c_str(), "");
      
      // Step 3: Send for translation
      showProcessing();
      response = sendQuestionToServer(text, MODE_TRANSLATE);
      break;
    }
  }
  
  // ──────────────────────────────────────────
  // Display the AI response
  // ──────────────────────────────────────────
  if (response.length() > 0) {
    Serial.printf("[RESULT] %s\n", response.c_str());
    showResponse(response);
    
    // Keep the response on screen for a while, or until button press
    delay(5000);
  }
  
  // Return to mode selection screen
  showMode();
}

// ─────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────

void setup() {
  // Initialize serial for debugging (115200 baud)
  Serial.begin(115200);
  Serial.println();
  Serial.println("==========================================");
  Serial.println("  AI Smart Glasses — ESP32-CAM Firmware");
  Serial.println("==========================================");
  Serial.println();
  
  // ── Status LED ──
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, HIGH);  // OFF (active LOW)
  
  // ── OLED Display ──
  displayReady = initDisplay();
  if (!displayReady) {
    Serial.println("[WARN] Running without display — check I2C wiring");
  }
  
  // ── Camera ──
  cameraReady = initCamera();
  if (!cameraReady) {
    Serial.println("[FATAL] Camera failed! Check hardware.");
    showStatus("CAMERA ERROR!", "Check ribbon cable", "Restarting...");
    delay(5000);
    ESP.restart();
  }
  
  // ── Wi-Fi ──
  wifiConnected = connectWiFi();
  if (!wifiConnected) {
    Serial.println("[FATAL] Wi-Fi failed! Check credentials.");
    delay(3000);
    ESP.restart();
  }
  
  // ── Server Check ──
  serverReachable = checkServer();
  if (serverReachable) {
    showStatus("Server: ONLINE", "All systems ready!", "");
    Serial.println("[OK] Flask server is reachable");
  } else {
    showStatus("Server: OFFLINE", "Start Flask server", "on your PC!");
    Serial.println("[WARN] Flask server not reachable — start it on your PC");
    Serial.printf("[WARN] Expected at: http://%s:%d\n", SERVER_IP, SERVER_PORT);
  }
  delay(2000);
  
  // ── Buttons (attach interrupts AFTER boot to avoid GPIO 0 conflict) ──
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_BTN_ACTION, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_MODE), onBtnModePress, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_ACTION), onBtnActionPress, FALLING);
  
  // ── Ready! ──
  Serial.println();
  Serial.println("[READY] AI Smart Glasses initialized!");
  Serial.printf("[INFO]  Mode: %s\n", MODE_NAMES[currentMode]);
  Serial.printf("[INFO]  Server: http://%s:%d\n", SERVER_IP, SERVER_PORT);
  Serial.printf("[INFO]  WiFi IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.println("==========================================");
  Serial.println("  BTN1 (GPIO 0)  = Cycle Mode");
  Serial.println("  BTN2 (GPIO 16) = Capture / Record");
  Serial.println("==========================================");
  Serial.println();
  
  // Show mode selection on OLED
  showMode();
}

// ─────────────────────────────────────────────────────────
//  MAIN LOOP
// ─────────────────────────────────────────────────────────

// Track previous mode to detect changes
int previousMode = -1;

void loop() {
  // ── Update display when mode changes ──
  if (currentMode != previousMode) {
    Serial.printf("[MODE] Changed to: %s\n", MODE_NAMES[currentMode]);
    showMode();
    previousMode = currentMode;
  }
  
  // ── Handle action button press ──
  if (actionTriggered) {
    actionTriggered = false;
    
    // Check Wi-Fi is still connected
    if (WiFi.status() != WL_CONNECTED) {
      showStatus("WiFi Lost!", "Reconnecting...", "");
      Serial.println("[WARN] Wi-Fi disconnected, reconnecting...");
      wifiConnected = connectWiFi();
      if (!wifiConnected) {
        showStatus("WiFi FAILED!", "Check connection", "");
        delay(3000);
        showMode();
        return;
      }
    }
    
    // Execute the mode action
    handleAction();
  }
  
  // Small delay to prevent watchdog timer issues
  delay(10);
}
