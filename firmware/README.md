# ESP32-CAM Firmware — Quick Start Guide

## 📁 Files

| File | Purpose |
|------|---------|
| `smart_glasses_firmware.ino` | Main Arduino sketch (all logic) |
| `config.h` | Configuration — Wi-Fi, server IP, pins, settings |

---

## 🛠️ Prerequisites

### Hardware for Programming
You need a **USB-to-Serial adapter** (FTDI FT232RL or CP2102) since the ESP32-CAM has no built-in USB.

### Wiring: FTDI ↔ ESP32-CAM

```
FTDI / CP2102              ESP32-CAM
┌──────────┐              ┌──────────┐
│ VCC (5V) ├──────────────┤ 5V       │
│ GND      ├──────────────┤ GND      │
│ TX       ├──────────────┤ U0R      │  (FTDI TX → ESP RX)
│ RX       ├──────────────┤ U0T      │  (FTDI RX → ESP TX)
└──────────┘              └──────────┘

        ALSO: GPIO 0 ──── GND   (enables flash mode)
```

### Software
1. **Arduino IDE 2.x** — [Download](https://www.arduino.cc/en/software)
2. **ESP32 Board Package** — Add this URL in File → Preferences → Additional Board URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   Then go to Tools → Board → Boards Manager → search "esp32" → Install

3. **Required Libraries** (install via Sketch → Include Library → Manage Libraries):

   | Library | Author | Version |
   |---------|--------|---------|
   | Adafruit SSD1306 | Adafruit | 2.5+ |
   | Adafruit GFX Library | Adafruit | 1.11+ |
   | ArduinoJson | Benoit Blanchon | 6.x or 7.x |

   > `WiFi.h`, `HTTPClient.h`, `Wire.h`, `esp_camera.h`, `driver/i2s.h`, and `base64.h` are included with the ESP32 board package.

---

## ⚙️ Configuration

### Step 1: Edit `config.h`

Open `config.h` and change these **three values**:

```cpp
#define WIFI_SSID       "YOUR_WIFI_SSID"       // Your Wi-Fi name
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"    // Your Wi-Fi password
#define SERVER_IP       "192.168.1.100"         // Your PC's local IP
```

**To find your PC's IP address:**
- Windows: Open CMD → type `ipconfig` → look for "IPv4 Address"
- Linux/Mac: Open terminal → type `ifconfig` or `ip addr`

### Step 2: Verify Pin Assignments

The default pins match the wiring in the main `README.md`:

| Function | GPIO | Default |
|----------|------|---------|
| OLED SDA | 13 | ✅ |
| OLED SCL | 14 | ✅ |
| Mic SCK | 12 | ✅ |
| Mic WS | 4 | ✅ |
| Mic SD | 15 | ✅ |
| Speaker DIN | 2 | ✅ |
| Button (Mode) | 0 | ✅ |
| Button (Action) | 16 | ✅ |

Only change these if you've wired differently.

---

## 📤 Upload Steps

### Arduino IDE Settings

| Setting | Value |
|---------|-------|
| **Board** | AI Thinker ESP32-CAM |
| **Upload Speed** | 115200 |
| **Flash Frequency** | 80MHz |
| **Partition Scheme** | Huge APP (3MB No OTA/1MB SPIFFS) |
| **Port** | Your COM port (e.g., COM3) |

### Upload Procedure

1. **Connect GPIO 0 to GND** (puts ESP32 in flash mode)
2. **Connect FTDI adapter** to ESP32-CAM (wiring above)
3. **Plug FTDI into PC** via USB
4. **Open** `smart_glasses_firmware.ino` in Arduino IDE
5. **Select** board and port in Tools menu
6. **Click Upload** (→ arrow button)
7. When console shows `Connecting...` → **press RESET button** on ESP32-CAM
8. Wait for `Done uploading`

### After Upload

1. **Disconnect GPIO 0 from GND**
2. **Press RESET** button on ESP32-CAM
3. **Open Serial Monitor** (Tools → Serial Monitor, baud: 115200)
4. You should see:
   ```
   ==========================================
     AI Smart Glasses — ESP32-CAM Firmware
   ==========================================
   [OK] OLED display initialized
   [OK] Camera initialized
   [OK] Wi-Fi connected! IP: 192.168.1.xxx
   [READY] AI Smart Glasses initialized!
   ```

---

## 🎮 How to Use

| Button | Action | What Happens |
|--------|--------|-------------|
| **BTN1** (GPIO 0) | Short press | Cycles mode: DESCRIBE → READ TEXT → SPEECH → Q&A → TRANSLATE |
| **BTN2** (GPIO 16) | Short press | Captures image (modes 0-1) or records audio (modes 2-4) and sends to server |

### Mode Details

| Mode | Input | AI Model | What It Does |
|------|-------|----------|-------------|
| 0 — DESCRIBE | Camera | Llama 4 Scout (Cloud) | Describes the scene for visually impaired users |
| 1 — READ TEXT | Camera | EasyOCR (Local) | Reads text from signs, books, labels |
| 2 — SPEECH | Microphone | Whisper tiny (Local) | Transcribes speech to text |
| 3 — Q&A | Microphone | Llama 3.3 70B (Cloud) | Answers spoken questions |
| 4 — TRANSLATE | Microphone | Llama 3.3 70B (Cloud) | Translates spoken text to English |

---

## ❗ Troubleshooting

| Problem | Solution |
|---------|----------|
| `Failed to connect to ESP32` | GPIO 0 not grounded. Wire GPIO 0 → GND, then press RESET |
| `No serial data received` | TX/RX wires are swapped. Swap them |
| COM port not showing | Install driver: [CH340](https://sparks.gogo.co.nz/ch340.html) or [CP2102](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) |
| `Brownout detector triggered` | Power issue — use 5V pin, add a capacitor |
| `Camera init failed: 0x20002` | Camera ribbon cable loose — reseat it firmly |
| `OLED not found at 0x3C` | Try changing `OLED_ADDR` to `0x3D` in config.h |
| `WiFi FAILED` | Check SSID/password in config.h. Make sure 2.4GHz (not 5GHz) |
| `Server: OFFLINE` | Start Flask server: `cd test_multi_model_project && python main.py` |
| Code runs but LED blinks during audio | Normal — GPIO 4 is shared between flash LED and I2S WS |

---

## 📐 Architecture

```
┌─────────────────────────────────────────────────┐
│              ESP32-CAM Firmware                  │
│                                                  │
│  setup()                                         │
│  ├── initDisplay()     → OLED on I2C (13, 14)   │
│  ├── initCamera()      → OV2640 via ribbon       │
│  ├── connectWiFi()     → Join network            │
│  ├── checkServer()     → Ping Flask /health      │
│  └── attachInterrupts  → Buttons on GPIO 0, 16   │
│                                                  │
│  loop()                                          │
│  ├── Check mode change → Update OLED             │
│  └── If action pressed → handleAction()          │
│       ├── MODE_DESCRIBE → captureImage → POST    │
│       ├── MODE_OCR      → captureImage → POST    │
│       ├── MODE_SPEECH   → recordAudio  → POST    │
│       ├── MODE_QA       → recordAudio  → POST x2 │
│       └── MODE_TRANSLATE→ recordAudio  → POST x2 │
│                                                  │
│  showResponse() → Display result on OLED         │
└─────────────────────────────────────────────────┘
          │
          │  HTTP POST (Wi-Fi)
          ▼
┌─────────────────────────────────────────────────┐
│         Flask Server (PC)                        │
│         http://192.168.1.xxx:5000               │
│                                                  │
│  /analyze-image  → Groq (describe) / OCR (local)│
│  /ask            → Groq Llama 3.3 (Q&A)         │
│  /transcribe     → Whisper tiny (local)          │
│  /health         → Server status                 │
└─────────────────────────────────────────────────┘
```
