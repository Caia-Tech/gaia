#!/bin/bash

echo "Testing improved fact retrieval..."
echo ""

# Clean start
rm -f gaia_memory.dat

# Test 1: Basic fact storage and retrieval
echo "Test 1: Basic fact storage and retrieval"
echo "Python is a programming language" | ./gaia_simple | grep -A1 "learned"
echo "What is Python?" | ./gaia_simple | grep -A1 "Python"
echo ""

# Test 2: User facts
echo "Test 2: User fact storage and retrieval"
echo -e "My favorite color is blue\nquit" | ./gaia_simple > /dev/null
echo "What's my favorite color?" | ./gaia_simple | grep -A1 "favorite"
echo ""

# Test 3: Pattern matching
echo "Test 3: Pattern matching"
echo -e "How are you?\nquit" | ./gaia_simple > /dev/null
echo "How are you doing?" | ./gaia_simple | grep -A1 "GAIA:"
echo ""

echo "Test complete!"