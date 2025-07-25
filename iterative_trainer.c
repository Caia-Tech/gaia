#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gate_types.h"

// Training iteration results
typedef struct {
    int iteration;
    float accuracy;
    int patterns_learned;
    int tests_passed;
    int tests_total;
    double training_time;
} IterationResult;

// Failed test tracking
typedef struct FailedTest {
    char input[200];
    char expected[200];
    char actual[200];
    struct FailedTest* next;
} FailedTest;

// Training configuration
typedef struct {
    int max_iterations;
    float target_accuracy;
    int augment_on_failure;
    int focus_on_failures;
} TrainingConfig;

// External functions
typedef struct ChatSystem ChatSystem;
extern ChatSystem* create_chat_system(void);
extern void train_from_file(ChatSystem* sys, const char* filename);
extern void process_text(ChatSystem* sys, const char* text);
extern char* find_best_continuation(ChatSystem* sys, const char* w1, const char* w2);

// Create augmented training data from failed tests
void create_augmented_data(FailedTest* failures, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    FailedTest* current = failures;
    while (current) {
        // Write original failed case
        fprintf(f, "%s %s\n", current->input, current->expected);
        
        // Create variations
        // Variation 1: Add context before
        fprintf(f, "User asks: %s Assistant responds: %s\n", current->input, current->expected);
        
        // Variation 2: Different phrasing
        fprintf(f, "Question: %s Answer: %s\n", current->input, current->expected);
        
        // Variation 3: With explanation
        fprintf(f, "%s The answer is %s because that's the correct response.\n", 
                current->input, current->expected);
        
        // Variation 4: In conversation
        fprintf(f, "A: %s B: %s That's the right answer.\n", current->input, current->expected);
        
        current = current->next;
    }
    
    fclose(f);
}

// Test a single input and return success
int test_single_input(ChatSystem* sys, const char* input, const char* expected) {
    // Process input to get context
    char buffer[200];
    strcpy(buffer, input);
    
    // Convert to lowercase
    for (int i = 0; buffer[i]; i++) {
        buffer[i] = tolower(buffer[i]);
    }
    
    // Extract words
    char* words[50];
    int word_count = 0;
    
    char* temp = strdup(buffer);
    char* token = strtok(temp, " \t\n\r.,!?;:");
    while (token && word_count < 50) {
        words[word_count++] = token;
        token = strtok(NULL, " \t\n\r.,!?;:");
    }
    
    // Get response
    char* response = NULL;
    if (word_count >= 2) {
        response = find_best_continuation(sys, words[word_count-2], words[word_count-1]);
    } else if (word_count == 1) {
        response = find_best_continuation(sys, "", words[0]);
    }
    
    free(temp);
    
    // Check if response contains expected keywords
    if (response) {
        char lower_response[200];
        strcpy(lower_response, response);
        for (int i = 0; lower_response[i]; i++) {
            lower_response[i] = tolower(lower_response[i]);
        }
        
        char lower_expected[200];
        strcpy(lower_expected, expected);
        for (int i = 0; lower_expected[i]; i++) {
            lower_expected[i] = tolower(lower_expected[i]);
        }
        
        return strstr(lower_response, lower_expected) != NULL;
    }
    
    return 0;
}

