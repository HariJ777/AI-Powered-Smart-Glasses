# 🕶️ AI Smart Glasses — ESP32-CAM

An affordable, AI-powered assistive wearable that captures images and audio, processes them through a hybrid cloud-local AI backend, and delivers real-time scene descriptions, OCR text reading, speech transcription, Q&A, and translation — all displayed on a tiny OLED screen and spoken through a micro speaker.

---

## 📑 Table of Contents+

- [Project Overview](#-project-overview)
- [System Architecture](#-system-architecture)
- [Hardware — Bill of Materials](#-hardware--bill-of-materials)
- [ESP32-CAM Pinout Reference](#-esp32-cam-pinout-reference)
- [Wiring Connections — Component by Component](#-wiring-connections--component-by-component)
  - [OLED Display (SSD1306)](#1-oled-display--ssd1306-096-128x64-i2c)
  - [INMP441 Microphone](#2-inmp441-i2s-digital-microphone)
  - [Audio Amplifier + Speaker](#3-audio-amplifier-max98357a--micro-speaker)
  - [Push Buttons](#4-push-buttons-x2)
  - [Power System](#5-power-system-lipo--tp4056--voltage-regulator)
- [Complete Wiring Summary Table](#-complete-wiring-summary-table)
- [Decoupling Capacitors](#-decoupling-capacitors)
- [Circuit Notes & Warnings](#-circuit-notes--warnings)
- [Power Consumption Estimates](#-power-consumption-estimates)
- [Software Stack](#-software-stack)
- [How It Works](#-how-it-works)
- [Getting Started](#-getting-started)
- [Project Team](#-project-team)

---

## 🔍 Project Overview

The AI Smart Glasses use the **ESP32-CAM** microcontroller as the core hardware. The glasses capture images via the onboard OV2640 camera and send them over Wi-Fi to a **Flask backend server** running on a local PC. The backend uses a **Smart Routing** architecture to distribute tasks across multiple AI models — heavy tasks are offloaded to cloud models via the **Groq API**, while lightweight tasks run locally.

### Five Core AI Capabilities

| # | Capability | AI Model | Where it Runs |
|---|-----------|----------|---------------|
| 1 | **Scene Description** | Llama 4 Scout 17B | ☁️ Cloud (Groq API) |
| 2 | **Question Answering** | Llama 3.3 70B | ☁️ Cloud (Groq API) |
| 3 | **Translation** | Llama 3.3 70B | ☁️ Cloud (Groq API) |
| 4 | **Text Extraction (OCR)** | EasyOCR 1.7+ | 💻 Local (~180 MB) |
| 5 | **Speech-to-Text** | Whisper tiny | 💻 Local (~145 MB) |

---

## 🏗️ System Architecture

```
┌───────────────────────────────────────────────────────┐
│                   SMART GLASSES                       │
│                                                       │
│  ┌──────────┐  ┌──────────┐  ┌────────┐  ┌────────┐ │
│  │ OV2640   │  │ INMP441  │  │ OLED   │  │Speaker │ │
│  │ Camera   │  │   Mic    │  │Display │  │+ Amp   │ │
│  └────┬─────┘  └────┬─────┘  └───┬────┘  └───┬────┘ │
│       │              │            │            │      │
│  ┌────┴──────────────┴────────────┴────────────┴────┐ │
│  │              ESP32-CAM Module                     │ │
│  │          (Wi-Fi + Bluetooth + Camera)             │ │
│  └───────────────────┬───────────────────────────────┘ │
│                      │ Wi-Fi (HTTP POST)              │
│  ┌──────┐ ┌──────┐  │                                │
│  │ BTN1 │ │ BTN2 │  │                                │
│  └──────┘ └──────┘  │                                │
│                      │  ┌─────────────────────┐      │
│  ┌───────────────┐   │  │ LiPo 3.7V + TP4056 │      │
│  │  3.3V LDO     │◄──┤  │ + AMS1117-3.3V     │      │
│  └───────────────┘   │  └─────────────────────┘      │
└──────────────────────┼───────────────────────────────┘
                       │
                       ▼  Wi-Fi
        ┌──────────────────────────────┐
        │    LOCAL PC (Flask Server)   │
        │                              │
        │  ┌────────────────────────┐  │
        │  │   Flask Smart Router   │  │
        │  │                        │  │
        │  │  ┌─────┐   ┌────────┐ │  │
        │  │  │Local │   │ Groq   │ │  │
        │  │  │Models│   │Cloud AI│ │  │
        │  │  └─────┘   └────────┘ │  │
        │  └────────────────────────┘  │
        └──────────────────────────────┘
```

---

## 🛒 Hardware — Bill of Materials

| # | Component | Specification | Qty | Purpose |
|---|-----------|--------------|-----|---------|
| 1 | **ESP32-CAM** | AI-Thinker module with OV2640 camera | x1 | Main controller + camera + Wi-Fi |
| 2 | **OLED Display** | SSD1306, 0.96", 128×64, I2C | x1 | Display AI responses |
| 3 | **INMP441 Microphone** | I2S digital MEMS microphone | x1 | Audio input for speech-to-text |
| 4 | **Audio Amplifier** | MAX98357A I2S amplifier module | x1 | Amplify audio output to speaker |
| 5 | **Micro Speaker** | 8Ω, 0.5W, 40mm | x1 | Text-to-speech audio output |
| 6 | **Push Buttons** | Momentary tactile switch (6×6mm) | x2 | Mode select + action trigger |
| 7 | **LiPo Battery** | 3.7V, 1000mAh (or 2000mAh) | x1 | Portable power supply |
| 8 | **TP4056 Charger** | With DW01 + FS8205A protection | x1 | USB charging + battery protection |
| 9 | **Voltage Regulator** | AMS1117-3.3V (or AP2112K-3.3) LDO | x1 | Regulate 3.7V → 3.3V |
| 10 | **Resistor 4.7kΩ** | 1/4W, through-hole | x2 | I2C pull-up (SDA & SCL) |
| 11 | **Resistor 10kΩ** | 1/4W, through-hole | x2 | Button pull-up resistors |
| 12 | **Capacitor 100nF** | Ceramic (104), through-hole | x2 | Decoupling for ESP32 + OLED |
| 13 | **Capacitor 10µF** | Electrolytic, 16V | x1 | Bulk power decoupling |
| 14 | **Jumper Wires** | Male-to-female, various lengths | 1 set | All connections |
| 15 | **JST Connector** | 2-pin JST PH 2.0mm | x1 | Battery connection |
| 16 | **Power Switch** | SPST slide switch (optional) | x1 | On/Off control |
| 17 | **Glasses Frame** | Any standard frame for mounting | x1 | Physical enclosure |

---

## 📌 ESP32-CAM Pinout Reference

The ESP32-CAM (AI-Thinker variant) has limited exposed GPIO pins because many are used internally by the camera. Here are the **available GPIO pins** for peripherals:

```
            ┌──────────────────┐
            │    ESP32-CAM     │
            │   (Top View)     │
            │  ┌────────────┐  │
            │  │  OV2640    │  │
            │  │  Camera    │  │
            │  └────────────┘  │
            │                  │
    5V  ────┤ 5V          3V3 ├──── 3.3V
   GND  ────┤ GND       GPIO16├──── (UART RX)
GPIO 12 ────┤ IO12      GPIO 0├──── (Boot / Button)
GPIO 13 ────┤ IO13       GND  ├──── GND
GPIO 15 ────┤ IO15      GPIO 2├──── (On-board LED)
GPIO 14 ────┤ IO14      GPIO 4├──── (Flash LED)
GPIO  2 ────┤ IO2       GPIO 3├──── (UART TX)
GPIO  4 ────┤ IO4              │
            └──────────────────┘
```

### GPIO Pin Availability

| GPIO | Default Function | Available? | Our Usage |
|------|-----------------|------------|-----------|
| **GPIO 0** | Boot button | ⚠️ After boot | Button 1 (Mode Select) |
| **GPIO 2** | On-board LED | ✅ Yes | I2S Data Out (Speaker) |
| **GPIO 3** | UART0 TX | ⚠️ Serial | — (keep for debugging) |
| **GPIO 4** | Flash LED | ✅ Yes | I2S Word Select (Mic WS) |
| **GPIO 12** | Free | ✅ Yes | I2S Serial Clock (Mic SCK) |
| **GPIO 13** | Free | ✅ Yes | I2C SDA (OLED) |
| **GPIO 14** | Free | ✅ Yes | I2C SCL (OLED) |
| **GPIO 15** | Free | ✅ Yes | I2S Serial Data (Mic SD) |
| **GPIO 16** | UART0 RX | ⚠️ Serial | Button 2 (Action Trigger) |

> **⚠️ Important:** GPIO 0 must be HIGH during boot (pulled up internally). It can be used as a button input after boot. GPIO 16 is shared with UART RX — if using serial debugging, use a different pin for Button 2.

---

## 🔌 Wiring Connections — Component by Component

### 1. OLED Display — SSD1306 (0.96", 128×64, I2C)

The OLED display uses the **I2C protocol** (2-wire) and requires external pull-up resistors.

| OLED Pin | Wire Color (Suggested) | Connects To | Notes |
|----------|----------------------|-------------|-------|
| **VCC** | 🔴 Red | **3.3V rail** | Power supply (do NOT use 5V) |
| **GND** | ⚫ Black | **GND rail** | Common ground |
| **SDA** | 🔵 Blue | **ESP32 GPIO 13** | Data line |
| **SCL** | 🟡 Yellow | **ESP32 GPIO 14** | Clock line |

#### Required Pull-up Resistors for I2C

```
3.3V ──┬──────────────────┬──
       │                  │
      [4.7kΩ]           [4.7kΩ]
       │                  │
       ├── SDA (GPIO 13)  ├── SCL (GPIO 14)
       │                  │
   (to OLED SDA)      (to OLED SCL)
```

- Connect a **4.7kΩ resistor** between **3.3V** and **GPIO 13 (SDA)**
- Connect a **4.7kΩ resistor** between **3.3V** and **GPIO 14 (SCL)**

> **Note:** The OLED's default I2C address is **0x3C**. If your display uses 0x3D, update the firmware code accordingly.

---

### 2. INMP441 I2S Digital Microphone

The INMP441 uses the **I2S protocol** (3-wire + power) for high-quality digital audio capture.

| INMP441 Pin | Wire Color (Suggested) | Connects To | Notes |
|-------------|----------------------|-------------|-------|
| **VDD** | 🔴 Red | **3.3V rail** | Power supply (1.8V–3.3V) |
| **GND** | ⚫ Black | **GND rail** | Common ground |
| **SCK** | 🟠 Orange | **ESP32 GPIO 12** | I2S serial clock |
| **WS** | 🟢 Green | **ESP32 GPIO 4** | I2S word select (L/R) |
| **SD** | 🟣 Purple | **ESP32 GPIO 15** | I2S serial data (audio out) |
| **L/R** | ⚫ Black | **GND** | Tie to GND for LEFT channel |

```
INMP441 Module          ESP32-CAM
┌──────────┐            ┌──────────┐
│ VDD      ├──── 🔴 ────┤ 3.3V     │
│ GND      ├──── ⚫ ────┤ GND      │
│ SCK      ├──── 🟠 ────┤ GPIO 12  │
│ WS       ├──── 🟢 ────┤ GPIO 4   │
│ SD       ├──── 🟣 ────┤ GPIO 15  │
│ L/R      ├──── ⚫ ────┤ GND      │
└──────────┘            └──────────┘
```

> **Note:** Tying L/R to GND selects the **LEFT channel**. Tie to VDD for RIGHT channel. Most code examples use LEFT.

---

### 3. Audio Amplifier (MAX98357A) + Micro Speaker

The MAX98357A is an I2S audio amplifier that directly drives the speaker. It takes I2S digital audio from the ESP32 and outputs analog audio to the speaker.

| MAX98357A Pin | Wire Color (Suggested) | Connects To | Notes |
|---------------|----------------------|-------------|-------|
| **VIN** | 🔴 Red | **3.3V rail** (or 5V) | Power supply (2.5V–5.5V) |
| **GND** | ⚫ Black | **GND rail** | Common ground |
| **BCLK** | 🟠 Orange | **ESP32 GPIO 12** | I2S bit clock (shared with mic SCK) |
| **LRC** | 🟢 Green | **ESP32 GPIO 4** | I2S word select (shared with mic WS) |
| **DIN** | ⚪ White | **ESP32 GPIO 2** | I2S data in (audio data) |
| **GAIN** | — | **Not connected** | Default 9dB gain (or tie to GND for 12dB) |
| **SD** | — | **Not connected** | Shutdown pin (internally pulled high) |

#### Speaker Connection to MAX98357A

| MAX98357A Output | Connects To |
|-----------------|-------------|
| **Speaker + (positive)** | Speaker **+** terminal (red wire) |
| **Speaker − (negative)** | Speaker **−** terminal (black wire) |

```
ESP32-CAM            MAX98357A            Speaker
┌──────────┐         ┌──────────┐         ┌────────┐
│ 3.3V     ├── 🔴 ──┤ VIN      │         │        │
│ GND      ├── ⚫ ──┤ GND      │         │   8Ω   │
│ GPIO 12  ├── 🟠 ──┤ BCLK     │  Spk+ ──┤  0.5W  │
│ GPIO 4   ├── 🟢 ──┤ LRC      │  Spk- ──┤        │
│ GPIO 2   ├── ⚪ ──┤ DIN      │         │        │
│          │         │ Spk+ ────┼── 🔴 ──┤ +      │
│          │         │ Spk- ────┼── ⚫ ──┤ -      │
└──────────┘         └──────────┘         └────────┘
```

> **⚠️ I2S Bus Sharing:** The BCLK (GPIO 12) and LRC/WS (GPIO 4) lines are shared between the INMP441 microphone and the MAX98357A amplifier. This is valid because the ESP32 I2S peripheral can be configured to use the same clock lines. **However, the mic and speaker should NOT operate simultaneously** — the Smart Router ensures only one is active at a time.

---

### 4. Push Buttons (x2)

Two momentary tactile push buttons are used for user interaction:
- **Button 1 (Mode Select):** Cycles through modes (Describe / OCR / Speech / Q&A / Translate)
- **Button 2 (Action Trigger):** Captures image or starts audio recording

| Button | Wire Color (Suggested) | One Leg Connects To | Other Leg Connects To |
|--------|----------------------|--------------------|-----------------------|
| **Button 1** (Mode) | 🟤 Brown | **ESP32 GPIO 0** | **GND** |
| **Button 2** (Action) | 🟤 Brown | **ESP32 GPIO 16** | **GND** |

#### Pull-up Resistor Configuration

Each button requires a **10kΩ pull-up resistor** to keep the pin HIGH when the button is not pressed:

```
3.3V ────[10kΩ]────┬──── GPIO 0 (Button 1)
                    │
                  [BTN1]
                    │
                   GND

3.3V ────[10kΩ]────┬──── GPIO 16 (Button 2)
                    │
                  [BTN2]
                    │
                   GND
```

**How it works:**
- **Button NOT pressed:** GPIO reads HIGH (3.3V through 10kΩ pull-up)
- **Button pressed:** GPIO reads LOW (directly connected to GND)
- In firmware, configure GPIO as `INPUT_PULLUP` and detect `LOW` as a press

> **Note:** GPIO 0 is also the BOOT pin. If Button 1 is held during power-on, the ESP32 will enter flashing mode. This is actually useful for firmware updates!

---

### 5. Power System (LiPo + TP4056 + Voltage Regulator)

The power system converts the 3.7V LiPo battery to a stable 3.3V supply for all components.

#### Power Chain

```
                    ┌─────────────────┐
  USB Micro ──────► │    TP4056       │
  (5V Charging)     │  Charger Module │
                    │  (with DW01     │
                    │   protection)   │
                    └──┬──────────┬───┘
                       │B+      B-│
                       │          │
                  ┌────┴───┐      │
                  │  LiPo  │      │
                  │  3.7V  │      │
                  │1000mAh │      │
                  └────┬───┘      │
                       │          │
                    ┌──┴──┐       │
                    │SWITCH│      │ (optional power switch)
                    └──┬──┘       │
                       │          │
                  ┌────┴──────────┴───┐
                  │  AMS1117-3.3V     │
                  │  Voltage Regulator│
                  │                   │
                  │  IN    OUT    GND │
                  └──┬─────┬──────┬──┘
                     │     │      │
              3.7V from    │      │
              battery  3.3V out  GND
                       │      │
                    ┌──┴──────┴──┐
                    │  3.3V Rail  │ ──► All components
                    │  GND Rail   │ ──► All components
                    └─────────────┘
```

#### TP4056 Connections

| TP4056 Pin | Connects To | Notes |
|-----------|-------------|-------|
| **USB Micro** | USB cable (for charging) | 5V input |
| **B+** | LiPo battery **positive (+)** | Battery positive terminal |
| **B−** | LiPo battery **negative (−)** | Battery negative terminal |
| **OUT+** | AMS1117 **VIN** | Regulated battery output |
| **OUT−** | **GND rail** | Common ground |

#### AMS1117-3.3V Connections

| AMS1117 Pin | Connects To | Notes |
|------------|-------------|-------|
| **VIN (Input)** | TP4056 **OUT+** | 3.7V–4.2V from battery |
| **VOUT (Output)** | **3.3V power rail** | Stable 3.3V for all components |
| **GND** | **GND rail** | Common ground |

> **⚠️ Important:** Place a **10µF electrolytic capacitor** across the AMS1117 output (between VOUT and GND) for stability, and a **100nF ceramic capacitor** close to the ESP32-CAM VCC pin for noise filtering.

---

## 📋 Complete Wiring Summary Table

This is the master reference for **every wire** in the project:

| # | From (Source) | Pin / Terminal | To (Destination) | Pin / Terminal | Wire Color | Protocol |
|---|--------------|----------------|-------------------|----------------|------------|----------|
| **— OLED Display (I2C) —** | | | | | | |
| 1 | OLED | VCC | 3.3V Rail | — | 🔴 Red | Power |
| 2 | OLED | GND | GND Rail | — | ⚫ Black | Power |
| 3 | OLED | SDA | ESP32-CAM | GPIO 13 | 🔵 Blue | I2C Data |
| 4 | OLED | SCL | ESP32-CAM | GPIO 14 | 🟡 Yellow | I2C Clock |
| 5 | 3.3V Rail | — | Resistor 4.7kΩ | → GPIO 13 | — | I2C Pull-up |
| 6 | 3.3V Rail | — | Resistor 4.7kΩ | → GPIO 14 | — | I2C Pull-up |
| **— INMP441 Microphone (I2S) —** | | | | | | |
| 7 | INMP441 | VDD | 3.3V Rail | — | 🔴 Red | Power |
| 8 | INMP441 | GND | GND Rail | — | ⚫ Black | Power |
| 9 | INMP441 | SCK | ESP32-CAM | GPIO 12 | 🟠 Orange | I2S Clock |
| 10 | INMP441 | WS | ESP32-CAM | GPIO 4 | 🟢 Green | I2S Word Select |
| 11 | INMP441 | SD | ESP32-CAM | GPIO 15 | 🟣 Purple | I2S Data |
| 12 | INMP441 | L/R | GND Rail | — | ⚫ Black | Channel Select |
| **— MAX98357A Amplifier + Speaker (I2S) —** | | | | | | |
| 13 | MAX98357A | VIN | 3.3V Rail | — | 🔴 Red | Power |
| 14 | MAX98357A | GND | GND Rail | — | ⚫ Black | Power |
| 15 | MAX98357A | BCLK | ESP32-CAM | GPIO 12 | 🟠 Orange | I2S Clock |
| 16 | MAX98357A | LRC | ESP32-CAM | GPIO 4 | 🟢 Green | I2S Word Select |
| 17 | MAX98357A | DIN | ESP32-CAM | GPIO 2 | ⚪ White | I2S Data |
| 18 | MAX98357A | Spk+ | Speaker | + Terminal | 🔴 Red | Analog Audio |
| 19 | MAX98357A | Spk− | Speaker | − Terminal | ⚫ Black | Analog Audio |
| **— Push Buttons —** | | | | | | |
| 20 | Button 1 | Leg A | ESP32-CAM | GPIO 0 | 🟤 Brown | Digital Input |
| 21 | Button 1 | Leg B | GND Rail | — | ⚫ Black | Ground |
| 22 | 3.3V Rail | — | Resistor 10kΩ | → GPIO 0 | — | Pull-up |
| 23 | Button 2 | Leg A | ESP32-CAM | GPIO 16 | 🟤 Brown | Digital Input |
| 24 | Button 2 | Leg B | GND Rail | — | ⚫ Black | Ground |
| 25 | 3.3V Rail | — | Resistor 10kΩ | → GPIO 16 | — | Pull-up |
| **— Power System —** | | | | | | |
| 26 | LiPo Battery | + | TP4056 | B+ | 🔴 Red | Power |
| 27 | LiPo Battery | − | TP4056 | B− | ⚫ Black | Power |
| 28 | TP4056 | OUT+ | Power Switch | IN | 🔴 Red | Power |
| 29 | Power Switch | OUT | AMS1117 | VIN | 🔴 Red | Power |
| 30 | AMS1117 | VOUT | 3.3V Rail | — | 🔴 Red | Power (3.3V) |
| 31 | AMS1117 | GND | GND Rail | — | ⚫ Black | Power |
| 32 | ESP32-CAM | 3V3 | 3.3V Rail | — | 🔴 Red | Power |
| 33 | ESP32-CAM | GND | GND Rail | — | ⚫ Black | Power |

**Total wire connections: 33**

---

## 🔋 Decoupling Capacitors

Decoupling capacitors are essential for stable operation. Place them as close as possible to the component's power pins.

| Capacitor | Value | Location | Purpose |
|-----------|-------|----------|---------|
| **C1** | 100nF (ceramic) | Across ESP32-CAM VCC & GND | High-frequency noise filtering |
| **C2** | 100nF (ceramic) | Across OLED VCC & GND | Display power stability |
| **C3** | 10µF (electrolytic) | Across AMS1117 VOUT & GND | Bulk power decoupling / LDO stability |

```
          3.3V Rail
             │
        ┌────┴────┐
        │  100nF  │ (C1 - near ESP32)
        └────┬────┘
             │
            GND

          3.3V Rail
             │
        ┌────┴────┐
        │  100nF  │ (C2 - near OLED)
        └────┬────┘
             │
            GND

       AMS1117 VOUT
             │
        ┌────┴────┐
        │  10µF   │ (C3 - at regulator output)
        └────┬────┘
             │
            GND
```

---

## ⚠️ Circuit Notes & Warnings

### Critical Warnings

1. **Never supply 5V to the OLED or INMP441** — they are 3.3V devices. Applying 5V will permanently damage them.

2. **GPIO 0 (Boot Pin):** If Button 1 is held LOW during power-up, the ESP32 enters **flash/download mode** and will not run your code. Release the button before powering on during normal use.

3. **GPIO 4 (Flash LED):** This pin also controls the on-board white flash LED. When used for I2S WS, the flash LED may flicker briefly during audio operations — this is normal and harmless.

4. **Camera Pin Conflict:** The ESP32-CAM uses many GPIOs internally for the camera (GPIO 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39). **Do NOT use these pins** for any peripheral.

5. **I2S Bus Sharing:** GPIO 12 (SCK/BCLK) and GPIO 4 (WS/LRC) are shared between the microphone and speaker. The firmware must ensure **only one I2S device is active at a time**.

6. **GPIO 12 Boot Voltage:** On some ESP32 variants, GPIO 12 controls the flash voltage at boot. If you experience boot failures, try adding a **pull-down resistor (10kΩ)** on GPIO 12, or use `efuse_wr 0x0` to set flash voltage to 3.3V.

### Assembly Tips

- Use **short jumper wires** (< 10cm) for I2C and I2S connections to minimize signal interference.
- **Twist** the SDA/SCL wires together to reduce electromagnetic interference.
- Solder connections for the final prototype — breadboard jumper wires can cause intermittent failures on a wearable.
- Route the **speaker wires away** from the I2C/I2S signal wires to prevent audio noise.
- Mount the **microphone** facing outward on the glasses frame, away from the speaker to avoid feedback loops.

---

## ⚡ Power Consumption Estimates

| Component | Active Current | Standby Current |
|-----------|---------------|----------------|
| ESP32-CAM (Wi-Fi active) | ~160 mA | ~10 mA (light sleep) |
| ESP32-CAM (camera capture) | ~200 mA (momentary) | — |
| OLED SSD1306 | ~20 mA | ~0.01 mA (display off) |
| INMP441 Microphone | ~1.4 mA | ~0.01 mA |
| MAX98357A + Speaker | ~10–30 mA | ~0.01 mA (shutdown) |
| AMS1117 Regulator | ~5 mA (quiescent) | ~5 mA |
| **Total (active)** | **~250–400 mA** | **~15 mA** |

**Estimated battery life with 1000mAh LiPo:**
- Active use: ~2.5 – 4 hours
- With 2000mAh: ~5 – 8 hours

---

## 💻 Software Stack

### On the ESP32-CAM (Arduino / C++)
- Arduino IDE 2.x
- ESP32 board support package
- Libraries: `Wire.h` (I2C), `driver/i2s.h` (I2S), `Adafruit_SSD1306`, `WiFi.h`, `HTTPClient.h`

### On the Local PC (Python)
- Python 3.10+
- Flask 3.0+ (backend server with Smart Router)
- Groq Python SDK v1.2 (cloud API access)
- EasyOCR 1.7+ (local OCR)
- OpenAI Whisper tiny (local speech-to-text)
- PyTorch 2.0+ (deep learning backend)
- psutil (system monitoring)
- python-dotenv (API key management)

---

## 🔄 How It Works

```
1. User presses BUTTON 1 → Cycles mode (Describe / OCR / Speech / Q&A / Translate)
2. User presses BUTTON 2 → Triggers capture
3. ESP32-CAM captures image (camera) or audio (microphone)
4. Data is Base64 encoded and sent via Wi-Fi HTTP POST to Flask server
5. Flask Smart Router identifies request type and routes to correct AI model:
     ├── Scene Description  → Llama 4 Scout (Groq Cloud)
     ├── OCR Text Reading   → EasyOCR (Local)
     ├── Speech-to-Text     → Whisper tiny (Local)
     ├── Q&A                → Llama 3.3 70B (Groq Cloud)
     └── Translation        → Llama 3.3 70B (Groq Cloud)
6. AI response is sent back as JSON to ESP32
7. Response text displayed on OLED screen
8. (Optional) Response spoken aloud through speaker via TTS
```

---

## 🚀 Getting Started

### 1. Assemble the Hardware
Follow the [Wiring Connections](#-wiring-connections--component-by-component) section above to connect all components.

### 2. Set Up the Backend Server
```bash
cd test_multi_model_project
pip install -r requirements.txt
cp .env.example .env
# Edit .env and add your Groq API key
python main.py
```

### 3. Flash the ESP32-CAM Firmware
1. Open Arduino IDE
2. Select **AI-Thinker ESP32-CAM** board
3. Connect ESP32-CAM via USB-to-TTL adapter (GPIO 0 → GND for flash mode)
4. Upload the firmware
5. Disconnect GPIO 0 from GND and reset

### 4. Connect & Test
1. Power on the glasses
2. ESP32 connects to Wi-Fi automatically
3. Press Button 1 to select a mode
4. Press Button 2 to capture and process
5. View response on OLED / hear through speaker

---

## 👥 Project Team

**AI Smart Glasses using ESP32-CAM**

| Name | USN |
|------|-----|
| Arya B Shetty | 4AL23IC007 |
| Harinand J | 4AL23IC012 |
| Karthik P | 4AL23IC016 |
| Pavan SN | 4AL23IC033 |

**Department of Computer Science (IoT Cybersecurity Including Blockchain)**  
**Alva's Institute of Engineering & Technology, Moodbidri**

**Guide:** Prof. Jyothibha R Chinchankar

---

> **Document Version:** 1.0  
> **Last Updated:** September 2, 2026
