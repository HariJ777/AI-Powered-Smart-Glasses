"""Report content chapters 7-10"""

def write_ch7(doc, h1, h2, body, bullet):
    h1(doc, 'CHAPTER 7: EXPECTED OUTCOME', new_page=True)
    body(doc, 'Based on the system design, implementation, and testing conducted during the development phase, the following outcomes are expected and have been partially validated through Phase 1 testing:')

    h2(doc, '7.1 Functional Outcomes')
    bullet(doc, 'A working smart glasses prototype that captures images through the ESP32-CAM OV2640 camera module and wirelessly transmits them to the Flask backend server for AI processing. The processed AI response is displayed on the SSD1306 OLED screen and can be spoken through the speaker.')
    bullet(doc, 'A Flask backend with Smart Routing that correctly distributes requests between cloud AI models (Groq API - Llama 4 Scout for vision, Llama 3.3 70B for language) and local models (EasyOCR for OCR, Whisper-tiny for speech recognition).')
    bullet(doc, 'Five fully functional AI capabilities accessible through the glasses: (1) Scene Description - the camera captures an image and the AI provides a detailed textual description, (2) Text Extraction - the OCR system reads printed text from signs, books, and labels, (3) Speech-to-Text - the microphone captures speech and transcribes it, (4) Question Answering - the user asks a question and receives an AI-generated answer, and (5) Translation - text is translated between supported languages.')

    h2(doc, '7.2 Performance Outcomes')
    body(doc, 'The following performance metrics have been verified through testing:')
    bullet(doc, 'Vision (Scene Description): 0.44 seconds average response time via Groq API')
    bullet(doc, 'Question Answering: 0.54 seconds average response time via Groq API')
    bullet(doc, 'Translation: 0.44 seconds average response time via Groq API')
    bullet(doc, 'OCR (Text Extraction): 1.82 seconds average response time using local EasyOCR')
    bullet(doc, 'Speech-to-Text: 0.37 seconds average response time using local Whisper-tiny')
    bullet(doc, 'Peak Memory Usage: Under 600 MB GPU RAM (verified through memory management tests)')
    bullet(doc, 'API Availability: 14,400 free requests per day through Groq free tier')

    h2(doc, '7.3 Cost Outcome')
    body(doc, 'The total hardware cost of the prototype is Rs.1,407 (approximately USD 17), which is approximately 60 times cheaper than Google Glass and 18 times cheaper than Meta Ray-Ban Smart Glasses. The operational cost is zero since the system uses the Groq free tier for cloud AI processing and open-source models for local processing.')

def write_ch8(doc, h1, h2, body, bullet):
    h1(doc, 'CHAPTER 8: ADVANTAGES AND FUTURE SCOPE', new_page=True)
    h2(doc, '8.1 Advantages')
    bullet(doc, 'Extremely Low Cost: The entire hardware prototype costs under Rs.1,500, making it accessible to users in developing countries and low-income communities.')
    bullet(doc, 'Zero Operational Cost: By utilizing the Groq free-tier API (14,400 daily requests) and open-source local models, the system incurs no recurring subscription or cloud computing charges.')
    bullet(doc, 'Memory Efficient: The Smart Routing architecture keeps peak GPU memory usage under 600 MB, enabling deployment on entry-level consumer hardware such as laptops with NVIDIA GTX 1650 or RTX 3050 GPUs.')
    bullet(doc, 'Multi-Modal AI Capabilities: The system integrates computer vision, natural language processing, optical character recognition, and speech recognition in a single wearable device.')
    bullet(doc, 'Fast Response Times: All AI tasks complete within 2 seconds, providing near-real-time assistance to the user.')
    bullet(doc, 'Scalable Architecture: The Flask backend can serve multiple smart glasses clients simultaneously, and the cloud-local workload distribution can be adjusted based on network availability.')
    bullet(doc, 'Open Source Stack: The entire software stack uses open-source technologies (Python, Flask, EasyOCR, Whisper, PyTorch), eliminating vendor lock-in and enabling community contributions.')
    bullet(doc, 'Dual Output Modes: AI responses are delivered through both visual (OLED display) and auditory (speaker with TTS) channels, accommodating users with varying degrees of visual impairment.')

    h2(doc, '8.2 Future Scope')
    bullet(doc, 'Real-Time Object Detection: Integration of lightweight object detection models such as YOLOv8-nano for continuous real-time object identification and obstacle warning.')
    bullet(doc, 'GPS Navigation: Adding a GPS module (NEO-6M) and integrating with mapping APIs to provide turn-by-turn navigation assistance for visually impaired users.')
    bullet(doc, 'Multi-Language Voice Commands: Extending Whisper integration to support voice commands in regional Indian languages including Kannada, Hindi, and Tamil.')
    bullet(doc, 'Custom PCB Design: Replacing the breadboard prototype with a custom-designed PCB to reduce size, weight, and improve reliability for daily use.')
    bullet(doc, 'Mobile App Integration: Developing a companion Android/iOS application that provides remote configuration, usage analytics, and caregiver alerts.')
    bullet(doc, 'Battery Optimization: Implementing deep sleep modes and wake-on-button interrupt to extend battery life to 8-10 hours.')
    bullet(doc, 'Edge AI Processing: Porting lightweight models to run directly on the ESP32-S3 (with PSRAM) to enable basic offline functionality without server connectivity.')

