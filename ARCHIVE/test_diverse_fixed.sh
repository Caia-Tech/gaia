#!/bin/bash

echo "GAIA V5 Comprehensive Test Suite"
echo "================================"

# Create test questions
questions=(
    # Mathematics
    "What is two plus two?"
    "What is five times three?"
    "What is pi?"
    "What comes after 5 in fibonacci?"
    
    # Science
    "What is gravity?"
    "How fast does light travel?"
    "What is photosynthesis?"
    "What is quantum mechanics?"
    
    # Philosophy
    "What is consciousness?"
    "What is the meaning of life?"
    "What is reality?"
    
    # Technology
    "What is a computer?"
    "What is artificial intelligence?"
    "What is blockchain?"
    
    # Nature
    "What do trees need to grow?"
    "How do birds fly?"
    "Why is the sky blue?"
    
    # Abstract/Ambiguous
    "The bank"
    "The light"
    "The wave"
    "The spring"
    "Time"
    "Love"
    "Hope"
    "Dreams"
    
    # Conversational
    "Hello, how are you?"
    "What's your name?"
    "Can you help me?"
    "Tell me a story"
    
    # Knowledge queries
    "What is a mammal?"
    "What are neurons?"
    "Define entropy"
    
    # Derivatives
    "What is the derivative of x squared?"
    
    # Incomplete sentences
    "The cat sat on the"
    "Once upon a time"
    "To be or not to"
    
    # Complex questions
    "If Felix is a cat, what is Felix?"
    "When water freezes what happens?"
    
    # Contextual
    "After the rain comes"
    "Before the storm"
    "In spring the flowers"
)

# Function to test a single question
test_question() {
    local question=$1
    local flags=$2
    echo -e "$question\nquit" | ./gaia_chat_v5 $flags 2>&1 | grep "gaia:" | head -1 | sed 's/.*gaia: //'
}

# Test subset with both modes
echo -e "\n=== Comparative Test (Baseline vs Superposition) ===\n"

# Test first 20 questions
for i in {0..19}; do
    q="${questions[$i]}"
    printf "%-45s" "${q:0:45}"
    
    # Baseline
    baseline=$(test_question "$q" "")
    printf " B: %-25s" "${baseline:0:25}"
    
    # Superposition
    super=$(test_question "$q" "--superposition")
    printf " S: %-25s\n" "${super:0:25}"
done

# Focus on ambiguous prompts
echo -e "\n=== Ambiguous Prompts - Response Variety Test ===\n"

ambiguous=("The bank" "The spring" "The wave" "Time" "Light" "Love")

for prompt in "${ambiguous[@]}"; do
    echo -e "\n'$prompt' responses:"
    
    # Collect 5 baseline responses
    echo "  Baseline:"
    for i in {1..5}; do
        response=$(test_question "$prompt" "")
        if [[ -n "$response" ]]; then
            echo "    $i: ${response:0:50}"
        else
            echo "    $i: [empty]"
        fi
    done
    
    # Collect 5 superposition responses
    echo "  Superposition:"
    for i in {1..5}; do
        response=$(test_question "$prompt" "--superposition")
        if [[ -n "$response" ]]; then
            echo "    $i: ${response:0:50}"
        else
            echo "    $i: [empty]"
        fi
    done
done

# Test with debug to see activation patterns
echo -e "\n=== Superposition Activation Analysis ===\n"

for prompt in "${ambiguous[@]}"; do
    echo "Testing '$prompt':"
    result=$(echo -e "$prompt\nquit" | ./gaia_chat_v5 --superposition --debug-superposition 2>&1)
    
    # Check if superposition activated
    if echo "$result" | grep -q "SUPERPOSITION:"; then
        echo "  ✓ Superposition ACTIVATED"
        echo "$result" | grep "SUPERPOSITION:" | head -1 | sed 's/^/    /'
    else
        echo "  ✗ Superposition did not activate"
    fi
    
    # Show the response
    response=$(echo "$result" | grep "gaia:" | head -1 | sed 's/.*gaia: //')
    echo "    Response: ${response:0:50}"
done

# Summary statistics
echo -e "\n=== Summary Statistics ===\n"

total_baseline=0
total_super=0
empty_baseline=0
empty_super=0
function_baseline=0
function_super=0

for q in "${questions[@]:0:30}"; do
    b=$(test_question "$q" "")
    s=$(test_question "$q" "--superposition")
    
    [[ -n "$b" ]] && ((total_baseline++)) || ((empty_baseline++))
    [[ -n "$s" ]] && ((total_super++)) || ((empty_super++))
    
    # Check for function responses (numbers, calculations)
    [[ "$b" =~ ^[0-9]+$ ]] || [[ "$b" == "3.14159" ]] || [[ "$b" =~ ^[0-9]+x$ ]] && ((function_baseline++))
    [[ "$s" =~ ^[0-9]+$ ]] || [[ "$s" == "3.14159" ]] || [[ "$s" =~ ^[0-9]+x$ ]] && ((function_super++))
done

echo "Tested ${#questions[@]:0:30} questions"
echo ""
echo "Baseline mode:"
echo "  Responses: $total_baseline"
echo "  Empty: $empty_baseline"
echo "  Functions: $function_baseline"
echo ""
echo "Superposition mode:"
echo "  Responses: $total_super"
echo "  Empty: $empty_super"
echo "  Functions: $function_super"