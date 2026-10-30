"""
Patches the web_dashboard HTML inside main.py with a clean, user-friendly UI.
"""
import re

NEW_HTML = r'''    html = """
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
    """'''

with open('main.py', 'r', encoding='utf-8') as f:
    content = f.read()

# Find the html = """ block inside web_dashboard and replace it up to the closing """
# Use regex: from `    html = """` to the closing `    """` then `    return html`
pattern = re.compile(
    r'(    html = """).*?(    """)\s*\n(\s*return html)',
    re.DOTALL
)

replacement = NEW_HTML + '\n    return html'
new_content, n = pattern.subn(replacement, content, count=1)

if n == 0:
    print("ERROR: pattern not matched!")
else:
    with open('main.py', 'w', encoding='utf-8') as f:
        f.write(new_content)
    print(f"SUCCESS: replaced {n} block(s).")