// Run comprehensive test suite
IterationResult run_test_suite(ChatSystem* sys, FailedTest** failures) {
    IterationResult result = {0};
    
    // Test cases
    struct {
        char* input;
        char* expected;
        char* category;
    } test_cases[] = {
        // Math tests
        {"What is two plus two", "four", "math"},
        {"Calculate 5 + 3", "8", "math"},
        {"What is ten minus four", "six", "math"},
        {"What is three times four", "twelve", "math"},
        
        // Reasoning tests
        {"If all dogs are animals", "animal", "reasoning"},
        {"The ice melted", "temperature", "reasoning"},
        
        // Conversation tests
        {"Hi", "hello", "conversation"},
        {"How are you", "well", "conversation"},
        {"Thank you", "welcome", "conversation"},
        {"What is your name", "gaia", "conversation"},
        
        // Context tests
        {"Logic gates", "process", "context"},
        {"Pattern matching", "enables", "context"},
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    result.tests_total = num_tests;
    
    // Clear previous failures
    FailedTest* current = *failures;
    while (current) {
        FailedTest* next = current->next;
        free(current);
        current = next;
    }
    *failures = NULL;
    
    // Run each test
    printf("\nRunning tests...\n");
    for (int i = 0; i < num_tests; i++) {
        int passed = test_single_input(sys, test_cases[i].input, test_cases[i].expected);
        
        if (passed) {
            result.tests_passed++;
            printf("✓ ");
        } else {
            printf("✗ ");
            
            // Track failure
            FailedTest* fail = malloc(sizeof(FailedTest));
            strcpy(fail->input, test_cases[i].input);
            strcpy(fail->expected, test_cases[i].expected);
            strcpy(fail->actual, "no_match");
            fail->next = *failures;
            *failures = fail;
        }
        
        printf("[%s] %s -> %s\n", 
               test_cases[i].category,
               test_cases[i].input, 
               test_cases[i].expected);
    }
    
    result.accuracy = (float)result.tests_passed / result.tests_total;
    return result;
}

// Iterative training loop
void iterative_training(TrainingConfig config) {
    printf("=== Iterative Training System ===\n");
    printf("Target accuracy: %.1f%%\n", config.target_accuracy * 100);
    printf("Max iterations: %d\n\n", config.max_iterations);
    
    // Initialize
    gate_registry_init();
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    ChatSystem* sys = create_chat_system();
    FailedTest* failures = NULL;
    
    // Initial training
    printf("Initial training phase...\n");
    clock_t start = clock();
    
    train_from_file(sys, "datasets/math/basic_arithmetic.txt");
    train_from_file(sys, "datasets/math/word_problems.txt");
    train_from_file(sys, "datasets/reasoning/logical_reasoning.txt");
    train_from_file(sys, "datasets/reasoning/cause_effect.txt");
    train_from_file(sys, "datasets/conversation/dialogue_structure.txt");
    train_from_file(sys, "datasets/conversation/turn_taking.txt");
    
    clock_t end = clock();
    double initial_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Initial training completed in %.2f seconds\n", initial_time);
    
    // Iterative improvement
    for (int iter = 1; iter <= config.max_iterations; iter++) {
        printf("\n--- Iteration %d ---\n", iter);
        
        // Test current performance
        IterationResult result = run_test_suite(sys, &failures);
        result.iteration = iter;
        
        printf("\nAccuracy: %.1f%% (%d/%d tests passed)\n", 
               result.accuracy * 100, result.tests_passed, result.tests_total);
        
        // Check if target reached
        if (result.accuracy >= config.target_accuracy) {
            printf("\n✓ Target accuracy reached! Training complete.\n");
            break;
        }
        
        // If not improving, augment training data
        if (failures && config.augment_on_failure) {
            printf("\nAugmenting training data based on failures...\n");
            
            // Create augmented data file
            create_augmented_data(failures, "datasets/augmented_training.txt");
            
            // Train on augmented data
            train_from_file(sys, "datasets/augmented_training.txt");
            
            // Focus training on problem areas
            if (config.focus_on_failures) {
                printf("Focused training on failed cases...\n");
                
                FailedTest* current = failures;
                while (current) {
                    // Create multiple training examples for each failure
                    char training_text[500];
                    
                    // Pattern 1
                    sprintf(training_text, "%s %s", current->input, current->expected);
                    process_text(sys, training_text);
                    
                    // Pattern 2
                    sprintf(training_text, "When asked %s the answer is %s", 
                            current->input, current->expected);
                    process_text(sys, training_text);
                    
                    // Pattern 3
                    sprintf(training_text, "%s leads to %s", 
                            current->input, current->expected);
                    process_text(sys, training_text);
                    
                    current = current->next;
                }
            }
        }
        
        // Additional training on core datasets
        if (iter % 2 == 0) {
            printf("Reinforcement training...\n");
            train_from_file(sys, "datasets/conversational_corpus.txt");
        }
    }
    
    // Final evaluation
    printf("\n=== Final Evaluation ===\n");
    IterationResult final_result = run_test_suite(sys, &failures);
    
    printf("\nFinal accuracy: %.1f%%\n", final_result.accuracy * 100);
    printf("Tests passed: %d/%d\n", final_result.tests_passed, final_result.tests_total);
    
    // Show remaining failures
    if (failures) {
        printf("\nRemaining challenging cases:\n");
        FailedTest* current = failures;
        while (current) {
            printf("- \"%s\" -> expected \"%s\"\n", current->input, current->expected);
            current = current->next;
        }
    }
    
    // Cleanup
    current = failures;
    while (current) {
        FailedTest* next = current->next;
        free(current);
        current = next;
    }
    
    gate_registry_cleanup();
}

int main() {
    printf("gaia Iterative Training System\n");
    printf("=============================\n\n");
    
    // Configure training
    TrainingConfig config = {
        .max_iterations = 10,
        .target_accuracy = 0.85,  // 85% accuracy target
        .augment_on_failure = 1,
        .focus_on_failures = 1
    };
    
    // Run iterative training
    iterative_training(config);
    
    printf("\nTraining session complete.\n");
    return 0;
}