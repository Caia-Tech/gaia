#!/bin/bash

echo "Running 100 iterations to measure superposition impact"
echo "===================================================="

# Function to run test and count successes
run_test() {
    local mode=$1
    local successes=0
    local superposition_activations=0
    
    for i in {1..100}; do
        if [ "$mode" == "superposition" ]; then
            result=$(echo "The bank" | ./gaia_chat_v5 --superposition --debug-superposition 2>&1)
            if echo "$result" | grep -q "SUPERPOSITION:"; then
                ((superposition_activations++))
            fi
        else
            result=$(echo "The bank" | ./gaia_chat_v5 2>&1)
        fi
        
        # Check if response contains valid continuation
        if echo "$result" | grep -q "gaia: .* money\|gaia: .* river\|gaia: .* loans"; then
            ((successes++))
        fi
    done
    
    echo "Mode: $mode"
    echo "Success rate: $successes/100 ($((successes))%)"
    if [ "$mode" == "superposition" ]; then
        echo "Superposition activated: $superposition_activations/100 times"
    fi
}

echo -e "\nBaseline (no superposition):"
run_test "baseline"

echo -e "\nWith superposition:"
run_test "superposition"

# Test variety 
echo -e "\nTesting response variety (10 runs each):"
echo -e "\nBaseline responses:"
for i in {1..10}; do
    echo "The bank" | ./gaia_chat_v5 2>&1 | grep "gaia:" | sed 's/.*gaia: //' | head -c 30
    echo
done | sort | uniq -c

echo -e "\nSuperposition responses:"
for i in {1..10}; do
    echo "The bank" | ./gaia_chat_v5 --superposition 2>&1 | grep "gaia:" | sed 's/.*gaia: //' | head -c 30
    echo  
done | sort | uniq -c