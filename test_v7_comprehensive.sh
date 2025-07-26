#!/bin/bash

echo "=== GAIA V7 Comprehensive Test ==="
echo ""

echo "1. Multi-part calculation and explanation:"
echo "What is 25 times 4? Also, what is 100 divided by 5? Can you explain multiplication?" | ./gaia_chat_v7 2>/dev/null | grep -A2 "GAIA V7:"
echo ""

echo "2. Simple queries:"
echo "What is 7 plus 3?" | ./gaia_chat_v7 2>/dev/null | grep "GAIA V7:"
echo ""

echo "3. Interactive workflow test (with debug):"
echo -e "toggle-debug\nWhat is 42 minus 17? Explain subtraction.\nquit" | ./gaia_chat_v7 2>/dev/null | grep -A20 "GAIA V7:" | head -25
echo ""

echo "4. Complex reasoning test:"
echo "Calculate the factorial of 5 and explain what a factorial is" | ./gaia_chat_v7 --debug-workflows 2>/dev/null | grep -A15 "Decomposed"
echo ""

echo "5. Testing edge cases:"
echo "What is 10 divided by 0?" | ./gaia_chat_v7 2>/dev/null | grep "GAIA V7:"
echo ""

echo "=== V7 Feature Summary ==="
echo "- Dynamic workflows decompose complex queries"
echo "- Multi-step reasoning with context management"
echo "- Iterative refinement with backtracking"
echo "- Integration with V6 analysis functions"
echo "- Synthesis of results from multiple steps"
echo ""