def write_ch9(doc, h1, body):
    h1(doc, 'CHAPTER 9: CONCLUSION', new_page=True)
    body(doc, 'This project successfully demonstrates that advanced AI capabilities including image understanding, text extraction, speech recognition, and question answering can be delivered through an affordable wearable device costing under Rs.1,500. The AI-powered smart glasses system built using the ESP32-CAM microcontroller provides a practical and cost-effective assistive technology solution for visually impaired individuals.')
    body(doc, 'The hybrid cloud-local architecture implemented through the Flask Smart Router represents the key technical contribution of this project. By offloading computationally intensive vision and language tasks to the Groq free-tier cloud API (Llama 4 Scout 17B for image description, Llama 3.3 70B for question answering and translation) while processing lightweight OCR and speech recognition tasks locally (EasyOCR, Whisper-tiny), the system achieves a practical balance between performance, cost, and resource utilization.')
    body(doc, 'The Smart Routing memory management strategy ensures that only one local AI model occupies GPU memory at any given time, with explicit model unloading after each request. This approach maintains peak RAM usage at approximately 600 MB, making the system compatible with entry-level NVIDIA GPUs such as the GTX 1650 and RTX 3050. Without this optimization, loading all four AI models simultaneously would require over 12 GB of VRAM, rendering the system impractical for consumer hardware.')
    body(doc, 'Comprehensive testing validated the system across three tiers: all 5 AI model tests passed confirming correct functionality, all 3 memory management tests passed confirming efficient resource utilization, and all 5 Flask endpoint tests passed confirming reliable API behavior. The system achieves response times under 2 seconds for all AI tasks, with cloud-based tasks completing in 0.4-0.6 seconds and local tasks completing in 0.4-1.8 seconds.')
    body(doc, 'The project proves that the combination of affordable IoT hardware, free-tier cloud AI services, and intelligent software architecture can democratize access to AI-powered assistive technology. The total prototype cost of Rs.1,407 represents a reduction of over 98% compared to commercial alternatives, making this technology accessible to users in developing countries and underserved communities.')

def write_ch10(doc, h1, body, bullet):
    h1(doc, 'CHAPTER 10: REFERENCES', new_page=True)
    refs = [
        '[1] S. Das, S. Saxena, and N. K. Rout, "Low Cost Smart-Glass using ESP-32," in 2021 IEEE 2nd International Conference on Applied Electromagnetics, Signal Processing, & Communication (AESPC), 2021.',
        '[2] S. S. V. Ramesh Babu, M. Gowtham, G. Jayakrishna, and A. L. S. S. Manasa, "AI-Powered Smart Glasses For The Blind Using OpenCV & Python," in 2025 IEEE Wireless Antenna and Microwave Symposium (WAMS), 2025.',
        '[3] J. Y. Lin, C. C. Yao, C. L. Chiang, and M. C. Chen, "Smart Glasses Application System for Visually Impaired People Based on Deep Learning," in Indo-Taiwan 2nd International Conference on Computing, Analytics and Networks (Indo-Taiwan ICAN), 2020.',
        '[4] A. Radford, J. W. Kim, T. Xu, G. Brockman, C. McLeavey, and I. Sutskever, "Robust Speech Recognition via Large-Scale Weak Supervision," in Proceedings of the 40th International Conference on Machine Learning (ICML), 2023.',
        '[5] H. Touvron, L. Martin, K. Stone, et al., "Llama 2: Open Foundation and Fine-Tuned Chat Models," Meta AI Research, 2023.',
        '[6] JaidedAI, "EasyOCR: Ready-to-Use OCR with 80+ Supported Languages," GitHub, 2020. Available: https://github.com/JaidedAI/EasyOCR',
        '[7] N. Gospodinov and G. Krastev, "Object detection with smart glasses," in 2022 30th Telecommunications Forum (TELFOR), Belgrade, Serbia, Nov. 2022.',
        '[8] P. Sharma, M. Sadasivan, A. S. Babu, and K. T. A. Robert, "Smart Glasses for the Blind: A Real-Time AI-Driven Wearable System for Autonomous Navigation," in 2025 International Conference on Computing and Communications (COMPUTINGCON), Pune, India, Sep. 2025.',
        '[9] U. R, H. P.S., R. N.G., and N. A, "Third Eye - A Smart Wearable Glass With Deep Learning Technology Powered With Artificial Intelligence for Visually Impaired People," in 2023 IEEE International Students\' Conference on Electrical, Electronics and Computer Science (SCEECS), 2023.',
        '[10] J. Chen and X. Ran, "Deep Learning with Edge Computing: A Review," in Proceedings of the IEEE, vol. 107, no. 8, pp. 1655-1674, 2019.',
        '[11] Espressif Systems, "ESP32-CAM Module Datasheet," 2020. Available: https://www.espressif.com',
        '[12] Groq Inc., "Groq API Documentation," 2024. Available: https://console.groq.com/docs',
    ]
    from docx.shared import Pt, Cm
    for ref in refs:
        p = doc.add_paragraph()
        p.alignment = 3  # JUSTIFY
        p.paragraph_format.space_after = Pt(4)
        p.paragraph_format.line_spacing = Pt(16)
        p.paragraph_format.left_indent = Cm(1)
        p.paragraph_format.first_line_indent = Cm(-1)
        from report_helpers import set_run
        r = p.add_run(ref)
        set_run(r, size=11)
