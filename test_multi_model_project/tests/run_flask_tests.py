"""
Flask Server Integration Test
Phase 1D-E: Tests the Flask server endpoints with real requests
Starts the server, sends test requests, validates responses
"""
import sys
import os
import io
import time
import json
import threading
import logging

# Fix Windows console encoding
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

logging.basicConfig(level='WARNING')  # Suppress Flask debug logs


def start_flask_server():
    """Start the Flask server in a background thread"""
    import config
    from flask import Flask
    
    # Import the app
    sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
    from main import app
    
    # Run in a thread
    server_thread = threading.Thread(
        target=lambda: app.run(host='127.0.0.1', port=5001, debug=False, use_reloader=False),
        daemon=True
    )
    server_thread.start()
    time.sleep(2)  # Wait for server to start
    return server_thread


def test_health():
    """Test /health endpoint"""
    print("\n" + "=" * 60)
    print("TEST: /health endpoint")
    print("=" * 60)
    
    import requests
    try:
        resp = requests.get('http://127.0.0.1:5001/health', timeout=10)
        data = resp.json()
        print(f"  Status Code: {resp.status_code}")
        print(f"  Status: {data.get('status')}")
        print(f"  Uptime: {data.get('uptime_seconds', 0)}s")
        
        if resp.status_code == 200 and data.get('status') == 'healthy':
            print("  [PASS] /health endpoint working")
            return True
    except Exception as e:
        print(f"  [FAIL] {e}")
    return False


def test_status():
    """Test /status endpoint"""
    print("\n" + "=" * 60)
    print("TEST: /status endpoint")
    print("=" * 60)
    
    import requests
    try:
        resp = requests.get('http://127.0.0.1:5001/status', timeout=10)
        data = resp.json()
        print(f"  Status Code: {resp.status_code}")
        print(f"  Models: {json.dumps(data.get('models', {}), indent=4)}")
        
        if resp.status_code == 200:
            print("  [PASS] /status endpoint working")
            return True
    except Exception as e:
        print(f"  [FAIL] {e}")
    return False


def test_analyze_image():
    """Test /analyze-image endpoint with image description"""
    print("\n" + "=" * 60)
    print("TEST: /analyze-image (description mode)")
    print("=" * 60)
    
    import requests
    # 10x10 red square PNG
    test_image = "iVBORw0KGgoAAAANSUhEUgAAAAoAAAAKCAIAAAACUFjqAAAAE0lEQVR4nGP8z4APMOGVZRip0gBBLAETee26JgAAAABJRU5ErkJggg=="
    
    try:
        start = time.time()
        resp = requests.post('http://127.0.0.1:5001/analyze-image', json={
            'image_base64': test_image,
            'analysis_type': 'description',
            'prompt': 'What color is this image?'
        }, timeout=30)
        elapsed = time.time() - start
        
        data = resp.json()
        print(f"  Status Code: {resp.status_code}")
        print(f"  Model: {data.get('model', 'N/A')}")
        print(f"  Time: {elapsed:.2f}s")
        
        if data.get('status') == 'success':
            print(f"  Analysis: {data.get('analysis', '')[:100]}")
            print("  [PASS] /analyze-image (description) working")
            return True
        else:
            print(f"  Error: {data.get('error', 'Unknown')}")
            print("  [FAIL] /analyze-image (description) failed")
    except Exception as e:
        print(f"  [FAIL] {e}")
    return False


def test_analyze_image_ocr():
    """Test /analyze-image endpoint with OCR mode"""
    print("\n" + "=" * 60)
    print("TEST: /analyze-image (OCR mode)")
    print("=" * 60)
    
    import requests
    test_image = "iVBORw0KGgoAAAANSUhEUgAAAAoAAAAKCAIAAAACUFjqAAAAE0lEQVR4nGP8z4APMOGVZRip0gBBLAETee26JgAAAABJRU5ErkJggg=="
    
    try:
        start = time.time()
        resp = requests.post('http://127.0.0.1:5001/analyze-image', json={
            'image_base64': test_image,
            'analysis_type': 'ocr'
        }, timeout=30)
        elapsed = time.time() - start
        
        data = resp.json()
        print(f"  Status Code: {resp.status_code}")
        print(f"  Model: {data.get('model', 'N/A')}")
        print(f"  Time: {elapsed:.2f}s")
        print(f"  Regions Found: {data.get('regions_found', 'N/A')}")
        print("  [PASS] /analyze-image (OCR) working")
        return True
    except Exception as e:
        print(f"  [FAIL] {e}")
    return False


def test_ask():
    """Test /ask endpoint"""
    print("\n" + "=" * 60)
    print("TEST: /ask endpoint (Q&A)")
    print("=" * 60)
    
    import requests
    try:
        start = time.time()
        resp = requests.post('http://127.0.0.1:5001/ask', json={
            'question': 'What is the speed of light?',
            'context': 'Physics'
        }, timeout=30)
        elapsed = time.time() - start
        
        data = resp.json()
        print(f"  Status Code: {resp.status_code}")
        print(f"  Model: {data.get('model', 'N/A')}")
        print(f"  Time: {elapsed:.2f}s")
        
        if data.get('status') == 'success':
            print(f"  Answer: {data.get('answer', '')[:150]}")
            print("  [PASS] /ask endpoint working")
            return True
        else:
            print(f"  Error: {data.get('error', 'Unknown')}")
    except Exception as e:
        print(f"  [FAIL] {e}")
    return False


def main():
    print("=" * 60)
    print("  FLASK SERVER INTEGRATION TEST")
    print("  Phase 1D-E: Endpoint Validation")
    print("=" * 60)
    
    # Start server
    print("\n  Starting Flask server on port 5001...")
    try:
        start_flask_server()
        print("  [OK] Server started")
    except Exception as e:
        print(f"  [FAIL] Could not start server: {e}")
        return False
    
    results = {}
    results["/health"] = test_health()
    results["/status"] = test_status()
    results["/analyze-image (description)"] = test_analyze_image()
    results["/analyze-image (OCR)"] = test_analyze_image_ocr()
    results["/ask (Q&A)"] = test_ask()
    
    # Summary
    print("\n" + "=" * 60)
    print("  FLASK SERVER TEST SUMMARY")
    print("=" * 60)
    
    passed = 0
    for name, result in results.items():
        status = "[PASS]" if result else "[FAIL]"
        print(f"  {status} {name}")
        if result:
            passed += 1
    
    print(f"\n  Passed: {passed}/{len(results)}")
    
    if passed == len(results):
        print("\n  >>> ALL SERVER TESTS PASSED! Flask routing works. <<<")
    else:
        print(f"\n  >>> {len(results) - passed} endpoint(s) need review. <<<")
    
    return passed == len(results)


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
