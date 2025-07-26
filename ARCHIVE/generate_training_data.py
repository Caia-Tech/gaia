#!/usr/bin/env python3
"""
Generate high-quality conversational and instructional training data for GAIA
"""

import random
import json

# Templates for conversational patterns
GREETINGS = [
    ("Hello!", "Hi there! How can I help you today?"),
    ("Good morning", "Good morning! What would you like to know?"),
    ("Hey", "Hey! What's on your mind?"),
    ("Hi", "Hello! How can I assist you?"),
    ("Greetings", "Greetings! What can I do for you?"),
]

FAREWELLS = [
    ("Goodbye", "Goodbye! Have a great day!"),
    ("Bye", "Bye! Take care!"),
    ("See you later", "See you later! Feel free to come back anytime!"),
    ("Thanks, bye", "You're welcome! Goodbye!"),
    ("Take care", "You too! Farewell!"),
]

# Question-Answer pairs
QA_PAIRS = [
    # Clarification patterns
    ("What do you mean?", "I mean that {previous_topic}. Would you like me to explain differently?"),
    ("Can you clarify?", "Of course! Let me rephrase that. {rephrased_explanation}"),
    ("I don't understand", "No problem, let me try again. {simpler_explanation}"),
    
    # Acknowledgments
    ("That makes sense", "Great! Is there anything else you'd like to know?"),
    ("I see", "Excellent! Do you have any other questions?"),
    ("Got it", "Perfect! What else can I help with?"),
    
    # Requests for help
    ("Can you help me?", "Absolutely! What do you need help with?"),
    ("I need assistance", "I'm here to help! What's the issue?"),
    ("Please explain", "I'd be happy to explain. What topic would you like me to cover?"),
]

# Instruction following patterns
INSTRUCTIONS = [
    # Math instructions
    ("Calculate {num1} plus {num2}", "The sum of {num1} and {num2} is {result}"),
    ("What is {num1} times {num2}?", "{num1} times {num2} equals {result}"),
    ("Add {num1} and {num2} together", "{num1} plus {num2} gives us {result}"),
    
    # List instructions  
    ("List three {category}", "Here are three {category}: {item1}, {item2}, and {item3}"),
    ("Name some {category}", "Some {category} include: {item1}, {item2}, {item3}, and {item4}"),
    ("Give me examples of {category}", "Examples of {category} are: {item1}, {item2}, and {item3}"),
    
    # Explanation instructions
    ("Explain {concept} simply", "{concept} is {simple_explanation}"),
    ("What is {concept}?", "{concept} is {definition}"),
    ("Define {concept}", "The definition of {concept} is: {definition}"),
    ("Tell me about {topic}", "{topic} is fascinating. {explanation}"),
]

# Complete the sentence patterns
COMPLETIONS = [
    ("The cat sat on the", "mat"),
    ("The cat sat on the", "chair"),
    ("The cat sat on the", "windowsill"),
    ("The dog ran through the", "park"),
    ("The dog ran through the", "field"),
    ("The sun rises in the", "east"),
    ("The sun sets in the", "west"),
    ("Once upon a time there was a", "brave knight"),
    ("Once upon a time there was a", "clever princess"),
    ("Once upon a time there was a", "magical forest"),
    ("To be or not to", "be, that is the question"),
    ("All that glitters is not", "gold"),
    ("A penny saved is a penny", "earned"),
    ("Better late than", "never"),
    ("Actions speak louder than", "words"),
    ("The early bird catches the", "worm"),
    ("When in Rome, do as the", "Romans do"),
]

# Context-appropriate responses
CONTEXTUAL = [
    ("I'm feeling sad", "I'm sorry to hear that. Is there anything I can do to help?"),
    ("I'm happy today", "That's wonderful! What's making you happy?"),
    ("I'm confused", "That's okay! What's confusing you? I'll try to help clarify."),
    ("This is difficult", "I understand it can be challenging. Let's work through it together."),
    ("I did it!", "Congratulations! That's fantastic! Well done!"),
    ("Thank you so much", "You're very welcome! I'm glad I could help!"),
    ("You're amazing", "Thank you! I'm here to help whenever you need me."),
]

