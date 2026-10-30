# 🔧 AI Smart Glasses — Step-by-Step Wiring Guide

This guide walks you through connecting every component **in the correct order**, with testing at each step so you can catch problems early.

> **Tip:** Use a breadboard for prototyping first. Only solder the final version after everything works.

---

## 📦 What You Need on Your Table

Before starting, gather all these components:

| # | Component | Check |
|---|-----------|-------|
| 1 | ESP32-CAM module (AI-Thinker, with OV2640 camera) | ☐ |
| 2 | FTDI / CP2102 USB-to-Serial adapter (for programming only) | ☐ |
| 3 | OLED display SSD1306 (0.96", 4-pin I2C version) | ☐ |
| 4 | MAX98357A I2S amplifier breakout | ☐ |
| 5 | Micro speaker (4Ω or 8Ω, 0.5W) | ☐ |
| 6 | **1×** Push button (6×6mm tactile) | ☐ |
| 7 | 2× 4.7kΩ resistors (yellow-violet-red) — for OLED I2C pull-up | ☐ |
| 8 | LiPo battery 3.7V + TP4056 charger module | ☐ |
| 9 | Jumper wires (male-to-female + male-to-male) | ☐ |
| 10 | USB micro cable (for FTDI adapter) | ☐ |

> **Note:** The current firmware (`smart_glasses_master`) uses a **single smart button** (short press = capture, long press = change mode). The INMP441 microphone is **not used** in the current firmware.

---

## ⚡ Quick Reference Wiring Tables

### 1. 🖥️ OLED Display (SSD1306 0.96" I2C) ➔ ESP32-CAM

| OLED Pin | ESP32-CAM Pin | Wire Color (Rec.) | Notes |
|:---|:---|:---:|:---|
| **VCC** | **3.3V / 3V3** | 🔴 Red | ⚠️ **Must use 3.3V**, do not connect to 5V! |
| **GND** | **GND** | ⚫ Black | Common Ground |
| **SDA** | **GPIO 13** | 🔵 Blue | I2C Data (requires 4.7kΩ pull-up to 3.3V) |
| **SCL / SCK** | **GPIO 14** | 🟡 Yellow | I2C Clock (requires 4.7kΩ pull-up to 3.3V) |

---

### 2. 🔊 Speaker System (MAX98357A Amplifier) ➔ ESP32-CAM

| MAX98357A Pin | ESP32-CAM Pin | Wire Color (Rec.) | Notes |
|:---|:---|:---:|:---|
| **VIN** | **5V** | 🔴 Red | Connect to 5V (gives clear & louder volume) |
| **GND** | **GND** | ⚫ Black | Common Ground |
| **LRC (WS)** | **GPIO 2** | 🟢 Green | I2S Word Select / Frame Clock |
| **BCLK** | **GPIO 4** | 🟠 Orange | I2S Bit Clock |
| **DIN** | **GPIO 12** | 🟣 Purple | I2S Serial Audio Data |
| **SD** | *Leave Empty* | — | Unconnected (Always-on active mode) |
| **GAIN** | *Leave Empty* | — | Unconnected (Default +9dB gain) |

#### 🔈 Speaker ➔ MAX98357A Board:
| Speaker Wire | MAX98357A Terminal |
|:---|:---|
| **Red (+)** | **Speaker +** terminal |
| **Black (−)** | **Speaker −** terminal |

---

### 3. 🔘 Smart Push Button ➔ ESP32-CAM

| Button Leg | ESP32-CAM Pin | Notes |
|:---|:---|:---|
| **Leg A** | **GPIO 15** | Signal pin |
| **Leg B** | **GND** | Ground (Firmware uses internal pull-up, **no resistor needed**) |

---

### 4. 🔋 Battery & Power System (3.7V LiPo + TP4056) ➔ ESP32-CAM

| From (Battery / Module) | To (Destination) | Wire Color (Rec.) | Notes |
|:---|:---|:---:|:---|
| **LiPo Battery (+ Red)** | **TP4056 B+** | 🔴 Red | Battery positive input |
| **LiPo Battery (− Black)** | **TP4056 B−** | ⚫ Black | Battery negative input |
| **TP4056 OUT+** | **ESP32-CAM 5V pin** | 🔴 Red | Powers onboard regulator & amp |
| **TP4056 OUT−** | **ESP32-CAM GND pin** | ⚫ Black | Common system ground |

*(Optional: Place an ON/OFF slide switch on the wire between **TP4056 OUT+** and the **ESP32-CAM 5V** pin).*

---

### 📌 Master ESP32-CAM Pin Map Summary

| ESP32-CAM Pin | Connected To | Function |
|:---|:---|:---|
| **5V** | TP4056 OUT+ & MAX98357A VIN | Power input / Amp power |
| **3V3** | OLED VCC (and 4.7kΩ pull-up resistors) | 3.3V Logic rail |
| **GND** | TP4056 OUT−, OLED GND, MAX98357A GND, Button Leg B | Common Ground |
| **GPIO 2** | MAX98357A **LRC** | Speaker Word Select |
| **GPIO 4** | MAX98357A **BCLK** | Speaker Bit Clock |
| **GPIO 12** | MAX98357A **DIN** | Speaker Data |
| **GPIO 13** | OLED **SDA** | I2C Data |
| **GPIO 14** | OLED **SCL** | I2C Clock |
| **GPIO 15** | Button Leg A | Single Smart Button |

---

## 🔌 Step 1: Power Rails on Breadboard

Set up the power rails first. Everything else connects to these.

```
Breadboard Layout:
┌─────────────────────────────────────────────────────────┐
│  + + + + + + + + + + + + + + + + + +   ← 3.3V Rail (Red)│
│  - - - - - - - - - - - - - - - - - -   ← GND Rail (Blue)│
│                                                          │
│  (Component area - rows a through j)                     │
│                                                          │
│  + + + + + + + + + + + + + + + + + +   ← 3.3V Rail      │
│  - - - - - - - - - - - - - - - - - -   ← GND Rail       │
└─────────────────────────────────────────────────────────┘
```

**For now**, the FTDI adapter will provide power during programming/testing. Later we'll switch to the battery.

---

## 🔌 Step 2: ESP32-CAM + FTDI Adapter (Programming Setup)

This is the first thing to get working — you need to be able to upload code.

### Connections

Use **female-to-female** jumper wires (ESP32-CAM has header pins):

| Wire # | From (FTDI) | To (ESP32-CAM) | Wire Color | Notes |
|--------|-------------|----------------|------------|-------|
| 1 | **5V** | **5V** | 🔴 Red | Power from USB |
| 2 | **GND** | **GND** | ⚫ Black | Common ground |
| 3 | **TX** | **U0R** (GPIO 3) | 🟡 Yellow | FTDI transmit → ESP receive |
| 4 | **RX** | **U0T** (GPIO 1) | 🟢 Green | FTDI receive → ESP transmit |
| 5 | — | **GPIO 0 → GND** | ⚫ Black | Short with jumper wire for flash mode |

```
                FTDI                    ESP32-CAM
           ┌──────────┐           ┌──────────────┐
   USB ◄───┤          │           │   [Camera]   │
           │  5V    ──├── 🔴 ────►├── 5V         │
           │  GND   ──├── ⚫ ────►├── GND        │
           │  TX    ──├── 🟡 ────►├── U0R        │
           │  RX    ──├── 🟢 ────►├── U0T        │
           │          │           │              │
           └──────────┘           │  GPIO 0 ──┐  │
                                  │  GND    ──┘  │ ← Jumper wire
                                  └──────────────┘
```

### ✅ Test Step 2

1. Plug FTDI into PC via USB
2. Open Arduino IDE → Select **AI Thinker ESP32-CAM** board
3. Select the correct COM port
4. Upload the blink test sketch from the firmware README
5. **When "Connecting..." appears → press the RESET button on ESP32-CAM**
6. If upload succeeds → the flash LED should blink every 1 second ✅
7. **Disconnect GPIO 0 from GND** after upload, then press RESET

> **If it fails:** TX and RX are probably swapped. Switch the yellow and green wires.

---

## 🔌 Step 3: OLED Display (I2C)

Now add the display so you can see output.

### What You're Connecting

The OLED uses **I2C protocol** which needs only 2 data wires (SDA + SCL) plus power.

### Connections

| Wire # | From (OLED) | To | Wire Color | Notes |
|--------|-------------|-----|------------|-------|
| 1 | **VCC** | **3.3V** rail on breadboard | 🔴 Red | ⚠️ NOT 5V! |
| 2 | **GND** | **GND** rail on breadboard | ⚫ Black | |
| 3 | **SDA** | **ESP32-CAM GPIO 13** | 🔵 Blue | I2C data |
| 4 | **SCL** | **ESP32-CAM GPIO 14** | 🟡 Yellow | I2C clock |

### Pull-up Resistors (Required!)

I2C needs pull-up resistors on both data lines. Without these, the display will not work.

**How to install them on a breadboard:**

```
Step A: Take a 4.7kΩ resistor
        → One leg into the 3.3V rail (+)
        → Other leg into the same row as GPIO 13 (SDA wire)

Step B: Take another 4.7kΩ resistor
        → One leg into the 3.3V rail (+)
        → Other leg into the same row as GPIO 14 (SCL wire)
```

Visual on breadboard:

```
3.3V Rail:  ─────────┬───────────┬─────────
                      │           │
                  [4.7kΩ]     [4.7kΩ]
                      │           │
Breadboard row:  ─────┼───────────┼─────────
                      │           │
                 GPIO 13       GPIO 14
                 (SDA)         (SCL)
                      │           │
                 to OLED SDA  to OLED SCL
```

### Also Connect ESP32-CAM Power to Breadboard Rails

Run wires from the ESP32-CAM to the breadboard power rails so all components share the same power:

| Wire # | From (ESP32-CAM) | To (Breadboard) | Wire Color |
|--------|------------------|-----------------|------------|
| 5 | **3V3** pin | **3.3V rail (+)** | 🔴 Red |
| 6 | **GND** pin | **GND rail (-)** | ⚫ Black |

### Decoupling Capacitor

Place a **100nF ceramic capacitor** (marked "104") across the OLED power:
- One leg into the same row as OLED VCC
- Other leg into the GND rail

### ✅ Test Step 3

Upload this test sketch:

```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PIN_SDA 13
#define PIN_SCL 14

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAILED! Check wiring.");
    while (true);  // Stop here
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("HELLO!");
  display.setTextSize(1);
  display.println();
  display.println("OLED is working!");
  display.display();
  
  Serial.println("OLED OK!");
}

void loop() {}
```

**Expected result:** OLED shows "HELLO!" and "OLED is working!" ✅

> **If blank screen:** Try changing `0x3C` to `0x3D`. If still blank, check the 4.7kΩ pull-up resistors are properly connected.

---

## 🔌 Step 4: Single Smart Button (Multifunction)

The current master firmware (`smart_glasses_master.ino`) uses a **single smart button** with internal pull-up enabled (`pinMode(PIN_BUTTON, INPUT_PULLUP)`). **No external resistors are needed!**

### What You're Connecting

A single push button connects between **GPIO 15** and **GND**.

- **Short press** (< 1.2 sec) = Take photo & query AI
- **Long press** (≥ 1.2 sec) = Switch mode (`VISION` → `OCR` → `TRANSLATE`)

### Connections

| Wire # | From | To | Wire Color | Notes |
|--------|------|----|------------|-------|
| 1 | Button Leg A | **ESP32-CAM GPIO 15** | 🟤 Brown | Signal line |
| 2 | Button Leg B | **GND rail** | ⚫ Black | Ground return |

```
ESP32-CAM                      Button
┌──────────┐               ┌────────────┐
│          │               │  Leg A ─┐  │
│ GPIO 15  ├── 🟤 Brown ──►│         │  │ (Press connects Leg A to B)
│          │               │  Leg B ─┼──┼── ⚫ Black ──► GND Rail
└──────────┘               └────────────┘
```

> **Why no resistor?** The ESP32 has internal pull-up resistors inside the chip that keep GPIO 15 at HIGH (3.3V) when released. When you press the button, it shorts to GND (LOW).

### ✅ Test Step 4

Upload this test sketch to verify your button wiring:

```cpp
#define PIN_BUTTON 15

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  Serial.println("Smart Button test ready!");
  Serial.println("Try short tap (<1.2s) and long press (>=1.2s)...");
}

void loop() {
  if (digitalRead(PIN_BUTTON) == LOW) {
    delay(50); // Debounce
    if (digitalRead(PIN_BUTTON) == LOW) {
      unsigned long pressStart = millis();
      bool longPressTriggered = false;

      while (digitalRead(PIN_BUTTON) == LOW) {
        if (!longPressTriggered && (millis() - pressStart >= 1200)) {
          Serial.println(">> LONG PRESS DETECTED! (Switch Mode)");
          longPressTriggered = true;
        }
        delay(10);
      }

      unsigned long duration = millis() - pressStart;
      if (duration < 1200) {
        Serial.printf(">> SHORT TAP DETECTED! Duration: %lu ms (Capture Photo)\n", duration);
      }
      delay(300); // Cooldown
    }
  }
}
```

**Expected result in Serial Monitor:**
- Short tap prints `>> SHORT TAP DETECTED!` ✅
- Holding > 1.2 sec prints `>> LONG PRESS DETECTED!` ✅

---

## 🔌 Step 5: MAX98357A I2S Amplifier + Speaker

The MAX98357A takes digital audio from the ESP32 via I2S and drives the micro speaker. It is used for audio feedback (boot chime, shutter click, mode beep, error sound).

> **Note:** The microphone (INMP441) is **not used** in the current firmware. The audio system is purely for speaker output.

### MAX98357A Pinout Reference

```
  MAX98357A Breakout Board:
  ┌───────────────────────────────────┐
  │  VIN   GND   SD   GAIN   DIN      │
  │                          BCLK LRC │
  │                                   │
  │     [Speaker +]     [Speaker -]   │
  └───────────────────────────────────┘
```

### Connections

| Wire # | From (MAX98357A) | To (ESP32-CAM) | Wire Color | Notes |
|--------|------------------|----------------|------------|-------|
| 1 | **VIN** | **5V** (or 3.3V) | 🔴 Red | 5V gives full volume |
| 2 | **GND** | **GND rail** | ⚫ Black | Common ground |
| 3 | **LRC** (WS) | **ESP32-CAM GPIO 2** | 🟢 Green | Word select (L/R clock) |
| 4 | **BCLK** | **ESP32-CAM GPIO 4** | 🟠 Orange | Bit clock |
| 5 | **DIN** | **ESP32-CAM GPIO 12** | 🟣 Purple | Audio data into amp |
| 6 | **SD** | *Unconnected* | — | Leave floating (always-on mode) |
| 7 | **GAIN** | *Unconnected* | — | Leave floating (+9dB default) |

### Speaker Wires

| Wire # | From (MAX98357A) | To (Speaker) | Notes |
|--------|------------------|--------------|-------|
| 8 | **Speaker +** terminal | Speaker **red wire (+)** | Solder or screw terminal |
| 9 | **Speaker −** terminal | Speaker **black wire (−)** | Solder or screw terminal |

```
ESP32-CAM               MAX98357A                 Speaker
┌──────────────┐       ┌────────────┐           ┌─────────┐
│ 5V         ──├─ 🔴 ──┤ VIN        │           │         │
│ GND        ──├─ ⚫ ──┤ GND        │   Spk + ──┤ + (Red) │
│ GPIO 2     ──├─ 🟢 ──┤ LRC        │   Spk - ──┤ - (Blk) │
│ GPIO 4     ──├─ 🟠 ──┤ BCLK       │           │  [🔈]   │
│ GPIO 12    ──├─ 🟣 ──┤ DIN        │           └─────────┘
└──────────────┘       └────────────┘
```

### ✅ Test Step 5

Upload this short test sketch to hear a test beep through the speaker:

```cpp
#include "driver/i2s.h"

#define I2S_LRC_PIN   2
#define I2S_BCLK_PIN  4
#define I2S_DOUT_PIN 12

void playTone(int freqHz, int durationMs) {
  int samples = (16000 * durationMs) / 1000;
  int period = 16000 / freqHz;
  int16_t sampleBuf[64];
  size_t bytesWritten;

  for (int i = 0; i < samples; ) {
    int chunk = min(32, samples - i);
    for (int j = 0; j < chunk; j++) {
      int16_t val = ((i + j) % period < period / 2) ? 6000 : -6000;
      sampleBuf[j * 2]     = val; // Left channel
      sampleBuf[j * 2 + 1] = val; // Right channel
    }
    i2s_write(I2S_NUM_0, sampleBuf, chunk * 4, &bytesWritten, portMAX_DELAY);
    i2s_write(I2S_NUM_0, sampleBuf, chunk * 4, &bytesWritten, portMAX_DELAY);
    i += chunk;
  }
}

void setup() {
  Serial.begin(115200);

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK_PIN,
    .ws_io_num = I2S_LRC_PIN,
    .data_out_num = I2S_DOUT_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);

  Serial.println("Playing test tone on speaker...");
  playTone(880, 200);  // A5 tone
  delay(100);
  playTone(1760, 300); // A6 tone
  Serial.println("Speaker test complete!");
}

void loop() {}
```

**Expected result:** You will hear two crisp test tones from the speaker ✅

> **If no sound:** Check that VIN is powered (5V or 3.3V) and verify that GPIO 2 (LRC), GPIO 4 (BCLK), and GPIO 12 (DIN) are firmly connected.

---

## 🔌 Step 6: Power System (Battery — Final Step)

> **Do this step LAST**, after all other components are tested using the FTDI adapter for power.

### Connection Order

**Part A: TP4056 Charging Module**

| Wire # | From | To | Notes |
|--------|------|----|-------|
| 1 | LiPo Battery **+ (red)** | TP4056 **B+** | Battery positive |
| 2 | LiPo Battery **− (black)** | TP4056 **B−** | Battery negative |

**Part B: Voltage Regulator**

| Wire # | From | To | Notes |
|--------|------|----|-------|
| 3 | TP4056 **OUT+** | AMS1117 **VIN** (input) | Battery output → regulator input |
| 4 | TP4056 **OUT−** | AMS1117 **GND** | Shared ground |
| 5 | AMS1117 **VOUT** (output) | Breadboard **3.3V rail** | Regulated 3.3V to all components |
| 6 | AMS1117 **GND** | Breadboard **GND rail** | Shared ground |

**Part C: ESP32-CAM Power (switch from FTDI to battery)**

| Wire # | From | To | Notes |
|--------|------|----|-------|
| 7 | Breadboard **3.3V rail** | ESP32-CAM **3V3** pin | Power from battery |
| 8 | Breadboard **GND rail** | ESP32-CAM **GND** pin | Shared ground |

**Part D: Bulk Decoupling Capacitor**

Place a **10µF electrolytic capacitor** across the AMS1117 output:
- **Longer leg (+)** → 3.3V rail
- **Shorter leg (−)** → GND rail

> ⚠️ Electrolytic capacitors are **polarized**! The longer leg is positive. Reversing it can cause it to explode.

```
LiPo Battery        TP4056           AMS1117-3.3V        Breadboard
┌────────┐      ┌──────────┐       ┌────────────┐       ┌─────────┐
│ + (red)├─ 🔴 ─┤ B+       │       │            │       │         │
│ -(black├─ ⚫ ─┤ B-       │       │            │       │ 3.3V + ─┤→ All VCC
│        │      │      OUT+├─ 🔴 ──┤ VIN    VOUT├─ 🔴 ──┤         │
│        │      │      OUT-├─ ⚫ ──┤ GND     GND├─ ⚫ ──┤ GND  - ─┤→ All GND
└────────┘      └──────────┘       └────────────┘       └─────────┘
                                         │    │
                                       [10µF cap]
                                       [100nF cap]
```

### Optional: Power Switch

Add a slide switch between TP4056 OUT+ and AMS1117 VIN to turn the glasses on/off without unplugging the battery.

### ✅ Test Step 6

1. **Disconnect the FTDI adapter** completely
2. Charge the battery via TP4056 USB (red LED = charging, blue LED = full)
3. Check voltage at the 3.3V rail with a multimeter — should read **3.3V ± 0.1V**
4. The ESP32-CAM should boot up automatically
5. Check the OLED displays the mode selection screen

---

## 📋 Complete Connection Checklist — Current Master Firmware

After all steps, verify every connection against this master list.
This matches **`smart_glasses_master.ino`** exactly.

---

### OLED SSD1306 Display — 4 wires + 2 pull-up resistors

| Wire | From (OLED) | To (ESP32-CAM) |
|------|-------------|----------------|
| 1 | VCC | 3.3V |
| 2 | GND | GND |
| 3 | SDA | **GPIO 13** |
| 4 | SCL | **GPIO 14** |

- [ ] OLED VCC → 3.3V
- [ ] OLED GND → GND
- [ ] OLED SDA → **GPIO 13**
- [ ] OLED SCL → **GPIO 14**
- [ ] 4.7kΩ resistor between 3.3V and GPIO 13 (SDA pull-up)
- [ ] 4.7kΩ resistor between 3.3V and GPIO 14 (SCL pull-up)

---

### Smart Button (Single) — 2 wires, no resistor needed

| Wire | From (Button) | To (ESP32-CAM) |
|------|---------------|----------------|
| 1 | Leg A | **GPIO 15** |
| 2 | Leg B | **GND** |

> Internal pull-up is enabled in firmware (`INPUT_PULLUP`) — **no external resistor needed**.

- [ ] Button Leg A → **GPIO 15**
- [ ] Button Leg B → **GND**

**Button behaviour:**
- Short press (< 1.2s) = Take photo / run AI
- Long press (≥ 1.2s) = Cycle mode (VISION → OCR → TRANSLATE)

---

### MAX98357A I2S Amplifier + Speaker — 5 wires + 2 speaker wires

| Wire | From (MAX98357A) | To (ESP32-CAM) | Notes |
|------|-----------------|----------------|-------|
| 1 | VIN | **5V** | Use 5V for full volume |
| 2 | GND | **GND** | |
| 3 | LRC (WS) | **GPIO 2** | Word select |
| 4 | BCLK | **GPIO 4** | Bit clock |
| 5 | DIN | **GPIO 12** | Audio data |
| 6 | SD | *Unconnected* | Always-on mode |
| 7 | Speaker + terminal | Speaker **+** wire | |
| 8 | Speaker − terminal | Speaker **−** wire | |

- [ ] MAX98357A VIN → **5V**
- [ ] MAX98357A GND → **GND**
- [ ] MAX98357A LRC → **GPIO 2**
- [ ] MAX98357A BCLK → **GPIO 4**
- [ ] MAX98357A DIN → **GPIO 12**
- [ ] MAX98357A SD → *leave unconnected*
- [ ] Speaker + → MAX98357A Speaker +
- [ ] Speaker − → MAX98357A Speaker −

**Sound events:**
- Boot ready → 3-note ascending chime
- Photo taken → short click beep
- Mode changed → two-tone blip
- AI result received → rising beep
- Error → descending double beep

---

### Power — Battery to ESP32-CAM

| Wire | From | To |
|------|------|----|
| 1 | Battery + (via TP4056 OUT+) | ESP32-CAM **5V** pin |
| 2 | Battery − (via TP4056 OUT−) | ESP32-CAM **GND** pin |

- [ ] TP4056 B+ → LiPo battery +
- [ ] TP4056 B− → LiPo battery −
- [ ] TP4056 OUT+ → ESP32-CAM 5V
- [ ] TP4056 OUT− → ESP32-CAM GND

---

### All GPIO Used — Quick Reference

```
ESP32-CAM
├── GPIO  2  →  Speaker LRC (Word Select)
├── GPIO  4  →  Speaker BCLK (Bit Clock)
├── GPIO 12  →  Speaker DIN (Data)
├── GPIO 13  →  OLED SDA
├── GPIO 14  →  OLED SCL
├── GPIO 15  →  Button (other leg to GND)
├── 3.3V     →  OLED VCC
├── 5V       →  MAX98357A VIN + Battery in
└── GND      →  OLED GND, Button GND, MAX98357A GND
```

**Total connections: ~16** ✅ (much simpler than old version)

---

## ⚠️ Common Mistakes

| Mistake | What Happens | Fix |
|---------|-------------|-----|
| Connecting OLED to 5V instead of 3.3V | OLED burns out permanently | Always use 3.3V |
| Forgetting I2C pull-up resistors | OLED stays black / shows garbage | Add 4.7kΩ from 3.3V to SDA and SCL |
| Swapping TX and RX on FTDI | "Failed to connect" error | Swap yellow and green wires |
| GPIO 0 still connected to GND after flashing | Code won't run (stuck in flash mode) | Remove the GPIO 0 → GND wire |
| Reversed electrolytic capacitor | Capacitor may pop/explode | Longer leg = positive (+) |
| Connecting button to wrong pin or adding external pull-down | Button presses not detected | Connect between GPIO 15 and GND only |
| Speaker connected directly to GPIO | Very faint/no audio + possible GPIO damage | Must use MAX98357A amplifier |
| MAX98357A connected to 3.3V with high volume | Distortion or brownout reset | Power MAX98357A from 5V rail |
| Loose breadboard connections | Intermittent failures | Press wires firmly; use short wires |

---

## 🔄 Assembly Order Summary

```
Step 1 → ESP32-CAM + FTDI adapter
          Verify: can upload code via Arduino IDE

Step 2 → OLED SSD1306 (GPIO 13 SDA, GPIO 14 SCL)
          Verify: OLED shows boot screen text

Step 3 → Smart Button (GPIO 15 → GND)
          Verify: short press and long press detected in Serial Monitor

Step 4 → MAX98357A + Speaker (GPIO 2 LRC, GPIO 4 BCLK, GPIO 12 DIN)
          Verify: startup chime plays on boot

Step 5 → Battery (LiPo + TP4056 → ESP32-CAM 5V)
          Verify: boots without FTDI connected

Then  → Upload smart_glasses_master.ino
      → Start the Flask server on your PC
      → Short press button to capture & get AI result
      → Long press button to switch mode (VISION / OCR / TRANSLATE)
```
