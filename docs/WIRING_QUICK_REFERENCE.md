# ⚡ AI Smart Glasses — Quick Wiring Reference & Diagram

A concise, single-page wiring cheat sheet for the AI Smart Glasses assembly matching `smart_glasses_master.ino`.

> **Architecture:** Companion Device Model (ESP32-CAM captures image + SSD1306 OLED HUD; speech synthesis is streamed directly to the user's **Wireless Bluetooth Earbuds** via the smartphone/web companion).

---

## 🔌 Connection Details by Component

### 1. 🖥️ OLED Display (0.96" SSD1306 I2C)
> **⚠️ Critical:** Connect VCC to **3.3V only** (5V will destroy the OLED). Internal pull-ups are enabled in firmware.

| OLED Pin | ESP32-CAM Pin | Recommended Wire Color | Notes |
|:---|:---|:---:|:---|
| **VCC** | **3.3V / 3V3** | 🔴 Red | 3.3V power only |
| **GND** | **GND** | ⚫ Black | Common Ground |
| **SDA** | **GPIO 13 (Pin 4)** | 🔵 Blue | I2C Data line |
| **SCL / SCK** | **GPIO 14 (Pin 6)** | 🟡 Yellow | I2C Clock line |

---

### 2. 🔘 Single Smart Push Button (Multifunction)
> **💡 Note:** Uses ESP32 internal pull-up (`INPUT_PULLUP`). **No external resistor needed!**

| Button Leg | ESP32-CAM Pin | Function |
|:---|:---|:---|
| **Leg A** | **GPIO 15 (Pin 5)** | Signal pin |
| **Leg B** | **GND** | Ground return |

- **Short Press (< 1.2s):** Capture Photo & Query AI
- **Long Press (≥ 1.2s):** Switch Mode (`VISION` ➔ `OCR` ➔ `TRANSLATE`)

---

### 3. 🔋 Battery & Power System (3.7V LiPo + TP4056)

| From | To | Wire Color | Notes |
|:---|:---|:---:|:---|
| **LiPo Battery Red (+)** | **TP4056 B+** | 🔴 Red | Battery positive |
| **LiPo Battery Black (−)** | **TP4056 B−** | ⚫ Black | Battery negative |
| **TP4056 OUT+** | **ESP32-CAM 5V** | 🔴 Red | Powers board regulator |
| **TP4056 OUT−** | **ESP32-CAM GND** | ⚫ Black | System ground |

*(Optional: Place a small slide switch in series on the **OUT+ ➔ 5V** wire for ON/OFF control).*

---

### 4. 🎧 Audio & Speech System
- **Microphone (INMP441):** Cancelled / Removed (all interactions are button/web controlled).
- **On-board Speaker:** Removed from frame to prevent brownouts, save battery, and ensure hearing privacy.
- **Wireless Earbuds:** Connect seamlessly via the companion mobile dashboard via Bluetooth.

---

## 📌 Master ESP32-CAM Pin Map Summary

```
                    ┌──────────────────────┐
                    │      ESP32-CAM       │
                    │                      │
        Battery + ──┤ 5V      [CAMERA]     │
       Common GND ──┤ GND                  │
         OLED VCC ──┤ 3V3                  │
                    │ GPIO 12      GPIO 2  │
         OLED SDA ──┤ GPIO 13      GPIO 4  │
    Button to GND ──┤ GPIO 15      GPIO 0  │
         OLED SCL ──┤ GPIO 14      GPIO 16 │
                    └──────────────────────┘
```

| ESP32-CAM Pin | Connected To | Function / Protocol |
|:---|:---|:---|
| **5V** | TP4056 OUT+ | 5V Power Input |
| **3V3** | OLED VCC | 3.3V Logic Rail |
| **GND** | TP4056 OUT−, OLED GND, Button Leg B | Common Ground |
| **GPIO 13 (Pin 4)** | OLED **SDA** | I2C Serial Data |
| **GPIO 14 (Pin 6)** | OLED **SCL** | I2C Serial Clock |
| **GPIO 15 (Pin 5)** | Button Leg A | Single Multifunction Button |

---

## ⚠️ 4 Key Rules to Avoid Damage

1. **OLED is 3.3V only:** Connecting OLED VCC to 5V will burn out the display.
2. **I2C Pull-ups:** If OLED is blank, ensure 4.7kΩ resistors are installed between 3.3V and GPIO 13 (SDA), and 3.3V and GPIO 14 (SCL).
3. **Flashing vs Running:**
   - **Flashing code:** Bridge **GPIO 0 to GND** and press reset.
   - **Running code:** **Disconnect GPIO 0 from GND** and press reset.
4. **Speaker Power:** Powering MAX98357A from 5V prevents audio distortion or brownout resets during peak volume.
