#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "gate_types.h"

// Test result structure
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    char test_name[100];
    char failure_reason[500];
} TestResult;

// Test categories
typedef enum {
    TEST_MATH,
    TEST_REASONING,
    TEST_CONVERSATION,
    TEST_COHERENCE,
    TEST_CONTEXT
} TestCategory;

// Forward declarations
typedef struct Pattern Pattern;
typedef struct ChatSystem ChatSystem;

// External functions from gaia_chat.c
extern ChatSystem* create_chat_system(void);
extern void train_from_file(ChatSystem* sys, const char* filename);
extern char* find_best_continuation(ChatSystem* sys, const char* w1, const char* w2);
extern void process_text(ChatSystem* sys, const char* text);

// Test case structure
typedef struct {
    char* input;
    char* expected_keywords[5];  // Keywords that should appear in response
    TestCategory category;
    char* description;
} TestCase;

// Math test cases
TestCase math_tests[] = {
    {
        "What is two plus two?",
        {"four", "4", NULL},
        TEST_MATH,
        "Basic addition"
    },
    {
        "Calculate 5 + 3",
        {"8", "eight", NULL},
        TEST_MATH,
        "Addition calculation"
    },
    {
        "What is ten minus four?",
        {"six", "6", NULL},
        TEST_MATH,
        "Basic subtraction"
    },
    {
        "What is three times four?",
        {"twelve", "12", NULL},
        TEST_MATH,
        "Basic multiplication"
    },
    {
        "What is twenty divided by five?",
        {"four", "4", NULL},
        TEST_MATH,
        "Basic division"
    }
};

// Reasoning test cases
TestCase reasoning_tests[] = {
    {
        "If all dogs are animals and Max is a dog, what can we conclude?",
        {"animal", "max", NULL},
        TEST_REASONING,
        "Basic syllogism"
    },
    {
        "The ice melted. What caused this?",
        {"temperature", "heat", "warm", NULL},
        TEST_REASONING,
        "Cause and effect"
    },
    {
        "If A is bigger than B, and B is bigger than C, what about A and C?",
        {"bigger", "greater", "larger", NULL},
        TEST_REASONING,
        "Transitive property"
    }
};

// Conversation test cases
TestCase conversation_tests[] = {
    {
        "Hi",
        {"hello", "hi", "greetings", NULL},
        TEST_CONVERSATION,
        "Basic greeting"
    },
    {
        "How are you?",
        {"well", "good", "fine", "functioning", NULL},
        TEST_CONVERSATION,
        "Status inquiry"
    },
    {
        "Thank you",
        {"welcome", "pleasure", "glad", NULL},
        TEST_CONVERSATION,
        "Gratitude response"
    },
    {
        "What is your name?",
        {"gaia", NULL},
        TEST_CONVERSATION,
        "Identity question"
    }
};

// Helper function to check if response contains expected keywords
int check_keywords(const char* response, char* keywords[]) {
    if (!response) return 0;
    
    // Convert response to lowercase for comparison
    char lower_response[1000];
    int i = 0;
    while (response[i] && i < 999) {
        lower_response[i] = tolower(response[i]);
        i++;
    }
    lower_response[i] = '\0';
    
    // Check each keyword
    for (int j = 0; keywords[j] != NULL; j++) {
        if (strstr(lower_response, keywords[j])) {
            return 1;  // Found at least one keyword
        }
    }
    return 0;
}

// Generate response using the chat system
char* generate_test_response(ChatSystem* sys, const char* input) {
    static char response[1000];
    response[0] = '\0';
    
    // Process input to extract context
    char buffer[500];
    strcpy(buffer, input);
    
    // Convert to lowercase
    for (int i = 0; buffer[i]; i++) {
        buffer[i] = tolower(buffer[i]);
    }
    
    // Extract last two words for context
    char* words[50];
    int word_count = 0;
    
    char* token = strtok(buffer, " \t\n\r.,!?;:");
    while (token && word_count < 50) {
        words[word_count++] = token;
        token = strtok(NULL, " \t\n\r.,!?;:");
    }
    
    if (word_count >= 2) {
        char* continuation = find_best_continuation(sys, words[word_count-2], words[word_count-1]);
        if (continuation) {
            strcpy(response, continuation);
        }
    } else if (word_count == 1) {
        char* continuation = find_best_continuation(sys, "", words[0]);
        if (continuation) {
            strcpy(response, continuation);
        }
    }
    
    return response;
}

// Run a single test case
TestResult run_test_case(ChatSystem* sys, TestCase* test) {
    TestResult result = {0};
    result.total_tests = 1;
    strcpy(result.test_name, test->description);
    
    printf("Testing: %s\n", test->description);
    printf("Input: %s\n", test->input);
    
    // Generate response
    char* response = generate_test_response(sys, test->input);
    printf("Response: %s\n", response);
    
    // Check for expected keywords
    if (check_keywords(response, test->expected_keywords)) {
        result.passed_tests = 1;
        printf("✓ PASSED\n");
    } else {
        result.failed_tests = 1;
        sprintf(result.failure_reason, "Expected keywords not found in response");
        printf("✗ FAILED: %s\n", result.failure_reason);
    }
    
    printf("---\n");
    return result;
}

