#!/bin/bash

echo "=== GAIA V6 Manual Test Suite ==="
echo ""

# Test 1: Math calculation
echo "Test 1: Math calculation"
echo "Input: Calculate 5 plus 3"
echo "Expected: 8"
echo "Actual:"
echo "Calculate 5 plus 3" | ./gaia_chat_v6 --no-analysis 2>/dev/null | grep "GAIA V6:" | head -1
echo ""

# Test 2: Greeting
echo "Test 2: Greeting"  
echo "Input: Hello"
echo "Expected: greeting response"
echo "Actual:"
echo "Hello" | ./gaia_chat_v6 2>/dev/null | grep "GAIA V6:" | head -1
echo ""

# Test 3: Farewell
echo "Test 3: Farewell"
echo "Input: Goodbye"  
echo "Expected: farewell response"
echo "Actual:"
echo "Goodbye" | ./gaia_chat_v6 2>/dev/null | grep "GAIA V6:" | head -1
echo ""

# Test 4: Math with analysis
echo "Test 4: Math with analysis enabled"
echo "Input: What is 2 times 3?"
echo "Expected: 6"
echo "Actual:"
echo "What is 2 times 3?" | ./gaia_chat_v6 2>/dev/null | grep "GAIA V6:" | head -1
echo ""

# Test 5: Superposition mode
echo "Test 5: Superposition mode" 
echo "Input: The weather is"
echo "Expected: continuation with debug info"
echo "Actual:"
echo "The weather is" | ./gaia_chat_v6 --superposition --debug-superposition 2>/dev/null | head -5
echo ""

echo "=== Test Complete ==="