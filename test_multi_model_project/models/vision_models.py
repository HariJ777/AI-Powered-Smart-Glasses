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

# Current Groq vision model (supports multimodal image+text)
VISION_MODEL = "meta-llama/llama-4-scout-17b-16e-instruct"


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


def analyze_image_with_llava(image_base64: str, prompt: str = "What do you see in this image?") -> dict:
    """
    Analyze image using Llama 4 Scout (multimodal) via Groq API
    
    Args:
        image_base64: Base64 encoded image
        prompt: Analysis prompt
    
    Returns:
        dict with analysis result
    """
    logger.info(f"Calling {VISION_MODEL} via Groq API for image analysis...")
    
    try:
        # Groq SDK v1.2 uses OpenAI-compatible chat.completions.create
        response = groq_client.chat.completions.create(
            model=VISION_MODEL,
            messages=[
                {
                    "role": "user",
                    "content": [
                        {
                            "type": "image_url",
                            "image_url": {
                                "url": f"data:image/png;base64,{image_base64}",
                            },
                        },
                        {
                            "type": "text",
                            "text": prompt
                        }
                    ],
                }
            ],
            max_tokens=1024,
        )
        
        result = response.choices[0].message.content
        logger.info(f"Vision analysis complete: {result[:100]}...")
        
        return {
            "status": "success",
            "model": VISION_MODEL,
            "analysis": result,
            "source": "groq_api"
        }
    except Exception as e:
        logger.error(f"Vision API error: {str(e)}")
        return {
            "status": "error",
            "model": VISION_MODEL,
            "error": str(e),
            "source": "groq_api"
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
