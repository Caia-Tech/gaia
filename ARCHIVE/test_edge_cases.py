#!/usr/bin/env python3
"""
GAIA V5 Edge Cases and Stress Test Suite
"""

import subprocess
import time
import random
import string

# Edge case categories
EDGE_CASES = {
    "Empty/Minimal": [
        "",  # Empty input
        " ",  # Just space
        "?",  # Single punctuation
        "a",  # Single letter
        "...",  # Just dots
    ],
    
    "Very Long": [
        "What " * 50 + "is happening?",  # Repetitive long
        " ".join([f"word{i}" for i in range(100)]),  # Many words
        "a" * 500,  # Very long single word
    ],
    
    "Special Characters": [
        "Hello! @#$%^&*()",  # Special chars
        "What is 2+2=?",  # Math symbols
        "Test\nwith\nnewlines",  # Newlines
        "Test\twith\ttabs",  # Tabs
        "Unicode: 你好 مرحبا שלום",  # Unicode
    ],
    
    "Nonsense": [
        "xyzqwerty asdfgh",  # Random letters
        "123 456 789",  # Just numbers
        "the the the the",  # Repetition
        "".join(random.choices(string.ascii_lowercase, k=20)),  # Random string
    ],
    
    "Edge Math": [
        "What is 0 plus 0?",  # Zero
        "What is -5 plus 3?",  # Negative (if supported)
        "What is 999999999 plus 1?",  # Large numbers
        "What is 1.5 plus 2.5?",  # Decimals
    ],
    
    "Malformed Questions": [
        "What is",  # Incomplete
        "plus two three",  # Wrong order
        "The the bank bank",  # Duplicates
        "?????",  # Multiple question marks
    ],
    
    "Context Bombs": [
        " ".join(["word"] * 200),  # Exceed context window
        "The " + " and the ".join([f"item{i}" for i in range(50)]),  # Long list
    ],
    
    "Mixed Languages": [
        "Hello, comment ça va?",  # English/French
        "What is dos plus dos?",  # English/Spanish math
    ],
}

def run_test_safe(prompt, executable="./gaia_chat_v5", flags="", timeout=2):
    """Run test with safety measures"""
    # Escape prompt for shell
    escaped_prompt = prompt.replace('"', '\\"').replace('\\n', '\\\\n')
    cmd = f'echo -e "{escaped_prompt}\\nquit" | {executable} {flags} 2>&1'
    
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, 
                              text=True, timeout=timeout)
        
        for line in result.stdout.split('\n'):
            if 'gaia:' in line:
                return line.split('gaia:')[1].strip()
                
        return "[NO RESPONSE]"
    except subprocess.TimeoutExpired:
        return "[TIMEOUT]"
    except Exception as e:
        return f"[ERROR: {type(e).__name__}]"

def test_response_time(prompt, runs=5):
    """Measure response time"""
    times = []
    
    for _ in range(runs):
        start = time.time()
        response = run_test_safe(prompt)
        end = time.time()
        
        if not response.startswith("["):
            times.append(end - start)
    
    if times:
        return {
            'avg_time': sum(times) / len(times),
            'min_time': min(times),
            'max_time': max(times)
        }
    else:
        return None

def stress_test_concurrent(prompt, num_concurrent=10):
    """Test concurrent requests"""
    from concurrent.futures import ThreadPoolExecutor, as_completed
    
    with ThreadPoolExecutor(max_workers=num_concurrent) as executor:
        futures = [executor.submit(run_test_safe, prompt) for _ in range(num_concurrent)]
        responses = [f.result() for f in as_completed(futures)]
    
    unique_responses = set(responses)
    return {
        'total': num_concurrent,
        'unique': len(unique_responses),
        'responses': list(unique_responses)[:5]  # Sample
    }

def main():
    print("GAIA V5 Edge Cases and Stress Test")
    print("=" * 50)
    
    # Test edge cases
    print("\n--- EDGE CASE TESTING ---")
    edge_results = {}
    
    for category, cases in EDGE_CASES.items():
        print(f"\n{category}:")
        results = []
        
        for prompt in cases:
            response_baseline = run_test_safe(prompt)
            response_super = run_test_safe(prompt, flags="--superposition")
            
            # Classify response
            if response_baseline.startswith("["):
                status = response_baseline
            elif len(response_baseline) == 0:
                status = "[EMPTY]"
            elif len(response_baseline) > 100:
                status = f"[LONG: {len(response_baseline)} chars]"
            else:
                status = "OK"
            
            results.append({
                'prompt': prompt[:50] + "..." if len(prompt) > 50 else prompt,
                'status': status,
                'baseline_response': response_baseline[:50],
                'super_response': response_super[:50],
                'differs': response_baseline != response_super
            })
            
            # Print summary
            print(f"  '{results[-1]['prompt']}' → {status}")
        
        edge_results[category] = results
    
    # Performance testing
    print("\n\n--- PERFORMANCE TESTING ---")
    
    test_prompts = [
        ("Simple", "Hello"),
        ("Math", "What is two plus two?"),
        ("Complex", "What is the meaning of life?"),
        ("Ambiguous", "The bank"),
    ]
    
    for name, prompt in test_prompts:
        timing = test_response_time(prompt)
        if timing:
            print(f"\n{name} ('{prompt}'):")
            print(f"  Avg response time: {timing['avg_time']*1000:.1f}ms")
            print(f"  Range: {timing['min_time']*1000:.1f}ms - {timing['max_time']*1000:.1f}ms")
    
    # Stress testing
    print("\n\n--- STRESS TESTING ---")
    
    print("\nConcurrent requests (10 simultaneous):")
    for name, prompt in [("Math", "What is two plus two?"), ("Ambiguous", "The bank")]:
        result = stress_test_concurrent(prompt, num_concurrent=10)
        print(f"\n{name}:")
        print(f"  Unique responses: {result['unique']}/{result['total']}")
        print(f"  Samples: {result['responses'][:3]}")
    
    # Context window test
    print("\n\nContext window stress test:")
    for size in [50, 100, 150, 200]:
        prompt = " ".join([f"word{i}" for i in range(size)])
        response = run_test_safe(prompt)
        status = "OK" if not response.startswith("[") else response
        print(f"  {size} words: {status}")
    
    # Summary
    print("\n\n--- SUMMARY ---")
    total_edge_cases = sum(len(cases) for cases in EDGE_CASES.values())
    failed_cases = sum(
        1 for results in edge_results.values() 
        for r in results 
        if r['status'].startswith("[") and r['status'] != "[EMPTY]"
    )
    
    print(f"Total edge cases tested: {total_edge_cases}")
    print(f"Failed cases: {failed_cases}")
    print(f"Success rate: {(total_edge_cases - failed_cases) / total_edge_cases * 100:.1f}%")
    
    # Check for crashes or hangs
    crash_count = sum(
        1 for results in edge_results.values()
        for r in results
        if r['status'] in ["[TIMEOUT]", "[ERROR]"]
    )
    
    if crash_count > 0:
        print(f"\n⚠️  WARNING: {crash_count} cases caused timeouts or errors!")
    else:
        print("\n✓ No crashes or timeouts detected")
    
    # Save detailed results
    import json
    with open('edge_case_results.json', 'w') as f:
        json.dump({
            'edge_results': edge_results,
            'summary': {
                'total_cases': total_edge_cases,
                'failed_cases': failed_cases,
                'crash_count': crash_count
            }
        }, f, indent=2)
    
    print("\nDetailed results saved to edge_case_results.json")

if __name__ == "__main__":
    main()