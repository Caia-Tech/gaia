#!/usr/bin/env python3
"""
Generate instruction-following training data for GAIA
Focus on clear command-response patterns
"""

import random

INSTRUCTION_TEMPLATES = {
    "math_operations": [
        ("Calculate {n1} + {n2}", lambda n1, n2: f"{n1} + {n2} = {n1 + n2}"),
        ("What is {n1} plus {n2}?", lambda n1, n2: f"{n1} plus {n2} equals {n1 + n2}"),
        ("Add {n1} to {n2}", lambda n1, n2: f"Adding {n1} to {n2} gives {n1 + n2}"),
        ("Compute {n1} - {n2}", lambda n1, n2: f"{n1} - {n2} = {n1 - n2}"),
        ("What is {n1} minus {n2}?", lambda n1, n2: f"{n1} minus {n2} equals {n1 - n2}"),
        ("Subtract {n2} from {n1}", lambda n1, n2: f"Subtracting {n2} from {n1} gives {n1 - n2}"),
        ("Multiply {n1} by {n2}", lambda n1, n2: f"{n1} × {n2} = {n1 * n2}"),
        ("What is {n1} times {n2}?", lambda n1, n2: f"{n1} times {n2} equals {n1 * n2}"),
        ("Find the product of {n1} and {n2}", lambda n1, n2: f"The product of {n1} and {n2} is {n1 * n2}"),
    ],
    
    "comparisons": [
        ("Which is larger: {n1} or {n2}?", lambda n1, n2: f"{max(n1, n2)} is larger"),
        ("Compare {n1} and {n2}", lambda n1, n2: f"{n1} is {'greater than' if n1 > n2 else 'less than' if n1 < n2 else 'equal to'} {n2}"),
        ("Is {n1} greater than {n2}?", lambda n1, n2: f"{'Yes' if n1 > n2 else 'No'}, {n1} is {'greater than' if n1 > n2 else 'not greater than'} {n2}"),
        ("What's the maximum of {n1} and {n2}?", lambda n1, n2: f"The maximum is {max(n1, n2)}"),
        ("What's the minimum of {n1} and {n2}?", lambda n1, n2: f"The minimum is {min(n1, n2)}"),
    ],
    
    "lists": [
        ("List 5 {category}", lambda cat, items: f"Here are 5 {cat}: {', '.join(items[:5])}"),
        ("Give me {n} examples of {category}", lambda n, cat, items: f"Here are {n} {cat}: {', '.join(items[:n])}"),
        ("Name some {category}", lambda cat, items: f"Some {cat} include: {', '.join(items[:4])}"),
        ("What are examples of {category}?", lambda cat, items: f"Examples of {cat} are: {', '.join(items[:5])}"),
        ("Show me {category}", lambda cat, items: f"Here are some {cat}: {', '.join(items[:4])}"),
    ],
    
    "definitions": [
        ("What is {term}?", lambda term, def_: f"{term} is {def_}"),
        ("Define {term}", lambda term, def_: f"Definition: {term} is {def_}"),
        ("Explain {term}", lambda term, def_: f"{term} means {def_}"),
        ("What does {term} mean?", lambda term, def_: f"{term} refers to {def_}"),
        ("Tell me about {term}", lambda term, def_: f"{term} is {def_}"),
    ],
    
    "actions": [
        ("Reverse the word '{word}'", lambda word: f"'{word}' reversed is '{word[::-1]}'"),
        ("Spell '{word}' backwards", lambda word: f"'{word}' spelled backwards is '{word[::-1]}'"),
        ("Count the letters in '{word}'", lambda word: f"'{word}' has {len(word)} letters"),
        ("Is '{word}' a palindrome?", lambda word: f"{'Yes' if word == word[::-1] else 'No'}, '{word}' is {'a palindrome' if word == word[::-1] else 'not a palindrome'}"),
        ("Capitalize '{word}'", lambda word: f"'{word}' capitalized is '{word.upper()}'"),
        ("Make '{word}' lowercase", lambda word: f"'{word}' in lowercase is '{word.lower()}'"),
    ],
    
    "sequences": [
        ("What comes after {n} in counting?", lambda n: f"After {n} comes {n + 1}"),
        ("What comes before {n}?", lambda n: f"Before {n} comes {n - 1}"),
        ("Continue the sequence: {n1}, {n2}, {n3}", lambda n1, n2, n3: f"The next number is {n3 + (n2 - n1)}"),
        ("What's next: {n1}, {n2}, {n3}?", lambda n1, n2, n3: f"Next comes {n3 + (n2 - n1)}"),
        ("Complete: {n1}, {n2}, ?", lambda n1, n2: f"The pattern continues with {n2 + (n2 - n1)}"),
    ],
    
    "yes_no": [
        ("Is {n1} even?", lambda n1: f"{'Yes' if n1 % 2 == 0 else 'No'}, {n1} is {'even' if n1 % 2 == 0 else 'odd'}"),
        ("Is {n1} odd?", lambda n1: f"{'Yes' if n1 % 2 == 1 else 'No'}, {n1} is {'odd' if n1 % 2 == 1 else 'even'}"),
        ("Is {n1} positive?", lambda n1: f"{'Yes' if n1 > 0 else 'No'}, {n1} is {'positive' if n1 > 0 else 'negative' if n1 < 0 else 'zero'}"),
        ("Is {n1} divisible by {n2}?", lambda n1, n2: f"{'Yes' if n2 != 0 and n1 % n2 == 0 else 'No'}, {n1} {'is' if n2 != 0 and n1 % n2 == 0 else 'is not'} divisible by {n2}"),
    ],
    
    "conversions": [
        ("Convert {n} hours to minutes", lambda n: f"{n} hours = {n * 60} minutes"),
        ("How many seconds in {n} minutes?", lambda n: f"{n} minutes = {n * 60} seconds"),
        ("Convert {n} days to hours", lambda n: f"{n} days = {n * 24} hours"),
        ("How many minutes in {n} hours?", lambda n: f"{n} hours = {n * 60} minutes"),
    ],
}

