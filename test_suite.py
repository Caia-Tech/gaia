#!/usr/bin/env python3
"""
GAIA V5 Comprehensive Test Suite
Tests various categories of prompts and analyzes performance
"""

import subprocess
import json
import time
import statistics
from collections import defaultdict, Counter
from datetime import datetime
import re
import sys

class TestCategory:
    MATH = "Mathematics"
    SCIENCE = "Science"
    PHILOSOPHY = "Philosophy"
    AMBIGUOUS = "Ambiguous"
    CONVERSATIONAL = "Conversational"
    KNOWLEDGE = "Knowledge"
    SEQUENCE = "Sequence"
    INCOMPLETE = "Incomplete"
    CONTEXTUAL = "Contextual"
    CREATIVE = "Creative"
    TECHNICAL = "Technical"
    LOGICAL = "Logical"
    TEMPORAL = "Temporal"
    SPATIAL = "Spatial"
    EMOTIONAL = "Emotional"

class TestCase:
    def __init__(self, prompt, category, keywords=None, requires_function=False):
        self.prompt = prompt
        self.category = category
        self.keywords = keywords or []
        self.requires_function = requires_function

# Comprehensive test cases
TEST_CASES = [
    # Mathematics
    TestCase("What is two plus two?", TestCategory.MATH, ["4", "four"], True),
    TestCase("What is five times three?", TestCategory.MATH, ["15", "fifteen"], True),
    TestCase("What is ten minus seven?", TestCategory.MATH, ["3", "three"]),
    TestCase("What is pi?", TestCategory.MATH, ["3.14"], True),
    TestCase("What is the derivative of x squared?", TestCategory.MATH, ["2x", "two x"], True),
    TestCase("Calculate 7 plus 8", TestCategory.MATH, ["15", "fifteen"]),
    TestCase("What is 100 divided by 4?", TestCategory.MATH, ["25", "twenty-five"]),
    TestCase("What's the factorial of 5?", TestCategory.MATH, ["120"]),
    
    # Science
    TestCase("What is gravity?", TestCategory.SCIENCE, ["force", "mass", "attraction"]),
    TestCase("How fast does light travel?", TestCategory.SCIENCE, ["speed", "fast", "299"]),
    TestCase("What is photosynthesis?", TestCategory.SCIENCE, ["plants", "light", "energy"]),
    TestCase("What is quantum mechanics?", TestCategory.SCIENCE, ["quantum", "probability", "particles"]),
    TestCase("What is DNA?", TestCategory.SCIENCE, ["genetic", "molecule", "heredity"]),
    TestCase("What causes rain?", TestCategory.SCIENCE, ["water", "clouds", "precipitation"]),
    TestCase("Why is the sky blue?", TestCategory.SCIENCE, ["light", "scatter", "wavelength"]),
    TestCase("What is entropy?", TestCategory.SCIENCE, ["disorder", "thermodynamics", "energy"]),
    TestCase("How do magnets work?", TestCategory.SCIENCE, ["field", "poles", "attract"]),
    TestCase("What is evolution?", TestCategory.SCIENCE, ["change", "species", "natural selection"]),
    
    # Philosophy
    TestCase("What is consciousness?", TestCategory.PHILOSOPHY, ["aware", "mind", "experience"]),
    TestCase("What is the meaning of life?", TestCategory.PHILOSOPHY, ["meaning", "purpose", "existence"]),
    TestCase("Is there free will?", TestCategory.PHILOSOPHY, ["choice", "determinism", "freedom"]),
    TestCase("What is reality?", TestCategory.PHILOSOPHY, ["exist", "perception", "truth"]),
    TestCase("What is truth?", TestCategory.PHILOSOPHY, ["fact", "reality", "knowledge"]),
    TestCase("What is beauty?", TestCategory.PHILOSOPHY, ["aesthetic", "perception", "subjective"]),
    TestCase("What is justice?", TestCategory.PHILOSOPHY, ["fair", "right", "moral"]),
    TestCase("What is knowledge?", TestCategory.PHILOSOPHY, ["understanding", "information", "truth"]),
    TestCase("Does God exist?", TestCategory.PHILOSOPHY, ["believe", "faith", "exist"]),
    TestCase("What is the self?", TestCategory.PHILOSOPHY, ["identity", "consciousness", "being"]),
    
    # Ambiguous (should trigger superposition)
    TestCase("The bank", TestCategory.AMBIGUOUS, ["river", "money", "holds", "near"]),
    TestCase("The spring", TestCategory.AMBIGUOUS, ["flowers", "coiled", "water", "brings"]),
    TestCase("The wave", TestCategory.AMBIGUOUS, ["crashed", "carried", "enthusiasm"]),
    TestCase("Time", TestCategory.AMBIGUOUS, ["flies", "heals", "waits"]),
    TestCase("Light", TestCategory.AMBIGUOUS, ["travels", "weighs", "illuminates"]),
    TestCase("Fire", TestCategory.AMBIGUOUS, ["burns", "warm", "dangerous"]),
    TestCase("The key", TestCategory.AMBIGUOUS, ["unlock", "important", "music"]),
    TestCase("The current", TestCategory.AMBIGUOUS, ["flow", "electric", "present"]),
    TestCase("The match", TestCategory.AMBIGUOUS, ["fire", "game", "pair"]),
    TestCase("The field", TestCategory.AMBIGUOUS, ["grass", "study", "force"]),
    
    # Conversational
    TestCase("Hello, how are you?", TestCategory.CONVERSATIONAL, ["well", "fine", "good", "help"]),
    TestCase("What's your name?", TestCategory.CONVERSATIONAL, ["gaia", "ai", "assistant"]),
    TestCase("Can you help me?", TestCategory.CONVERSATIONAL, ["yes", "help", "assist", "sure"]),
    TestCase("Tell me a story", TestCategory.CONVERSATIONAL, ["once", "upon", "time", "story"]),
    TestCase("Thank you", TestCategory.CONVERSATIONAL, ["welcome", "pleasure", "glad"]),
    TestCase("Good morning", TestCategory.CONVERSATIONAL, ["morning", "good", "day"]),
    TestCase("How's the weather?", TestCategory.CONVERSATIONAL, ["weather", "nice", "sunny", "rain"]),
    TestCase("What do you think?", TestCategory.CONVERSATIONAL, ["think", "believe", "opinion"]),
    TestCase("Nice to meet you", TestCategory.CONVERSATIONAL, ["nice", "meet", "too", "pleasure"]),
    TestCase("Goodbye", TestCategory.CONVERSATIONAL, ["bye", "farewell", "see"]),
    
    # Knowledge queries
    TestCase("What is a mammal?", TestCategory.KNOWLEDGE, ["animal", "warm", "milk", "fur"]),
    TestCase("What are neurons?", TestCategory.KNOWLEDGE, ["brain", "cell", "nerve", "signal"]),
    TestCase("Define democracy", TestCategory.KNOWLEDGE, ["government", "people", "vote", "rule"]),
    TestCase("What is a computer?", TestCategory.KNOWLEDGE, ["machine", "calculate", "process", "data"]),
    TestCase("What is the internet?", TestCategory.KNOWLEDGE, ["network", "connect", "global", "web"]),
    TestCase("What is artificial intelligence?", TestCategory.KNOWLEDGE, ["machine", "learning", "intelligence"]),
    TestCase("What is blockchain?", TestCategory.KNOWLEDGE, ["distributed", "ledger", "crypto", "chain"]),
    TestCase("What is a virus?", TestCategory.KNOWLEDGE, ["infect", "disease", "replicate"]),
    TestCase("What is capitalism?", TestCategory.KNOWLEDGE, ["economic", "private", "market"]),
    TestCase("What is psychology?", TestCategory.KNOWLEDGE, ["mind", "behavior", "study"]),
    
    # Sequences
    TestCase("What comes after 5 in fibonacci?", TestCategory.SEQUENCE, ["8", "eight"], True),
    TestCase("Continue: 2, 4, 6, 8", TestCategory.SEQUENCE, ["10", "ten"]),
    TestCase("Next in sequence: A, B, C", TestCategory.SEQUENCE, ["D"]),
    TestCase("Complete: 1, 1, 2, 3, 5", TestCategory.SEQUENCE, ["8", "eight"]),
    TestCase("What follows: red, orange, yellow", TestCategory.SEQUENCE, ["green"]),
    TestCase("Continue pattern: big, small, big", TestCategory.SEQUENCE, ["small"]),
    TestCase("Next: Monday, Tuesday", TestCategory.SEQUENCE, ["Wednesday"]),
    TestCase("Complete: 10, 20, 30", TestCategory.SEQUENCE, ["40", "forty"]),
    TestCase("Pattern: 1, 4, 9, 16", TestCategory.SEQUENCE, ["25", "twenty-five"]),
    TestCase("Sequence: dog, god, fog", TestCategory.SEQUENCE, ["hog", "log"]),
    
    # Incomplete sentences
    TestCase("The cat sat on the", TestCategory.INCOMPLETE, ["mat", "chair", "floor", "roof"]),
    TestCase("Once upon a time", TestCategory.INCOMPLETE, ["there", "lived", "was", "in"]),
    TestCase("To be or not to", TestCategory.INCOMPLETE, ["be"]),
    TestCase("In the beginning", TestCategory.INCOMPLETE, ["was", "there", "god", "created"]),
    TestCase("All that glitters is not", TestCategory.INCOMPLETE, ["gold"]),
    TestCase("A penny saved is", TestCategory.INCOMPLETE, ["penny", "earned"]),
    TestCase("When in Rome", TestCategory.INCOMPLETE, ["do", "as", "romans"]),
    TestCase("The early bird", TestCategory.INCOMPLETE, ["catches", "gets", "worm"]),
    TestCase("Actions speak louder than", TestCategory.INCOMPLETE, ["words"]),
    TestCase("Better late than", TestCategory.INCOMPLETE, ["never"]),
    
    # Contextual
    TestCase("After the rain comes", TestCategory.CONTEXTUAL, ["sun", "sunshine", "rainbow", "clear"]),
    TestCase("Before the storm", TestCategory.CONTEXTUAL, ["calm", "quiet", "clouds", "wind"]),
    TestCase("In spring the flowers", TestCategory.CONTEXTUAL, ["bloom", "grow", "blossom", "appear"]),
    TestCase("During winter we", TestCategory.CONTEXTUAL, ["cold", "snow", "freeze", "warm"]),
    TestCase("At night the stars", TestCategory.CONTEXTUAL, ["shine", "appear", "twinkle", "visible"]),
    TestCase("In the morning", TestCategory.CONTEXTUAL, ["wake", "sun", "breakfast", "rise"]),
    TestCase("Under the sea", TestCategory.CONTEXTUAL, ["fish", "water", "ocean", "deep"]),
    TestCase("Above the clouds", TestCategory.CONTEXTUAL, ["sky", "sun", "fly", "high"]),
    TestCase("Behind closed doors", TestCategory.CONTEXTUAL, ["secret", "private", "hidden"]),
    TestCase("Across the bridge", TestCategory.CONTEXTUAL, ["other side", "cross", "water"]),
    
    # Creative/Imaginative
    TestCase("Describe a sunset", TestCategory.CREATIVE, ["orange", "red", "beautiful", "sky"]),
    TestCase("What does happiness feel like?", TestCategory.CREATIVE, ["joy", "warm", "good", "smile"]),
    TestCase("Paint a picture with words", TestCategory.CREATIVE, ["color", "image", "see", "beautiful"]),
    TestCase("Imagine a perfect world", TestCategory.CREATIVE, ["peace", "happy", "no", "everyone"]),
    TestCase("What color is the wind?", TestCategory.CREATIVE, ["invisible", "clear", "see", "color"]),
    TestCase("How does music taste?", TestCategory.CREATIVE, ["sweet", "bitter", "taste", "sense"]),
    TestCase("What shape is love?", TestCategory.CREATIVE, ["heart", "round", "shape", "form"]),
    TestCase("Describe the sound of silence", TestCategory.CREATIVE, ["quiet", "nothing", "peaceful", "still"]),
    TestCase("What do dreams look like?", TestCategory.CREATIVE, ["images", "visions", "sleep", "see"]),
    TestCase("How heavy is a thought?", TestCategory.CREATIVE, ["weightless", "light", "heavy", "mind"]),
    
    # Technical
    TestCase("What is HTTP?", TestCategory.TECHNICAL, ["protocol", "web", "transfer", "hypertext"]),
    TestCase("Explain TCP/IP", TestCategory.TECHNICAL, ["protocol", "network", "internet", "transmission"]),
    TestCase("What is a database?", TestCategory.TECHNICAL, ["data", "store", "information", "organized"]),
    TestCase("How do algorithms work?", TestCategory.TECHNICAL, ["steps", "process", "solve", "instructions"]),
    TestCase("What is machine learning?", TestCategory.TECHNICAL, ["learn", "data", "pattern", "train"]),
    TestCase("Define API", TestCategory.TECHNICAL, ["interface", "application", "programming", "connect"]),
    TestCase("What is encryption?", TestCategory.TECHNICAL, ["secure", "code", "data", "protect"]),
    TestCase("Explain cloud computing", TestCategory.TECHNICAL, ["server", "internet", "remote", "service"]),
    TestCase("What is Git?", TestCategory.TECHNICAL, ["version", "control", "repository", "code"]),
    TestCase("How does WiFi work?", TestCategory.TECHNICAL, ["wireless", "network", "radio", "connect"]),
    
    # Logical reasoning
    TestCase("If Felix is a cat, what is Felix?", TestCategory.LOGICAL, ["animal", "mammal", "cat"], True),
    TestCase("All roses are flowers. This is a rose. What is it?", TestCategory.LOGICAL, ["flower"]),
    TestCase("If A equals B and B equals C, what does A equal?", TestCategory.LOGICAL, ["C"]),
    TestCase("True or false: All birds can fly", TestCategory.LOGICAL, ["false", "no", "not"]),
    TestCase("If it's raining, the ground is wet. The ground is wet. Is it raining?", TestCategory.LOGICAL, ["maybe", "not necessarily", "possibly"]),
    TestCase("Which is heavier: a pound of feathers or a pound of steel?", TestCategory.LOGICAL, ["same", "equal", "neither"]),
    TestCase("If today is Monday, what day was yesterday?", TestCategory.LOGICAL, ["Sunday"]),
    TestCase("Two plus two equals four. Four minus two equals?", TestCategory.LOGICAL, ["two", "2"]),
    TestCase("All dogs are animals. Spot is a dog. What is Spot?", TestCategory.LOGICAL, ["animal"]),
    TestCase("If X is greater than Y, and Y is greater than Z, compare X and Z", TestCategory.LOGICAL, ["greater", "larger", "bigger"]),
]

