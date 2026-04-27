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
        
        # SMART ROUTING: Choose appropriate model
        if analysis_type == 'description':
            logger.info("Route: LLaVA (Groq API) - Image description")
            result = vision_models.analyze_image_with_llava(
                image_base64,
                prompt=custom_prompt or "What do you see in this image?"
            )
            request_count["groq"] += 1
        elif analysis_type == 'ocr':
            logger.info("Route: EasyOCR (Local) - Text extraction")
            result = vision_models.extract_text_with_easyocr(image_base64)
            request_count["local"] += 1
        else:
            return jsonify({"error": f"Unknown analysis_type: {analysis_type}"}), 400
        
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


if __name__ == '__main__':
    logger.info("Starting Flask server with smart routing...")
    logger.info("Solution 2 + 3: Smart routing + Groq API")
    logger.info(f"API Key configured: {bool(config.GROQ_API_KEY)}")
    
    app.run(
        host='0.0.0.0',
        port=config.FLASK_PORT,
        debug=config.FLASK_DEBUG
    )
