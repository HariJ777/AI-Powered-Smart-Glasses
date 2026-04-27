# AI Smart Glasses - Implementation Plan
## Solution 2 + 3 Hybrid Approach

**Date**: April 23, 2026  
**Status**: Planning Phase  

---

## Executive Summary
Implement a **hybrid AI backend** combining:
- **Solution 2**: Smart routing in Flask (load only 1 model per request)
- **Solution 3**: Groq API (free tier for LLMs, zero local RAM)

---

## Architecture Overview

### Component Breakdown

#### 1. **Cloud-Based (Groq API - Free Tier)**
- **Llama 4 Scout 17B** (`meta-llama/llama-4-scout-17b-16e-instruct`): Image understanding, object detection
- **Llama 3.3 70B** (`llama-3.3-70b-versatile`): Q&A, translation, reasoning
- **Rate Limit**: 14,400 requests/day (free tier) ✅ Sufficient for college demo
- **Cost**: $0
- **Response Time**: 0.4-0.7s ✅ TESTED

#### 2. **Local Machine (RTX 3050 GPU)**
- **EasyOCR**: Text extraction from images (~180 MB RAM)
- **Whisper-tiny**: Speech-to-text (~145 MB RAM)
- **Response Time**: 0.4-1.8s ✅ TESTED
- **Peak RAM Usage**: ~600 MB (both unloaded after use)

#### 3. **Flask Smart Router**
- Analyzes request type
- Routes to appropriate model
- **Never loads all 4 models simultaneously**
- Unload models after use to free memory

---

## Request Flow (Solution 2 + 3)

```
ESP32-CAM → Flask Router
    ↓
Request Type Check:
    ├─ Image Analysis? → Call Groq LLaVA API
    ├─ Text Extraction? → Load EasyOCR locally → Return
    ├─ Audio Transcription? → Load Whisper-tiny locally → Return
    └─ Q&A / Translation? → Call Groq Llama 3.2 API
    ↓
Response → Base64 encode → HTTP POST → ESP32-CAM
    ↓
Display on OLED + Speaker (TTS via Groq)
```

---

## Technical Stack

| Component | Tech | Resource | Cost |
|-----------|------|----------|------|
| Web Server | Flask 3.0 | Local | Free |
| LLaVA Vision | Groq API | Cloud | Free (14.4k req/day) |
| Llama 3.2 | Groq API | Cloud | Free |
| EasyOCR | Local | GPU/CPU | Free |
| Whisper-tiny | Local | GPU/CPU | Free |
| GPU | RTX 3050 | Local | Already owned |

---

## Testing Strategy

### Phase 1: Model Load Testing ✅ (This Phase)
- Test if multiple models can coexist in memory
- Measure RAM usage per model
- Test sequential loading/unloading
- Validate response times

### Phase 2: Integration Testing
- Test Flask routing logic
- Test Groq API integration
- Test ESP32 communication

### Phase 3: Production Deployment
- Deploy to main smart glasses project
- Field testing with real-world data

---

## Key Decisions

### Decision 1: Why Groq over Local Ollama?
| Aspect | Groq Free API | Local Ollama |
|--------|---------------|-------------|
| RAM Required | 0 GB | 8-12 GB |
| Internet Required | Yes | No |
| Response Time | 1-2s | 3-5s |
| Setup Complexity | Simple API key | Complex setup |
| Free Tier | 14,400 req/day | Yes |
| **Recommendation** | ✅ Better for project | May strain GPU |

### Decision 2: Local vs Cloud
- **Keep Local**: EasyOCR (always needed), Whisper-tiny (privacy for audio)
- **Move Cloud**: LLaVA, Llama 3.2 (free API, zero RAM)

### Decision 3: Smart Routing Benefits
- Never load 4 models = Save 8-12 GB RAM
- Each request loads only 1 model
- Faster inference (cloud APIs are optimized)
- Free tier sufficient for demo project

---

## Resource Requirements

### Before Optimization (All 4 Local)
- GPU VRAM: 8-12 GB ❌ (RTX 3050 has 8GB, too tight)
- RAM: 4-6 GB ❌ (System overload)

### After Optimization (Solution 2+3)
- GPU VRAM: 2 GB ✅ (EasyOCR + Whisper-tiny)
- RAM: 2 GB ✅ (Comfortable margin)
- Internet: Required (but most places have WiFi)

---

## Success Criteria

✅ All models can be tested independently  
✅ Response times < 3s per request  
✅ No RAM crashes during testing  
✅ Groq API works reliably (14.4k req/day)  
✅ Flask routing correctly switches models  
✅ Memory freed after each request  

---

## Potential Risks & Mitigations

| Risk | Mitigation |
|------|-----------|
| Groq free tier rate limit exhausted | Track requests, cache responses |
| Internet disconnection | Fallback to local Ollama (but slower) |
| Model loading delay | Lazy load on first request, cache |
| API key exposure | Use .env file, never commit |

---

## Next Steps

1. ✅ **Phase 1A**: Create test project structure
2. ✅ **Phase 1B**: Test individual models locally
3. ✅ **Phase 1C**: Test Groq API integration — ALL 5/5 MODELS PASSED
4. ✅ **Phase 1D**: Test Flask smart routing — ALL 5/5 ENDPOINTS PASSED
5. ✅ **Phase 1E**: Validate resource usage — ALL 3/3 MEMORY TESTS PASSED
6. **→ NEXT: Move to main smart glasses project**

---

## File Structure (Test Project)

```
test_multi_model_project/
├── main.py                 # Flask app with smart routing
├── models/
│   ├── vision_models.py    # LLaVA (Groq) + EasyOCR
│   ├── language_models.py  # Llama 3.2 (Groq)
│   └── speech_models.py    # Whisper-tiny
├── config.py               # API keys, model settings
├── test_individual.py      # Test each model separately
├── test_memory.py          # Monitor RAM/GPU usage
├── requirements.txt        # Dependencies
└── .env.example            # Environment variables template
```

---

## Estimated Timeline

- **Phase 1A-B (Today)**: 2 hours - Create test project, load individual models
- **Phase 1C (Today)**: 1 hour - Test Groq API
- **Phase 1D (Tomorrow)**: 2 hours - Flask routing logic
- **Phase 1E (Tomorrow)**: 1 hour - Validate & document results
- **Integration (Next 3 days)**: Apply to main project

**Total Validation Time**: ~6 hours  
**Go/No-Go Decision**: By tomorrow evening

---

## Sign-Off

- [ ] Implementation plan approved
- [ ] Test project setup complete
- [ ] Models tested individually
- [ ] Flask routing validated
- [ ] Ready for main project deployment
