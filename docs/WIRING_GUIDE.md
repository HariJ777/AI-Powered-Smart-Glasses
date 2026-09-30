# 🔧 AI Smart Glasses — Step-by-Step Wiring Guide

This guide walks you through connecting every component **in the correct order**, with testing at each step so you can catch problems early.

> **Tip:** Use a breadboard for prototyping first. Only solder the final version after everything works.

---

## 📦 What You Need on Your Table

Before starting, gather all these components:

| # | Component | Check |
|---|-----------|-------|
| 1 | ESP32-CAM module (AI-Thinker, with OV2640 camera) | ☐ |
| 2 | FTDI / CP2102 USB-to-Serial adapter | ☐ |
| 3 | OLED display SSD1306 (0.96", 4-pin I2C version) | ☐ |
| 4 | INMP441 I2S microphone breakout | ☐ |
| 5 | MAX98357A I2S amplifier breakout | ☐ |
| 6 | Micro speaker (8Ω, 0.5W) | ☐ |
| 7 | 2× Push buttons (6×6mm tactile) | ☐ |
| 8 | 2× 4.7kΩ resistors (yellow-violet-red) | ☐ |
| 9 | 2× 10kΩ resistors (brown-black-orange) | ☐ |
| 10 | 2× 100nF ceramic capacitors (marked "104") | ☐ |
| 11 | 1× 10µF electrolytic capacitor | ☐ |
| 12 | LiPo battery 3.7V + TP4056 charger + AMS1117-3.3V | ☐ |
| 13 | Breadboard (full-size recommended) | ☐ |
| 14 | Jumper wires (male-to-female + male-to-male) | ☐ |
| 15 | USB micro cable (for FTDI adapter) | ☐ |

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

## 🔌 Step 4: Push Buttons

Add the two user-input buttons.

### What You're Connecting

Each button connects between a GPIO pin and GND. A pull-up resistor keeps the pin HIGH when the button is NOT pressed.

### Connections

**Button 1 (Mode Select):**

| Wire # | From | To | Notes |
|--------|------|----|-------|
| 1 | Button Leg A | **ESP32-CAM GPIO 0** | 🟤 Brown wire |
| 2 | Button Leg B | **GND** rail | ⚫ Black wire |
| 3 | **10kΩ resistor** | Between **3.3V** rail and **GPIO 0** row | Pull-up |

**Button 2 (Action / Capture):**

| Wire # | From | To | Notes |
|--------|------|----|-------|
| 4 | Button Leg A | **ESP32-CAM GPIO 16** | 🟤 Brown wire |
| 5 | Button Leg B | **GND** rail | ⚫ Black wire |
| 6 | **10kΩ resistor** | Between **3.3V** rail and **GPIO 16** row | Pull-up |

### How a Button Looks on a Breadboard

```
  A tactile button has 4 legs arranged in 2 pairs:

       Leg A ──┐     ┌── Leg A
               │ BTN │
       Leg B ──┘     └── Leg B

  Place it across the center gap of the breadboard:

     3.3V ──[10kΩ]──┬── GPIO 0 (or 16)
                     │
                   [BTN]
                     │
                    GND
```

### ✅ Test Step 4

Upload this test sketch:

```cpp
#define BTN_MODE   0    // GPIO 0
#define BTN_ACTION 16   // GPIO 16

void setup() {
  Serial.begin(115200);
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_ACTION, INPUT_PULLUP);
  Serial.println("Press buttons to test...");
}

void loop() {
  if (digitalRead(BTN_MODE) == LOW) {
    Serial.println("Button 1 (MODE) pressed!");
    delay(300);
  }
  if (digitalRead(BTN_ACTION) == LOW) {
    Serial.println("Button 2 (ACTION) pressed!");
    delay(300);
  }
}
```

**Expected result:** Serial Monitor shows messages when each button is pressed ✅

> ⚠️ **Remember:** GPIO 0 must be disconnected from GND before testing (it was connected for flashing). The 10kΩ pull-up resistor now holds it HIGH.

---

## 🔌 Step 5: INMP441 Microphone (I2S)

### What You're Connecting

The INMP441 is a digital microphone that outputs audio data via the I2S protocol (3 signal wires + power).

### INMP441 Board Pinout

```
  INMP441 breakout board (top view):
  ┌──────────────────┐
  │                  │
  │  [  mic hole  ]  │
  │                  │
  │ VDD  GND  SD     │
  │ L/R  WS   SCK    │
  └──────────────────┘
```

### Connections

| Wire # | From (INMP441) | To | Wire Color | Notes |
|--------|----------------|-----|------------|-------|
| 1 | **VDD** | **3.3V** rail | 🔴 Red | Power (1.8V–3.3V) |
| 2 | **GND** | **GND** rail | ⚫ Black | |
| 3 | **SCK** | **ESP32-CAM GPIO 12** | 🟠 Orange | I2S bit clock |
| 4 | **WS** | **ESP32-CAM GPIO 4** | 🟢 Green | I2S word select |
| 5 | **SD** | **ESP32-CAM GPIO 15** | 🟣 Purple | I2S data output |
| 6 | **L/R** | **GND** rail | ⚫ Black | Selects LEFT channel |

```
INMP441                     ESP32-CAM
┌──────────┐               ┌──────────────┐
│ VDD    ──├── 🔴 ────────►├── (3.3V rail)│
│ GND    ──├── ⚫ ────────►├── (GND rail) │
│ SCK    ──├── 🟠 ────────►├── GPIO 12    │
│ WS     ──├── 🟢 ────────►├── GPIO 4     │
│ SD     ──├── 🟣 ────────►├── GPIO 15    │
│ L/R    ──├── ⚫ ────────►├── (GND rail) │
└──────────┘               └──────────────┘
```

### Decoupling Capacitor

Place a **100nF ceramic capacitor** near the INMP441:
- One leg into the same row as INMP441 VDD
- Other leg into the GND rail

### ✅ Test Step 5

Upload this test sketch:

```cpp
#include <driver/i2s.h>

#define I2S_SCK  12
#define I2S_WS    4
#define I2S_SD   15

void setup() {
  Serial.begin(115200);
  
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
  
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  
  Serial.println("Microphone ready! Speak into it...");
  Serial.println("You should see numbers change when you speak.");
}

void loop() {
  int16_t sample;
  size_t bytesRead;
  
  i2s_read(I2S_NUM_0, &sample, sizeof(sample), &bytesRead, portMAX_DELAY);
  
  // Print absolute amplitude — should spike when you speak
  int amplitude = abs(sample);
  
  // Only print significant values to avoid flooding serial
  if (amplitude > 500) {
    Serial.printf("Amplitude: %d ", amplitude);
    // Visual bar
    int bars = amplitude / 500;
    if (bars > 40) bars = 40;
    for (int i = 0; i < bars; i++) Serial.print("█");
    Serial.println();
  }
}
```

**Expected result:** Serial Monitor shows amplitude bars that spike when you speak or clap near the mic ✅

> **If you see only zeros:** Check that L/R pin is connected to GND, and double-check SCK, WS, SD wires.

---

## 🔌 Step 6: MAX98357A Amplifier + Speaker (I2S)

### What You're Connecting

The MAX98357A takes digital audio from the ESP32 via I2S and drives the speaker.

### MAX98357A Board Pinout

```
  MAX98357A breakout board:
  ┌──────────────────────────────┐
  │  VIN  GND  SD  GAIN  DIN    │
  │                    BCLK LRC │
  │                              │
  │        [Speaker +]  [Speaker -]
  └──────────────────────────────┘
```

### Connections

| Wire # | From (MAX98357A) | To | Wire Color | Notes |
|--------|------------------|-----|------------|-------|
| 1 | **VIN** | **3.3V** rail | 🔴 Red | Power (2.5V–5.5V) |
| 2 | **GND** | **GND** rail | ⚫ Black | |
| 3 | **BCLK** | **ESP32-CAM GPIO 12** | 🟠 Orange | Shared with mic SCK |
| 4 | **LRC** | **ESP32-CAM GPIO 4** | 🟢 Green | Shared with mic WS |
| 5 | **DIN** | **ESP32-CAM GPIO 2** | ⚪ White | Audio data to speaker |
| 6 | **SD** | Not connected | — | Leave floating (internally pulled HIGH) |
| 7 | **GAIN** | Not connected | — | Default 9dB gain |

### Speaker Wires

| Wire # | From (MAX98357A) | To (Speaker) | Notes |
|--------|------------------|--------------|-------|
| 8 | **Speaker +** terminal | Speaker **red wire (+)** | Solder or screw terminal |
| 9 | **Speaker −** terminal | Speaker **black wire (−)** | Solder or screw terminal |

```
ESP32-CAM          MAX98357A              Speaker
┌──────────┐      ┌───────────┐          ┌────────┐
│ 3.3V   ──├─ 🔴 ─┤ VIN       │          │        │
│ GND    ──├─ ⚫ ─┤ GND       │          │  ┌──┐  │
│ GPIO 12──├─ 🟠 ─┤ BCLK      │  Spk+ ──┼──┤  │  │
│ GPIO 4 ──├─ 🟢 ─┤ LRC       │  Spk- ──┼──┤  │  │
│ GPIO 2 ──├─ ⚪ ─┤ DIN       │          │  └──┘  │
└──────────┘      └───────────┘          └────────┘
```

> ⚠️ **I2S Sharing:** GPIO 12 (BCLK) and GPIO 4 (LRC) are shared between the microphone and the amplifier. This is fine — the firmware switches between mic and speaker mode, never running both at the same time.

### ✅ Test Step 6

Speaker testing requires more code — for now, verify the wiring is correct by checking that the MAX98357A power LED is on. Full audio testing will happen with the final firmware.

---

## 🔌 Step 7: Power System (Battery — Final Step)

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

### ✅ Test Step 7

1. **Disconnect the FTDI adapter** completely
2. Charge the battery via TP4056 USB (red LED = charging, blue LED = full)
3. Check voltage at the 3.3V rail with a multimeter — should read **3.3V ± 0.1V**
4. The ESP32-CAM should boot up automatically
5. Check the OLED displays the mode selection screen

---

## 📋 Complete Connection Checklist

After all steps, verify every connection against this master list:

### OLED Display (4 wires + 2 resistors + 1 cap)
- [ ] OLED VCC → 3.3V rail
- [ ] OLED GND → GND rail
- [ ] OLED SDA → GPIO 13
- [ ] OLED SCL → GPIO 14
- [ ] 4.7kΩ resistor: 3.3V → GPIO 13
- [ ] 4.7kΩ resistor: 3.3V → GPIO 14
- [ ] 100nF cap near OLED VCC/GND

### INMP441 Microphone (6 wires + 1 cap)
- [ ] INMP441 VDD → 3.3V rail
- [ ] INMP441 GND → GND rail
- [ ] INMP441 SCK → GPIO 12
- [ ] INMP441 WS → GPIO 4
- [ ] INMP441 SD → GPIO 15
- [ ] INMP441 L/R → GND rail
- [ ] 100nF cap near INMP441 VDD/GND

### MAX98357A + Speaker (5 wires + 2 speaker wires)
- [ ] MAX98357A VIN → 3.3V rail
- [ ] MAX98357A GND → GND rail
- [ ] MAX98357A BCLK → GPIO 12 (shared with mic)
- [ ] MAX98357A LRC → GPIO 4 (shared with mic)
- [ ] MAX98357A DIN → GPIO 2
- [ ] Speaker + → MAX98357A Speaker +
- [ ] Speaker − → MAX98357A Speaker −

### Push Buttons (4 wires + 2 resistors)
- [ ] Button 1 Leg A → GPIO 0
- [ ] Button 1 Leg B → GND rail
- [ ] 10kΩ resistor: 3.3V → GPIO 0
- [ ] Button 2 Leg A → GPIO 16
- [ ] Button 2 Leg B → GND rail
- [ ] 10kΩ resistor: 3.3V → GPIO 16

### Power System (6 wires + 2 caps)
- [ ] Battery + → TP4056 B+
- [ ] Battery − → TP4056 B−
- [ ] TP4056 OUT+ → AMS1117 VIN
- [ ] TP4056 OUT− → AMS1117 GND (and GND rail)
- [ ] AMS1117 VOUT → 3.3V rail
- [ ] AMS1117 GND → GND rail
- [ ] 10µF electrolytic cap: AMS1117 VOUT to GND (check polarity!)
- [ ] 100nF ceramic cap: near ESP32-CAM 3V3 to GND

### ESP32-CAM to Breadboard (2 wires)
- [ ] ESP32-CAM 3V3 → 3.3V rail
- [ ] ESP32-CAM GND → GND rail

**Total: ~33 connections** ✅

---

## ⚠️ Common Mistakes

| Mistake | What Happens | Fix |
|---------|-------------|-----|
| Connecting OLED to 5V instead of 3.3V | OLED burns out permanently | Always use 3.3V |
| Forgetting I2C pull-up resistors | OLED stays black / shows garbage | Add 4.7kΩ from 3.3V to SDA and SCL |
| Swapping TX and RX on FTDI | "Failed to connect" error | Swap yellow and green wires |
| GPIO 0 still connected to GND after flashing | Code won't run (stuck in flash mode) | Remove the GPIO 0 → GND wire |
| Reversed electrolytic capacitor | Capacitor may pop/explode | Longer leg = positive (+) |
| Mic L/R pin left floating | Mic outputs no data | Connect L/R to GND |
| Speaker connected directly to GPIO | Very faint/no audio + possible GPIO damage | Must use MAX98357A amplifier |
| Loose breadboard connections | Intermittent failures | Press wires firmly; use short wires |

---

## 🔄 Assembly Order Summary

```
Step 1 → Set up breadboard power rails
Step 2 → ESP32-CAM + FTDI (verify you can upload code)
Step 3 → OLED display (verify text shows on screen)
Step 4 → Push buttons (verify presses detected)
Step 5 → INMP441 microphone (verify audio amplitude)
Step 6 → MAX98357A + speaker (verify power LED on)
Step 7 → Battery power system (verify 3.3V output)

Then → Upload the full smart_glasses_firmware.ino
     → Start the Flask server on your PC
     → Test all 5 modes!
```
