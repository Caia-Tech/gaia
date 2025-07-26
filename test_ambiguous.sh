#!/bin/bash

echo "Testing GAIA V5 on ambiguous contexts"
echo "====================================="

# Create test file with ambiguous patterns
cat > datasets/ambiguous_test.txt << 'EOF'
The bank is near the river. The bank holds money. The bank provides loans.
Light travels fast. Light weighs nothing. Light illuminates darkness.
The spring brings flowers. The spring is coiled. The spring flows water.
Time flies quickly. Time heals wounds. Time waits for nobody.
The wave crashed loudly. The wave carried surfers. The wave of enthusiasm.
EOF

# Retrain with ambiguous data
echo "Training on ambiguous patterns..."
./gaia_chat_v5 << 'EOF' > /dev/null 2>&1
quit
EOF

# Test ambiguous prompts
echo -e "\nTesting ambiguous contexts:\n"

prompts=(
    "The bank"
    "Light"  
    "The spring"
    "Time"
    "The wave"
)

echo "WITHOUT superposition:"
for p in "${prompts[@]}"; do
    result=$(echo "$p" | ./gaia_chat_v5 2>&1 | grep "gaia:" | head -1 | sed 's/.*gaia: //')
    echo "$p → $result"
done

echo -e "\nWITH superposition:"
for p in "${prompts[@]}"; do
    result=$(echo "$p" | ./gaia_chat_v5 --superposition 2>&1 | grep "gaia:" | head -1 | sed 's/.*gaia: //')
    echo "$p → $result"
done

echo -e "\nWITH debug (showing when superposition activates):"
for p in "${prompts[@]}"; do
    echo -e "\nPrompt: $p"
    echo "$p" | ./gaia_chat_v5 --superposition --debug-superposition 2>&1 | grep -E "(SUPERPOSITION:|gaia:)" | head -3
done