class GAIATestSuite:
    def __init__(self, executable="./gaia_chat_v5"):
        self.executable = executable
        self.results = defaultdict(list)
        
    def run_single_test(self, prompt, flags=""):
        """Run a single test and return the response"""
        cmd = f'echo -e "{prompt}\\nquit" | {self.executable} {flags} 2>&1'
        
        try:
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=5)
            
            # Extract GAIA's response
            for line in result.stdout.split('\n'):
                if 'gaia:' in line:
                    response = line.split('gaia:')[1].strip()
                    return response
                    
            return ""
        except subprocess.TimeoutExpired:
            return "[TIMEOUT]"
        except Exception as e:
            return f"[ERROR: {e}]"
    
    def check_keywords(self, response, keywords):
        """Check if response contains any of the keywords"""
        if not response or not keywords:
            return False
            
        response_lower = response.lower()
        return any(keyword.lower() in response_lower for keyword in keywords)
    
    def run_test_category(self, category, mode="baseline"):
        """Run all tests in a category"""
        flags = "--superposition" if mode == "superposition" else ""
        results = []
        
        category_tests = [tc for tc in TEST_CASES if tc.category == category]
        
        for test_case in category_tests:
            response = self.run_single_test(test_case.prompt, flags)
            
            result = {
                'prompt': test_case.prompt,
                'response': response,
                'has_response': len(response) > 0 and not response.startswith('['),
                'keyword_match': self.check_keywords(response, test_case.keywords),
                'response_length': len(response) if response else 0,
                'requires_function': test_case.requires_function
            }
            
            # Check if it's a function response
            if test_case.requires_function and result['has_response']:
                try:
                    float(response.split()[0])
                    result['is_function'] = True
                except:
                    result['is_function'] = False
            else:
                result['is_function'] = False
                
            results.append(result)
            
        return results
    
    def test_variety(self, prompt, num_runs=10):
        """Test response variety for a single prompt"""
        baseline_responses = []
        super_responses = []
        
        for i in range(num_runs):
            baseline_responses.append(self.run_single_test(prompt, ""))
            super_responses.append(self.run_single_test(prompt, "--superposition"))
            time.sleep(0.01)  # Ensure different seeds
            
        baseline_unique = len(set(baseline_responses))
        super_unique = len(set(super_responses))
        
        return {
            'prompt': prompt,
            'baseline_unique': baseline_unique,
            'super_unique': super_unique,
            'baseline_responses': baseline_responses,
            'super_responses': super_responses
        }
    
    def run_full_suite(self):
        """Run the complete test suite"""
        print("Running GAIA V5 Comprehensive Test Suite...")
        print("=" * 60)
        
        all_results = {
            'baseline': {},
            'superposition': {}
        }
        
        # Get all unique categories
        categories = list(set(tc.category for tc in TEST_CASES))
        
        # Run tests for each category in both modes
        for mode in ['baseline', 'superposition']:
            print(f"\n--- Testing {mode.upper()} mode ---")
            
            for category in categories:
                print(f"Testing {category}...", end=' ', flush=True)
                results = self.run_test_category(category, mode)
                all_results[mode][category] = results
                
                # Calculate quick stats
                responded = sum(1 for r in results if r['has_response'])
                matched = sum(1 for r in results if r['keyword_match'])
                print(f"{responded}/{len(results)} responded, {matched} keyword matches")
        
        # Test variety on ambiguous prompts
        print("\n--- Testing Response Variety ---")
        variety_results = []
        ambiguous_prompts = ["The bank", "The spring", "The wave", "Time", "Light"]
        
        for prompt in ambiguous_prompts:
            print(f"Testing '{prompt}'...", end=' ', flush=True)
            variety = self.test_variety(prompt)
            variety_results.append(variety)
            print(f"Baseline: {variety['baseline_unique']} unique, "
                  f"Superposition: {variety['super_unique']} unique")
        
        return all_results, variety_results
    
    def generate_report(self, all_results, variety_results):
        """Generate a detailed report"""
        report = []
        report.append("GAIA V5 Comprehensive Test Report")
        report.append("=" * 60)
        report.append(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report.append(f"Total test cases: {len(TEST_CASES)}")
        report.append("")
        
        # Overall statistics
        for mode in ['baseline', 'superposition']:
            report.append(f"\n{mode.upper()} MODE RESULTS:")
            report.append("-" * 40)
            
            total_tests = 0
            total_responded = 0
            total_matched = 0
            total_functions = 0
            category_stats = {}
            
            for category, results in all_results[mode].items():
                total_tests += len(results)
                responded = sum(1 for r in results if r['has_response'])
                matched = sum(1 for r in results if r['keyword_match'])
                functions = sum(1 for r in results if r.get('is_function', False))
                
                total_responded += responded
                total_matched += matched
                total_functions += functions
                
                category_stats[category] = {
                    'total': len(results),
                    'responded': responded,
                    'matched': matched,
                    'accuracy': (matched / responded * 100) if responded > 0 else 0
                }
            
            report.append(f"Total tests: {total_tests}")
            report.append(f"Response rate: {total_responded}/{total_tests} "
                         f"({total_responded/total_tests*100:.1f}%)")
            report.append(f"Keyword accuracy: {total_matched}/{total_responded} "
                         f"({total_matched/total_responded*100:.1f}%)")
            report.append(f"Function calls: {total_functions}")
            
            report.append("\nCategory breakdown:")
            for category, stats in sorted(category_stats.items()):
                report.append(f"  {category:20s}: {stats['responded']:3d}/{stats['total']:3d} "
                             f"responded, {stats['accuracy']:5.1f}% accurate")
        
        # Variety analysis
        report.append("\n\nVARIETY ANALYSIS:")
        report.append("-" * 40)
        
        total_variety_improvement = 0
        for result in variety_results:
            improvement = result['super_unique'] - result['baseline_unique']
            total_variety_improvement += improvement
            
            report.append(f"\n'{result['prompt']}':")
            report.append(f"  Baseline: {result['baseline_unique']} unique responses")
            report.append(f"  Superposition: {result['super_unique']} unique responses "
                         f"({'+' if improvement >= 0 else ''}{improvement})")
            
            # Show sample responses
            if result['super_unique'] > 1:
                unique_super = list(set(result['super_responses']))[:3]
                report.append(f"  Sample variety: {unique_super}")
        
        avg_improvement = total_variety_improvement / len(variety_results)
        report.append(f"\nAverage variety improvement: {avg_improvement:+.1f}")
        
        # Save report
        report_text = '\n'.join(report)
        with open('test_suite_report.txt', 'w') as f:
            f.write(report_text)
            
        # Also save JSON data for further analysis
        with open('test_suite_data.json', 'w') as f:
            json.dump({
                'timestamp': datetime.now().isoformat(),
                'results': all_results,
                'variety': variety_results
            }, f, indent=2)
            
        return report_text

def main():
    # Check if executable exists
    import os
    if not os.path.exists('./gaia_chat_v5'):
        print("Error: gaia_chat_v5 not found. Please compile it first.")
        sys.exit(1)
    
    # Run test suite
    suite = GAIATestSuite()
    all_results, variety_results = suite.run_full_suite()
    
    # Generate and print report
    report = suite.generate_report(all_results, variety_results)
    print("\n\n" + report)
    
    print("\nDetailed reports saved to:")
    print("  - test_suite_report.txt (human-readable)")
    print("  - test_suite_data.json (machine-readable)")

if __name__ == "__main__":
    main()