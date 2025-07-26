#!/bin/bash

echo "Testing GAIA V5 with enhanced training data"
echo "==========================================="

# First, train GAIA with the new data
echo -e "\n1. Training GAIA with enhanced dataset..."
echo "quit" | ./gaia_chat_v5 > /dev/null 2>&1

# Test the same prompts as before
echo -e "\n2. Testing response quality (baseline mode):\n"

test_prompts=(
    "Hello, how are you?"
    "What's your name?"
    "Can you help me?"
    "What is a mammal?"
    "The cat sat on the"
    "Calculate 25 plus 37"
    "List three colors"
    "Define democracy"
    "Is 42 even?"
    "Reverse the word 'hello'"
)

for prompt in "${test_prompts[@]}"; do
    echo "Q: $prompt"
    response=$(echo -e "$prompt\nquit" | ./gaia_chat_v5 2>&1 | grep "gaia:" | head -1 | sed 's/.*gaia: //')
    echo "A: $response"
    echo
done

# Test with superposition for variety
echo -e "\n3. Testing ambiguous prompts with superposition:\n"

ambiguous=("The bank" "The spring" "The wave")

for prompt in "${ambiguous[@]}"; do
    echo "Prompt: '$prompt' (3 runs with superposition)"
    for i in {1..3}; do
        response=$(echo -e "$prompt\nquit" | ./gaia_chat_v5 --superposition 2>&1 | grep "gaia:" | head -1 | sed 's/.*gaia: //')
        echo "  Run $i: $response"
        sleep 0.01
    done
    echo
done

# Run quick test suite to measure improvement
echo -e "\n4. Running quick test suite for comparison..."
python3 test_suite_quick.py | tail -20