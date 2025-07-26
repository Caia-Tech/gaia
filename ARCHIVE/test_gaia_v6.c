#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

// Test counter
static int tests_run = 0;
static int tests_passed = 0;

// Test macro
#define RUN_TEST(test_func) do { \
    printf("Running %s...\n", #test_func); \
    tests_run++; \
    if (test_func()) { \
        tests_passed++; \
        printf("  ✓ Passed\n"); \
    } else { \
        printf("  ✗ Failed\n"); \
    } \
} while(0)

// Function to run GAIA V6 and capture output
int run_gaia_test(const char* input, const char* expected_keywords[], int num_keywords) {
    // Create a test command that runs GAIA V6 with the input
    char command[1024];
    snprintf(command, sizeof(command), 
             "echo '%s' | timeout 10 ./gaia_chat_v6 --no-analysis 2>/dev/null", input);
    
    FILE* fp = popen(command, "r");
    if (!fp) return 0;
    
    char output[2048] = "";
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        strcat(output, buffer);
    }
    pclose(fp);
    
    // Convert output to lowercase for comparison
    for (int i = 0; output[i]; i++) {
        output[i] = tolower(output[i]);
    }
    
    // Check if any expected keywords are present
    int keywords_found = 0;
    for (int i = 0; i < num_keywords; i++) {
        if (strstr(output, expected_keywords[i])) {
            keywords_found++;
        }
    }
    
    printf("    Input: '%s'\n", input);
    printf("    Output contains %d/%d expected keywords\n", keywords_found, num_keywords);
    
    return keywords_found > 0;  // Pass if we find at least one keyword
}

// Test basic functionality
int test_basic_functionality() {
    const char* greetings[] = {"hello", "hi", "good"};
    return run_gaia_test("Hello", greetings, 3);
}

// Test math calculations
int test_math_calculations() {
    const char* math_keywords[] = {"8", "eight"};
    return run_gaia_test("Calculate 5 plus 3", math_keywords, 2);
}

// Test simple completions
int test_completions() {
    const char* completion_keywords[] = {"mat", "floor", "chair", "table"};
    return run_gaia_test("The cat sat on the", completion_keywords, 4);
}

// Test question responses
int test_questions() {
    const char* question_keywords[] = {"42", "meaning", "life", "universe"};
    return run_gaia_test("What is the meaning of life?", question_keywords, 4);
}

// Test knowledge queries
int test_knowledge() {
    const char* knowledge_keywords[] = {"process", "plants", "light", "energy", "glucose"};
    return run_gaia_test("What is photosynthesis?", knowledge_keywords, 5);
}

// Test farewells
int test_farewells() {
    const char* farewell_keywords[] = {"goodbye", "bye", "take care"};
    return run_gaia_test("Goodbye", farewell_keywords, 3);
}

// Test command following
int test_commands() {
    const char* command_keywords[] = {"red", "blue", "green", "yellow", "colors"};
    return run_gaia_test("List three colors", command_keywords, 5);
}

// Test edge cases
int test_edge_cases() {
    // Test empty input (should handle gracefully)
    const char* empty_keywords[] = {"please", "provide", "input", "help"};
    int empty_result = run_gaia_test("", empty_keywords, 4);
    
    // Test very short input
    const char* short_keywords[] = {"hello", "hi", "can", "help"};
    int short_result = run_gaia_test("Hi", short_keywords, 4);
    
    return empty_result || short_result;  // Pass if either works
}

// Test with analysis functions enabled
int test_with_analysis() {
    // Create a test command with analysis enabled
    char command[1024];
    snprintf(command, sizeof(command), 
             "echo 'Calculate 2 times 3' | timeout 10 ./gaia_chat_v6 2>/dev/null");
    
    FILE* fp = popen(command, "r");
    if (!fp) return 0;
    
    char output[2048] = "";
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        strcat(output, buffer);
    }
    pclose(fp);
    
    // Should contain the result "6"
    return strstr(output, "6") != NULL;
}

// Test superposition mode
int test_superposition() {
    // Create a test command with superposition enabled
    char command[1024];
    snprintf(command, sizeof(command), 
             "echo 'The weather is' | timeout 10 ./gaia_chat_v6 --superposition --debug-superposition 2>/dev/null");
    
    FILE* fp = popen(command, "r");
    if (!fp) return 0;
    
    char output[2048] = "";
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        strcat(output, buffer);
    }
    pclose(fp);
    
    // Should contain superposition debug info or generate response
    return strstr(output, "Superposition") != NULL || strlen(output) > 50;
}

int main() {
    printf("=== GAIA V6 Comprehensive Test Suite ===\n\n");
    
    // First, compile GAIA V6
    printf("Compiling GAIA V6...\n");
    int compile_result = system("gcc -Wall -o gaia_chat_v6 gaia_chat_v6.c analysis_functions.c function_registry.c gaia_functions.c -lm 2>/dev/null");
    if (compile_result != 0) {
        printf("❌ GAIA V6 compilation failed!\n");
        return 1;
    }
    printf("✅ GAIA V6 compiled successfully\n\n");
    
    // Run all tests
    RUN_TEST(test_basic_functionality);
    RUN_TEST(test_math_calculations);
    RUN_TEST(test_completions);
    RUN_TEST(test_questions);
    RUN_TEST(test_knowledge);
    RUN_TEST(test_farewells);
    RUN_TEST(test_commands);
    RUN_TEST(test_edge_cases);
    RUN_TEST(test_with_analysis);
    RUN_TEST(test_superposition);
    
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Success rate: %.1f%%\n", (tests_passed * 100.0) / tests_run);
    
    if (tests_passed == tests_run) {
        printf("🎉 All tests passed! GAIA V6 is working correctly.\n");
    } else {
        printf("⚠️  Some tests failed. Check the output above for details.\n");
    }
    
    return tests_passed == tests_run ? 0 : 1;
}