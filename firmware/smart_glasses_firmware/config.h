/*
 * ============================================================
 *  AI Smart Glasses — ESP32-CAM Firmware Configuration
 * ============================================================
 *  Edit this file to match your setup before uploading.
 *  DO NOT change pin assignments unless you re-wire hardware.
 * ============================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

// ─────────────────────────────────────────────
// Wi-Fi Configuration
// ─────────────────────────────────────────────
#define WIFI_SSID       "YOUR_WIFI_SSID"        // ← Change this
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"     // ← Change this

// ─────────────────────────────────────────────
// Flask Backend Server
// ─────────────────────────────────────────────
// IP address of the PC running the Flask server.
// Find it with: ipconfig (Windows) or ifconfig (Linux/Mac)
#define SERVER_IP       "192.168.1.100"          // ← Change this
#define SERVER_PORT     5000

// API Endpoints (must match Flask routes in main.py)
#define ENDPOINT_DESCRIBE   "/analyze-image"
#define ENDPOINT_OCR        "/analyze-image"
#define ENDPOINT_ASK        "/ask"
#define ENDPOINT_TRANSCRIBE "/transcribe"
#define ENDPOINT_HEALTH     "/health"

// ─────────────────────────────────────────────
// GPIO Pin Assignments
// ─────────────────────────────────────────────

// -- I2C Pins (OLED Display) --
#define PIN_SDA         13    // GPIO 13 → OLED SDA
#define PIN_SCL         14    // GPIO 14 → OLED SCL

// -- I2S Pins (Microphone INMP441) --
#define PIN_I2S_SCK     12    // GPIO 12 → INMP441 SCK (bit clock)
#define PIN_I2S_WS       4    // GPIO  4 → INMP441 WS  (word select)
#define PIN_I2S_SD_IN   15    // GPIO 15 → INMP441 SD  (data in from mic)

// -- I2S Pins (Speaker MAX98357A) --
// BCLK and LRC are shared with mic (GPIO 12 and GPIO 4)
#define PIN_I2S_SD_OUT   2    // GPIO  2 → MAX98357A DIN (data out to speaker)

// -- Push Buttons --
#define PIN_BTN_MODE     0    // GPIO  0 → Button 1 (Mode Select)
#define PIN_BTN_ACTION  16    // GPIO 16 → Button 2 (Action / Capture)

// -- On-board LEDs --
#define PIN_FLASH_LED    4    // GPIO  4 → On-board flash LED (shared with I2S WS)
#define PIN_STATUS_LED  33    // GPIO 33 → On-board red status LED (active LOW)

// ─────────────────────────────────────────────
// OLED Display Settings
// ─────────────────────────────────────────────
#define OLED_WIDTH      128
#define OLED_HEIGHT      64
#define OLED_ADDR       0x3C  // I2C address (try 0x3D if 0x3C doesn't work)
#define OLED_RESET       -1   // No hardware reset pin

// ─────────────────────────────────────────────
// Camera Settings (OV2640)
// ─────────────────────────────────────────────
// Image resolution for capture — lower = faster upload
// Options: FRAMESIZE_QQVGA (160x120), FRAMESIZE_QVGA (320x240),
//          FRAMESIZE_VGA (640x480), FRAMESIZE_SVGA (800x600)
#define CAMERA_RESOLUTION  FRAMESIZE_VGA    // 640x480 — good balance
#define CAMERA_QUALITY     12               // JPEG quality: 0-63 (lower = better quality, larger file)

// ─────────────────────────────────────────────
// Audio Settings (I2S Microphone)
// ─────────────────────────────────────────────
#define AUDIO_SAMPLE_RATE   16000   // 16 kHz (standard for speech recognition)
#define AUDIO_BITS          16      // 16-bit audio
#define AUDIO_CHANNELS      1       // Mono
#define AUDIO_RECORD_SECS   5       // Max recording duration in seconds
#define I2S_BUFFER_SIZE     1024    // I2S DMA buffer size in bytes

// ─────────────────────────────────────────────
// Timing & Debounce
// ─────────────────────────────────────────────
#define DEBOUNCE_MS         250     // Button debounce time (ms)
#define WIFI_TIMEOUT_MS     15000   // Wi-Fi connection timeout (ms)
#define HTTP_TIMEOUT_MS     30000   // HTTP request timeout (ms)
#define OLED_SCROLL_DELAY   100     // Text scroll speed on OLED (ms)

// ─────────────────────────────────────────────
// Operating Modes
// ─────────────────────────────────────────────
// These correspond to the Flask Smart Router endpoints
enum Mode {
  MODE_DESCRIBE  = 0,   // Scene description (Llama 4 Scout via Groq)
  MODE_OCR       = 1,   // Text extraction (EasyOCR local)
  MODE_SPEECH    = 2,   // Speech-to-text (Whisper tiny local)
  MODE_QA        = 3,   // Question & Answer (Llama 3.3 via Groq)
  MODE_TRANSLATE = 4,   // Translation (Llama 3.3 via Groq)
  MODE_COUNT     = 5    // Total number of modes
};

// Mode display names (shown on OLED)
const char* MODE_NAMES[] = {
  "DESCRIBE",
  "READ TEXT",
  "SPEECH",
  "Q & A",
  "TRANSLATE"
};

#endif // CONFIG_H
