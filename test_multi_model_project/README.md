"""
README: Test Project for AI Smart Glasses
Testing Solution 2 + 3: Smart Routing + Groq API
"""

# Test Project Structure

This test project validates whether running multiple AI models is feasible with:
- **Solution 2**: Smart routing (Flask loads only 1 model per request)
- **Solution 3**: Groq API (free tier for LLMs, zero local RAM)

## Quick Start

### 1. Setup

```bash
# Navigate to project
cd test_multi_model_project

# Create .env file from template
cp .env.example .env

# Edit .env and add your Groq API key
# Get free key from: https://console.groq.com
```

### 2. Install Dependencies

```bash
pip install -r requirements.txt
```

### 3. Run Tests

#### Test Individual Models
```bash
python tests/test_individual.py
```

Tests each model in isolation:
- ✅ LLaVA (Groq API)
- ✅ Llama 3.2 Q&A (Groq API)
- ✅ Llama 3.2 Translation (Groq API)
- ✅ EasyOCR (Local)
- ✅ Whisper-tiny (Local)

#### Test Memory Usage
```bash
python tests/test_memory.py
```

Validates:
- Scenario 1: LLaVA API call (no memory impact)
- Scenario 2: EasyOCR load/unload (memory cleanup)
- Scenario 3: Sequential operations (smart routing pattern)

### 4. Run Flask Server

```bash
python main.py
```

Server runs on: `http://localhost:5000`

Endpoints:
- `GET /health` - Health check + memory stats
- `GET /status` - Detailed model status
- `POST /analyze-image` - Smart image analysis (LLaVA or EasyOCR)
- `POST /ask` - Q&A with Llama 3.2
- `POST /transcribe` - Speech-to-text with Whisper

## API Usage Examples

### Analyze Image (LLaVA Description)
```bash
curl -X POST http://localhost:5000/analyze-image \
  -H "Content-Type: application/json" \
  -d '{
    "image_base64": "iVBORw0KG...",
    "analysis_type": "description",
    "prompt": "What objects are in this image?"
  }'
```

### Ask Question
```bash
curl -X POST http://localhost:5000/ask \
  -H "Content-Type: application/json" \
  -d '{
    "question": "What is artificial intelligence?",
    "context": ""
  }'
```

### Health Check
```bash
curl http://localhost:5000/health
```

## Key Files

| File | Purpose |
|------|---------|
| `main.py` | Flask app with smart routing |
| `config.py` | Configuration management |
| `models/vision_models.py` | LLaVA + EasyOCR |
| `models/language_models.py` | Llama 3.2 Q&A & translation |
| `models/speech_models.py` | Whisper-tiny |
| `tests/test_individual.py` | Model validation tests |
| `tests/test_memory.py` | Memory usage validation |

## Technical Specifications

### Models Used

| Model | Source | RAM | GPU VRAM | Cost |
|-------|--------|-----|----------|------|
| LLaVA 1.5 7B | Groq API | 0 MB | 0 GB | Free |
| Llama 3.2 | Groq API | 0 MB | 0 GB | Free |
| EasyOCR | Local | ~200 MB | 0.5 GB | Free |
| Whisper-tiny | Local | ~150 MB | 0.3 GB | Free |

**Total Local RAM**: ~2 GB (when all loaded)  
**Total Local GPU**: ~0.8 GB (when all loaded)  
**Never loads all simultaneously** ✅

### Groq Free Tier

- **Rate Limit**: 14,400 requests/day
- **Models**: LLaVA 1.5, Llama 3.2, Mixtral, Gemma
- **Response Time**: 1-2 seconds
- **Perfect for**: College demo projects

### Response Times

| Operation | Time | Source |
|-----------|------|--------|
| LLaVA image analysis | 1-2s | Groq API |
| Llama Q&A | 1.5-2s | Groq API |
| EasyOCR text extraction | 1-3s | Local GPU |
| Whisper-tiny transcription | 2-5s | Local GPU |

## Expected Test Results

### Passing Criteria ✅

```
Test 1: LLaVA API - ✅ PASS (Requires GROQ_API_KEY)
Test 2: Llama 3.2 Q&A - ✅ PASS (Requires GROQ_API_KEY)
Test 3: Llama Translation - ✅ PASS (Requires GROQ_API_KEY)
Test 4: EasyOCR Load - ✅ PASS (Model loads successfully)
Test 5: Whisper Load - ✅ PASS (Model loads successfully)

Memory Scenario 1: ✅ PASS (API calls use no local memory)
Memory Scenario 2: ✅ PASS (Models unload properly)
Memory Scenario 3: ✅ PASS (Sequential ops maintain low memory)

Result: Solution 2 + 3 is FEASIBLE ✅
```

## Troubleshooting

### "GROQ_API_KEY not configured"
- Get free key from https://console.groq.com
- Add to `.env` file: `GROQ_API_KEY=your_key_here`

### "CUDA out of memory"
- Reduce model batch size
- Ensure no other GPU processes running
- Try with CPU: Remove GPU acceleration

### "Slow response times"
- Check internet connection (Groq API needs it)
- Verify RTX 3050 GPU drivers are updated
- Monitor with: `nvidia-smi`

### "Memory not freeing after model unload"
- Restart Flask server to clear all cached models
- Check for memory leaks in model loading

## Next Steps

After validating this test project:
1. ✅ Models work individually
2. ✅ Memory stays within limits
3. ✅ Response times are acceptable

**Then integrate into main Smart Glasses project:**
- Adapt Flask routes for ESP32-CAM communication
- Add image preprocessing/postprocessing
- Integrate with OLED display + speaker
- Add battery management optimization

## References

- **Groq API Docs**: https://console.groq.com/docs
- **EasyOCR**: https://github.com/JaidedAI/EasyOCR
- **Whisper (OpenAI)**: https://github.com/openai/whisper
- **Flask**: https://flask.palletsprojects.com

---

**Status**: Ready for Testing 🚀  
**Created**: April 23, 2026  
**Purpose**: Validate Solution 2 + 3 Architecture  
**Target**: Main Smart Glasses Project Integration