# Multi-turn conversations
CONVERSATIONS = [
    [
        ("What's the weather like?", "I don't have access to current weather data, but I can help you understand weather patterns!"),
        ("Oh, what causes rain?", "Rain occurs when water vapor in clouds condenses into droplets that become heavy enough to fall."),
        ("That's interesting!", "Yes! Weather is fascinating. The water cycle is constantly moving water around our planet."),
    ],
    [
        ("Can you teach me something?", "Of course! What would you like to learn about?"),
        ("How about space?", "Space is amazing! Did you know that space is almost completely silent because sound needs air to travel?"),
        ("Why is that?", "Sound waves need molecules to vibrate through. In the vacuum of space, there are so few molecules that sound can't propagate."),
        ("Cool!", "It really is! Space is full of incredible phenomena like this."),
    ],
    [
        ("I need help with math", "I'd be happy to help with math! What type of problem are you working on?"),
        ("Addition", "Great! Addition is fundamental. Do you have a specific problem to solve?"),
        ("What's 25 plus 37?", "25 plus 37 equals 62. Would you like me to show you how to work through it?"),
        ("Yes please", "Sure! 25 + 37: First add 20 + 30 = 50, then 5 + 7 = 12, finally 50 + 12 = 62."),
    ],
]

def generate_variations(template, count=5):
    """Generate variations of a template with different values"""
    variations = []
    
    # Categories for list instructions
    categories = {
        "colors": ["red", "blue", "green", "yellow", "orange", "purple", "pink", "brown"],
        "animals": ["dog", "cat", "elephant", "lion", "tiger", "bear", "rabbit", "horse"],
        "fruits": ["apple", "banana", "orange", "grape", "strawberry", "mango", "pear", "cherry"],
        "countries": ["France", "Japan", "Brazil", "Canada", "Australia", "Egypt", "India", "Mexico"],
        "foods": ["pizza", "pasta", "sushi", "tacos", "curry", "salad", "soup", "sandwich"],
    }
    
    # Concepts for explanation
    concepts = {
        "gravity": "the force that attracts objects toward each other",
        "democracy": "a system of government where people vote for their leaders",
        "photosynthesis": "the process plants use to convert sunlight into energy",
        "evolution": "the gradual change in species over time through natural selection",
        "internet": "a global network of connected computers that share information",
    }
    
    for _ in range(count):
        prompt, response = template
        
        # Fill in numbers
        if "{num1}" in prompt:
            num1 = random.randint(1, 50)
            num2 = random.randint(1, 50)
            prompt = prompt.replace("{num1}", str(num1))
            prompt = prompt.replace("{num2}", str(num2))
            response = response.replace("{num1}", str(num1))
            response = response.replace("{num2}", str(num2))
            
            if "plus" in prompt or "Add" in prompt:
                response = response.replace("{result}", str(num1 + num2))
            elif "times" in prompt:
                response = response.replace("{result}", str(num1 * num2))
        
        # Fill in categories
        if "{category}" in prompt:
            cat_name = random.choice(list(categories.keys()))
            items = random.sample(categories[cat_name], min(4, len(categories[cat_name])))
            
            prompt = prompt.replace("{category}", cat_name)
            response = response.replace("{category}", cat_name)
            
            for i, item in enumerate(items):
                response = response.replace(f"{{item{i+1}}}", item)
        
        # Fill in concepts
        if "{concept}" in prompt:
            concept = random.choice(list(concepts.keys()))
            definition = concepts[concept]
            
            prompt = prompt.replace("{concept}", concept)
            response = response.replace("{concept}", concept)
            response = response.replace("{definition}", definition)
            response = response.replace("{simple_explanation}", definition)
        
        # Fill in topics
        if "{topic}" in prompt:
            topics = {
                "the ocean": "The ocean covers over 70% of Earth's surface and contains 97% of Earth's water",
                "mountains": "Mountains are formed by tectonic forces or volcanism and can affect local weather patterns",
                "computers": "Computers process information using binary code and have revolutionized how we work and communicate",
            }
            topic = random.choice(list(topics.keys()))
            explanation = topics[topic]
            
            prompt = prompt.replace("{topic}", topic)
            response = response.replace("{topic}", topic.capitalize())
            response = response.replace("{explanation}", explanation)
        
        variations.append((prompt, response))
    
    return variations

