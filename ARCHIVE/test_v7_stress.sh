#!/bin/bash

echo "=== GAIA V7 Stress Test Suite ==="
echo "Testing system robustness and edge cases..."
echo ""

# Function to run test and check output
run_test() {
    local test_name="$1"
    local input="$2"
    local expected="$3"
    
    echo -n "Testing: $test_name... "
    
    output=$(echo "$input" | timeout 5 ./gaia_chat_v7 2>&1 | grep -A2 "GAIA V7:" | head -5)
    
    if [[ "$output" == *"$expected"* ]]; then
        echo "PASSED ✓"
        return 0
    else
        echo "FAILED ✗"
        echo "  Expected: $expected"
        echo "  Got: $output"
        return 1
    fi
}

# Initialize counters
total=0
passed=0

# === STRESS TESTS ===

echo "=== 1. Input Validation Tests ==="

# Empty and whitespace
((total++))
if run_test "Empty input" "" "Please provide some input"; then ((passed++)); fi

((total++))
if run_test "Only spaces" "   " "Please provide some input"; then ((passed++)); fi

((total++))
if run_test "Only newlines" $'\n\n\n' "Please provide some input"; then ((passed++)); fi

echo ""
echo "=== 2. Boundary Value Tests ==="

# Very large numbers
((total++))
if run_test "Max int addition" "What is 2147483647 plus 1?" "2147483647 plus 1"; then ((passed++)); fi

((total++))
if run_test "Large multiplication" "What is 99999 times 99999?" "times"; then ((passed++)); fi

((total++))
if run_test "Negative boundary" "What is -2147483648 minus 1?" "minus"; then ((passed++)); fi

echo ""
echo "=== 3. Malformed Input Tests ==="

((total++))
if run_test "No spaces" "Whatis5plus3?" "GAIA V7:"; then ((passed++)); fi

((total++))
if run_test "Random punctuation" "What@#$is%%%5^^^plus&&&3???" "GAIA V7:"; then ((passed++)); fi

((total++))
if run_test "Unicode characters" "What is 五 plus 三?" "GAIA V7:"; then ((passed++)); fi

echo ""
echo "=== 4. Rapid Fire Tests ==="

# Multiple queries in quick succession
for i in {1..5}; do
    ((total++))
    if run_test "Rapid query $i" "What is $i plus $i?" "$i plus $i ="; then ((passed++)); fi
done

echo ""
echo "=== 5. Memory/Buffer Tests ==="

# Very long input
long_input="What is "
for i in {1..100}; do
    long_input+="1 plus "
done
long_input+="1?"

((total++))
if run_test "Very long calculation chain" "$long_input" "GAIA V7:"; then ((passed++)); fi

# Repeated pattern
((total++))
repeated="What is 5 plus 3? "
for i in {1..20}; do
    repeated+="What is 5 plus 3? "
done
if run_test "Repeated questions" "$repeated" "5 plus 3"; then ((passed++)); fi

echo ""
echo "=== 6. Special Commands Test ==="

# Test all special commands
commands=("stats" "toggle-workflows" "toggle-debug" "workflow-test" "log-summary")
for cmd in "${commands[@]}"; do
    ((total++))
    output=$(echo -e "$cmd\nquit" | timeout 5 ./gaia_chat_v7 2>&1 | grep -v "^You:" | head -10)
    if [[ -n "$output" ]]; then
        echo "Command '$cmd'... PASSED ✓"
        ((passed++))
    else
        echo "Command '$cmd'... FAILED ✗"
    fi
done

echo ""
echo "=== 7. Concurrent Workflow Tests ==="

# Test workflow with interruptions
((total++))
test_sequence=$(cat << EOF
What is 10 plus 20? Also explain addition.
stats
What is 30 minus 10?
quit
EOF
)

output=$(echo "$test_sequence" | timeout 10 ./gaia_chat_v7 2>&1)
if [[ "$output" == *"10 plus 20 = 30"* ]] && [[ "$output" == *"Total patterns"* ]]; then
    echo "Workflow interruption test... PASSED ✓"
    ((passed++))
else
    echo "Workflow interruption test... FAILED ✗"
fi

echo ""
echo "=== 8. Error Recovery Tests ==="

# Division by zero followed by valid query
((total++))
test_recovery=$(cat << EOF
What is 10 divided by 0?
What is 10 divided by 2?
quit
EOF
)

output=$(echo "$test_recovery" | timeout 10 ./gaia_chat_v7 2>&1)
if [[ "$output" == *"Error: division by zero"* ]] && [[ "$output" == *"10 divided 2 = 5"* ]]; then
    echo "Error recovery test... PASSED ✓"
    ((passed++))
else
    echo "Error recovery test... FAILED ✗"
fi

echo ""
echo "=== 9. Performance Tests ==="

# Time a simple calculation
start_time=$(date +%s.%N)
echo "What is 5 plus 3?" | timeout 5 ./gaia_chat_v7 >/dev/null 2>&1
end_time=$(date +%s.%N)
response_time=$(echo "$end_time - $start_time" | bc)

echo "Simple calculation response time: ${response_time}s"
if (( $(echo "$response_time < 2.0" | bc -l) )); then
    echo "Performance test... PASSED ✓"
    ((passed++))
    ((total++))
else
    echo "Performance test... FAILED ✗ (too slow)"
    ((total++))
fi

echo ""
echo "=== 10. Workflow Edge Cases ==="

# Test workflow with no valid steps
((total++))
if run_test "Invalid workflow query" "ajsdfkjasdf asdfasdf" "GAIA V7:"; then ((passed++)); fi

# Test workflow with only explanations
((total++))
if run_test "Explanation only" "Explain explain explain" "GAIA V7:"; then ((passed++)); fi

# Test deeply nested questions
((total++))
if run_test "Nested questions" "What is 5 plus what is 3 plus 2?" "GAIA V7:"; then ((passed++)); fi

echo ""
echo "=== STRESS TEST SUMMARY ==="
echo "Total tests: $total"
echo "Passed: $passed ($(( passed * 100 / total ))%)"
echo "Failed: $(( total - passed ))"
echo ""

# Run memory check
echo "=== Memory Usage Check ==="
if command -v valgrind >/dev/null 2>&1; then
    echo "Running valgrind memory check..."
    echo "What is 5 plus 3?" | valgrind --leak-check=summary --error-exitcode=1 ./gaia_chat_v7 >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "Memory check... PASSED ✓"
    else
        echo "Memory check... FAILED ✗ (memory leaks detected)"
    fi
else
    echo "Valgrind not installed, skipping memory check"
fi

exit $((total - passed))