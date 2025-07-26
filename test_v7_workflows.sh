#!/bin/bash

echo "=== GAIA V7 Dynamic Workflows Test ==="
echo ""

echo "Test 1: Simple calculation"
echo "Calculate 5 plus 3" | ./gaia_chat_v7 2>/dev/null | grep "GAIA V7:"
echo ""

echo "Test 2: Multi-part question"
echo "What is 10 times 5? Also, what is 100 divided by 20?" | ./gaia_chat_v7 2>/dev/null | grep "GAIA V7:"
echo ""

echo "Test 3: With debug output"
echo "What is 2 plus 2?" | ./gaia_chat_v7 --debug-workflows 2>/dev/null | head -20
echo ""

echo "Test 4: Complex reasoning"
echo "List three colors and explain why the sky is blue" | ./gaia_chat_v7 --debug-workflows 2>/dev/null | head -20
echo ""

echo "Test 5: Workflow test command"
echo -e "workflow-test\nquit" | ./gaia_chat_v7 2>/dev/null | grep -A5 "Testing workflow"
echo ""

echo "=== Tests Complete ==="