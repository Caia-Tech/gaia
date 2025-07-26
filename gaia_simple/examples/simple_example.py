"""
Simple Example of Using GAIA Simple

This example demonstrates how to use the GAIA Simple solver.
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from main import GAIASolver


def main():
    # Initialize the solver
    solver = GAIASolver()
    
    # Example questions
    questions = [
        "What is 15 * 23 + 47?",
        "What is the capital of Japan?",
        "Calculate the factorial of 10.",
    ]
    
    print("GAIA Simple - Example Usage\n")
    print("="*50)
    
    for i, question in enumerate(questions, 1):
        print(f"\nQuestion {i}: {question}")
        print("-"*50)
        
        try:
            # Solve the question
            result = solver.solve_sync(question)
            
            # Display results
            print(f"Answer: {result['answer']}")
            print(f"Confidence: {result['confidence']:.2f}")
            print(f"Explanation: {result['explanation'][:200]}...")
            
        except Exception as e:
            print(f"Error: {str(e)}")
        
        print("="*50)


if __name__ == "__main__":
    main()