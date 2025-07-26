#!/bin/bash

echo "Testing GAIA V8..."

# Create minimal training data if needed
if [ ! -f conversational_flow.txt ]; then
    echo "Creating minimal training data..."
    echo "hello world" > conversational_flow.txt
    echo "how are you doing" >> conversational_flow.txt
    echo "what is math" >> conversational_flow.txt
fi

# Test basic arithmetic
echo "=== Test 1: Basic arithmetic ==="
echo -e "What is 2 plus 2?\nquit" | ./gaia_chat_v8 2>&1 | grep -A5 "GAIA V8:"

echo -e "\n=== Test 2: Factorial ==="
echo -e "What is factorial of 5?\nquit" | ./gaia_chat_v8 2>&1 | grep -A5 "GAIA V8:"

echo -e "\n=== Test 3: Prime check ==="
echo -e "Is 17 prime?\nquit" | ./gaia_chat_v8 2>&1 | grep -A5 "GAIA V8:"

echo -e "\n=== Test 4: Explanation ==="
echo -e "Explain what addition is\nquit" | ./gaia_chat_v8 2>&1 | grep -A5 "GAIA V8:"

echo -e "\n=== Test 5: Debug refinement ==="
echo -e "explain addition\nquit" | ./gaia_chat_v8 --debug-refinement 2>&1 | grep -E "(V8|Refinement)" | head -20