# Data for categories
CATEGORIES = {
    "colors": ["red", "blue", "green", "yellow", "orange", "purple", "pink", "brown", "black", "white"],
    "animals": ["dog", "cat", "elephant", "lion", "tiger", "bear", "rabbit", "horse", "eagle", "dolphin"],
    "fruits": ["apple", "banana", "orange", "grape", "strawberry", "mango", "pear", "cherry", "peach", "plum"],
    "vegetables": ["carrot", "broccoli", "spinach", "tomato", "cucumber", "lettuce", "potato", "onion"],
    "countries": ["France", "Japan", "Brazil", "Canada", "Australia", "Egypt", "India", "Mexico", "Italy", "Spain"],
    "programming languages": ["Python", "JavaScript", "Java", "C++", "Ruby", "Go", "Rust", "Swift"],
    "planets": ["Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"],
    "emotions": ["happy", "sad", "angry", "excited", "nervous", "calm", "confused", "proud"],
    "sports": ["soccer", "basketball", "tennis", "baseball", "swimming", "running", "cycling", "golf"],
    "instruments": ["piano", "guitar", "violin", "drums", "flute", "trumpet", "saxophone", "cello"],
}

# Definitions
DEFINITIONS = {
    "algorithm": "a step-by-step procedure for solving a problem",
    "democracy": "a system of government where citizens vote for their leaders",
    "photosynthesis": "the process by which plants convert sunlight into energy",
    "gravity": "the force that attracts objects toward each other",
    "evolution": "the gradual change in species over time through natural selection",
    "ecosystem": "a community of living organisms and their environment",
    "metabolism": "the chemical processes that occur within a living organism",
    "inflation": "the general increase in prices over time",
    "renewable energy": "energy from sources that naturally replenish",
    "artificial intelligence": "computer systems that can perform tasks requiring human intelligence",
    "blockchain": "a distributed ledger technology for recording transactions",
    "quantum mechanics": "the physics of matter at atomic and subatomic scales",
    "climate change": "long-term shifts in global temperatures and weather patterns",
    "biodiversity": "the variety of life forms in a particular habitat",
    "democracy": "government by the people through elected representatives",
}

# Common words for actions
COMMON_WORDS = ["hello", "world", "python", "computer", "science", "level", "radar", "civic", 
                "amazing", "simple", "complex", "beautiful", "quick", "learn", "teach", "create"]