def generate_dataset():
    """Generate complete training dataset"""
    dataset = []
    
    # Add all static patterns
    dataset.extend(GREETINGS)
    dataset.extend(FAREWELLS)
    dataset.extend(CONTEXTUAL)
    dataset.extend(COMPLETIONS)
    
    # Add multi-turn conversations as connected sequences
    for conversation in CONVERSATIONS:
        conv_text = ""
        for prompt, response in conversation:
            if conv_text:
                conv_text += " "
            conv_text += f"{prompt} {response}"
        dataset.append(("CONVERSATION", conv_text))
    
    # Generate variations of templates
    for template in INSTRUCTIONS:
        dataset.extend(generate_variations(template, count=10))
    
    # Add question-answer pairs
    qa_expanded = []
    for prompt, response in QA_PAIRS:
        # Simple fill-ins
        response = response.replace("{previous_topic}", "the relationship between patterns and intelligence")
        response = response.replace("{rephrased_explanation}", "Patterns are recurring structures that help us understand and predict")
        response = response.replace("{simpler_explanation}", "Think of patterns like recipes - they tell us what usually happens next")
        qa_expanded.append((prompt, response))
    dataset.extend(qa_expanded)
    
    return dataset

def create_flowing_text(dataset, samples_per_pattern=3):
    """Convert dataset into flowing conversational text"""
    flowing_texts = []
    
    # Create multiple flowing conversations
    for _ in range(samples_per_pattern):
        # Mix different types of interactions
        conversation = []
        
        # Start with greeting
        greeting = random.choice([d for d in dataset if d[0] in [g[0] for g in GREETINGS]])
        conversation.append(f"{greeting[0]} {greeting[1]}")
        
        # Add some Q&A
        qa_samples = random.sample([d for d in dataset if len(d) == 2 and d[0] not in [g[0] for g in GREETINGS + FAREWELLS]], 5)
        for prompt, response in qa_samples:
            conversation.append(f"{prompt} {response}")
        
        # Add a completion
        completion = random.choice(COMPLETIONS)
        conversation.append(f"{completion[0]} {completion[1]}")
        
        # End with farewell
        farewell = random.choice(FAREWELLS)
        conversation.append(f"{farewell[0]} {farewell[1]}")
        
        flowing_texts.append(" ".join(conversation))
    
    # Also create focused pattern groups
    # Math-focused conversations
    math_patterns = [d for d in dataset if any(word in str(d[0]) for word in ["plus", "times", "Calculate", "Add"])]
    for _ in range(10):
        math_conv = []
        math_conv.append("Let's do some math together!")
        for prompt, response in random.sample(math_patterns, min(5, len(math_patterns))):
            math_conv.append(f"{prompt} {response}")
        math_conv.append("Great job with the math!")
        flowing_texts.append(" ".join(math_conv))
    
    # Explanation-focused conversations  
    explain_patterns = [d for d in dataset if any(word in str(d[0]) for word in ["Explain", "What is", "Define", "Tell me about"])]
    for _ in range(10):
        explain_conv = []
        explain_conv.append("I'd like to learn about some concepts.")
        for prompt, response in random.sample(explain_patterns, min(5, len(explain_patterns))):
            explain_conv.append(f"{prompt} {response}")
        explain_conv.append("Thanks for the explanations!")
        flowing_texts.append(" ".join(explain_conv))
    
    return flowing_texts

def main():
    print("Generating conversational training data for GAIA...")
    
    # Generate dataset
    dataset = generate_dataset()
    print(f"Generated {len(dataset)} prompt-response pairs")
    
    # Create flowing conversational text
    flowing_texts = create_flowing_text(dataset, samples_per_pattern=5)
    print(f"Created {len(flowing_texts)} flowing conversations")
    
    # Save as training file
    with open("datasets/conversational_training.txt", "w") as f:
        for text in flowing_texts:
            f.write(text + "\n\n")
    
    # Also save structured dataset for analysis
    structured_data = {
        "greetings": GREETINGS,
        "farewells": FAREWELLS,
        "completions": COMPLETIONS,
        "contextual": CONTEXTUAL,
        "conversations": CONVERSATIONS,
        "total_patterns": len(dataset),
        "total_conversations": len(flowing_texts)
    }
    
    with open("datasets/conversational_structured.json", "w") as f:
        json.dump(structured_data, f, indent=2)
    
    print("\nSample conversations:")
    for i, text in enumerate(flowing_texts[:3]):
        print(f"\n--- Conversation {i+1} ---")
        print(text[:200] + "..." if len(text) > 200 else text)
    
    print(f"\nTraining data saved to datasets/conversational_training.txt")
    print(f"Structured data saved to datasets/conversational_structured.json")
    
    # Statistics
    total_words = sum(len(text.split()) for text in flowing_texts)
    print(f"\nStatistics:")
    print(f"  Total words: {total_words:,}")
    print(f"  Average conversation length: {total_words // len(flowing_texts)} words")
    print(f"  Unique patterns: {len(dataset)}")

if __name__ == "__main__":
    main()