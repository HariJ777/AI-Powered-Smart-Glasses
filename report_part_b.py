"""Report content chapters 3-6"""

def write_ch3(doc, h1, h2, body, bullet):
    h1(doc, 'CHAPTER 3: PROBLEM STATEMENT AND OBJECTIVES', new_page=True)
    h2(doc, '3.1 Problem Statement')
    body(doc, 'Visually impaired individuals face significant challenges in understanding and navigating their physical environment. While assistive devices such as white canes and guide dogs provide basic obstacle avoidance, they cannot offer visual understanding capabilities such as reading text from signs, identifying objects in a scene, or answering questions about the surroundings. Modern AI technologies possess these capabilities, but deploying them in a wearable form factor presents three critical challenges:')
    bullet(doc, 'Cost Barrier: Commercial smart glasses with AI capabilities such as Google Glass (Rs.83,000) and Meta Ray-Ban (Rs.25,000) are prohibitively expensive for the majority of users, especially in developing countries like India where the average monthly income is approximately Rs.31,900.')
    bullet(doc, 'Computational Resource Requirements: State-of-the-art AI vision models such as GPT-4 Vision and LLaVA require powerful GPUs with 12+ GB of dedicated VRAM for local inference. This makes it impractical to run these models on a user\'s personal laptop or any edge computing device without expensive hardware upgrades.')
    bullet(doc, 'Memory Management Complexity: Running multiple AI models simultaneously (vision, language, OCR, speech recognition) would require over 12 GB of combined memory, exceeding the capacity of most consumer-grade GPUs and making the system impossible to deploy on affordable hardware.')
    body(doc, 'There is a clear need for an affordable smart glasses solution that can deliver advanced AI capabilities using consumer-grade hardware and free cloud services, making assistive technology accessible to users regardless of their economic background.')

    h2(doc, '3.2 Objectives')
    body(doc, 'The specific objectives of this project are:')
    bullet(doc, 'To design and fabricate a wearable smart glasses prototype using the ESP32-CAM microcontroller with OV2640 camera module, SSD1306 OLED display, INMP441 I2S microphone, and speaker, at a total hardware cost not exceeding Rs.1,500.')
    bullet(doc, 'To develop a Flask-based backend server with a Smart Routing architecture that intelligently distributes AI tasks between cloud-based models (Groq API) and locally-running models (EasyOCR, Whisper-tiny).')
    bullet(doc, 'To implement five core AI functionalities: scene description, text extraction via OCR, speech-to-text transcription, question answering, and language translation.')
    bullet(doc, 'To achieve response times under 2 seconds for all AI tasks while maintaining peak memory usage below 600 MB through the Smart Routing memory management strategy.')
    bullet(doc, 'To validate the system through comprehensive testing including AI model accuracy tests, memory management tests, and Flask endpoint tests, achieving a 100% pass rate across all test categories.')
    bullet(doc, 'To provide text-to-speech audio output and OLED display output for delivering AI responses to the user in both visual and auditory formats.')

