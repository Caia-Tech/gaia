#!/usr/bin/env python3

import subprocess
import time
import sys
import re
import json
from typing import List, Tuple, Dict, Optional
from datetime import datetime

class TestResult:
    def __init__(self, name: str, category: str, passed: bool, 
                 expected: List[str], actual: str, duration: float, error: str = ""):
        self.name = name
        self.category = category
        self.passed = passed
        self.expected = expected
        self.actual = actual
        self.duration = duration
        self.error = error

class GAIATestRunner:
    def __init__(self, executable: str = "./gaia_chat_v7"):
        self.executable = executable
        self.results: List[TestResult] = []
        
    def run_test(self, name: str, category: str, input_text: str, 
                 expected_patterns: List[str], timeout: int = 5) -> TestResult:
        """Run a single test case"""
        start_time = time.time()
        
        try:
            # Run GAIA
            process = subprocess.Popen(
                [self.executable],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            stdout, stderr = process.communicate(
                input=input_text + "\nquit\n", 
                timeout=timeout
            )
            
            duration = time.time() - start_time
            
            # Extract GAIA response
            gaia_match = re.search(r'GAIA V7:(.+?)(?:You:|$)', stdout, re.DOTALL)
            actual_output = gaia_match.group(1).strip() if gaia_match else "No GAIA response found"
            
            # Check patterns
            passed = all(pattern in actual_output for pattern in expected_patterns)
            
            result = TestResult(name, category, passed, expected_patterns, 
                              actual_output, duration, stderr)
            
        except subprocess.TimeoutExpired:
            process.kill()
            duration = time.time() - start_time
            result = TestResult(name, category, False, expected_patterns, 
                              "TIMEOUT", duration, "Test timed out")
            
        except Exception as e:
            duration = time.time() - start_time
            result = TestResult(name, category, False, expected_patterns, 
                              "ERROR", duration, str(e))
        
        self.results.append(result)
        return result

    def run_test_suite(self, tests: List[Tuple[str, str, str, List[str]]]):
        """Run all tests and generate report"""
        total = len(tests)
        print(f"=== GAIA V7 Extended Test Suite ===")
        print(f"Started at: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"Running {total} tests...\n")
        
        for i, (name, category, input_text, expected) in enumerate(tests):
            print(f"[{i+1:3d}/{total}] {category:15} {name:40}", end=" ", flush=True)
            
            result = self.run_test(name, category, input_text, expected)
            
            if result.passed:
                print(f"✅ {result.duration:.3f}s")
            elif result.error == "Test timed out":
                print(f"⏱️  TIMEOUT")
            elif result.error:
                print(f"❌ ERROR")
            else:
                print(f"❌ {result.duration:.3f}s")
            
            # Progress update
            if (i + 1) % 25 == 0:
                passed_so_far = sum(1 for r in self.results if r.passed)
                print(f"\n--- Progress: {i+1}/{total} tests, {passed_so_far} passed ---\n")
    
    def generate_report(self):
        """Generate comprehensive test report"""
        total = len(self.results)
        passed = sum(1 for r in self.results if r.passed)
        failed = total - passed
        
        # Category statistics
        categories = {}
        for result in self.results:
            if result.category not in categories:
                categories[result.category] = {"total": 0, "passed": 0, "duration": 0}
            categories[result.category]["total"] += 1
            categories[result.category]["duration"] += result.duration
            if result.passed:
                categories[result.category]["passed"] += 1
        
        # Print summary
        print("\n" + "="*70)
        print("TEST SUMMARY")
        print("="*70)
        print(f"Total Tests: {total}")
        print(f"Passed: {passed} ({passed/total*100:.1f}%)")
        print(f"Failed: {failed} ({failed/total*100:.1f}%)")
        print(f"Total Duration: {sum(r.duration for r in self.results):.2f}s")
        
        # Category breakdown
        print("\n" + "="*70)
        print("CATEGORY PERFORMANCE")
        print("="*70)
        print(f"{'Category':<20} {'Passed':<10} {'Rate':<10} {'Avg Time':<10}")
        print("-"*70)
        
        for cat, stats in sorted(categories.items()):
            pass_rate = stats["passed"] / stats["total"] * 100
            avg_time = stats["duration"] / stats["total"]
            print(f"{cat:<20} {stats['passed']:>3}/{stats['total']:<3} "
                  f"{pass_rate:>6.1f}% {avg_time:>8.3f}s")
        
        # Failed tests
        failed_tests = [r for r in self.results if not r.passed]
        if failed_tests:
            print("\n" + "="*70)
            print("FAILED TESTS (First 15)")
            print("="*70)
            
            for i, result in enumerate(failed_tests[:15]):
                print(f"\n{i+1}. [{result.category}] {result.name}")
                print(f"   Expected: {result.expected}")
                if len(result.actual) > 100:
                    print(f"   Actual: {result.actual[:100]}...")
                else:
                    print(f"   Actual: {result.actual}")
                if result.error:
                    print(f"   Error: {result.error}")
        
        # Save detailed results to JSON
        self.save_results()
        
        return passed == total
    
    def save_results(self):
        """Save detailed results to JSON file"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"test_results_{timestamp}.json"
        
        data = {
            "timestamp": timestamp,
            "total_tests": len(self.results),
            "passed": sum(1 for r in self.results if r.passed),
            "results": [
                {
                    "name": r.name,
                    "category": r.category,
                    "passed": r.passed,
                    "expected": r.expected,
                    "actual": r.actual[:200],  # Truncate long outputs
                    "duration": r.duration,
                    "error": r.error
                }
                for r in self.results
            ]
        }
        
        with open(filename, 'w') as f:
            json.dump(data, f, indent=2)
        
        print(f"\nDetailed results saved to: {filename}")

def get_all_tests() -> List[Tuple[str, str, str, List[str]]]:
    """Define all test cases"""
    return [
        # Basic Arithmetic (15 tests)
        ("Add small numbers", "Arithmetic", "What is 2 plus 2?", ["2 plus 2 = 4"]),
        ("Add large numbers", "Arithmetic", "What is 999 plus 1?", ["999 plus 1 = 1000"]),
        ("Add three numbers", "Arithmetic", "What is 5 plus 3 plus 2?", ["GAIA V7:"]),
        ("Subtract positive", "Arithmetic", "What is 10 minus 3?", ["10 minus 3 = 7"]),
        ("Subtract to zero", "Arithmetic", "What is 5 minus 5?", ["5 minus 5 = 0"]),
        ("Subtract to negative", "Arithmetic", "What is 3 minus 5?", ["3 minus 5 = -2"]),
        ("Multiply small", "Arithmetic", "What is 3 times 4?", ["3 times 4 = 12"]),
        ("Multiply by zero", "Arithmetic", "What is 100 times 0?", ["100 times 0 = 0"]),
        ("Multiply negative", "Arithmetic", "What is -5 times 3?", ["-5 times 3 = -15"]),
        ("Divide evenly", "Arithmetic", "What is 20 divided by 4?", ["20 divided 4 = 5"]),
        ("Divide with remainder", "Arithmetic", "What is 10 divided by 3?", ["10 divided 3 = 3"]),
        ("Divide by one", "Arithmetic", "What is 7 divided by 1?", ["7 divided 1 = 7"]),
        ("Divide zero", "Arithmetic", "What is 0 divided by 5?", ["0 divided 5 = 0"]),
        ("Divide by zero", "Arithmetic", "What is 5 divided by 0?", ["Error: division by zero"]),
        ("Mixed arithmetic", "Arithmetic", "Calculate 10 plus 5 minus 3", ["GAIA V7:"]),
        
        # Word Problems (10 tests)
        ("Word add", "Word Problems", "What is five plus three?", ["5 plus 3 = 8"]),
        ("Word subtract", "Word Problems", "What is ten minus four?", ["10 minus 4 = 6"]),
        ("Word multiply", "Word Problems", "What is six times seven?", ["6 times 7 = 42"]),
        ("Word divide", "Word Problems", "What is twenty divided by five?", ["20 divided 5 = 4"]),
        ("Mixed words/numbers", "Word Problems", "What is 5 plus three?", ["5 plus 3 = 8"]),
        ("Calculate verb", "Word Problems", "Calculate 15 plus 25", ["15 plus 25 = 40"]),
        ("Compute verb", "Word Problems", "Compute 100 minus 37", ["100 minus 37 = 63"]),
        ("Find verb", "Word Problems", "Find the sum of 8 and 12", ["8", "12", "20"]),
        ("What equals", "Word Problems", "What equals 5 times 5?", ["5 times 5 = 25"]),
        ("Result of", "Word Problems", "What is the result of 10 divided by 2?", ["10 divided 2 = 5"]),
        
        # Multi-part Questions (15 tests)
        ("Two calculations", "Multi-part", "What is 5 plus 3? What is 10 minus 2?", 
         ["5 plus 3 = 8", "10 minus 2 = 8"]),
        ("Three calculations", "Multi-part", 
         "What is 2 times 3? What is 12 divided by 4? What is 10 plus 5?",
         ["2 times 3 = 6", "12 divided 4 = 3", "10 plus 5 = 15"]),
        ("With 'also'", "Multi-part", "What is 15 plus 27? Also, what is 50 minus 20?",
         ["15 plus 27 = 42", "50 minus 20 = 30"]),
        ("With 'and'", "Multi-part", "Calculate 5 times 5 and 10 times 10",
         ["5 times 5 = 25", "10 times 10 = 100"]),
        ("Question and explain", "Multi-part", 
         "What is 10 plus 20? Can you explain addition?",
         ["10 plus 20 = 30", "Addition is"]),
        ("Multiple with also", "Multi-part",
         "What is 100 minus 25? Also calculate 50 plus 50. Also find 10 times 5.",
         ["100 minus 25 = 75", "50 plus 50 = 100", "10 times 5 = 50"]),
        ("Comma separated", "Multi-part", "Calculate 5+5, 10+10, 15+15",
         ["GAIA V7:"]),
        ("Different operations", "Multi-part",
         "Add 5 and 3, subtract 10 from 20, multiply 4 by 6",
         ["5", "3", "8"]),
        ("Complex multi", "Multi-part",
         "What is 2 plus 2? Explain why. What is 3 times 3? Is it odd?",
         ["2 plus 2 = 4"]),
        ("Nested questions", "Multi-part",
         "What is 5 plus what is 3 plus 2?",
         ["GAIA V7:"]),
        ("Sequential calc", "Multi-part",
         "First calculate 10 plus 5. Then multiply the result by 2.",
         ["10 plus 5 = 15"]),
        ("Comparison", "Multi-part",
         "What is 10 plus 5? What is 20 minus 5? Which is larger?",
         ["10 plus 5 = 15", "20 minus 5 = 15"]),
        ("Related calcs", "Multi-part",
         "What is 5 squared? What is the square root of 25?",
         ["GAIA V7:"]),
        ("With context", "Multi-part",
         "If I have 10 apples and eat 3, how many are left? What if I eat 2 more?",
         ["10", "3", "7"]),
        ("Chain calc", "Multi-part",
         "Start with 100. Subtract 20. Then divide by 4. What's the result?",
         ["100", "20", "80"]),
        
        # Functions (10 tests)
        ("Factorial 5", "Functions", "What is factorial of 5?", ["factorial", "120"]),
        ("Factorial 0", "Functions", "What is factorial of 0?", ["factorial", "1"]),
        ("Factorial 10", "Functions", "Calculate the factorial of 10", ["factorial"]),
        ("Prime 17", "Functions", "Is 17 a prime number?", ["prime", "17"]),
        ("Prime 20", "Functions", "Is 20 prime?", ["prime", "20"]),
        ("Fibonacci 10", "Functions", "What is the 10th Fibonacci number?", ["fibonacci", "55"]),
        ("Fibonacci 1", "Functions", "What is the 1st Fibonacci number?", ["fibonacci", "1"]),
        ("Pi precision", "Functions", "Calculate pi to 5 decimal places", ["pi", "3.14"]),
        ("Define word", "Functions", "Define the word 'algorithm'", ["define"]),
        ("Explain concept", "Functions", "Explain quantum computing", ["explain"]),
        
        # Greetings & Chat (8 tests)
        ("Hello", "Greetings", "Hello", ["Hello"]),
        ("Hi there", "Greetings", "Hi there!", ["Hello"]),
        ("Good morning", "Greetings", "Good morning", ["Hello"]),
        ("Goodbye", "Greetings", "Goodbye", ["Goodbye"]),
        ("Bye", "Greetings", "Bye!", ["Goodbye"]),
        ("See you", "Greetings", "See you later", ["Goodbye"]),
        ("How are you", "Greetings", "How are you?", ["GAIA V7:"]),
        ("Thanks", "Greetings", "Thank you!", ["GAIA V7:"]),
        
        # Lists (8 tests)
        ("List colors", "Lists", "List three colors", ["red", "blue", "green"]),
        ("List animals", "Lists", "Can you list some animals?", ["Here are"]),
        ("List numbers", "Lists", "List the first 5 prime numbers", ["2", "3", "5"]),
        ("List request", "Lists", "Give me a list of fruits", ["Here are"]),
        ("Enumerate", "Lists", "Enumerate types of vehicles", ["Here are"]),
        ("Name some", "Lists", "Name some programming languages", ["Here are"]),
        ("What are", "Lists", "What are some planets?", ["Here are"]),
        ("Show me", "Lists", "Show me examples of shapes", ["Here are"]),
        
        # Edge Cases (15 tests)
        ("Empty input", "Edge Cases", "", ["Please provide some input"]),
        ("Just spaces", "Edge Cases", "     ", ["Please provide some input"]),
        ("Very long", "Edge Cases", "What is " + " plus 1" * 50 + "?", ["GAIA V7:"]),
        ("Special chars", "Edge Cases", "What is 5 + 3?", ["GAIA V7:"]),
        ("Unicode", "Edge Cases", "What is 五 plus 三?", ["GAIA V7:"]),
        ("Mixed case", "Edge Cases", "WhAt Is FiVe PlUs ThReE?", ["5 plus 3 = 8"]),
        ("Extra spaces", "Edge Cases", "What    is     5    plus    3?", ["5 plus 3 = 8"]),
        ("Punctuation", "Edge Cases", "What is 5 plus 3???!!!", ["5 plus 3 = 8"]),
        ("Quotes", "Edge Cases", 'What is "five" plus "three"?', ["5 plus 3 = 8"]),
        ("Parentheses", "Edge Cases", "What is (5 plus 3)?", ["5 plus 3 = 8"]),
        ("Newlines", "Edge Cases", "What is 5\nplus 3?", ["GAIA V7:"]),
        ("Tabs", "Edge Cases", "What is 5\tplus\t3?", ["GAIA V7:"]),
        ("HTML tags", "Edge Cases", "What is <b>5</b> plus <i>3</i>?", ["GAIA V7:"]),
        ("Repeated", "Edge Cases", "What is 5 plus 3? " * 5, ["5 plus 3 = 8"]),
        ("Nonsense", "Edge Cases", "asdfghjkl zxcvbnm qwerty", ["GAIA V7:"]),
        
        # Explanations (10 tests)
        ("Explain addition", "Explanations", "Explain what addition is",
         ["Addition is a mathematical operation"]),
        ("Explain multiply", "Explanations", "What is multiplication?",
         ["Addition is a mathematical operation"]),
        ("How subtract", "Explanations", "How does subtraction work?",
         ["Addition is a mathematical operation"]),
        ("Why divide", "Explanations", "Why do we divide numbers?",
         ["GAIA V7:"]),
        ("Describe calc", "Explanations", "Describe how to calculate percentages",
         ["GAIA V7:"]),
        ("What is math", "Explanations", "What is mathematics?",
         ["GAIA V7:"]),
        ("Define number", "Explanations", "Define what a number is",
         ["GAIA V7:"]),
        ("Explain zero", "Explanations", "Explain the concept of zero",
         ["GAIA V7:"]),
        ("How to add", "Explanations", "How do you add two numbers?",
         ["GAIA V7:"]),
        ("Purpose of", "Explanations", "What is the purpose of arithmetic?",
         ["GAIA V7:"]),
        
        # Error Handling (10 tests)
        ("Div by zero", "Errors", "What is 10 divided by 0?", ["Error: division by zero"]),
        ("Invalid calc", "Errors", "What is blue plus red?", ["GAIA V7:"]),
        ("Syntax error", "Errors", "What is 5 plus plus 3?", ["GAIA V7:"]),
        ("Missing number", "Errors", "What is plus 5?", ["GAIA V7:"]),
        ("Wrong operator", "Errors", "What is 5 pls 3?", ["GAIA V7:"]),
        ("Incomplete", "Errors", "What is 5 plus", ["GAIA V7:"]),
        ("Just operator", "Errors", "plus", ["GAIA V7:"]),
        ("Random symbols", "Errors", "@#$% * &^%$", ["GAIA V7:"]),
        ("SQL injection", "Errors", "'; DROP TABLE users; --", ["GAIA V7:"]),
        ("Command inject", "Errors", "$(rm -rf /)", ["GAIA V7:"]),
        
        # Performance (5 tests)
        ("Quick calc", "Performance", "5+3", ["GAIA V7:"]),
        ("Large factorial", "Performance", "What is factorial of 20?", ["factorial"]),
        ("Many parts", "Performance", 
         " ".join([f"What is {i} plus {i}?" for i in range(1, 11)]),
         ["1 plus 1 = 2"]),
        ("Rapid fire", "Performance", "2+2? 3+3? 4+4? 5+5?", ["GAIA V7:"]),
        ("Complex workflow", "Performance",
         "Calculate 15% of 200, then add 50, then divide by 2, explain each step",
         ["GAIA V7:"]),
    ]

def main():
    runner = GAIATestRunner()
    all_tests = get_all_tests()
    
    # Run tests
    runner.run_test_suite(all_tests)
    
    # Generate report
    success = runner.generate_report()
    
    # Overall result
    print("\n" + "="*70)
    if success:
        print("🎉 ALL TESTS PASSED! GAIA V7 is working perfectly! 🎉")
    else:
        passed = sum(1 for r in runner.results if r.passed)
        total = len(runner.results)
        if passed >= total * 0.8:
            print("✅ GOOD: Over 80% of tests passed. GAIA V7 is mostly working.")
        elif passed >= total * 0.6:
            print("⚠️  WARNING: 60-80% passed. Some features need attention.")
        else:
            print("❌ CRITICAL: Less than 60% passed. Major issues detected.")
    
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())