# Quick Start Guide

## ⚡ 5-Minute Quick Start

### Step 1: Get Groq API Key (2 minutes)
1. Visit https://console.groq.com
2. Sign up (free)
3. Go to API Keys section
4. Copy your API key

### Step 2: Setup Project (2 minutes)
```bash
cd c:\Users\harin\Desktop\smart Glasses\test_multi_model_project

# Create .env file
copy .env.example .env

# Edit .env and paste your Groq API key
# Open .env in any editor and replace:
# GROQ_API_KEY=your_groq_api_key_here
```

### Step 3: Install & Run (1 minute)
```bash
# Install dependencies
pip install -r requirements.txt

# Run individual model tests
python tests/test_individual.py

# Then run memory tests
python tests/test_memory.py
```

---

## 📊 Expected Output

### Test Individual Models (5-10 minutes)
```
TEST 1: LLaVA Vision (Groq API)
Status: success
✅ LLaVA test PASSED

TEST 2: Llama 3.2 Q&A (Groq API)
Status: success
Answer: The capital of France is Paris...
✅ Llama 3.2 test PASSED

TEST 3: Llama 3.2 Translation (Groq API)
Status: success
Translation: Hola, ¿cómo estás?
✅ Translation test PASSED

TEST 4: EasyOCR Text Extraction (Local)
Status: success
✅ EasyOCR test PASSED (model loaded)

TEST 5: Whisper-tiny Speech-to-Text (Local)
Status: success
✅ Whisper test PASSED (model loaded)

Passed: 5/5

🎉 All tests passed! Ready for integration.
```

### Memory Tests (10-15 minutes)
```
SCENARIO 1: LLaVA API Call (Groq) - No Local Memory
RAM: ~250 MB (no change)
GPU: 0 GB (API call, no local GPU)
✅ Scenario 1: Expected minimal memory increase

SCENARIO 2: EasyOCR Load + Unload (Local Model)
After Load: ~2500 MB
After Unload: ~250 MB ✅ Memory freed!
✅ Scenario 2: Memory increase on load, decrease on unload

SCENARIO 3: Sequential Operations (Smart Routing Pattern)
Op 1 (LLaVA API): 250 MB
Op 2 (EasyOCR Load): 2500 MB
Op 2 (EasyOCR Unload): 300 MB ✅
Op 3 (Llama Q&A): 300 MB
✅ Scenario 3: Smart routing prevents memory overflow
```

---

## 🚀 Run Flask Server

Once tests pass, start the server:

```bash
python main.py

# Will output:
# Running on http://0.0.0.0:5000
```

### Test with curl

```bash
# Health check
curl http://localhost:5000/health

# Analyze image
curl -X POST http://localhost:5000/analyze-image \
  -H "Content-Type: application/json" \
  -d '{
    "image_base64": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==",
    "analysis_type": "description"
  }'

# Ask question
curl -X POST http://localhost:5000/ask \
  -H "Content-Type: application/json" \
  -d '{"question": "What is AI?"}'
```

---

## ✅ Success Criteria - VALIDATION CHECKLIST

- [ ] Groq API Key configured
- [ ] All models test successfully
- [ ] Memory usage stays under 3 GB
- [ ] Response times under 3 seconds
- [ ] Flask server starts without errors
- [ ] API endpoints respond correctly

---

## 🎯 What Happens Next

Once all tests pass ✅:

1. **Document findings** in test_results.txt
2. **Identify bottlenecks** (if any)
3. **Optimize response times** (if needed)
4. **Integrate into main Smart Glasses project**
   - Adapt for ESP32-CAM input
   - Format output for OLED display
   - Implement battery optimizations

---

## 📝 File Organization

```
test_multi_model_project/
├── main.py                    ← Flask app (run this)
├── config.py                  ← Configuration
├── requirements.txt           ← Dependencies
├── .env.example              ← Copy to .env and add API key
├── README.md                 ← Full documentation
├── QUICKSTART.md            ← This file
│
├── models/
│   ├── __init__.py
│   ├── vision_models.py      ← LLaVA + EasyOCR
│   ├── language_models.py    ← Llama 3.2
│   └── speech_models.py      ← Whisper-tiny
│
└── tests/
    ├── test_individual.py    ← Run first (5-10 min)
    └── test_memory.py        ← Run second (10-15 min)
```

---

## 🆘 Troubleshooting

### Error: "GROQ_API_KEY not configured"
**Solution**: Create .env file with your API key

### Error: "Module not found"
**Solution**: `pip install -r requirements.txt`

### Error: "CUDA out of memory"
**Solution**: Close other GPU apps or use CPU

### Slow response times (>5 seconds)
**Solution**: 
- Check internet connection (Groq API needs it)
- Verify RTX 3050 GPU: `nvidia-smi`
- Check network latency to Groq servers

---

## 💡 Key Points

✅ **Solution 2 + 3 Benefits**:
- Never loads all 4 models = Save 8-12 GB RAM
- CPU/GPU stays cool = Battery friendly
- Free Groq tier = $0 cost
- Fast responses = 1.5-3 seconds

✅ **Our Architecture**:
- LLaVA (Groq) + Llama 3.2 (Groq) = Cloud ☁️
- EasyOCR + Whisper-tiny = Local 🖥️
- Smart routing = Load only what's needed 🎯

✅ **Result**: 
- RTX 3050 can handle it ✅
- College demo project viable ✅
- Ready to scale to smart glasses ✅

---

**Status**: Ready to Test 🚀  
**Estimated Time**: 30 minutes total  
**Next**: Run `python tests/test_individual.py`
