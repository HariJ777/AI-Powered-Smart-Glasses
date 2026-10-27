"""
Vision Models: Llama 4 Scout (via Groq) and EasyOCR (local)
"""
import base64
import io
from PIL import Image
import easyocr
from groq import Groq
import config
import logging

logger = logging.getLogger(__name__)

# Initialize clients
groq_client = Groq(api_key=config.GROQ_API_KEY)
ocr_reader = None  # Lazy loaded

# Current vision model and fallback pool with independent quota allocations
VISION_MODELS = [
    "gemini-3.5-flash-lite",     # Most reliable, fast, zero deadline errors
    "gemini-flash-lite-latest",
    "gemini-3.1-flash-lite",
    "gemini-flash-latest",
    "gemini-3.8-flash",
    "gemini-3.5-flash"
]
VISION_MODEL = VISION_MODELS[0]

# Configure Gemini API keys (supports comma-separated list of keys for multi-key rotation)
raw_keys = getattr(config, 'GEMINI_API_KEY', '') or ''
GEMINI_API_KEYS = [k.strip() for k in raw_keys.split(',') if k.strip()]
if GEMINI_API_KEYS:
    import google.generativeai as genai
    genai.configure(api_key=GEMINI_API_KEYS[0])


def get_ocr_reader():
    """Lazy load EasyOCR reader to save memory"""
    global ocr_reader
    if ocr_reader is None:
        logger.info("Loading EasyOCR model...")
        # Auto-detect GPU availability
        import torch
        use_gpu = torch.cuda.is_available()
        ocr_reader = easyocr.Reader(config.EASYOCR_LANGUAGES, gpu=use_gpu)
        logger.info(f"EasyOCR loaded successfully (GPU: {use_gpu})")
    return ocr_reader


def analyze_image_with_gemini(image_base64: str, prompt: str = "Describe what you see in maximum 12 words. Be direct and concise. Do not use filler like 'The image shows'.") -> dict:
    """
    Analyze image using Google Gemini API with automatic multi-model and multi-key fallback
    
    Args:
        image_base64: Base64 encoded image
        prompt: Analysis prompt
    
    Returns:
        dict with analysis result
    """
    try:
        import google.generativeai as genai
        
        # Decode base64 to image for Gemini
        image_data = base64.b64decode(image_base64)
        image = Image.open(io.BytesIO(image_data))
        
        last_error = "No API keys configured"
        keys_to_try = GEMINI_API_KEYS if GEMINI_API_KEYS else [getattr(config, 'GEMINI_API_KEY', '')]
        
        for key_idx, current_key in enumerate(keys_to_try):
            if not current_key:
                continue
            genai.configure(api_key=current_key)
            
            # Iterate through models with separate quota pools
            for model_name in VISION_MODELS:
                try:
                    logger.info(f"Analyzing vision with model {model_name} (key #{key_idx + 1})...")
                    model = genai.GenerativeModel(model_name)
                    # 6 second fail-safe timeout per model for snappy responses
                    response = model.generate_content([prompt, image], request_options={"timeout": 6})
                    
                    result = response.text.strip()
                    logger.info(f"Vision analysis complete ({model_name}): {result[:100]}...")
                    
                    return {
                        "status": "success",
                        "model": model_name,
                        "result": result,
                        "source": "gemini_api"
                    }
                except Exception as model_err:
                    err_msg = str(model_err)
                    last_error = err_msg
                    if "429" in err_msg or "ResourceExhausted" in err_msg or "Quota" in err_msg or "quota" in err_msg:
                        logger.warning(f"Quota 429 on {model_name}. Auto-falling back to next model...")
                    else:
                        logger.warning(f"Model {model_name} issue: {err_msg[:80]}. Falling back...")
                    continue
        
        # If all fallbacks exhausted
        logger.error(f"All vision models exhausted. Last error: {last_error}")
        return {
            "status": "error",
            "model": "all_fallbacks_exhausted",
            "error": last_error,
            "source": "gemini_api"
        }
            
    except Exception as e:
        logger.error(f"Vision API fatal error: {str(e)}")
        return {
            "status": "error",
            "model": VISION_MODELS[0],
            "error": str(e),
            "source": "gemini_api"
        }



def extract_text_with_easyocr(image_base64: str) -> dict:
    """
    Extract text from image using EasyOCR (local)
    
    Args:
        image_base64: Base64 encoded image
    
    Returns:
        dict with extracted text
    """
    logger.info("Extracting text with EasyOCR...")
    
    try:
        # Decode base64 to image
        image_data = base64.b64decode(image_base64)
        image = Image.open(io.BytesIO(image_data))
        
        # Convert to numpy array
        import numpy as np
        image_array = np.array(image)
        
        # Get OCR reader
        reader = get_ocr_reader()
        
        # Extract text
        results = reader.readtext(image_array)
        
        # Format results
        extracted_text = "\n".join([text[1] for text in results])
        confidence = sum([text[2] for text in results]) / len(results) if results else 0
        
        logger.info(f"EasyOCR extracted {len(results)} text regions")
        
        return {
            "status": "success",
            "model": "easyocr",
            "text": extracted_text,
            "regions_found": len(results),
            "average_confidence": round(confidence, 3),
            "source": "local"
        }
    except Exception as e:
        logger.error(f"EasyOCR error: {str(e)}")
        return {
            "status": "error",
            "model": "easyocr",
            "error": str(e),
            "source": "local"
        }


def unload_ocr_model():
    """Unload EasyOCR to free memory"""
    global ocr_reader
    if ocr_reader is not None:
        logger.info("Unloading EasyOCR model...")
        del ocr_reader
        ocr_reader = None
        import gc
        gc.collect()
