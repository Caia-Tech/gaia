#!/usr/bin/env python3
"""
GAIA V5 Quick Test Suite - Focused on key capabilities
"""

import subprocess
import json
import time
from collections import defaultdict
from datetime import datetime

# Core test cases covering main capabilities
QUICK_TESTS = {
    "Mathematics": [
        ("What is two plus two?", ["4", "four"], True),
        ("What is pi?", ["3.14"], True),
        ("What comes after 5 in fibonacci?", ["8", "eight"], True),
    ],
    "Science": [
        ("What is quantum mechanics?", ["quantum", "probability"], False),
        ("What is photosynthesis?", ["plants", "light", "energy"], False),
    ],
    "Ambiguous": [
        ("The bank", ["river", "money", "holds"], False),
        ("The spring", ["flowers", "coiled", "water"], False),
        ("The wave", ["crashed", "carried", "enthusiasm"], False),
    ],
    "Conversational": [
        ("Hello, how are you?", ["well", "fine", "good", "help"], False),
        ("What's your name?", ["gaia", "ai"], False),
    ],
    "Knowledge": [
        ("What is a mammal?", ["animal", "warm", "milk"], False),
        ("If Felix is a cat, what is Felix?", ["animal", "mammal"], True),
    ],
    "Incomplete": [
        ("The cat sat on the", ["mat", "chair", "floor"], False),
        ("To be or not to", ["be"], False),
    ],
}

def run_test(prompt, executable="./gaia_chat_v5", flags=""):
    """Run a single test"""
    cmd = f'echo -e "{prompt}\\nquit" | {executable} {flags} 2>&1'
    
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=3)
        
        for line in result.stdout.split('\n'):
            if 'gaia:' in line:
                return line.split('gaia:')[1].strip()
                
        return ""
    except:
        return ""

def check_keywords(response, keywords):
    """Check if response contains keywords"""
    if not response or not keywords:
        return False
    response_lower = response.lower()
    return any(keyword.lower() in response_lower for keyword in keywords)

def test_variety(prompt, runs=5):
    """Test response variety"""
    baseline_responses = []
    super_responses = []
    
    for i in range(runs):
        baseline_responses.append(run_test(prompt))
        super_responses.append(run_test(prompt, flags="--superposition"))
        time.sleep(0.01)
        
    return {
        'baseline_unique': len(set(baseline_responses)),
        'super_unique': len(set(super_responses)),
        'baseline_first': baseline_responses[0] if baseline_responses else "",
        'super_samples': list(set(super_responses))[:3]
    }

def main():
    print("GAIA V5 Quick Test Suite")
    print("=" * 50)
    
    results = {'baseline': {}, 'superposition': {}}
    
    # Test each category
    for category, tests in QUICK_TESTS.items():
        print(f"\n{category}:")
        
        for mode in ['baseline', 'superposition']:
            flags = "--superposition" if mode == "superposition" else ""
            cat_results = []
            
            for prompt, keywords, is_function in tests:
                response = run_test(prompt, flags=flags)
                has_response = len(response) > 0
                keyword_match = check_keywords(response, keywords)
                
                cat_results.append({
                    'prompt': prompt,
                    'response': response,
                    'has_response': has_response,
                    'keyword_match': keyword_match,
                    'is_function': is_function
                })
                
            results[mode][category] = cat_results
            
            # Print summary
            responded = sum(1 for r in cat_results if r['has_response'])
            matched = sum(1 for r in cat_results if r['keyword_match'])
            print(f"  {mode}: {responded}/{len(tests)} responded, {matched} matched")
    
    # Test variety on ambiguous prompts
    print("\n\nVariety Test:")
    print("-" * 50)
    
    variety_prompts = ["The bank", "The spring", "The wave"]
    total_improvement = 0
    
    for prompt in variety_prompts:
        variety = test_variety(prompt)
        improvement = variety['super_unique'] - variety['baseline_unique']
        total_improvement += improvement
        
        print(f"\n'{prompt}':")
        print(f"  Baseline: {variety['baseline_unique']} unique → '{variety['baseline_first']}'")
        print(f"  Superposition: {variety['super_unique']} unique → {variety['super_samples']}")
        print(f"  Improvement: {'+' if improvement >= 0 else ''}{improvement}")
    
    avg_improvement = total_improvement / len(variety_prompts)
    
    # Summary
    print("\n\nSUMMARY")
    print("=" * 50)
    
    for mode in ['baseline', 'superposition']:
        total_tests = sum(len(tests) for tests in QUICK_TESTS.values())
        total_responded = sum(
            sum(1 for r in results[mode][cat] if r['has_response'])
            for cat in QUICK_TESTS.keys()
        )
        total_matched = sum(
            sum(1 for r in results[mode][cat] if r['keyword_match'])
            for cat in QUICK_TESTS.keys()
        )
        
        print(f"\n{mode.upper()}:")
        print(f"  Response rate: {total_responded}/{total_tests} ({total_responded/total_tests*100:.1f}%)")
        print(f"  Keyword accuracy: {total_matched}/{total_responded} ({total_matched/total_responded*100:.1f}%)")
        
        # Function accuracy
        function_tests = sum(
            sum(1 for r in results[mode][cat] if r['is_function'] and r['has_response'])
            for cat in QUICK_TESTS.keys()
        )
        print(f"  Function calls: {function_tests}")
    
    print(f"\nAverage variety improvement: {avg_improvement:+.1f}")
    
    # Save results
    with open('quick_test_results.json', 'w') as f:
        json.dump({
            'timestamp': datetime.now().isoformat(),
            'results': results,
            'summary': {
                'baseline_response_rate': f"{total_responded}/{total_tests}",
                'variety_improvement': avg_improvement
            }
        }, f, indent=2)
    
    print("\nResults saved to quick_test_results.json")

if __name__ == "__main__":
    main()