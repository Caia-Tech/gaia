#!/bin/bash

echo "=== GAIA V6 Complete Feature Test ==="
echo ""

echo "Test 1: Math with formatted response"
echo "Calculate 7 plus 5" | ./gaia_chat_v6 2>/dev/null | grep "GAIA V6:" | head -1
echo ""

echo "Test 2: List request with formatting"  
echo "List three colors" | ./gaia_chat_v6 2>/dev/null | grep -A5 "GAIA V6:"
echo ""

echo "Test 3: Superposition with logging"
echo "The weather is" | ./gaia_chat_v6 --superposition --debug-superposition 2>/dev/null | head -10
echo ""

echo "Test 4: Experiment logging commands"
echo -e "Hello\nlog-summary\nquit" | ./gaia_chat_v6 2>/dev/null | tail -10
echo ""

echo "=== Test Complete ==="