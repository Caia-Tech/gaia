#!/bin/bash

echo "Testing GAIA V5 with and without superposition mode"
echo "=================================================="

# Test questions
questions=(
    "What is two plus two?"
    "What is pi?"
    "What comes after 5 in fibonacci?"
    "What is a mammal?"
    "If Felix is a cat, what is Felix?"
    "what is the derivative of x squared?"
    "Hello, how are you today?"
    "Can you explain quantum mechanics?"
)

# Test without superposition
echo -e "\n1. Testing WITHOUT superposition (baseline):\n"
for q in "${questions[@]}"; do
    echo "Q: $q"
    echo "$q" | ./gaia_chat_v5 2>/dev/null | grep "gaia:" | head -1
    echo
done

# Test with superposition
echo -e "\n2. Testing WITH superposition:\n"
for q in "${questions[@]}"; do
    echo "Q: $q"
    echo "$q" | ./gaia_chat_v5 --superposition 2>/dev/null | grep "gaia:" | head -1
    echo
done

# Test with debug to see when superposition activates
echo -e "\n3. Testing WITH debug superposition (to see when it activates):\n"
for q in "${questions[@]}"; do
    echo "Q: $q"
    echo "$q" | ./gaia_chat_v5 --superposition --debug-superposition 2>&1 | grep -E "(gaia:|SUPERPOSITION:)" | head -2
    echo
done