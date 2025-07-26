#!/bin/bash

echo "Testing Superposition Variety with Different Seeds"
echo "================================================="

# Test function with explicit seed control
test_with_seed() {
    local prompt=$1
    local seed=$2
    # Use a subshell with different PID to force different random state
    (
        export RANDOM=$seed
        echo -e "$prompt\nquit" | ./gaia_chat_v5 --superposition 2>&1 | grep "gaia:" | head -1 | sed 's/.*gaia: //'
    )
}

# Test prompts that should trigger superposition
prompts=("The bank" "The spring" "The wave")

for prompt in "${prompts[@]}"; do
    echo -e "\nTesting '$prompt' with 20 different runs:"
    
    # Collect responses
    declare -A responses
    for i in {1..20}; do
        # Use timestamp + iteration as seed for variety
        seed=$(($(date +%s%N) + i))
        response=$(test_with_seed "$prompt" $seed)
        
        if [[ -n "$response" ]]; then
            # Track unique responses
            responses["$response"]=1
        fi
        
        # Small delay to ensure different timestamps
        sleep 0.01
    done
    
    # Show unique responses
    echo "Unique responses found: ${#responses[@]}"
    count=1
    for resp in "${!responses[@]}"; do
        echo "  $count: ${resp:0:60}"
        ((count++))
    done
done

# Test with debug to confirm probabilities
echo -e "\n\nDebug: Checking probability distributions"
echo "========================================="

for prompt in "${prompts[@]}"; do
    echo -e "\nPrompt: '$prompt'"
    for i in {1..3}; do
        echo "Run $i:"
        result=$(echo -e "$prompt\nquit" | ./gaia_chat_v5 --superposition --debug-superposition 2>&1)
        echo "$result" | grep "SUPERPOSITION:" | sed 's/^/  /'
        sleep 0.1
    done
done

# Create a C program to test the random number generation
echo -e "\n\nTesting C random number generation"
echo "=================================="

cat > test_random.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    printf("Testing random number generation:\n");
    
    for (int seed_offset = 0; seed_offset < 5; seed_offset++) {
        srand(time(NULL) + seed_offset);
        printf("Seed offset %d: ", seed_offset);
        for (int i = 0; i < 10; i++) {
            printf("%.3f ", (float)rand() / RAND_MAX);
        }
        printf("\n");
    }
    
    return 0;
}
EOF

gcc -o test_random test_random.c
./test_random

# Test multiple instances running simultaneously
echo -e "\n\nParallel execution test"
echo "======================="

echo "Running 10 parallel instances on 'The bank':"
for i in {1..10}; do
    (
        response=$(echo -e "The bank\nquit" | ./gaia_chat_v5 --superposition 2>&1 | grep "gaia:" | head -1 | sed 's/.*gaia: //')
        echo "Instance $i: $response"
    ) &
done
wait

# Clean up
rm -f test_random test_random.c