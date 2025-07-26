#!/bin/bash

echo "Testing GAIA V5 Superposition Variety"
echo "===================================="

# Test prompts known to trigger superposition
prompts=("The bank" "The spring" "The wave")

for prompt in "${prompts[@]}"; do
    echo -e "\n=== Testing '$prompt' - 20 runs ==="
    
    # Collect responses
    responses=""
    for i in {1..20}; do
        response=$(echo -e "$prompt\nquit" | ./gaia_chat_v5 --superposition 2>&1 | grep "gaia:" | head -1 | sed 's/.*gaia: //')
        if [[ -n "$response" ]]; then
            responses+="${response}\n"
        fi
        # Small delay to ensure different process IDs
        sleep 0.01
    done
    
    # Count unique responses
    echo "All responses:"
    echo -e "$responses" | sort | uniq -c | sort -nr
    
    unique_count=$(echo -e "$responses" | sort -u | wc -l)
    echo "Unique responses: $unique_count"
done

# Test with debug to see the probabilities
echo -e "\n\n=== Debug: Probability Distributions ==="
for prompt in "${prompts[@]}"; do
    echo -e "\nPrompt: '$prompt'"
    for i in {1..3}; do
        echo "Run $i:"
        echo -e "$prompt\nquit" | ./gaia_chat_v5 --superposition --debug-superposition 2>&1 | grep "SUPERPOSITION:" | sed 's/^/  /'
        sleep 0.1
    done
done

# Test baseline consistency
echo -e "\n\n=== Baseline Consistency Check ==="
echo "Testing 'The bank' without superposition (should be deterministic):"
for i in {1..10}; do
    response=$(echo -e "The bank\nquit" | ./gaia_chat_v5 2>&1 | grep "gaia:" | head -1 | sed 's/.*gaia: //')
    echo "$i: ${response:0:50}"
done

# Statistical test
echo -e "\n\n=== Statistical Analysis ==="
echo "Running 100 iterations on 'The bank' with superposition..."

declare -A response_counts
for i in {1..100}; do
    response=$(echo -e "The bank\nquit" | ./gaia_chat_v5 --superposition 2>&1 | grep "gaia:" | head -1 | sed 's/.*gaia: //' | cut -d' ' -f1)
    if [[ -n "$response" ]]; then
        ((response_counts[$response]++))
    fi
done

echo "Response distribution (first word):"
for word in "${!response_counts[@]}"; do
    count=${response_counts[$word]}
    percentage=$((count * 100 / 100))
    echo "  '$word': $count times ($percentage%)"
done | sort -k3 -nr