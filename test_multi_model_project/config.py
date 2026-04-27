import os
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# Groq API Configuration
GROQ_API_KEY = os.getenv('GROQ_API_KEY', '')

# Model Configuration
WHISPER_MODEL = os.getenv('WHISPER_MODEL', 'tiny')  # tiny, base, small, medium, large
EASYOCR_LANGUAGES = os.getenv('EASYOCR_LANGUAGES', 'en').split(',')

# Flask Configuration
FLASK_DEBUG = os.getenv('FLASK_DEBUG', 'True').lower() == 'true'
FLASK_PORT = int(os.getenv('FLASK_PORT', 5000))

# Memory Monitoring
ENABLE_MEMORY_LOGGING = os.getenv('ENABLE_MEMORY_LOGGING', 'True').lower() == 'true'
LOG_INTERVAL_SECONDS = int(os.getenv('LOG_INTERVAL_SECONDS', 5))

# Model Timeouts
GROQ_TIMEOUT = 30  # seconds
LOCAL_MODEL_TIMEOUT = 60  # seconds

# Max file sizes
MAX_IMAGE_SIZE = 50 * 1024 * 1024  # 50 MB
MAX_AUDIO_SIZE = 20 * 1024 * 1024  # 20 MB

# Logging
LOG_LEVEL = 'INFO'
