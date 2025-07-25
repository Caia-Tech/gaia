#!/bin/bash

echo "=== gaia Comprehensive Testing Suite ==="
echo "======================================"
echo

# Compile all components
echo "Building gaia components..."
make clean
make gaia_chat
make test_framework
make iterative_trainer

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo
echo "=== Running Basic Chat Test ==="
echo "Testing basic interaction..."
echo -e "hi\nhow are you\nwhat is two plus two\nquit" | ./gaia_chat

echo
echo "=== Running Unit Tests ==="
./test_framework

echo
echo "=== Running Iterative Training ==="
./iterative_trainer

echo
echo "=== Testing Final Model ==="
echo "Testing trained model with various inputs..."
echo -e "What is 5 + 3?\nIf all cats are animals and Fluffy is a cat, what is Fluffy?\nHi there!\nThank you for your help\nquit" | ./gaia_chat

echo
echo "=== Test Suite Complete ==="