#!/bin/bash

echo "GAIA V5 Comprehensive Test Suite"
echo "================================"

# Create diverse test questions covering different domains
cat > test_questions.txt << 'EOF'
# Mathematics
What is two plus two?
What is five times three?
What is the square root of sixteen?
Calculate pi to five decimal places

# Science
What is gravity?
How fast does light travel?
What is photosynthesis?
What is quantum mechanics?

# Philosophy
What is consciousness?
What is the meaning of life?
Is there free will?
What is reality?

# Technology
What is a computer?
How does the internet work?
What is artificial intelligence?
What is blockchain?

# Nature
What do trees need to grow?
How do birds fly?
Why is the sky blue?
What causes rain?

# Abstract/Ambiguous
The bank
The light
The wave
The spring
Time
Love
Hope
Dreams

# Conversational
Hello, how are you?
What's your name?
Can you help me?
Tell me a story
What do you think about life?
How was your day?

# Knowledge queries
What is a mammal?
What are neurons?
Define entropy
What is democracy?

# Sequential/Fibonacci
What comes after 5 in fibonacci?
What number follows 8 in the sequence?
Continue the pattern: 1, 1, 2, 3, 5

# Derivatives/Calculus
What is the derivative of x squared?
Derivative of sin(x)?
What is an integral?

# Incomplete sentences
The cat sat on the
Once upon a time there was
To be or not to
In the beginning

# Complex questions
If Felix is a cat, what is Felix?
If all roses are flowers and this is a rose, what is it?
When water freezes, what does it become?

# Contextual
After the rain comes
Before the storm there is
During winter we see
In spring the flowers

# Technical
What is HTTP?
Explain TCP/IP
What is a database?
How do algorithms work?

# Creative
Describe a sunset
What does happiness feel like?
Paint a picture with words
Imagine a perfect world
EOF

# Function to test a batch of questions
test_batch() {
    local mode=$1
    local mode_flags=$2
    echo -e "\n=== Testing $mode ===\n"
    
    local count=0
    local responses=0
    local functions=0
    local multi_word=0
    local single_word=0
    local empty=0
    
    while IFS= read -r question; do
        # Skip empty lines and comments
        [[ -z "$question" || "$question" =~ ^# ]] && continue
        
        ((count++))
        
        # Run the test
        result=$(echo "$question" | timeout 2 ./gaia_chat_v5 $mode_flags 2>&1 | grep "gaia:" | head -1)
        
        if [[ -n "$result" ]]; then
            ((responses++))
            response=$(echo "$result" | sed 's/.*gaia: //')
            
            # Count response types
            if [[ "$response" =~ ^[0-9]+$ ]] || [[ "$response" == "3.14159" ]] || [[ "$response" =~ ^[0-9]+x$ ]]; then
                ((functions++))
            elif [[ $(echo "$response" | wc -w) -gt 1 ]]; then
                ((multi_word++))
            elif [[ $(echo "$response" | wc -w) -eq 1 ]]; then
                ((single_word++))
            fi
            
            # Show first 10 for inspection
            if [[ $count -le 10 ]]; then
                printf "%-40s → %s\n" "${question:0:40}" "${response:0:60}"
            fi
        else
            ((empty++))
        fi
    done < test_questions.txt
    
    echo -e "\n--- Statistics for $mode ---"
    echo "Total questions: $count"
    echo "Responses: $responses ($((responses * 100 / count))%)"
    echo "Empty responses: $empty"
    echo "Function calls: $functions"
    echo "Multi-word: $multi_word"
    echo "Single-word: $single_word"
}

# Test without superposition
test_batch "BASELINE (no superposition)" ""

# Test with superposition
test_batch "SUPERPOSITION mode" "--superposition"

# Test variety on ambiguous prompts
echo -e "\n=== Variety Test (10 runs each on ambiguous prompts) ===\n"

for prompt in "The bank" "Time" "Love" "The spring" "Light"; do
    echo -e "\nPrompt: '$prompt'"
    
    echo "Baseline (deterministic):"
    for i in {1..5}; do
        echo "$prompt" | ./gaia_chat_v5 2>&1 | grep "gaia:" | sed 's/.*gaia: //' | head -c 40
        echo
    done | sort | uniq -c
    
    echo -e "\nSuperposition (probabilistic):"
    for i in {1..5}; do
        echo "$prompt" | ./gaia_chat_v5 --superposition 2>&1 | grep "gaia:" | sed 's/.*gaia: //' | head -c 40
        echo
    done | sort | uniq -c
done

# Debug mode test on specific ambiguous cases
echo -e "\n=== Debug Mode - When Superposition Activates ===\n"

ambiguous_prompts=("The bank" "Light" "Dreams" "Hope" "The wave")
for prompt in "${ambiguous_prompts[@]}"; do
    echo -e "\nPrompt: '$prompt'"
    echo "$prompt" | ./gaia_chat_v5 --superposition --debug-superposition 2>&1 | grep -E "(SUPERPOSITION:|gaia:)" | head -2
done