def write_ch4(doc, h1, h2, h3, body, bullet):
    h1(doc, 'CHAPTER 4: SYSTEM REQUIREMENTS AND SPECIFICATION', new_page=True)
    h2(doc, '4.1 Hardware Requirements')
    h3(doc, '4.1.1 Microcontroller - ESP32-CAM')
    body(doc, 'The ESP32-CAM serves as the central processing unit of the smart glasses. It is based on the ESP32-S module, featuring a dual-core Xtensa LX6 processor running at 240 MHz with 520 KB of SRAM and 4 MB of flash memory. The module includes an integrated OV2640 camera capable of capturing images at resolutions up to 1600x1200 pixels. For this project, images are captured at 640x480 (VGA) resolution to balance quality with transmission speed. The ESP32-CAM supports IEEE 802.11 b/g/n Wi-Fi and Bluetooth 4.2, enabling wireless communication with the backend server. Cost: Rs.450.')

    h3(doc, '4.1.2 Display - SSD1306 OLED (0.96 inch, 128x64, I2C)')
    body(doc, 'The SSD1306 OLED display is a compact, self-emitting display module with 128x64 pixel resolution. It communicates with the ESP32 via the I2C protocol using GPIO 14 (SDA) and GPIO 15 (SCL) pins. Two 4.7k ohm pull-up resistors are required on the I2C data and clock lines to ensure reliable communication. The display is mounted on the glasses frame near the user\'s field of vision and shows AI-generated text responses. Its low power consumption (typically 20 mA at 3.3V) makes it suitable for battery-powered operation. Cost: Rs.180.')

    h3(doc, '4.1.3 Microphone - INMP441 I2S Digital MEMS')
    body(doc, 'The INMP441 is a high-precision digital MEMS microphone that outputs audio data via the I2S (Inter-IC Sound) protocol. It provides a signal-to-noise ratio of 61 dB and a sensitivity of -26 dBFS. The I2S interface provides superior audio quality compared to analog microphones by eliminating analog-to-digital conversion noise. The microphone is connected to the ESP32 using three signal lines: WS (Word Select/LRCLK), SCK (Serial Clock/BCLK), and SD (Serial Data). It captures the user\'s voice commands for speech-to-text processing. Cost: Rs.250.')

    h3(doc, '4.1.4 Audio Output - Micro Speaker (8 ohm, 0.5W, 40mm)')
    body(doc, 'A miniature speaker with 8 ohm impedance and 0.5W power rating provides audio output for text-to-speech feedback. This allows the user to hear AI responses without needing to look at the OLED display, which is particularly important for visually impaired users. The speaker is driven through a simple amplifier circuit or directly through a DAC output from the ESP32. Cost: Rs.99.')

    h3(doc, '4.1.5 Input Controls - Push Buttons (x2)')
    body(doc, 'Two tactile push buttons provide user input for mode selection and action triggering. Button 1 cycles through available modes (scene description, text reading, voice input, question answering, translation), while Button 2 triggers the selected action. Each button is connected with a 10k ohm pull-up resistor to ensure clean digital signal transitions and prevent floating pin states. Cost: Rs.20.')

    h3(doc, '4.1.6 Power Supply')
    body(doc, 'The system is powered by a 3.7V, 1000 mAh lithium polymer (LiPo) rechargeable battery. A TP4056 module provides USB micro-B charging capability with overcharge and over-discharge protection. An AMS1117-3.3V linear voltage regulator converts the battery output (3.0V-4.2V depending on charge level) to a stable 3.3V required by all electronic components. Two 100nF ceramic decoupling capacitors are placed near the voltage regulator and the ESP32 power pins to filter high-frequency noise. Battery cost: Rs.240, Voltage regulator: Rs.18, Supporting components: Rs.150.')

    body(doc, 'Total Estimated Hardware Cost: Rs.1,407 (approximately USD 17)')

    h2(doc, '4.2 Software Requirements')
    h3(doc, '4.2.1 Programming Languages')
    bullet(doc, 'Python 3.10+: Primary language for the Flask backend server, AI model integration, Smart Routing logic, and memory management. Python\'s extensive library ecosystem provides access to all required AI frameworks and utilities.')
    bullet(doc, 'C/C++ (Arduino Framework): Used for ESP32-CAM firmware development within the Arduino IDE 2.x environment. The firmware handles camera initialization, image capture, Wi-Fi connectivity, HTTP POST requests, I2C display communication, and I2S microphone data acquisition.')

    h3(doc, '4.2.2 Frameworks and Libraries')
    bullet(doc, 'Flask 3.0+: Lightweight Python web framework implementing the Smart Router backend with RESTful API endpoints for each AI function (/analyze-image, /extract-text, /transcribe, /ask, /translate).')
    bullet(doc, 'Groq Python SDK (v1.2): Client library for accessing the Groq cloud API, providing access to Llama 4 Scout 17B (vision) and Llama 3.3 70B Versatile (language).')
    bullet(doc, 'EasyOCR 1.7+: Open-source OCR library supporting 80+ languages, used for local text extraction from camera images.')
    bullet(doc, 'OpenAI Whisper (tiny): Speech recognition model with 39M parameters, used for local speech-to-text transcription requiring approximately 145 MB RAM.')
    bullet(doc, 'PyTorch 2.0+: Deep learning framework providing the inference runtime for EasyOCR and Whisper models.')
    bullet(doc, 'psutil: System monitoring library for tracking real-time RAM and CPU usage during operation.')

    h3(doc, '4.2.3 Communication Protocols')
    bullet(doc, 'HTTP/REST: The ESP32-CAM communicates with the Flask backend via HTTP POST requests carrying Base64-encoded image or audio data in JSON format.')
    bullet(doc, 'I2C (Inter-Integrated Circuit): Two-wire serial protocol used for communication between the ESP32 and the SSD1306 OLED display at 400 kHz clock speed.')
    bullet(doc, 'I2S (Inter-IC Sound): Three-wire digital audio protocol used for high-quality audio data transfer from the INMP441 microphone to the ESP32.')
    bullet(doc, 'Wi-Fi (IEEE 802.11 b/g/n): Wireless networking protocol enabling communication between the ESP32-CAM and the backend server over the local network.')

    h2(doc, '4.3 Power Requirements')
    body(doc, 'The system operates on a 3.7V lithium polymer battery with the following power budget:')
    bullet(doc, 'ESP32-CAM (active with Wi-Fi): 160-260 mA at 3.3V')
    bullet(doc, 'SSD1306 OLED Display: 20 mA at 3.3V')
    bullet(doc, 'INMP441 Microphone: 1.4 mA at 3.3V')
    bullet(doc, 'Speaker (during playback): 60 mA at 3.3V')
    bullet(doc, 'Total peak consumption: approximately 340 mA')
    body(doc, 'With a 1000 mAh battery, the system provides approximately 2.5-3 hours of continuous active use. In practice, intermittent use (capturing images and waiting for responses) extends battery life to approximately 4-5 hours. The TP4056 module enables recharging via any standard USB power source.')

