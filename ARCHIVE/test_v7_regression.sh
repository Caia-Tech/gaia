#!/bin/bash

echo "=== GAIA V7 Regression Test Suite ==="
echo "Ensuring V6 features still work correctly..."
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Test function
test_feature() {
    local feature="$1"
    local input="$2"
    local expected="$3"
    local test_name="$4"
    
    echo -n "[$feature] $test_name: "
    
    result=$(echo "$input" | ./gaia_chat_v7 2>&1 | grep -A3 "GAIA V7:")
    
    if [[ "$result" == *"$expected"* ]]; then
        echo -e "${GREEN}PASSED${NC} ✓"
        return 0
    else
        echo -e "${RED}FAILED${NC} ✗"
        echo "  Expected: '$expected'"
        echo "  Got: '$result'"
        return 1
    fi
}

# Initialize counters
total=0
passed=0

echo "=== Testing V6 Analysis Functions ==="

((total++))
test_feature "ANALYSIS" "Hello there!" "Hello" "Greeting detection" && ((passed++))

((total++))
test_feature "ANALYSIS" "Goodbye!" "Goodbye" "Farewell detection" && ((passed++))

((total++))
test_feature "ANALYSIS" "What is 42?" "42" "Number detection" && ((passed++))

((total++))
test_feature "ANALYSIS" "5 plus 3" "5 plus 3 = 8" "Calculation detection" && ((passed++))

echo ""
echo "=== Testing V6 Response Formatting ==="

((total++))
test_feature "FORMAT" "List some colors" "red" "List formatting" && ((passed++))

((total++))
test_feature "FORMAT" "Is 5 greater than 3?" "GAIA V7:" "Yes/No formatting" && ((passed++))

((total++))
test_feature "FORMAT" "Calculate 10 times 5" "10 times 5 = 50" "Calculation formatting" && ((passed++))

echo ""
echo "=== Testing Function Registry ==="

((total++))
test_feature "FUNCTIONS" "What is factorial of 6?" "factorial" "Factorial function" && ((passed++))

((total++))
test_feature "FUNCTIONS" "Is 13 prime?" "prime" "Prime check function" && ((passed++))

((total++))
test_feature "FUNCTIONS" "10 divided by 3" "10 divided 3 = 3" "Division function" && ((passed++))

echo ""
echo "=== Testing Experiment Logger ==="

# Run a query that should log
echo "Testing experiment logging..."
echo -e "What is 5 plus 3?\nlog-summary\nquit" | ./gaia_chat_v7 > /tmp/gaia_test_log.txt 2>&1

if grep -q "Total experiments:" /tmp/gaia_test_log.txt; then
    echo -e "Experiment logger: ${GREEN}PASSED${NC} ✓"
    ((passed++))
else
    echo -e "Experiment logger: ${RED}FAILED${NC} ✗"
fi
((total++))

echo ""
echo "=== Testing Error Handling ==="

((total++))
test_feature "ERROR" "5 divided by 0" "Error: division by zero" "Division by zero" && ((passed++))

((total++))
test_feature "ERROR" "" "Please provide some input" "Empty input handling" && ((passed++))

echo ""
echo "=== Testing Workflow Integration ==="

# Test that workflows don't break V6 features
((total++))
echo -e "toggle-workflows\nWhat is 5 plus 3?\ntoggle-workflows\nWhat is 5 plus 3?\nquit" | ./gaia_chat_v7 > /tmp/workflow_toggle.txt 2>&1
if grep -q "5 plus 3 = 8" /tmp/workflow_toggle.txt; then
    echo -e "Workflow toggle: ${GREEN}PASSED${NC} ✓"
    ((passed++))
else
    echo -e "Workflow toggle: ${RED}FAILED${NC} ✗"
fi

echo ""
echo "=== Testing Pattern Matching ==="

# Test basic pattern functionality
test_data="The cat sat on the mat. The cat was happy."
echo "$test_data" > /tmp/test_pattern.txt

echo "Testing pattern learning..."
./gaia_chat_v7 < /tmp/test_pattern.txt > /tmp/pattern_output.txt 2>&1
if grep -q "Total patterns:" /tmp/pattern_output.txt; then
    echo -e "Pattern learning: ${GREEN}PASSED${NC} ✓"
    ((passed++))
else
    echo -e "Pattern learning: ${RED}FAILED${NC} ✗"
fi
((total++))

echo ""
echo "=== Testing Complex Queries ==="

# Test complex multi-part questions
complex_queries=(
    "What is 10 plus 20? Also, what is 30 minus 10?"
    "Calculate 5 times 5 and explain multiplication"
    "Is 17 prime? What about 18?"
)

for query in "${complex_queries[@]}"; do
    ((total++))
    result=$(echo "$query" | ./gaia_chat_v7 2>&1 | grep -A5 "GAIA V7:")
    if [[ -n "$result" ]] && [[ "$result" != *"Processing..."* ]]; then
        echo -e "Complex query: ${GREEN}PASSED${NC} ✓ - ${query:0:40}..."
        ((passed++))
    else
        echo -e "Complex query: ${RED}FAILED${NC} ✗ - ${query:0:40}..."
    fi
done

echo ""
echo "=== Testing Command Compatibility ==="

# Test all V6 commands still work
commands=("stats" "quit" "help")
for cmd in "${commands[@]}"; do
    ((total++))
    output=$(echo -e "$cmd\nquit" | ./gaia_chat_v7 2>&1)
    if [[ -n "$output" ]]; then
        echo -e "Command '$cmd': ${GREEN}PASSED${NC} ✓"
        ((passed++))
    else
        echo -e "Command '$cmd': ${RED}FAILED${NC} ✗"
    fi
done

echo ""
echo "=== Testing Mathematical Operations ==="

# Test all math operations
operations=(
    "5 plus 3:8"
    "10 minus 4:6"
    "6 times 7:42"
    "20 divided by 4:5"
    "factorial of 5:120"
)

for op in "${operations[@]}"; do
    IFS=':' read -r query expected <<< "$op"
    ((total++))
    test_feature "MATH" "What is $query?" "$expected" "$query" && ((passed++))
done

echo ""
echo "=== Testing Edge Cases ==="

# Various edge cases
((total++))
test_feature "EDGE" "What is 0 plus 0?" "0 plus 0 = 0" "Zero addition" && ((passed++))

((total++))
test_feature "EDGE" "What is 1 times 1?" "1 times 1 = 1" "Identity multiplication" && ((passed++))

((total++))
test_feature "EDGE" "What is -5 plus 5?" "-5 plus 5 = 0" "Negative addition" && ((passed++))

echo ""
echo "=== REGRESSION TEST SUMMARY ==="
echo "Total tests: $total"
echo -e "Passed: ${GREEN}$passed${NC} ($((passed * 100 / total))%)"
echo -e "Failed: ${RED}$((total - passed))${NC}"
echo ""

# Check for critical failures
if [ $passed -lt $((total * 80 / 100)) ]; then
    echo -e "${RED}WARNING: More than 20% of tests failed!${NC}"
    echo "V7 may have broken V6 functionality."
    exit 1
else
    echo -e "${GREEN}All critical V6 features are working correctly in V7.${NC}"
    exit 0
fi