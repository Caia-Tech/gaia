#!/usr/bin/env python3

import subprocess
import time
import sys
import re
from typing import List, Tuple, Dict

class TestCase:
    def __init__(self, name: str, input_text: str, expected_patterns: List[str], 
                 category: str = "General"):
        self.name = name
        self.input = input_text
        self.expected = expected_patterns
        self.category = category
        self.passed = False
        self.output = ""
        self.error = ""

def run_gaia_test(test_input: str, timeout: int = 5) -> Tuple[str, str]:
    """Run GAIA with input and return output and error"""
    try:
        process = subprocess.Popen(
            ["./gaia_chat_v7"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        stdout, stderr = process.communicate(input=test_input + "\nquit\n", timeout=timeout)
        return stdout, stderr
    except subprocess.TimeoutExpired:
        process.kill()
        return "", "TIMEOUT"
    except Exception as e:
        return "", str(e)

def check_output(output: str, patterns: List[str]) -> bool:
    """Check if all expected patterns are in output"""
    # Look for GAIA V7 response specifically
    gaia_match = re.search(r'GAIA V7:.*', output, re.DOTALL)
    if not gaia_match:
        return False
    
    gaia_output = gaia_match.group(0)
    
    for pattern in patterns:
        if pattern not in gaia_output:
            return False
    return True

def run_test_suite():
    """Run all tests"""
    
    # Define all test cases
    test_cases = [
        # Basic Arithmetic
        TestCase("Simple Addition", "What is 2 plus 2?", ["2 plus 2 = 4"], "Arithmetic"),
        TestCase("Simple Subtraction", "What is 10 minus 3?", ["10 minus 3 = 7"], "Arithmetic"),
        TestCase("Simple Multiplication", "What is 5 times 6?", ["5 times 6 = 30"], "Arithmetic"),
        TestCase("Simple Division", "What is 20 divided by 4?", ["20 divided 4 = 5"], "Arithmetic"),
        TestCase("Division by Zero", "What is 10 divided by 0?", ["Error: division by zero"], "Arithmetic"),
        TestCase("Negative Numbers", "What is -5 plus 3?", ["-5 plus 3 = -2"], "Arithmetic"),
        TestCase("Zero Operations", "What is 0 plus 0?", ["0 plus 0 = 0"], "Arithmetic"),
        TestCase("Large Numbers", "What is 999 plus 1?", ["999 plus 1 = 1000"], "Arithmetic"),
        
        # Multi-part Questions
        TestCase("Two Parts", "What is 5 plus 3? What is 10 minus 2?", 
                ["5 plus 3 = 8", "10 minus 2 = 8"], "Multi-part"),
        TestCase("With Also", "What is 15 plus 27? Also, can you explain what addition means?",
                ["15 plus 27 = 42", "Addition is a mathematical operation"], "Multi-part"),
        TestCase("Three Parts", "What is 2 times 3? What is 12 divided by 4? What is 10 plus 5?",
                ["2 times 3 = 6", "12 divided 4 = 3", "10 plus 5 = 15"], "Multi-part"),
        
        # Explanations
        TestCase("Explain Addition", "Explain what addition is",
                ["Addition is a mathematical operation"], "Explanation"),
        TestCase("How Does Work", "How does subtraction work?",
                ["Addition is a mathematical operation"], "Explanation"),
        
        # Greetings
        TestCase("Hello", "Hello", ["Hello"], "Greeting"),
        TestCase("Goodbye", "Goodbye", ["Goodbye"], "Greeting"),
        
        # Lists
        TestCase("List Colors", "List three colors", ["red", "blue", "green"], "Lists"),
        
        # Functions
        TestCase("Factorial", "What is the factorial of 5?", ["factorial", "120"], "Functions"),
        TestCase("Prime Check", "Is 17 a prime number?", ["prime", "17"], "Functions"),
        TestCase("Fibonacci", "What is the 10th Fibonacci number?", ["fibonacci", "55"], "Functions"),
        
        # Edge Cases
        TestCase("Empty Input", "", ["Please provide some input"], "Edge Cases"),
        TestCase("Multiple Spaces", "What    is     5    plus    3?", ["5 plus 3 = 8"], "Edge Cases"),
        TestCase("Mixed Case", "WhAt Is FiVe PlUs ThReE?", ["5 plus 3 = 8"], "Edge Cases"),
        
        # Special Commands
        TestCase("Stats Command", "stats", ["Total patterns:"], "Commands"),
        TestCase("Toggle Workflows", "toggle-workflows", ["workflows:"], "Commands"),
        
        # Complex Workflows
        TestCase("Complex Query", "Calculate 25 times 4 and explain multiplication",
                ["25 times 4 = 100"], "Workflows"),
        TestCase("Percentage", "What is 25 percent of 100?", ["GAIA V7:"], "Workflows"),
    ]
    
    # Categories for summary
    categories = {}
    total_tests = len(test_cases)
    passed_tests = 0
    failed_tests = []
    
    print("=== GAIA V7 Comprehensive Test Suite ===")
    print(f"Running {total_tests} tests...\n")
    
    start_time = time.time()
    
    # Run each test
    for i, test in enumerate(test_cases):
        print(f"[{i+1}/{total_tests}] {test.category}: {test.name}...", end=" ", flush=True)
        
        # Run test
        output, error = run_gaia_test(test.input)
        test.output = output
        test.error = error
        
        # Check result
        if error and error != "":
            if error == "TIMEOUT":
                print("TIMEOUT ⏱️")
            else:
                print("ERROR ❌")
            test.passed = False
        else:
            test.passed = check_output(output, test.expected)
            if test.passed:
                print("PASSED ✅")
                passed_tests += 1
            else:
                print("FAILED ❌")
                failed_tests.append(test)
        
        # Track category results
        if test.category not in categories:
            categories[test.category] = {"total": 0, "passed": 0}
        categories[test.category]["total"] += 1
        if test.passed:
            categories[test.category]["passed"] += 1
        
        # Progress update every 10 tests
        if (i + 1) % 10 == 0:
            print(f"\nProgress: {i+1}/{total_tests} completed ({passed_tests} passed)\n")
    
    end_time = time.time()
    elapsed = end_time - start_time
    
    # Print summary
    print("\n" + "="*50)
    print("TEST SUMMARY")
    print("="*50)
    print(f"Total Tests: {total_tests}")
    print(f"Passed: {passed_tests} ({passed_tests/total_tests*100:.1f}%)")
    print(f"Failed: {len(failed_tests)} ({len(failed_tests)/total_tests*100:.1f}%)")
    print(f"Execution Time: {elapsed:.2f} seconds")
    print(f"Average Time per Test: {elapsed/total_tests:.3f} seconds")
    
    # Category breakdown
    print("\n" + "="*50)
    print("CATEGORY BREAKDOWN")
    print("="*50)
    for category, stats in sorted(categories.items()):
        pass_rate = stats["passed"] / stats["total"] * 100
        print(f"{category:15} {stats['passed']:2}/{stats['total']:2} ({pass_rate:5.1f}%)")
    
    # Failed tests details
    if failed_tests:
        print("\n" + "="*50)
        print("FAILED TESTS DETAILS")
        print("="*50)
        for test in failed_tests[:10]:  # Show first 10 failures
            print(f"\n❌ {test.name}")
            print(f"   Input: {test.input[:50]}{'...' if len(test.input) > 50 else ''}")
            print(f"   Expected: {test.expected}")
            
            # Find GAIA output
            gaia_match = re.search(r'GAIA V7:.*', test.output)
            if gaia_match:
                gaia_line = gaia_match.group(0)[:100]
                print(f"   Got: {gaia_line}{'...' if len(gaia_line) >= 100 else ''}")
            else:
                print(f"   Got: No GAIA V7 response found")
            
            if test.error:
                print(f"   Error: {test.error}")
        
        if len(failed_tests) > 10:
            print(f"\n... and {len(failed_tests) - 10} more failed tests")
    
    # Success/failure message
    print("\n" + "="*50)
    if passed_tests == total_tests:
        print("🎉 ALL TESTS PASSED! 🎉")
    elif passed_tests >= total_tests * 0.8:
        print("✅ GOOD: Over 80% of tests passed")
    elif passed_tests >= total_tests * 0.6:
        print("⚠️  WARNING: Only 60-80% of tests passed")
    else:
        print("❌ CRITICAL: Less than 60% of tests passed")
    
    return passed_tests == total_tests

if __name__ == "__main__":
    success = run_test_suite()
    sys.exit(0 if success else 1)