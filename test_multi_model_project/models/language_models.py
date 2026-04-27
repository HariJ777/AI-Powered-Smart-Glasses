"""
Language Models: Llama 3.3 70B via Groq API
"""
from groq import Groq
import config
import logging

logger = logging.getLogger(__name__)

# Initialize Groq client
groq_client = Groq(api_key=config.GROQ_API_KEY)

# Current Groq language model
LANGUAGE_MODEL = "llama-3.3-70b-versatile"


def answer_question(question: str, context: str = "") -> dict:
    """
    Answer questions using Llama 3.3 via Groq API
    
    Args:
        question: User question
        context: Optional context for better answers
    
    Returns:
        dict with answer
    """
    logger.info(f"Processing question with {LANGUAGE_MODEL}: {question[:50]}...")
    
    try:
        prompt = f"""Context: {context}

Question: {question}

Please provide a clear and concise answer."""

        # Groq SDK v1.2 uses OpenAI-compatible chat.completions.create
        response = groq_client.chat.completions.create(
            model=LANGUAGE_MODEL,
            messages=[
                {
                    "role": "user",
                    "content": prompt
                }
            ],
            max_tokens=512,
            temperature=0.7,
        )
        
        answer = response.choices[0].message.content
        logger.info(f"Llama answer generated: {answer[:100]}...")
        
        return {
            "status": "success",
            "model": LANGUAGE_MODEL,
            "question": question,
            "answer": answer,
            "source": "groq_api"
        }
    except Exception as e:
        logger.error(f"Llama API error: {str(e)}")
        return {
            "status": "error",
            "model": LANGUAGE_MODEL,
            "error": str(e),
            "source": "groq_api"
        }


def translate_text(text: str, target_language: str = "Spanish") -> dict:
    """
    Translate text using Llama 3.3 via Groq API
    
    Args:
        text: Text to translate
        target_language: Target language
    
    Returns:
        dict with translated text
    """
    logger.info(f"Translating text to {target_language}...")
    
    try:
        prompt = f"""Translate the following text to {target_language}. Provide only the translation, no explanations.

Text: {text}"""

        response = groq_client.chat.completions.create(
            model=LANGUAGE_MODEL,
            messages=[
                {
                    "role": "user",
                    "content": prompt
                }
            ],
            max_tokens=256,
        )
        
        translation = response.choices[0].message.content
        logger.info(f"Translation complete: {translation[:100]}...")
        
        return {
            "status": "success",
            "model": LANGUAGE_MODEL,
            "original": text,
            "translation": translation,
            "target_language": target_language,
            "source": "groq_api"
        }
    except Exception as e:
        logger.error(f"Translation error: {str(e)}")
        return {
            "status": "error",
            "model": LANGUAGE_MODEL,
            "error": str(e),
            "source": "groq_api"
        }


def summarize_text(text: str) -> dict:
    """
    Summarize text using Llama 3.3 via Groq API
    
    Args:
        text: Text to summarize
    
    Returns:
        dict with summary
    """
    logger.info("Summarizing text...")
    
    try:
        prompt = f"""Provide a concise summary of the following text in 2-3 sentences:

{text}"""

        response = groq_client.chat.completions.create(
            model=LANGUAGE_MODEL,
            messages=[
                {
                    "role": "user",
                    "content": prompt
                }
            ],
            max_tokens=256,
        )
        
        summary = response.choices[0].message.content
        logger.info(f"Summary generated: {summary[:100]}...")
        
        return {
            "status": "success",
            "model": LANGUAGE_MODEL,
            "summary": summary,
            "source": "groq_api"
        }
    except Exception as e:
        logger.error(f"Summarization error: {str(e)}")
        return {
            "status": "error",
            "model": LANGUAGE_MODEL,
            "error": str(e),
            "source": "groq_api"
        }