// Run coherence test
TestResult run_coherence_test(ChatSystem* sys) {
    TestResult result = {0};
    result.total_tests = 1;
    strcpy(result.test_name, "Coherence Test");
    
    printf("\n=== Coherence Test ===\n");
    
    // Generate a longer response to test coherence
    const char* prompts[] = {
        "Logic gates are",
        "The system processes",
        "Pattern matching enables"
    };
    
    int coherent_count = 0;
    
    for (int i = 0; i < 3; i++) {
        printf("Prompt: %s\n", prompts[i]);
        
        // Generate multiple continuations
        char full_response[500] = "";
        strcat(full_response, prompts[i]);
        strcat(full_response, " ");
        
        // Get context
        char buffer[100];
        strcpy(buffer, prompts[i]);
        char* words[10];
        int word_count = 0;
        
        char* token = strtok(buffer, " ");
        while (token && word_count < 10) {
            words[word_count++] = token;
            token = strtok(NULL, " ");
        }
        
        // Generate 5 words
        char w1[50] = "";
        char w2[50] = "";
        
        if (word_count >= 2) {
            strcpy(w1, words[word_count-2]);
            strcpy(w2, words[word_count-1]);
        } else if (word_count == 1) {
            strcpy(w2, words[0]);
        }
        
        for (int j = 0; j < 5; j++) {
            char* next = find_best_continuation(sys, w1, w2);
            if (!next) break;
            
            strcat(full_response, next);
            strcat(full_response, " ");
            
            strcpy(w1, w2);
            strcpy(w2, next);
        }
        
        printf("Generated: %s\n", full_response);
        
        // Simple coherence check - response should be longer than prompt
        if (strlen(full_response) > strlen(prompts[i]) + 5) {
            coherent_count++;
        }
    }
    
    if (coherent_count >= 2) {
        result.passed_tests = 1;
        printf("✓ Coherence test PASSED\n");
    } else {
        result.failed_tests = 1;
        printf("✗ Coherence test FAILED\n");
    }
    
    return result;
}

// Run all tests
void run_all_tests(ChatSystem* sys) {
    TestResult overall = {0};
    
    printf("\n=== Running gaia Unit Tests ===\n\n");
    
    // Math tests
    printf("\n--- MATH TESTS ---\n");
    for (int i = 0; i < sizeof(math_tests)/sizeof(TestCase); i++) {
        TestResult result = run_test_case(sys, &math_tests[i]);
        overall.total_tests += result.total_tests;
        overall.passed_tests += result.passed_tests;
        overall.failed_tests += result.failed_tests;
    }
    
    // Reasoning tests
    printf("\n--- REASONING TESTS ---\n");
    for (int i = 0; i < sizeof(reasoning_tests)/sizeof(TestCase); i++) {
        TestResult result = run_test_case(sys, &reasoning_tests[i]);
        overall.total_tests += result.total_tests;
        overall.passed_tests += result.passed_tests;
        overall.failed_tests += result.failed_tests;
    }
    
    // Conversation tests
    printf("\n--- CONVERSATION TESTS ---\n");
    for (int i = 0; i < sizeof(conversation_tests)/sizeof(TestCase); i++) {
        TestResult result = run_test_case(sys, &conversation_tests[i]);
        overall.total_tests += result.total_tests;
        overall.passed_tests += result.passed_tests;
        overall.failed_tests += result.failed_tests;
    }
    
    // Coherence test
    TestResult coherence_result = run_coherence_test(sys);
    overall.total_tests += coherence_result.total_tests;
    overall.passed_tests += coherence_result.passed_tests;
    overall.failed_tests += coherence_result.failed_tests;
    
    // Summary
    printf("\n=== TEST SUMMARY ===\n");
    printf("Total tests: %d\n", overall.total_tests);
    printf("Passed: %d (%.1f%%)\n", overall.passed_tests, 
           (overall.passed_tests * 100.0) / overall.total_tests);
    printf("Failed: %d\n", overall.failed_tests);
    
    if (overall.failed_tests == 0) {
        printf("\n✓ ALL TESTS PASSED! 🎉\n");
    } else {
        printf("\n✗ Some tests failed. More training needed.\n");
    }
}

// Performance benchmark
void run_performance_test(ChatSystem* sys) {
    printf("\n=== Performance Benchmark ===\n");
    
    const int num_iterations = 1000;
    clock_t start = clock();
    
    for (int i = 0; i < num_iterations; i++) {
        find_best_continuation(sys, "the", "system");
    }
    
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Generated %d responses in %.3f seconds\n", num_iterations, cpu_time);
    printf("Average time per response: %.3f ms\n", (cpu_time * 1000) / num_iterations);
    printf("Responses per second: %.0f\n", num_iterations / cpu_time);
}

int main() {
    printf("gaia Test Framework\n");
    printf("==================\n\n");
    
    // Initialize gates
    gate_registry_init();
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    // Create and train system
    ChatSystem* sys = create_chat_system();
    
    printf("Training gaia on datasets...\n");
    train_from_file(sys, "datasets/math/basic_arithmetic.txt");
    train_from_file(sys, "datasets/math/word_problems.txt");
    train_from_file(sys, "datasets/reasoning/logical_reasoning.txt");
    train_from_file(sys, "datasets/reasoning/cause_effect.txt");
    train_from_file(sys, "datasets/conversation/dialogue_structure.txt");
    train_from_file(sys, "datasets/conversation/turn_taking.txt");
    train_from_file(sys, "datasets/conversational_corpus.txt");
    printf("Training complete!\n");
    
    // Run tests
    run_all_tests(sys);
    
    // Performance benchmark
    run_performance_test(sys);
    
    // Cleanup would go here
    gate_registry_cleanup();
    
    return 0;
}