def generate_instruction_examples():
    """Generate diverse instruction-response pairs"""
    examples = []
    
    # Math operations
    for template, func in INSTRUCTION_TEMPLATES["math_operations"]:
        for _ in range(5):
            n1 = random.randint(1, 100)
            n2 = random.randint(1, 100)
            prompt = template.format(n1=n1, n2=n2)
            response = func(n1, n2)
            examples.append((prompt, response))
    
    # Comparisons
    for template, func in INSTRUCTION_TEMPLATES["comparisons"]:
        for _ in range(3):
            n1 = random.randint(1, 100)
            n2 = random.randint(1, 100)
            prompt = template.format(n1=n1, n2=n2)
            response = func(n1, n2)
            examples.append((prompt, response))
    
    # Lists
    for template, func in INSTRUCTION_TEMPLATES["lists"]:
        for category, items in CATEGORIES.items():
            if "{n}" in template:
                n = random.randint(3, min(7, len(items)))
                prompt = template.format(n=n, category=category)
                response = func(n, category, random.sample(items, len(items)))
            else:
                prompt = template.format(category=category)
                response = func(category, random.sample(items, len(items)))
            examples.append((prompt, response))
    
    # Definitions
    for template, func in INSTRUCTION_TEMPLATES["definitions"]:
        for term, definition in list(DEFINITIONS.items())[:10]:
            prompt = template.format(term=term)
            response = func(term, definition)
            examples.append((prompt, response))
    
    # Word actions
    for template, func in INSTRUCTION_TEMPLATES["actions"]:
        for word in random.sample(COMMON_WORDS, 5):
            prompt = template.format(word=word)
            response = func(word)
            examples.append((prompt, response))
    
    # Sequences
    for template, func in INSTRUCTION_TEMPLATES["sequences"]:
        for _ in range(5):
            if "{n3}" in template:
                n1 = random.randint(1, 20)
                step = random.randint(1, 5)
                n2 = n1 + step
                n3 = n2 + step
                prompt = template.format(n1=n1, n2=n2, n3=n3)
                response = func(n1, n2, n3)
            elif "{n2}" in template:
                n1 = random.randint(1, 20)
                step = random.randint(1, 5)
                n2 = n1 + step
                prompt = template.format(n1=n1, n2=n2)
                response = func(n1, n2)
            else:
                n = random.randint(1, 50)
                prompt = template.format(n=n)
                response = func(n)
            examples.append((prompt, response))
    
    # Yes/No questions
    for template, func in INSTRUCTION_TEMPLATES["yes_no"]:
        for _ in range(3):
            if "{n2}" in template:
                n1 = random.randint(1, 100)
                n2 = random.randint(2, 10)
                prompt = template.format(n1=n1, n2=n2)
                response = func(n1, n2)
            else:
                n1 = random.randint(-50, 50)
                prompt = template.format(n1=n1)
                response = func(n1)
            examples.append((prompt, response))
    
    # Conversions
    for template, func in INSTRUCTION_TEMPLATES["conversions"]:
        for n in [1, 2, 5, 10, 24]:
            prompt = template.format(n=n)
            response = func(n)
            examples.append((prompt, response))
    
    return examples

def create_instruction_sequences(examples):
    """Create flowing instruction sequences"""
    sequences = []
    
    # Group by type
    math_examples = [e for e in examples if any(word in e[0] for word in ["Calculate", "plus", "minus", "times", "Add", "Subtract"])]
    list_examples = [e for e in examples if any(word in e[0] for word in ["List", "examples", "Name some"])]
    definition_examples = [e for e in examples if any(word in e[0] for word in ["What is", "Define", "Explain", "mean"])]
    action_examples = [e for e in examples if any(word in e[0] for word in ["Reverse", "Count", "Capitalize", "spell"])]
    
    # Create focused instruction sessions
    for _ in range(10):
        # Math session
        math_session = ["Let me help you with some calculations."]
        for prompt, response in random.sample(math_examples, min(5, len(math_examples))):
            math_session.append(f"{prompt} {response}")
        math_session.append("Those are the calculations!")
        sequences.append(" ".join(math_session))
        
        # Definition session
        def_session = ["I'll explain some concepts for you."]
        for prompt, response in random.sample(definition_examples, min(4, len(definition_examples))):
            def_session.append(f"{prompt} {response}")
        def_session.append("I hope those definitions help!")
        sequences.append(" ".join(def_session))
        
        # Mixed session
        mixed_session = ["I can help with various tasks."]
        all_examples = random.sample(examples, min(6, len(examples)))
        for prompt, response in all_examples:
            mixed_session.append(f"{prompt} {response}")
        mixed_session.append("Is there anything else you need?")
        sequences.append(" ".join(mixed_session))
    
    return sequences

def main():
    print("Generating instruction-following training data...")
    
    # Generate examples
    examples = generate_instruction_examples()
    print(f"Generated {len(examples)} instruction-response pairs")
    
    # Create sequences
    sequences = create_instruction_sequences(examples)
    print(f"Created {len(sequences)} instruction sequences")
    
    # Add individual examples as single exchanges
    for prompt, response in examples:
        sequences.append(f"{prompt} {response}")
    
    # Save training data
    with open("datasets/instruction_training.txt", "w") as f:
        for seq in sequences:
            f.write(seq + "\n\n")
    
    # Sample output
    print("\nSample instruction sequences:")
    for i, seq in enumerate(sequences[:3]):
        print(f"\n--- Sequence {i+1} ---")
        print(seq[:200] + "..." if len(seq) > 200 else seq)
    
    # Statistics
    total_words = sum(len(seq.split()) for seq in sequences)
    print(f"\nStatistics:")
    print(f"  Total sequences: {len(sequences)}")
    print(f"  Total words: {total_words:,}")
    print(f"  Average sequence length: {total_words // len(sequences)} words")
    print(f"\nTraining data saved to datasets/instruction_training.txt")

if __name__ == "__main__":
    main()