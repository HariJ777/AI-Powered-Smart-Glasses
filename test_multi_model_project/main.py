"""
Main Flask Application with Smart Routing (Solution 2 + 3)
- Solution 2: Smart routing (load only 1 model per request)
- Solution 3: Groq API for LLMs (zero local RAM for LLaVA + Llama)
"""
from flask import Flask, request, jsonify
import base64
import json
import logging
import time
import psutil
import os
import threading
from werkzeug.serving import run_simple

from models import vision_models, language_models, speech_models
import config

# Setup logging
logging.basicConfig(
    level=config.LOG_LEVEL,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

app = Flask(__name__)

# Request counter for rate limiting
request_count = {"groq": 0, "local": 0}
start_time = time.time()

# Cache the latest capture and AI result for the Web Dashboard
capture_count = 0
current_target_language = "Hindi"
latest_capture = {
    "capture_id": 0,
    "image_base64": "",
    "result": "Press the tactile button on your glasses to capture a frame. The live HUD will stream the image and speak the AI insight aloud in real time.",
    "mode": "STANDBY",
    "timestamp": time.strftime("%H:%M:%S"),
    "response_time": 0,
    "target_language": "Hindi"
}
capture_history = []


mirror_camera_feed = True  # Auto-correct horizontal camera inversion


@app.route('/set-language', methods=['POST'])
def set_language():
    """Update active translation target language"""
    global current_target_language
    data = request.json or {}
    lang = data.get('language', 'Hindi')
    current_target_language = lang
    latest_capture["target_language"] = current_target_language
    logger.info(f"Target translation language updated to: {current_target_language}")
    return jsonify({"status": "success", "target_language": current_target_language})


@app.route('/toggle-mirror', methods=['POST', 'GET'])
def toggle_mirror():
    """Toggle horizontal un-mirroring for camera feed"""
    global mirror_camera_feed
    if request.method == 'POST' and request.json and 'mirror' in request.json:
        mirror_camera_feed = bool(request.json['mirror'])
    else:
        mirror_camera_feed = not mirror_camera_feed
    logger.info(f"Camera horizontal un-mirror set to: {mirror_camera_feed}")
    return jsonify({"status": "success", "mirror": mirror_camera_feed})


def get_memory_stats():
    """Get current memory and GPU usage"""
    process = psutil.Process(os.getpid())
    memory_info = process.memory_info()
    
    try:
        import torch
        gpu_memory = torch.cuda.memory_allocated() / (1024**3)  # Convert to GB
    except:
        gpu_memory = 0
    
    return {
        "rss_mb": memory_info.rss / (1024**2),  # Resident Set Size in MB
        "vms_mb": memory_info.vms / (1024**2),  # Virtual Memory Size in MB
        "gpu_gb": round(gpu_memory, 3),
        "cpu_percent": process.cpu_percent(interval=0.1)
    }


@app.route('/', methods=['GET'])
def web_dashboard():
    """Live web viewer to see the smart glasses picture and AI result in real-time"""
    html = """
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Smart Glasses — Live View</title>
      <link rel="preconnect" href="https://fonts.googleapis.com">
      <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
      <style>
        *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

        body {
          font-family: 'Inter', sans-serif;
          background: #0f1117;
          color: #e8eaf0;
          min-height: 100vh;
          padding: 20px;
        }

        /* TOP BAR */
        .topbar {
          display: flex;
          align-items: center;
          justify-content: space-between;
          margin-bottom: 24px;
          flex-wrap: wrap;
          gap: 12px;
        }
        .app-title {
          display: flex;
          align-items: center;
          gap: 10px;
          font-size: 20px;
          font-weight: 700;
          color: #fff;
        }
        .app-title .sub {
          font-size: 12px;
          color: #6b7280;
          font-weight: 400;
          margin-top: 2px;
        }
        .status-badge {
          display: flex;
          align-items: center;
          gap: 7px;
          background: #1a2230;
          border: 1px solid #2d3748;
          border-radius: 30px;
          padding: 6px 14px;
          font-size: 13px;
          color: #a0aec0;
        }
        .dot {
          width: 8px; height: 8px;
          border-radius: 50%;
          background: #22c55e;
          box-shadow: 0 0 6px #22c55e;
          animation: blink 2s infinite;
        }
        @keyframes blink {
          0%, 100% { opacity: 1; }
          50%       { opacity: 0.3; }
        }

        /* GRID */
        .grid {
          display: grid;
          grid-template-columns: 1fr 1fr;
          gap: 20px;
          max-width: 1100px;
          margin: 0 auto;
        }
        @media (max-width: 800px) { .grid { grid-template-columns: 1fr; } }

        /* CARD */
        .card {
          background: #181d2a;
          border: 1px solid #252d3d;
          border-radius: 16px;
          padding: 20px;
        }
        .card-title {
          font-size: 13px;
          font-weight: 600;
          color: #6b7280;
          text-transform: uppercase;
          letter-spacing: 0.8px;
          margin-bottom: 14px;
        }

        /* CAMERA */
        .camera-frame {
          background: #0a0d14;
          border-radius: 12px;
          overflow: hidden;
          min-height: 240px;
          display: flex;
          align-items: center;
          justify-content: center;
        }
        #cameraImg {
          width: 100%;
          max-height: 360px;
          object-fit: contain;
          display: none;
          border-radius: 10px;
        }
        .no-image {
          text-align: center;
          padding: 40px 20px;
        }
        .no-image .big-icon { font-size: 48px; margin-bottom: 12px; opacity: 0.3; }
        .no-image p { font-size: 14px; color: #4b5563; }
        .cam-controls {
          display: flex;
          align-items: center;
          justify-content: space-between;
          margin-top: 12px;
        }
        .ts-label { font-size: 12px; color: #4b5563; }
        .btn-sm {
          background: #252d3d;
          border: 1px solid #2d3748;
          color: #9ca3af;
          padding: 5px 12px;
          border-radius: 8px;
          font-size: 12px;
          font-weight: 500;
          cursor: pointer;
          transition: all 0.2s;
          font-family: inherit;
        }
        .btn-sm:hover { background: #2d3748; color: #e5e7eb; }

        /* AI RESULT */
        .mode-row { display: flex; gap: 8px; margin-bottom: 14px; flex-wrap: wrap; }
        .mode-badge {
          font-size: 12px;
          font-weight: 600;
          padding: 4px 12px;
          border-radius: 20px;
          border: 1px solid #252d3d;
          color: #6b7280;
          background: #0f1117;
        }
        .mode-badge.active-scene { background: #1e3a5f; border-color: #2563eb; color: #60a5fa; }
        .mode-badge.active-ocr   { background: #14342b; border-color: #059669; color: #34d399; }
        .mode-badge.active-trans { background: #3b1f47; border-color: #9333ea; color: #c084fc; }

        .result-box {
          background: #0f1117;
          border: 1px solid #252d3d;
          border-radius: 12px;
          padding: 20px;
          min-height: 140px;
          font-size: 17px;
          line-height: 1.65;
          color: #f1f5f9;
          font-weight: 500;
          transition: border-color 0.3s, box-shadow 0.3s;
          word-break: break-word;
        }
        .result-box.flash {
          border-color: #3b82f6;
          box-shadow: 0 0 0 3px rgba(59, 130, 246, 0.12);
        }
        .trans-native { font-size: 20px; font-weight: 600; color: #e2e8f0; margin-bottom: 8px; }
        .trans-roman  { font-size: 14px; color: #9ca3af; font-style: italic; }

        .oled-chip {
          margin-top: 10px;
          background: #1a2230;
          border: 1px solid #2d3748;
          border-radius: 8px;
          padding: 8px 12px;
          font-family: 'Courier New', monospace;
          font-size: 12px;
          color: #22c55e;
          display: flex;
          align-items: center;
          gap: 8px;
        }
        .oled-chip .label { color: #4b5563; font-size: 10px; white-space: nowrap; }

        .stats-row { display: flex; gap: 10px; flex-wrap: wrap; margin-top: 14px; }
        .stat-chip {
          background: #0f1117;
          border: 1px solid #252d3d;
          border-radius: 8px;
          padding: 6px 12px;
          font-size: 12px;
          color: #6b7280;
          display: flex;
          gap: 5px;
        }
        .stat-chip span { color: #e5e7eb; font-weight: 600; }

        /* CONTROLS */
        .controls-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
        .ctrl-block label { display: block; font-size: 12px; color: #6b7280; margin-bottom: 6px; font-weight: 500; }
        .select-box {
          width: 100%;
          background: #0f1117;
          border: 1px solid #252d3d;
          color: #e5e7eb;
          padding: 8px 12px;
          border-radius: 10px;
          font-size: 14px;
          font-family: inherit;
          outline: none;
          cursor: pointer;
        }
        .select-box:focus { border-color: #3b82f6; }
        .select-box option { background: #181d2a; }

        .btn-voice {
          width: 100%;
          padding: 9px 12px;
          border-radius: 10px;
          border: 1px solid #2d3748;
          font-size: 14px;
          font-weight: 600;
          cursor: pointer;
          font-family: inherit;
          transition: all 0.2s;
          display: flex;
          align-items: center;
          justify-content: center;
          gap: 7px;
        }
        .btn-voice.on  { background: #14532d; border-color: #16a34a; color: #4ade80; }
        .btn-voice.off { background: #1f2937; border-color: #374151; color: #6b7280; }

        /* HISTORY */
        .history-strip {
          display: flex;
          gap: 10px;
          overflow-x: auto;
          padding-bottom: 4px;
          scrollbar-width: thin;
          scrollbar-color: #252d3d transparent;
        }
        .hist-card {
          flex-shrink: 0;
          width: 120px;
          background: #0f1117;
          border: 1px solid #252d3d;
          border-radius: 10px;
          overflow: hidden;
          cursor: pointer;
          transition: border-color 0.2s;
        }
        .hist-card:hover { border-color: #3b82f6; }
        .hist-card img { width: 100%; height: 70px; object-fit: cover; display: block; }
        .hist-card .hist-meta { padding: 5px 7px; }
        .hist-card .hist-mode { font-size: 10px; color: #60a5fa; font-weight: 600; text-transform: uppercase; }
        .hist-card .hist-time { font-size: 10px; color: #4b5563; margin-top: 1px; }

        /* SPEAKING INDICATOR */
        .speaking-ring {
          display: none;
          align-items: center;
          gap: 5px;
          font-size: 12px;
          color: #22c55e;
        }
        .speaking-ring.show { display: flex; }
        .bar { width: 3px; border-radius: 2px; background: #22c55e; }
        .bar:nth-child(1) { animation: grow 0.4s 0s    infinite alternate; }
        .bar:nth-child(2) { animation: grow 0.4s 0.1s  infinite alternate; }
        .bar:nth-child(3) { animation: grow 0.4s 0.2s  infinite alternate; }
        .bar:nth-child(4) { animation: grow 0.4s 0.1s  infinite alternate; }
        @keyframes grow { from { height: 4px; } to { height: 14px; } }
      </style>
    </head>
    <body>

      <!-- TOP BAR -->
      <div class="topbar">
        <div class="app-title">
          <span style="font-size:26px">🕶️</span>
          <div>
            Smart Glasses
            <div class="sub">AI Live View</div>
          </div>
        </div>
        <div style="display:flex; align-items:center; gap:10px; flex-wrap:wrap;">
          <div class="speaking-ring" id="speakRing">
            <div class="bar"></div><div class="bar"></div>
            <div class="bar"></div><div class="bar"></div>
            Speaking…
          </div>
          <div class="status-badge">
            <div class="dot"></div>
            <span>Connected</span>
          </div>
        </div>
      </div>

      <!-- MAIN GRID -->
      <div class="grid">

        <!-- CAMERA -->
        <div class="card">
          <div class="card-title">📷 Camera</div>
          <div class="camera-frame">
            <img id="cameraImg" alt="Captured frame">
            <div class="no-image" id="noImg">
              <div class="big-icon">📡</div>
              <p>Waiting for a capture…<br>Press the button on your glasses.</p>
            </div>
          </div>
          <div class="cam-controls">
            <span class="ts-label" id="camTs">—</span>
            <button class="btn-sm" id="mirrorBtn" onclick="toggleMirror()">🪞 Mirror Fix: ON</button>
          </div>
        </div>

        <!-- AI RESULT -->
        <div class="card">
          <div class="card-title">🤖 AI Result</div>
          <div class="mode-row">
            <span class="mode-badge" id="modeBadgeScene">👁 Scene</span>
            <span class="mode-badge" id="modeBadgeOCR">📝 OCR</span>
            <span class="mode-badge" id="modeBadgeTrans">🌐 Translate</span>
          </div>
          <div class="result-box" id="resultBox">
            Press the button on your glasses to capture a frame.
          </div>
          <div class="oled-chip" id="oledChip" style="display:none;">
            <span class="label">OLED →</span>
            <span id="oledText"></span>
          </div>
          <div class="stats-row">
            <div class="stat-chip">⚡ <span id="latencyVal">—</span></div>
            <div class="stat-chip">🕐 <span id="timeVal">—</span></div>
          </div>
        </div>

        <!-- CONTROLS -->
        <div class="card">
          <div class="card-title">⚙️ Controls</div>
          <div class="controls-grid">
            <div class="ctrl-block">
              <label>🌐 Translate to</label>
              <select id="langSelect" class="select-box" onchange="changeLanguage(this.value)">
                <option value="Hindi">Hindi (हिंदी)</option>
                <option value="Kannada">Kannada (ಕನ್ನಡ)</option>
                <option value="Tamil">Tamil (தமிழ்)</option>
                <option value="Telugu">Telugu (తెలుగు)</option>
                <option value="Spanish">Spanish</option>
                <option value="French">French</option>
                <option value="German">German</option>
                <option value="Japanese">Japanese (日本語)</option>
                <option value="English">English</option>
              </select>
            </div>
            <div class="ctrl-block">
              <label>🔊 Voice</label>
              <button id="voiceBtn" class="btn-voice on" onclick="toggleVoice()">
                🔊 Voice On
              </button>
            </div>
          </div>
        </div>

        <!-- HISTORY -->
        <div class="card">
          <div class="card-title">
            🕘 Recent Captures
            <span id="histCount" style="color:#3b82f6; font-size:11px; margin-left:4px;"></span>
          </div>
          <div class="history-strip" id="histStrip">
            <div style="color:#374151; font-size:13px; padding:20px 0;">No captures yet.</div>
          </div>
        </div>

      </div>

      <script>
        let voiceEnabled = true;
        let lastCaptureId = 0;
        let historyList = [];

        const LANG_CODE = {
          Hindi:'hi-IN', Kannada:'kn-IN', Tamil:'ta-IN', Telugu:'te-IN',
          Spanish:'es-ES', French:'fr-FR', German:'de-DE', Japanese:'ja-JP', English:'en-US'
        };

        function toggleVoice() {
          voiceEnabled = !voiceEnabled;
          const btn = document.getElementById('voiceBtn');
          if (voiceEnabled) {
            btn.className = 'btn-voice on';
            btn.innerHTML = '🔊 Voice On';
          } else {
            btn.className = 'btn-voice off';
            btn.innerHTML = '🔇 Voice Off';
            window.speechSynthesis && window.speechSynthesis.cancel();
          }
        }

        function speak(text, langKey) {
          if (!voiceEnabled || !text || !window.speechSynthesis) return;
          window.speechSynthesis.cancel();
          const utt = new SpeechSynthesisUtterance(text);
          const code = LANG_CODE[langKey] || 'en-US';
          utt.lang = code;
          utt.rate = 1.0;
          const voices = window.speechSynthesis.getVoices();
          const match = voices.find(v => v.lang.replace('_','-').startsWith(code.slice(0,2)));
          if (match) utt.voice = match;
          const ring = document.getElementById('speakRing');
          utt.onstart = () => ring.classList.add('show');
          utt.onend = utt.onerror = () => ring.classList.remove('show');
          window.speechSynthesis.speak(utt);
        }

        async function toggleMirror() {
          const res = await fetch('/toggle-mirror', { method:'POST' });
          const d = await res.json();
          document.getElementById('mirrorBtn').textContent =
            d.mirror ? '🪞 Mirror Fix: ON' : '🪞 Mirror Fix: OFF';
        }

        async function changeLanguage(lang) {
          await fetch('/set-language', {
            method:'POST',
            headers:{'Content-Type':'application/json'},
            body: JSON.stringify({language: lang})
          });
        }

        function setModeBadge(modeStr) {
          document.getElementById('modeBadgeScene').className = 'mode-badge';
          document.getElementById('modeBadgeOCR').className   = 'mode-badge';
          document.getElementById('modeBadgeTrans').className  = 'mode-badge';
          const m = modeStr.toUpperCase();
          if (m.includes('SCENE'))     document.getElementById('modeBadgeScene').className = 'mode-badge active-scene';
          else if (m.includes('OCR'))  document.getElementById('modeBadgeOCR').className   = 'mode-badge active-ocr';
          else if (m.includes('TRAN')) document.getElementById('modeBadgeTrans').className  = 'mode-badge active-trans';
        }

        function loadItem(idx) {
          const item = historyList[idx];
          if (!item) return;
          if (item.image_base64) {
            const img = document.getElementById('cameraImg');
            img.src = 'data:image/jpeg;base64,' + item.image_base64;
            img.style.display = 'block';
            document.getElementById('noImg').style.display = 'none';
          }
          document.getElementById('resultBox').textContent = item.result || '—';
          document.getElementById('timeVal').textContent   = item.timestamp || '—';
          setModeBadge(item.mode || '');
        }

        function renderHistory(list) {
          historyList = list;
          const strip = document.getElementById('histStrip');
          if (!list || list.length === 0) {
            strip.innerHTML = '<div style="color:#374151;font-size:13px;padding:20px 0">No captures yet.</div>';
            document.getElementById('histCount').textContent = '';
            return;
          }
          document.getElementById('histCount').textContent = list.length + ' frames';
          strip.innerHTML = list.map((item, i) => `
            <div class="hist-card" onclick="loadItem(${i})">
              <img src="data:image/jpeg;base64,${item.image_base64}" alt="">
              <div class="hist-meta">
                <div class="hist-mode">${item.mode || 'AI'}</div>
                <div class="hist-time">${item.timestamp || ''}</div>
              </div>
            </div>
          `).join('');
        }

        async function poll() {
          try {
            const res = await fetch('/latest');
            const d = await res.json();

            if (d.image_base64 && d.image_base64.length > 50) {
              const img = document.getElementById('cameraImg');
              if (img.getAttribute('src') !== 'data:image/jpeg;base64,' + d.image_base64)
                img.src = 'data:image/jpeg;base64,' + d.image_base64;
              img.style.display = 'block';
              document.getElementById('noImg').style.display = 'none';
            }

            if (d.result) {
              const box = document.getElementById('resultBox');
              const isNew = d.capture_id > lastCaptureId;
              if (d.mode && d.mode.toUpperCase().includes('TRANSLATE') && d.native && d.native !== d.result) {
                box.innerHTML = `
                  <div class="trans-native">${d.native}</div>
                  <div class="trans-roman">${d.romanized || ''}</div>
                `;
              } else {
                box.textContent = d.result;
              }
              if (isNew) {
                box.classList.add('flash');
                setTimeout(() => box.classList.remove('flash'), 800);
              }
            }

            if (d.oled_text) {
              document.getElementById('oledText').textContent  = d.oled_text;
              document.getElementById('oledChip').style.display = 'flex';
            }

            setModeBadge(d.mode || '');
            document.getElementById('latencyVal').textContent = (d.response_time || '—') + 's';
            document.getElementById('timeVal').textContent    = d.timestamp || '—';
            document.getElementById('camTs').textContent      = d.timestamp || '—';

            if (d.target_language) {
              const sel = document.getElementById('langSelect');
              if (document.activeElement !== sel) sel.value = d.target_language;
            }
            if (d.mirror !== undefined)
              document.getElementById('mirrorBtn').textContent =
                d.mirror ? '🪞 Mirror Fix: ON' : '🪞 Mirror Fix: OFF';

            if (d.capture_id && d.capture_id > lastCaptureId && lastCaptureId !== 0)
              setTimeout(() => speak(d.native || d.result || '', d.target_language || 'English'), 200);
            if (d.capture_id) lastCaptureId = d.capture_id;

            if (d.history) renderHistory(d.history);
          } catch(e) {}
        }

        setInterval(poll, 1000);
        poll();

        document.body.addEventListener('click', () => {
          if (window.speechSynthesis && window.speechSynthesis.paused)
            window.speechSynthesis.resume();
        }, { once: true });
      </script>
    </body>
    </html>
    """
    return html


@app.route('/latest', methods=['GET'])
def get_latest():
    """Returns the latest captured photo, AI result, and capture history"""
    return jsonify({
        **latest_capture,
        "mirror": mirror_camera_feed,
        "history": capture_history,
        "system": {
            "memory": get_memory_stats(),
            "uptime": round(time.time() - start_time)
        }
    })


@app.route('/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    memory = get_memory_stats()
    elapsed_time = time.time() - start_time
    
    return jsonify({
        "status": "healthy",
        "uptime_seconds": round(elapsed_time),
        "memory": memory,
        "requests_processed": request_count,
        "free_tier_requests_used": request_count["groq"],
        "free_tier_limit": 14400
    })


@app.route('/analyze-image', methods=['POST'])
def analyze_image():
    """
    Smart router for image analysis
    Routes to: LLaVA (Groq API) or EasyOCR (local)
    
    Request body:
    {
        "image_base64": "...",
        "analysis_type": "description" | "ocr",
        "prompt": "optional custom prompt"
    }
    """
    logger.info("=" * 50)
    logger.info("REQUEST: /analyze-image")
    
    memory_before = get_memory_stats()
    logger.info(f"Memory before: {memory_before}")
    
    try:
        data = request.json
        image_base64 = data.get('image_base64')
        analysis_type = data.get('analysis_type', 'description')  # description or ocr
        custom_prompt = data.get('prompt')
        
        if not image_base64:
            return jsonify({"error": "image_base64 required"}), 400
        
        # Check Groq request limit
        if analysis_type == 'description' and request_count["groq"] >= 14400:
            return jsonify({
                "error": "Groq free tier limit reached (14,400 requests/day)",
                "status": "rate_limit_exceeded"
            }), 429
        
        start_time_req = time.time()
        
        # Auto un-mirror camera frames if enabled so both HUD display and OCR are oriented correctly
        if mirror_camera_feed and image_base64:
            try:
                from PIL import Image, ImageOps
                import io
                img_bytes = base64.b64decode(image_base64)
                pil_img = Image.open(io.BytesIO(img_bytes))
                pil_img = ImageOps.mirror(pil_img)
                buf = io.BytesIO()
                pil_img.save(buf, format="JPEG", quality=90)
                image_base64 = base64.b64encode(buf.getvalue()).decode()
            except Exception as mirror_err:
                logger.warning(f"Mirror transformation bypassed: {mirror_err}")
        
        # SMART ROUTING: Choose appropriate model
        if analysis_type == 'description':
            logger.info("Route: Neural Vision Core - Image description")
            result = vision_models.analyze_image_with_gemini(
                image_base64,
                prompt=custom_prompt or "Describe what you see in maximum 12 words. Be direct and concise. Do not use filler like 'The image shows'."
            )
            request_count["groq"] += 1
        elif analysis_type == 'ocr':
            logger.info("Route: Neural Vision Core - High-Precision OCR Text Extraction")
            ocr_prompt = custom_prompt or (
                "Accurately read and transcribe ALL readable words, brand names, numbers, and signs visible in this image. "
                "Output ONLY the extracted text directly. Preserve line breaks between lines. "
                "Do NOT include conversational filler, explanations, or introductory phrases."
            )
            result = vision_models.analyze_image_with_gemini(
                image_base64,
                prompt=ocr_prompt
            )
            # Safe failover to local EasyOCR if cloud core is unavailable
            if result.get("status") != "success":
                logger.warning("Neural OCR failed, falling back to local EasyOCR...")
                result = vision_models.extract_text_with_easyocr(image_base64)
                request_count["local"] += 1
            else:
                request_count["groq"] += 1
        elif analysis_type == 'translate':
            target_lang = data.get('target_language', current_target_language)
            logger.info(f"Route: Multimodal Translation -> {target_lang}")
            trans_prompt = custom_prompt or (
                f"You are an AI Smart Glasses multimodal translator.\n"
                f"Translate any text, signs, or scene visible in this image into {target_lang}.\n"
                f"Because the smart glasses hardware OLED screen only supports standard ASCII (English alphabet) characters, you must provide:\n"
                f"1. Native: The translation in native {target_lang} script (for speech synthesis and browser display).\n"
                f"2. Romanized: The phonetic pronunciation in English/Latin alphabet (ASCII only, max 10 words) for the OLED screen.\n"
                f"Respond ONLY with valid JSON in this format:\n"
                f'{{"native": "native script translation", "romanized": "English alphabet pronunciation"}}'
            )
            result = vision_models.analyze_image_with_gemini(
                image_base64,
                prompt=trans_prompt
            )
            raw_res = result.get("result", "")
            native_val = raw_res
            romanized_val = raw_res
            try:
                import json
                clean_json = raw_res.strip().replace("```json", "").replace("```", "").strip()
                parsed = json.loads(clean_json)
                native_val = parsed.get("native", raw_res)
                romanized_val = parsed.get("romanized", raw_res)
            except Exception:
                pass
            
            lang_tag = target_lang[:2].upper()
            oled_display = f"[{lang_tag}] {romanized_val}"
            web_display = f"{native_val}\n({romanized_val})" if native_val != romanized_val else native_val
            
            result["result"] = oled_display     # Pure ASCII for the glasses OLED screen!
            result["oled_text"] = oled_display  # Dedicated OLED field
            result["native"] = native_val       # For Web TTS audio speech in native accent
            result["romanized"] = romanized_val
            result["display"] = web_display     # For Web HUD text box
            request_count["groq"] += 1
        else:
            return jsonify({"error": f"Unknown analysis_type: {analysis_type}"}), 400
        
        elapsed = time.time() - start_time_req
        
        # Update latest capture cache & history for Web Dashboard
        global capture_count
        capture_count += 1
        res_text = result.get("display") or result.get("result") or result.get("text") or "No text recognized"
        if analysis_type == 'description':
            mode_label = "SCENE VISION"
        elif analysis_type == 'ocr':
            mode_label = "TEXT OCR"
        elif analysis_type == 'translate':
            mode_label = f"TRANSLATE ({current_target_language.upper()})"
        else:
            mode_label = analysis_type.upper()

        latest_capture["capture_id"] = capture_count
        latest_capture["image_base64"] = image_base64
        latest_capture["result"] = res_text
        latest_capture["native"] = result.get("native", "")
        latest_capture["oled_text"] = result.get("oled_text", "")
        latest_capture["mode"] = mode_label
        latest_capture["response_time"] = round(elapsed, 2)
        latest_capture["timestamp"] = time.strftime("%H:%M:%S")
        latest_capture["target_language"] = current_target_language

        capture_history.insert(0, {
            "id": capture_count,
            "image_base64": image_base64,
            "result": res_text,
            "native": latest_capture["native"],
            "mode": mode_label,
            "timestamp": latest_capture["timestamp"],
            "response_time": latest_capture["response_time"]
        })
        if len(capture_history) > 10:
            capture_history.pop()

        memory_after = get_memory_stats()
        logger.info(f"Memory after: {memory_after}")
        logger.info(f"Response time: {elapsed:.2f}s")
        logger.info("=" * 50)
        
        return jsonify({
            **result,
            "response_time_seconds": round(elapsed, 3),
            "memory_used_mb": round(memory_after['rss_mb'] - memory_before['rss_mb'], 1),
            "gpu_memory_gb": memory_after['gpu_gb']
        })
    
    except Exception as e:
        logger.error(f"Error in /analyze-image: {str(e)}")
        memory_after = get_memory_stats()
        return jsonify({
            "error": str(e),
            "memory_mb": memory_after['rss_mb']
        }), 500


@app.route('/ask', methods=['POST'])
def ask_question():
    """
    Smart router for Q&A
    Routes to: Llama 3.2 (Groq API)
    
    Request body:
    {
        "question": "What is AI?",
        "context": "optional context"
    }
    """
    logger.info("=" * 50)
    logger.info("REQUEST: /ask")
    
    memory_before = get_memory_stats()
    logger.info(f"Memory before: {memory_before}")
    
    try:
        data = request.json
        question = data.get('question')
        context = data.get('context', '')
        
        if not question:
            return jsonify({"error": "question required"}), 400
        
        # Check rate limit
        if request_count["groq"] >= 14400:
            return jsonify({
                "error": "Groq free tier limit reached",
                "status": "rate_limit_exceeded"
            }), 429
        
        start_time_req = time.time()
        
        logger.info("Route: Llama 3.2 (Groq API) - Q&A")
        result = language_models.answer_question(question, context)
        request_count["groq"] += 1
        
        elapsed = time.time() - start_time_req
        
        memory_after = get_memory_stats()
        logger.info(f"Memory after: {memory_after}")
        logger.info(f"Response time: {elapsed:.2f}s")
        logger.info("=" * 50)
        
        return jsonify({
            **result,
            "response_time_seconds": round(elapsed, 3),
            "memory_used_mb": round(memory_after['rss_mb'] - memory_before['rss_mb'], 1),
            "gpu_memory_gb": memory_after['gpu_gb']
        })
    
    except Exception as e:
        logger.error(f"Error in /ask: {str(e)}")
        memory_after = get_memory_stats()
        return jsonify({
            "error": str(e),
            "memory_mb": memory_after['rss_mb']
        }), 500


@app.route('/transcribe', methods=['POST'])
def transcribe_audio():
    """
    Smart router for speech-to-text
    Routes to: Whisper-tiny (Local)
    
    Request body:
    {
        "audio_base64": "..."
    }
    """
    logger.info("=" * 50)
    logger.info("REQUEST: /transcribe")
    
    memory_before = get_memory_stats()
    logger.info(f"Memory before: {memory_before}")
    
    try:
        data = request.json
        audio_base64 = data.get('audio_base64')
        
        if not audio_base64:
            return jsonify({"error": "audio_base64 required"}), 400
        
        start_time_req = time.time()
        
        logger.info("Route: Whisper-tiny (Local) - Speech-to-text")
        result = speech_models.transcribe_audio(audio_base64)
        request_count["local"] += 1
        
        elapsed = time.time() - start_time_req
        
        memory_after = get_memory_stats()
        logger.info(f"Memory after: {memory_after}")
        logger.info(f"Response time: {elapsed:.2f}s")
        logger.info("=" * 50)
        
        return jsonify({
            **result,
            "response_time_seconds": round(elapsed, 3),
            "memory_used_mb": round(memory_after['rss_mb'] - memory_before['rss_mb'], 1),
            "gpu_memory_gb": memory_after['gpu_gb']
        })
    
    except Exception as e:
        logger.error(f"Error in /transcribe: {str(e)}")
        memory_after = get_memory_stats()
        return jsonify({
            "error": str(e),
            "memory_mb": memory_after['rss_mb']
        }), 500


@app.route('/status', methods=['GET'])
def status():
    """Get detailed status of all models and resources"""
    memory = get_memory_stats()
    
    return jsonify({
        "status": "running",
        "memory": memory,
        "requests": request_count,
        "groq_free_tier_remaining": 14400 - request_count["groq"],
        "models": {
            "llava": "Available (Groq API)",
            "llama_3_2": "Available (Groq API)",
            "easyocr": "Available (Local, lazy-loaded)",
            "whisper_tiny": "Available (Local, lazy-loaded)"
        }
    })


def run_https_listener():
    try:
        logger.info("Starting secondary HTTPS server on https://0.0.0.0:5001 for mobile browsers...")
        run_simple('0.0.0.0', 5001, app, ssl_context='adhoc', threaded=True)
    except Exception as e:
        logger.warning(f"Could not start HTTPS server on port 5001: {e}")


if __name__ == '__main__':
    logger.info("Starting Flask server with smart routing...")
    logger.info("Solution 2 + 3: Smart routing + Groq API")
    logger.info(f"API Key configured: {bool(config.GROQ_API_KEY)}")
    
    # Start secondary HTTPS server on port 5001 for mobile phones
    https_thread = threading.Thread(target=run_https_listener, daemon=True)
    https_thread.start()

    # Primary HTTP on port 5000 for ESP32-CAM and desktop
    app.run(
        host='0.0.0.0',
        port=config.FLASK_PORT,
        debug=False
    )