def write_ch5(doc, h1, h2, h3, body, bullet):
    h1(doc, 'CHAPTER 5: PROPOSED METHODOLOGY', new_page=True)
    h2(doc, '5.1 Software Development')
    h3(doc, '5.1.1 Control Code Development')
    body(doc, 'The software architecture follows a client-server model with the ESP32-CAM acting as the client and the Flask application serving as the backend server. The development process involves two parallel tracks:')

    body(doc, 'ESP32-CAM Firmware (C++/Arduino): The firmware initializes the camera module in JPEG mode at VGA resolution, establishes Wi-Fi connectivity with the local network, and enters a main loop that monitors button inputs. When the user presses the action button, the firmware captures an image frame, encodes it in Base64 format, constructs an HTTP POST request with JSON payload, and transmits it to the Flask server. Upon receiving the response, the firmware extracts the AI-generated text and renders it on the OLED display using the Adafruit SSD1306 library. For speech-to-text mode, the firmware samples audio from the INMP441 microphone via I2S at 16 kHz, buffers the audio data, and transmits it similarly to the server.')

    body(doc, 'Flask Backend Server (Python): The backend implements five RESTful API endpoints corresponding to the five AI capabilities. Each endpoint receives Base64-encoded data, decodes it, and routes the request to the appropriate AI model through the Smart Router. The Smart Router is the core architectural component that manages model lifecycle:')

    bullet(doc, 'For cloud-based tasks (scene description, Q&A, translation): The Smart Router calls the Groq API directly. Since these models run on Groq\'s servers, they consume zero local GPU memory. The Groq SDK handles authentication, request formatting, and response parsing.')
    bullet(doc, 'For local tasks (OCR, speech-to-text): The Smart Router first checks if the required model is already loaded in memory. If not, it unloads any currently loaded local model, then loads the required model. After processing the request and generating a response, the model is explicitly unloaded using Python garbage collection and PyTorch CUDA cache clearing.')

    h3(doc, '5.1.2 Data Collection and Real-Time Transmission')
    body(doc, 'The data flow through the system follows a four-stage pipeline:')
    bullet(doc, 'Stage 1 - Capture: The ESP32-CAM captures a JPEG image at 640x480 resolution (approximately 30-50 KB after compression) or records audio from the INMP441 microphone at 16 kHz sample rate.')
    bullet(doc, 'Stage 2 - Encode and Transmit: The captured data is Base64-encoded to ensure safe transmission over HTTP. The encoded data is wrapped in a JSON payload with metadata indicating the request type and transmitted as an HTTP POST request to the Flask server over Wi-Fi.')
    bullet(doc, 'Stage 3 - Process: The Flask Smart Router receives the request, decodes the data, and dispatches it to the appropriate AI model. Cloud API calls typically complete in 0.4-0.6 seconds, while local model processing takes 0.4-1.8 seconds depending on the task.')
    bullet(doc, 'Stage 4 - Respond: The AI-generated text response is formatted as JSON and sent back to the ESP32-CAM. The ESP32 renders the text on the OLED display and optionally converts it to speech for audio output.')

    h2(doc, '5.2 Testing and Simulation')
    body(doc, 'The testing strategy employs a three-tier approach to validate all aspects of the system:')

    h3(doc, '5.2.1 AI Model Tests (5 tests)')
    body(doc, 'Each AI model is tested independently to verify correct functionality. The Llama 4 Scout vision model is tested with a sample image of a park scene and validated to produce a coherent description. The Llama 3.3 Q&A model is tested with factual questions. The translation model is tested with English-to-Spanish conversion. EasyOCR is tested with an image containing printed text. Whisper-tiny is tested with a WAV audio file. All 5 tests passed successfully.')

    h3(doc, '5.2.2 Memory Management Tests (3 tests)')
    body(doc, 'Test 1 verifies that Groq API calls consume 0 GB of local GPU memory. Test 2 verifies that local models (EasyOCR, Whisper) load and unload cleanly without memory leaks. Test 3 verifies that the Smart Router maintains peak GPU memory usage below 600 MB when processing a mixed sequence of requests. All 3 tests passed.')

    h3(doc, '5.2.3 Flask Endpoint Tests (5 tests)')
    body(doc, 'Each of the five API endpoints is tested with valid input data and validated for correct HTTP response codes (200 OK) and properly formatted JSON responses. Error handling is tested by submitting malformed requests and verifying appropriate error codes (400 Bad Request). All 5 tests passed. Total: 13/13 tests passed (100% pass rate).')

