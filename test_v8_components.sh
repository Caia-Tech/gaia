#!/bin/bash

echo "=== Testing V8 Components ==="

# Test 1: Prime (direct function call)
echo -e "\n--- Test 1: Prime check ---"
echo -e "Is 17 prime?\nquit" | ./gaia_chat_v8 2>&1 | grep "GAIA V8:"

# Test 2: Simple calculation without enhancements
echo -e "\n--- Test 2: Calculation without V8 enhancements ---"
echo -e "What is 2 plus 2?\nquit" | ./gaia_chat_v8 --no-attention --no-refinement 2>&1 | grep "GAIA V8:"

# Test 3: Calculation with attention only
echo -e "\n--- Test 3: Calculation with attention only ---"
echo -e "What is 2 plus 2?\nquit" | ./gaia_chat_v8 --no-refinement 2>&1 | grep "GAIA V8:"

# Test 4: Full V8
echo -e "\n--- Test 4: Full V8 ---"
echo -e "What is 2 plus 2?\nquit" | ./gaia_chat_v8 2>&1 | grep "GAIA V8:"

# Test 5: Check what happens with debug
echo -e "\n--- Test 5: With all debug ---"
echo -e "What is 2 plus 2?\nquit" | ./gaia_chat_v8 --debug-workflows --no-attention --no-refinement 2>&1 | grep -E "(GAIA V8:|Base response)"