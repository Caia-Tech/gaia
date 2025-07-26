#!/usr/bin/env python3
"""Comprehensive test suite for GAIA V8 Enhanced with error handling and observability"""

import subprocess
import time
import json
import os
import sys

def run_test(command, test_input):
    """Run GAIA V8 Enhanced with input and capture output"""
    # Create input file
    with open("/tmp/test_input.txt", "w") as f:
        f.write(test_input + "\nquit\n")
    
    # Run command
    process = subprocess.Popen(
        command,
        stdin=open("/tmp/test_input.txt", "r"),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    
    stdout, stderr = process.communicate()
    
    # Clean up
    os.remove("/tmp/test_input.txt")
    
    return stdout, stderr

def extract_response(output):
    """Extract GAIA V8 response from output"""
    lines = output.split('\n')
    for i, line in enumerate(lines):
        if line.startswith("GAIA V8:"):
            # Look for the actual response on next lines
            for j in range(i+1, min(i+10, len(lines))):
                if lines[j].strip() and not lines[j].startswith("["):
                    return lines[j].strip()
    return None

def test_basic_calculations():
    """Test basic calculation capabilities"""
    print("=== Testing Basic Calculations ===")
    tests = [
        ("2 plus 2", "4"),
        ("5 times 3", "15"),
        ("10 minus 4", "6"),
        ("20 divided by 4", "5"),
        ("-5 plus 3", "-2"),
        ("100 divided by 10", "10"),
    ]
    
    passed = 0
    for query, expected in tests:
        stdout, _ = run_test(["./gaia_chat_v8_enhanced"], query)
        response = extract_response(stdout)
        
        if response and expected in response:
            print(f"✓ '{query}' → '{response}'")
            passed += 1
        else:
            print(f"✗ '{query}' → '{response}' (expected '{expected}')")
    
    print(f"Passed: {passed}/{len(tests)}\n")
    return passed, len(tests)

def test_edge_cases():
    """Test edge cases and error handling"""
    print("=== Testing Edge Cases ===")
    tests = [
        ("10 divided by 0", "error"),
        ("", ""),  # empty input
        ("hello world", ""),  # non-calculation
        ("what is 2 + 2", "4"),  # alternative format
    ]
    
    passed = 0
    for query, check in tests:
        stdout, stderr = run_test(["./gaia_chat_v8_enhanced"], query)
        response = extract_response(stdout)
        
        if query == "10 divided by 0":
            # Should see error in stderr or logs
            if "Division by zero" in stderr or "Division by zero" in stdout:
                print(f"✓ '{query}' → Error handled correctly")
                passed += 1
            else:
                print(f"✗ '{query}' → No error detected")
        elif query == "":
            print(f"✓ Empty input handled")
            passed += 1
        else:
            if response:
                print(f"✓ '{query}' → '{response}'")
                passed += 1
            else:
                print(f"✗ '{query}' → No response")
    
    print(f"Passed: {passed}/{len(tests)}\n")
    return passed, len(tests)

def test_prime_numbers():
    """Test prime number checking"""
    print("=== Testing Prime Numbers ===")
    tests = [
        ("is 11 a prime number?", "is a prime"),
        ("is 12 a prime number?", "is not a prime"),
        ("is 17 a prime number?", "is a prime"),
        ("is 1 a prime number?", "is not a prime"),
        ("is 2 a prime number?", "is a prime"),
    ]
    
    passed = 0
    for query, expected in tests:
        stdout, _ = run_test(["./gaia_chat_v8_enhanced"], query)
        response = extract_response(stdout)
        
        if response and expected in response:
            print(f"✓ '{query}' → '{response}'")
            passed += 1
        else:
            print(f"✗ '{query}' → '{response}' (expected '{expected}')")
    
    print(f"Passed: {passed}/{len(tests)}\n")
    return passed, len(tests)

def test_non_calculation_queries():
    """Test non-calculation queries"""
    print("=== Testing Non-Calculation Queries ===")
    tests = [
        ("What is the capital of France?", "Paris"),
        ("What is artificial intelligence?", "artificial intelligence"),
        ("help", ""),
    ]
    
    passed = 0
    for query, expected in tests:
        stdout, _ = run_test(["./gaia_chat_v8_enhanced"], query)
        response = extract_response(stdout)
        
        if response and expected in response:
            print(f"✓ '{query}' → '{response}'")
            passed += 1
        else:
            print(f"✗ '{query}' → '{response}' (expected '{expected}')")
    
    print(f"Passed: {passed}/{len(tests)}\n")
    return passed, len(tests)

def test_logging_levels():
    """Test different logging levels"""
    print("=== Testing Logging Levels ===")
    
    # Test with debug logging
    stdout, _ = run_test(["./gaia_chat_v8_enhanced", "--debug"], "2 plus 2")
    if "[DEBUG]" in stdout:
        print("✓ Debug logging works")
        debug_passed = 1
    else:
        print("✗ Debug logging not found")
        debug_passed = 0
    
    # Test with trace logging
    stdout, _ = run_test(["./gaia_chat_v8_enhanced", "--trace"], "2 plus 2")
    if "[TRACE]" in stdout:
        print("✓ Trace logging works")
        trace_passed = 1
    else:
        print("✗ Trace logging not found")
        trace_passed = 0
    
    print(f"Passed: {debug_passed + trace_passed}/2\n")
    return debug_passed + trace_passed, 2

def test_metrics_export():
    """Test metrics export functionality"""
    print("=== Testing Metrics Export ===")
    
    # Run observability test
    try:
        subprocess.run(["./test_v8_observability"], check=True, capture_output=True)
        
        # Check if metrics file was created
        if os.path.exists("test_metrics.json"):
            with open("test_metrics.json", "r") as f:
                metrics = json.load(f)
            
            if "metrics" in metrics and "v8_metrics" in metrics:
                print("✓ Metrics exported successfully")
                print(f"  - Found {len(metrics['metrics'])} metrics")
                print(f"  - V8 workflows: {metrics['v8_metrics']['workflows']['total']}")
                passed = 1
            else:
                print("✗ Metrics file incomplete")
                passed = 0
        else:
            print("✗ Metrics file not created")
            passed = 0
    except Exception as e:
        print(f"✗ Error running observability test: {e}")
        passed = 0
    
    print(f"Passed: {passed}/1\n")
    return passed, 1

def test_performance():
    """Test performance characteristics"""
    print("=== Testing Performance ===")
    
    # Test response time
    start = time.time()
    stdout, _ = run_test(["./gaia_chat_v8_enhanced"], "2 plus 2")
    elapsed = time.time() - start
    
    if elapsed < 1.0:  # Should respond in under 1 second
        print(f"✓ Response time: {elapsed:.3f}s")
        time_passed = 1
    else:
        print(f"✗ Response time too slow: {elapsed:.3f}s")
        time_passed = 0
    
    # Check for performance metrics in output
    if "Total response time:" in stdout:
        print("✓ Performance metrics tracked")
        metrics_passed = 1
    else:
        print("✗ Performance metrics not found")
        metrics_passed = 0
    
    print(f"Passed: {time_passed + metrics_passed}/2\n")
    return time_passed + metrics_passed, 2

def main():
    """Run all tests"""
    print("=== GAIA V8 Enhanced Test Suite ===\n")
    
    # Check if executable exists
    if not os.path.exists("./gaia_chat_v8_enhanced"):
        print("Error: gaia_chat_v8_enhanced not found. Please compile first.")
        sys.exit(1)
    
    total_passed = 0
    total_tests = 0
    
    # Run test suites
    test_suites = [
        test_basic_calculations,
        test_edge_cases,
        test_prime_numbers,
        test_non_calculation_queries,
        test_logging_levels,
        test_metrics_export,
        test_performance,
    ]
    
    for test_suite in test_suites:
        passed, tests = test_suite()
        total_passed += passed
        total_tests += tests
    
    # Summary
    print("=== Test Summary ===")
    print(f"Total Passed: {total_passed}/{total_tests}")
    print(f"Success Rate: {(total_passed/total_tests*100):.1f}%")
    
    if total_passed == total_tests:
        print("\n✅ All tests passed!")
        return 0
    else:
        print(f"\n❌ {total_tests - total_passed} tests failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())