def write_ch6(doc, h1, h2, h3, body, bullet):
    h1(doc, 'CHAPTER 6: SYSTEM ARCHITECTURE', new_page=True)
    body(doc, 'The system architecture consists of six major components working in concert to deliver AI-powered visual assistance. This chapter describes each component in detail.')

    h2(doc, '6.1 Power Supply Unit')
    body(doc, 'The power supply unit consists of a 3.7V 1000 mAh LiPo battery connected to a TP4056 charging module and an AMS1117-3.3V voltage regulator. The TP4056 provides safe lithium battery charging with automatic cutoff at 4.2V and undervoltage protection at 2.5V. The AMS1117 regulator provides a stable 3.3V output with dropout voltage of 1.1V, ensuring consistent operation across the battery\'s discharge curve. Two 100nF decoupling capacitors filter power supply noise.')

    h2(doc, '6.2 ESP32 Control Unit')
    body(doc, 'The ESP32-CAM module serves as the central control unit, executing the firmware that orchestrates all hardware interactions. The dual-core processor runs the main application loop on Core 1 while Core 0 handles Wi-Fi stack operations. The firmware manages camera frame buffer allocation, I2C bus initialization for the OLED display, I2S peripheral configuration for the microphone, GPIO interrupt handlers for button presses, and HTTP client operations for server communication.')

    h2(doc, '6.3 Sensors and Input Devices')
    body(doc, 'The OV2640 camera module captures visual data in JPEG format. The camera interfaces with the ESP32 through an 8-bit parallel data bus with XCLK, PCLK, VSYNC, and HREF timing signals. The INMP441 microphone captures audio data through the I2S interface. Two tactile push buttons with hardware debouncing (10k pull-up resistors) provide user input.')

    h2(doc, '6.4 OLED Display')
    body(doc, 'The SSD1306 OLED display is driven via I2C at address 0x3C. The display supports multiple font sizes and can render approximately 8 lines of text at the smallest font size. The firmware implements automatic text wrapping and scrolling for responses that exceed the display area. The self-emitting OLED technology provides high contrast and visibility in various lighting conditions.')

    h2(doc, '6.5 Flask Web Server (Smart Router)')
    body(doc, 'The Flask web server is the intelligence hub of the system. It exposes five RESTful API endpoints and implements the Smart Routing logic. The server architecture includes:')
    bullet(doc, 'Request Handler Layer: Receives HTTP POST requests, validates input format, and extracts Base64-encoded data.')
    bullet(doc, 'Smart Router Layer: Determines the appropriate AI model based on the endpoint called, manages model loading/unloading, and tracks memory usage via psutil.')
    bullet(doc, 'AI Model Layer: Contains wrapper functions for each AI model (Groq API client for cloud models, EasyOCR reader instance for OCR, Whisper model instance for speech recognition).')
    bullet(doc, 'Response Layer: Formats AI model outputs into standardized JSON responses with status codes, response text, processing time, and memory usage statistics.')

    body(doc, 'Block Diagram of System Architecture (Text Description):')
    body(doc, '[User] --> [Push Button] --> [ESP32-CAM] --> [Wi-Fi] --> [Flask Server] --> [Smart Router] --> [Groq Cloud API | Local EasyOCR | Local Whisper] --> [JSON Response] --> [ESP32-CAM] --> [OLED Display / Speaker] --> [User]')
