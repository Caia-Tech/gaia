#!/bin/bash

# Test the interactive system with pre-training

echo "Testing gaia interactive mode..."

# First, let's see what the train_gaia program produces
./train_gaia << 'EOF'
logic gates are fundamental
what are logic gates
how do patterns work
gaia learns from patterns
quit
EOF