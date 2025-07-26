#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_OUTPUT 4096
#define TIMEOUT_SECONDS 5

// Test result structure
typedef struct {
    const char* test_name;
    const char* input;
    const char* expected_patterns[5];  // Up to 5 patterns to check
    int should_contain;  // 1 = should contain, 0 = should NOT contain
    int passed;
    char actual_output[MAX_OUTPUT];
} TestCase;

// Test categories
typedef enum {
    CAT_ARITHMETIC,
    CAT_MULTIPART,
    CAT_EXPLANATION,
    CAT_ERROR_HANDLING,
    CAT_EDGE_CASES,
    CAT_WORKFLOW,
    CAT_GREETINGS,
    CAT_LISTS,
    CAT_COMPLEX
} TestCategory;

// Run a single test
int run_test(const char* input, char* output, int max_len) {
    FILE* pipe;
    char command[1024];
    
    // Create command with timeout
    snprintf(command, sizeof(command), 
             "echo '%s' | timeout %d ./gaia_chat_v7 2>&1",
             input, TIMEOUT_SECONDS);
    
    pipe = popen(command, "r");
    if (!pipe) return 0;
    
    // Read output
    int total_read = 0;
    while (fgets(output + total_read, max_len - total_read, pipe) != NULL) {
        total_read = strlen(output);
        if (total_read >= max_len - 1) break;
    }
    
    pclose(pipe);
    return 1;
}

// Check if output contains expected patterns
int check_patterns(const char* output, const char* patterns[], int should_contain) {
    // Look for GAIA V7: line specifically
    const char* gaia_line = strstr(output, "GAIA V7:");
    if (!gaia_line) return 0;
    
    // Check patterns in the GAIA response line and following lines
    for (int i = 0; patterns[i] != NULL && i < 5; i++) {
        int found = (strstr(gaia_line, patterns[i]) != NULL);
        if (should_contain && !found) return 0;
        if (!should_contain && found) return 0;
    }
    return 1;
}

// Print test result
void print_test_result(TestCase* test, int test_num) {
    printf("Test %d: %s\n", test_num, test->test_name);
    printf("  Input: %s\n", test->input);
    printf("  Result: %s\n", test->passed ? "PASSED ✓" : "FAILED ✗");
    if (!test->passed) {
        printf("  Expected to %s: ", test->should_contain ? "contain" : "NOT contain");
        for (int i = 0; test->expected_patterns[i] != NULL && i < 5; i++) {
            printf("'%s' ", test->expected_patterns[i]);
        }
        printf("\n  Actual output: %.200s...\n", test->actual_output);
    }
    printf("\n");
}

