#!/bin/bash

echo "=== V8 Trace Test ==="

# Test with debug workflows to see step execution
echo -e "\n--- Test 1: Simple calculation ---"
echo -e "What is 2 plus 2?\nquit" | ./gaia_chat_v8 --debug-workflows 2>&1 | grep -E "(You:|GAIA|V8|Step|response:|Output)" | head -50

echo -e "\n--- Test 2: Prime check (which worked before) ---"
echo -e "Is 17 prime?\nquit" | ./gaia_chat_v8 2>&1 | grep -E "(You:|GAIA|17)" | head -10

echo -e "\n--- Test 3: With all debugging ---"
echo -e "What is 5 times 3?\nquit" | ./gaia_chat_v8 --debug-workflows --debug-refinement 2>&1 | grep -E "(You:|GAIA|V8|Step|Base|Enhanced|Quality)" | head -50