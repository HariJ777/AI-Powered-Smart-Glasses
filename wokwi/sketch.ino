/*
 * ================================================================
 *  AI Smart Glasses — Wokwi Simulation Sketch
 * ================================================================
 *  
 *  This is a SIMULATION version of the firmware designed to run
 *  on Wokwi.com. It demonstrates:
 *    - OLED display (SSD1306 via I2C)
 *    - Button-driven mode cycling (5 modes)
 *    - Status LED indication
 *    - Simulated AI responses on the OLED
 *
 *  Differences from real hardware:
 *    - Uses standard ESP32 DevKit (not ESP32-CAM)
 *    - Default I2C pins: SDA=21, SCL=22 (instead of 13, 14)
 *    - No camera, microphone, or speaker (not available in Wokwi)
 *    - No Wi-Fi / Flask server communication
 *    - AI responses are simulated with placeholder text
 *
 *  How to use on Wokwi:
 *    1. Go to https://wokwi.com/projects/new/esp32
 *    2. Replace sketch.ino with this code
 *    3. Replace diagram.json with the provided diagram.json
 *    4. Add libraries: Adafruit SSD1306, Adafruit GFX, ArduinoJson
 *    5. Click the green Play button to start simulation
 *    6. Click the green button (MODE) to cycle modes
 *    7. Click the red button (ACTION) to simulate capture
 *
 *  Authors: Arya B Shetty, Harinand J, Karthik P, Pavan SN
 * ================================================================
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ─────────────────────────────────────────────
// Pin Definitions (for Wokwi standard ESP32)
// ─────────────────────────────────────────────
// NOTE: On real ESP32-CAM, SDA=13, SCL=14
//       On Wokwi ESP32 DevKit, use default I2C: SDA=21, SCL=22
#define PIN_SDA         21    // Default ESP32 I2C SDA
#define PIN_SCL         22    // Default ESP32 I2C SCL
#define PIN_BTN_MODE     4    // Button 1 — Mode Select
#define PIN_BTN_ACTION  15    // Button 2 — Action / Capture
#define PIN_STATUS_LED   2    // Status LED (on-board or external)

// OLED Display
#define OLED_WIDTH      128
#define OLED_HEIGHT      64
#define OLED_ADDR       0x3C

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// ─────────────────────────────────────────────
// Operating Modes
// ─────────────────────────────────────────────
enum Mode {
  MODE_DESCRIBE  = 0,
  MODE_OCR       = 1,
  MODE_SPEECH    = 2,
  MODE_QA        = 3,
  MODE_TRANSLATE = 4,
  MODE_COUNT     = 5
};

const char* MODE_NAMES[] = {
  "DESCRIBE",
  "READ TEXT",
  "SPEECH",
  "Q & A",
  "TRANSLATE"
};

// Simulated AI responses for each mode
const char* SIMULATED_RESPONSES[] = {
  "I see a desk with a laptop, a coffee mug, and some books. The room appears to be well-lit with natural light.",
  "Extracted text: 'Welcome to Alva's Institute of Engineering & Technology, Moodbidri'",
  "Transcription: 'What is the weather like today in Mangalore?'",
  "AI Answer: Artificial Intelligence is a branch of computer science that aims to create intelligent machines.",
  "Translation: 'Namaste' means 'Hello' in Hindi. It is a common greeting in India."
};

// State
volatile int currentMode = MODE_DESCRIBE;
volatile bool actionTriggered = false;
unsigned long lastBtnModePress = 0;
unsigned long lastBtnActionPress = 0;
int previousMode = -1;
bool isProcessing = false;

#define DEBOUNCE_MS 300

// ─────────────────────────────────────────────
// Button Interrupt Handlers
// ─────────────────────────────────────────────

void IRAM_ATTR onBtnModePress() {
  unsigned long now = millis();
  if (now - lastBtnModePress > DEBOUNCE_MS) {
    currentMode = (currentMode + 1) % MODE_COUNT;
    lastBtnModePress = now;
  }
}

void IRAM_ATTR onBtnActionPress() {
  unsigned long now = millis();
  if (now - lastBtnActionPress > DEBOUNCE_MS) {
    if (!isProcessing) {
      actionTriggered = true;
    }
    lastBtnActionPress = now;
  }
}

// ─────────────────────────────────────────────
// Display Functions
// ─────────────────────────────────────────────

void showSplashScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Draw a border
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  
  display.setCursor(10, 8);
  display.setTextSize(1);
  display.println("AI SMART GLASSES");
  
  display.setCursor(20, 22);
  display.println("ESP32-CAM v1.0");
  
  display.drawLine(10, 34, 118, 34, SSD1306_WHITE);
  
  display.setCursor(8, 40);
  display.println("Wokwi Simulation");
  
  display.setCursor(18, 52);
  display.println("Initializing...");
  
  display.display();
}

void showMode() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Header
  display.drawRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setCursor(4, 2);
  display.print(" AI SMART GLASSES");
  
  // Mode list
  display.setCursor(0, 16);
  for (int i = 0; i < MODE_COUNT; i++) {
    if (i == currentMode) {
      // Highlight current mode with inverse
      display.fillRect(0, 16 + (i * 8), 128, 8, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(2, 16 + (i * 8));
      display.print("> ");
      display.println(MODE_NAMES[i]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(2, 16 + (i * 8));
      display.print("  ");
      display.println(MODE_NAMES[i]);
    }
  }
  
  // Footer
  display.drawLine(0, 56, 128, 56, SSD1306_WHITE);
  display.setCursor(2, 57);
  display.print("GRN:Mode  RED:Action");
  
  display.display();
}

void showProcessing(const char* modeName) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print("[");
  display.print(modeName);
  display.println("]");
  display.println("----------------");
  display.println();
  
  // Different messages based on mode
  if (currentMode == MODE_DESCRIBE || currentMode == MODE_OCR) {
    display.println("  Capturing image");
    display.println("  from camera...");
  } else {
    display.println("  Recording audio");
    display.println("  from mic...");
  }
  
  display.println();
  
  // Animated progress bar
  display.drawRect(14, 50, 100, 10, SSD1306_WHITE);
  display.display();
  
  for (int i = 0; i < 96; i += 4) {
    display.fillRect(16, 52, i, 6, SSD1306_WHITE);
    display.display();
    delay(30);
  }
}

void showResponse(const char* modeName, const char* response) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Header
  display.setCursor(0, 0);
  display.print("[");
  display.print(modeName);
  display.println("]");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
  
  // Response text (word-wrapped by Adafruit library)
  display.setCursor(0, 12);
  
  // Truncate to fit screen (~21 chars x 6 lines = 126 chars)
  String resp = String(response);
  if (resp.length() > 120) {
    resp = resp.substring(0, 117) + "...";
  }
  display.print(resp);
  
  display.display();
}

void showServerInfo() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.drawRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setCursor(8, 2);
  display.print("SYSTEM STATUS");
  
  display.setCursor(0, 16);
  display.println("WiFi: SIMULATED");
  display.println("IP: 192.168.1.100");
  display.println("Server: SIMULATED");
  display.println("Port: 5000");
  display.println();
  display.println("All systems ready!");
  
  display.display();
}

// ─────────────────────────────────────────────
// Action Handler (Simulated)
// ─────────────────────────────────────────────

void handleAction() {
  isProcessing = true;
  
  Serial.println();
  Serial.println("========================================");
  Serial.print("[ACTION] Mode: ");
  Serial.println(MODE_NAMES[currentMode]);
  
  // Status LED ON
  digitalWrite(PIN_STATUS_LED, HIGH);
  
  // Show processing animation
  showProcessing(MODE_NAMES[currentMode]);
  
  // Simulate network delay
  Serial.println("[INFO] Simulating AI processing...");
  delay(500);
  
  // Get simulated response
  const char* response = SIMULATED_RESPONSES[currentMode];
  
  Serial.print("[RESULT] ");
  Serial.println(response);
  Serial.println("========================================");
  Serial.println();
  
  // Status LED OFF
  digitalWrite(PIN_STATUS_LED, LOW);
  
  // Show response on OLED
  showResponse(MODE_NAMES[currentMode], response);
  
  // Keep response on screen for 5 seconds
  delay(5000);
  
  isProcessing = false;
  
  // Return to mode selection
  showMode();
}

// ─────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("==========================================");
  Serial.println("  AI Smart Glasses — Wokwi Simulation");
  Serial.println("==========================================");
  Serial.println();
  
  // Status LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);
  
  // Buttons
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_BTN_ACTION, INPUT_PULLUP);
  
  // Initialize OLED
  Wire.begin(PIN_SDA, PIN_SCL);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[ERROR] OLED not found!");
    while (true) { delay(1000); }
  }
  Serial.println("[OK] OLED display initialized");
  
  // Show splash screen
  showSplashScreen();
  delay(2000);
  
  // Show simulated server status
  showServerInfo();
  Serial.println("[OK] WiFi: SIMULATED (192.168.1.100)");
  Serial.println("[OK] Server: SIMULATED (port 5000)");
  delay(2000);
  
  // Attach button interrupts
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_MODE), onBtnModePress, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_ACTION), onBtnActionPress, FALLING);
  
  // Show mode selection
  showMode();
  
  Serial.println();
  Serial.println("[READY] Simulation running!");
  Serial.println("  Green Button = Cycle Mode");
  Serial.println("  Red Button   = Trigger Action");
  Serial.println("==========================================");
  Serial.println();
}

// ─────────────────────────────────────────────
// MAIN LOOP
// ─────────────────────────────────────────────

void loop() {
  // Update display when mode changes
  if (currentMode != previousMode && !isProcessing) {
    Serial.print("[MODE] Changed to: ");
    Serial.println(MODE_NAMES[currentMode]);
    showMode();
    previousMode = currentMode;
  }
  
  // Handle action button press
  if (actionTriggered) {
    actionTriggered = false;
    handleAction();
  }
  
  delay(10);
}
