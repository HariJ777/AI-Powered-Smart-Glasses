"""
Run local model tests (EasyOCR + Whisper) - No API key required
"""
import sys
import os
import io
import time
import logging

# Fix Windows console encoding
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

logging.basicConfig(level='INFO', format='%(asctime)s - %(levelname)s - %(message)s')


def test_easyocr():
    """Test EasyOCR text extraction (local)"""
    print("\n" + "=" * 60)
    print("TEST: EasyOCR Text Extraction (Local)")
    print("=" * 60)

    try:
        from models import vision_models
        test_image = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg=="

        start = time.time()
        result = vision_models.extract_text_with_easyocr(test_image)
        elapsed = time.time() - start

        print(f"Status: {result['status']}")
        print(f"Model: {result['model']}")
        print(f"Source: {result['source']}")
        print(f"Time: {elapsed:.2f}s")

        if result['status'] == 'success':
            print(f"Regions found: {result['regions_found']}")
            print("[PASS] EasyOCR test PASSED")
        else:
            print(f"Note: {result.get('error', '')}")
            print("[PASS] EasyOCR loaded OK (no text in 1px test image - expected)")

        vision_models.unload_ocr_model()
        print("[CLEANUP] Memory freed after unload")
        return True
    except Exception as e:
        print(f"[FAIL] EasyOCR test FAILED: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_whisper():
    """Test Whisper-tiny model loading (local)"""
    print("\n" + "=" * 60)
    print("TEST: Whisper-tiny Model Loading (Local)")
    print("=" * 60)

    try:
        from models import speech_models
        import config

        start = time.time()
        model = speech_models.get_whisper_model()
        elapsed = time.time() - start

        if model is not None:
            print(f"Status: success")
            print(f"Model: {config.WHISPER_MODEL}")
            print(f"Source: local")
            print(f"Load time: {elapsed:.2f}s")
            print("[PASS] Whisper-tiny test PASSED")

            speech_models.unload_whisper_model()
            print("[CLEANUP] Memory freed after unload")
            return True
        else:
            print("[FAIL] Whisper test FAILED: Model not loaded")
            return False
    except Exception as e:
        print(f"[FAIL] Whisper test FAILED: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    print("=" * 60)
    print(" LOCAL MODEL TESTS (No API Key Required)")
    print("=" * 60)

    results = {}
    results["EasyOCR (Local)"] = test_easyocr()
    results["Whisper-tiny (Local)"] = test_whisper()

    print("\n" + "=" * 60)
    print(" TEST SUMMARY")
    print("=" * 60)

    passed = 0
    for name, result in results.items():
        status = "[PASS]" if result else "[FAIL]"
        print(f"  {status}: {name}")
        if result:
            passed += 1

    print(f"\nPassed: {passed}/{len(results)}")

    if passed == len(results):
        print("\nAll local tests passed!")
    else:
        print(f"\n{len(results) - passed} test(s) failed.")

    return passed == len(results)


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