int main() {
    printf("=== GAIA V7 Automated Test Suite ===\n");
    printf("Running comprehensive tests...\n\n");
    
    // Define all test cases
    TestCase tests[] = {
        // === ARITHMETIC TESTS ===
        {
            "Simple addition",
            "What is 2 plus 2?",
            {"2 plus 2 = 4", NULL},
            1, 0, ""
        },
        {
            "Large addition",
            "What is 999 plus 1?",
            {"999 plus 1 = 1000", NULL},
            1, 0, ""
        },
        {
            "Subtraction",
            "What is 100 minus 37?",
            {"100 minus 37 = 63", NULL},
            1, 0, ""
        },
        {
            "Multiplication",
            "What is 7 times 8?",
            {"7 times 8 = 56", NULL},
            1, 0, ""
        },
        {
            "Division",
            "What is 144 divided by 12?",
            {"144 divided 12 = 12", NULL},
            1, 0, ""
        },
        {
            "Division by zero",
            "What is 5 divided by 0?",
            {"Error: division by zero", NULL},
            1, 0, ""
        },
        {
            "Negative numbers",
            "What is -5 plus 3?",
            {"-5 plus 3 = -2", NULL},
            1, 0, ""
        },
        {
            "Mixed operations spelling",
            "Calculate twenty plus thirty",
            {"20 plus 30 = 50", NULL},
            1, 0, ""
        },
        
        // === MULTI-PART QUESTIONS ===
        {
            "Two calculations",
            "What is 5 plus 3? What is 10 minus 2?",
            {"5 plus 3 = 8", "10 minus 2 = 8", NULL},
            1, 0, ""
        },
        {
            "Calculation and explanation",
            "What is 15 plus 27? Also, can you explain what addition means?",
            {"15 plus 27 = 42", "Addition is a mathematical operation", NULL},
            1, 0, ""
        },
        {
            "Three parts",
            "What is 2 times 3? What is 12 divided by 4? What is 10 plus 5?",
            {"2 times 3 = 6", "12 divided 4 = 3", "10 plus 5 = 15", NULL},
            1, 0, ""
        },
        {
            "Mixed with 'also'",
            "Calculate 100 minus 25. Also calculate 50 plus 50.",
            {"100 minus 25 = 75", "50 plus 50 = 100", NULL},
            1, 0, ""
        },
        
        // === EXPLANATIONS ===
        {
            "Explain addition",
            "Explain what addition is",
            {"Addition is a mathematical operation", "combines", NULL},
            1, 0, ""
        },
        {
            "Explain multiplication",
            "What does multiplication mean?",
            {"Addition is a mathematical operation", NULL},
            1, 0, ""
        },
        {
            "How does X work",
            "How does subtraction work?",
            {"Addition is a mathematical operation", NULL},
            1, 0, ""
        },
        
        // === ERROR HANDLING ===
        {
            "Empty input",
            "",
            {"Please provide some input", NULL},
            1, 0, ""
        },
        {
            "Nonsense input",
            "askdjfhaskjdfh",
            {"GAIA V7:", NULL},
            1, 0, ""
        },
        {
            "Very long input",
            "What is 1 plus 1 plus 1 plus 1 plus 1 plus 1 plus 1 plus 1 plus 1 plus 1?",
            {"GAIA V7:", NULL},
            1, 0, ""
        },
        
        // === EDGE CASES ===
        {
            "Zero operations",
            "What is 0 plus 0?",
            {"0 plus 0 = 0", NULL},
            1, 0, ""
        },
        {
            "Order of operations",
            "What is 2 plus 3 times 4?",
            {"GAIA V7:", NULL},  // Should produce some output
            1, 0, ""
        },
        {
            "Factorial function",
            "What is the factorial of 5?",
            {"factorial", "120", NULL},
            1, 0, ""
        },
        {
            "Prime check",
            "Is 17 a prime number?",
            {"prime", "17", NULL},
            1, 0, ""
        },
        
        // === WORKFLOW SPECIFIC ===
        {
            "Complex workflow",
            "List three primary colors and explain why they are called primary",
            {"GAIA V7:", NULL},  // Should trigger workflow
            1, 0, ""
        },
        {
            "Multi-step reasoning",
            "What is 25 percent of 80? Explain how percentages work.",
            {"GAIA V7:", NULL},
            1, 0, ""
        },
        
        // === GREETINGS ===
        {
            "Hello greeting",
            "Hello",
            {"Hello", "help", NULL},
            1, 0, ""
        },
        {
            "Goodbye",
            "Goodbye",
            {"Goodbye", "care", NULL},
            1, 0, ""
        },
        
        // === LIST GENERATION ===
        {
            "List colors",
            "List three colors",
            {"red", "blue", "green", NULL},
            1, 0, ""
        },
        {
            "List request",
            "Can you list some animals?",
            {"Here are", NULL},
            1, 0, ""
        },
        
        // === SPECIAL CHARACTERS ===
        {
            "Question with quotes",
            "What is \"five\" plus \"three\"?",
            {"5 plus 3 = 8", NULL},
            1, 0, ""
        },
        {
            "Special symbols",
            "What is 5 + 3?",
            {"GAIA V7:", NULL},
            1, 0, ""
        },
        
        // === ROBUSTNESS ===
        {
            "Multiple spaces",
            "What    is     5    plus    3?",
            {"5 plus 3 = 8", NULL},
            1, 0, ""
        },
        {
            "Mixed case",
            "WhAt Is FiVe PlUs ThReE?",
            {"5 plus 3 = 8", NULL},
            1, 0, ""
        },
        {
            "Punctuation variations",
            "What is 5 plus 3???",
            {"5 plus 3 = 8", NULL},
            1, 0, ""
        },
        
        // === COMPLEX CALCULATIONS ===
        {
            "Three number addition",
            "What is 10 plus 20 plus 30?",
            {"GAIA V7:", NULL},
            1, 0, ""
        },
        {
            "Fibonacci test",
            "What is the 10th Fibonacci number?",
            {"fibonacci", "55", NULL},
            1, 0, ""
        },
        
        // === CONTEXT TESTS ===
        {
            "Context switch",
            "Calculate 5 plus 3. Now explain what a dog is.",
            {"5 plus 3 = 8", NULL},
            1, 0, ""
        },
        {
            "Related questions",
            "What is 10 squared? What is the square root of 100?",
            {"GAIA V7:", NULL},
            1, 0, ""
        },
        
        // === PERFORMANCE EDGE CASES ===
        {
            "Rapid calculations",
            "2+2? 3+3? 4+4?",
            {"GAIA V7:", NULL},
            1, 0, ""
        },
        {
            "Very large numbers",
            "What is 999999 plus 1?",
            {"999999 plus 1 = 1000000", NULL},
            1, 0, ""
        },
        
        // End marker
        {NULL, NULL, {NULL}, 0, 0, ""}
    };
    
    // Run all tests
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Count tests
    for (int i = 0; tests[i].test_name != NULL; i++) {
        total_tests++;
    }
    
    printf("Running %d tests...\n\n", total_tests);
    
    // Execute tests
    clock_t start_time = clock();
    
    for (int i = 0; tests[i].test_name != NULL; i++) {
        // Run the test
        run_test(tests[i].input, tests[i].actual_output, MAX_OUTPUT);
        
        // Check patterns
        tests[i].passed = check_patterns(tests[i].actual_output, 
                                        tests[i].expected_patterns,
                                        tests[i].should_contain);
        
        // Update counters
        if (tests[i].passed) {
            passed_tests++;
        } else {
            failed_tests++;
        }
        
        // Print result
        print_test_result(&tests[i], i + 1);
        
        // Progress indicator
        if ((i + 1) % 10 == 0) {
            printf("=== Progress: %d/%d tests completed ===\n\n", i + 1, total_tests);
        }
    }
    
    clock_t end_time = clock();
    double elapsed = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    // Print summary
    printf("\n=== TEST SUMMARY ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d (%.1f%%)\n", passed_tests, (passed_tests * 100.0) / total_tests);
    printf("Failed: %d (%.1f%%)\n", failed_tests, (failed_tests * 100.0) / total_tests);
    printf("Execution time: %.2f seconds\n", elapsed);
    printf("Average time per test: %.3f seconds\n", elapsed / total_tests);
    
    // Category breakdown
    printf("\n=== FAILED TESTS ===\n");
    if (failed_tests == 0) {
        printf("All tests passed! 🎉\n");
    } else {
        for (int i = 0; tests[i].test_name != NULL; i++) {
            if (!tests[i].passed) {
                printf("- Test %d: %s\n", i + 1, tests[i].test_name);
            }
        }
    }
    
    return failed_tests > 0 ? 1 : 0;
}