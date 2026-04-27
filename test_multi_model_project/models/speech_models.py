"""
Speech Models: Whisper-tiny (local)
"""
import base64
import io
import whisper
import config
import logging

logger = logging.getLogger(__name__)

# Initialize Whisper model
whisper_model = None


def get_whisper_model():
    """Lazy load Whisper model to save memory"""
    global whisper_model
    if whisper_model is None:
        logger.info(f"Loading Whisper model ({config.WHISPER_MODEL})...")
        whisper_model = whisper.load_model(config.WHISPER_MODEL)
        logger.info("Whisper model loaded successfully")
    return whisper_model


def transcribe_audio(audio_base64: str) -> dict:
    """
    Transcribe audio using Whisper-tiny (local)
    
    Args:
        audio_base64: Base64 encoded audio file
    
    Returns:
        dict with transcription
    """
    logger.info("Transcribing audio with Whisper...")
    
    try:
        # Decode base64 to audio file
        audio_data = base64.b64decode(audio_base64)
        
        # Save temporarily
        import tempfile
        with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as tmp_file:
            tmp_file.write(audio_data)
            tmp_path = tmp_file.name
        
        # Get Whisper model
        model = get_whisper_model()
        
        # Transcribe
        result = model.transcribe(tmp_path, language="en")
        
        # Clean up
        import os
        os.remove(tmp_path)
        
        transcription = result["text"]
        logger.info(f"Whisper transcription: {transcription[:100]}...")
        
        return {
            "status": "success",
            "model": "whisper-tiny",
            "transcription": transcription,
            "language": result.get("language", "en"),
            "source": "local"
        }
    except Exception as e:
        logger.error(f"Whisper error: {str(e)}")
        return {
            "status": "error",
            "model": "whisper-tiny",
            "error": str(e),
            "source": "local"
        }


def unload_whisper_model():
    """Unload Whisper to free memory"""
    global whisper_model
    if whisper_model is not None:
        logger.info("Unloading Whisper model...")
        del whisper_model
        whisper_model = None
        import gc
        gc.collect()
