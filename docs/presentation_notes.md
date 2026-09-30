# AI Smart Glasses Project: Progress & Architecture Report

## 1. Project Overview
The goal of this project is to build a wearable pair of **AI Smart Glasses** that can see the world, process images using advanced AI, and display real-time insights (scene descriptions or extracted text) on a tiny OLED screen built into the glasses.

## 2. Hardware Architecture
We successfully wired and assembled the core hardware components:
*   **Microcontroller & Camera:** ESP32-CAM module (handles Wi-Fi, takes pictures).
*   **Display:** 0.96" SSD1306 OLED Display (I2C protocol).
*   **Power:** 5V USB power spliced directly to the breadboard/microcontroller.
*   **Networking:** The ESP32 connects to a local Wi-Fi network to communicate with a powerful backend server running on a PC.

## 3. Software Architecture (The 3-Part System)
Getting an AI model to run on a tiny ESP32 is impossible due to memory constraints. We solved this by splitting the workload into three parts:

### A. The ESP32 Firmware (C++)
1.  **Web Server:** We programmed the ESP32 to host its own internal web page.
2.  **Image Capture:** When triggered, it grabs a JPEG frame from the camera sensor and converts it into a Base64 string.
3.  **HTTP POST:** It sends this Base64 string securely to the PC over Wi-Fi.
4.  **Display Logic:** It parses the JSON response from the PC and formats the text to fit neatly on the OLED screen.

### B. The PC Backend Server (Python / Flask)
1.  **API Gateway:** We built a Flask server on the PC that listens on port `5000` for incoming images from the glasses.
2.  **Smart Routing:** The server has two distinct AI pipelines depending on what the user requests:
    *   **Scene Description:** Routes the image to a Vision Large Language Model (VLM).
    *   **Text Reading (OCR):** Routes the image to EasyOCR, a local Optical Character Recognition engine using PyTorch.

### C. The AI Engine (Ollama / Groq)
1.  **Cloud vs Local:** Initially, we used Groq's cloud APIs for lightning-fast Llama vision models. However, when those models were decommissioned, we seamlessly migrated to **Ollama** running locally.
2.  **Llava Model:** We are now running the `llava` vision model 100% locally on the PC hardware. This provides completely private, offline image analysis.

## 4. Key Challenges & Solutions

### Challenge 1: The Broken Serial Monitor
*   **Problem:** Once the glasses were disconnected from the computer and running on battery/external power, we couldn't send commands (like "take a picture") via the Serial Monitor.
*   **Solution:** We built a **Web UI Control Panel**. The ESP32 serves an interactive HTML dashboard to any smartphone on the network, allowing the user to trigger commands remotely.

### Challenge 2: Browser Security Restrictions
*   **Problem:** When we tried to show the captured image in the Web UI, modern browsers blocked it because of "data URI redirect" security restrictions.
*   **Solution:** We rebuilt the image delivery system. Instead of redirecting the browser, the frontend JavaScript makes a silent `fetch()` call to download the raw text and seamlessly updates the image tag on the screen.

### Challenge 3: Blocking Web Servers & Latency
*   **Problem:** Local AI models (like Ollama) take 1-3 minutes to process an image. Because the ESP32 is single-threaded, waiting for the AI completely froze the web server, preventing the user from even seeing the picture that was taken!
*   **Solution:** We implemented a **Fully Asynchronous Architecture**. 
    *   Step 1: The browser commands the ESP32 to snap a photo.
    *   Step 2: The browser instantly downloads and displays the photo.
    *   Step 3: The browser tells the ESP32 to send the photo to the PC.
    *   This ensures the user gets immediate visual feedback while the heavy AI processing happens in the background.

### Challenge 4: Flask Auto-Reloader Crashes
*   **Problem:** When the EasyOCR PyTorch library ran for the first time, it generated cache files. Flask's debug mode detected these new files and aggressively rebooted the server mid-request, causing `HTTP -5` connection timeouts on the ESP32.
*   **Solution:** We disabled Flask's aggressive debug reloader (`FLASK_DEBUG=False`), stabilizing the backend for heavy AI workloads.

## 5. Current Status & Next Steps
*   **Working:** The camera captures images, the Web UI controls the flow, the PC processes OCR/Ollama, and the result displays on the OLED.
*   **Next Phase:** Finalizing the hardware assembly (transitioning from a breadboard to a soldered, 3D-printed enclosure).
