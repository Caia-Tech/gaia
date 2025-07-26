#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "analysis_functions.h"

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

// Test prompt classification
int test_prompt_classification() {
    struct {
        const char* input;
        PromptType expected;
    } test_cases[] = {
        // Questions
        {"What is the meaning of life?", PROMPT_QUESTION},
        {"How are you?", PROMPT_QUESTION},
        {"Can you help me?", PROMPT_QUESTION},
        
        // Commands
        {"Calculate 2 plus 2", PROMPT_COMMAND},
        {"List three colors", PROMPT_COMMAND},
        {"Tell me about dogs", PROMPT_COMMAND},
        {"Explain quantum physics", PROMPT_COMMAND},
        
        // Greetings
        {"Hello", PROMPT_GREETING},
        {"Good morning", PROMPT_GREETING},
        {"Hi there", PROMPT_GREETING},
        
        // Farewells
        {"Goodbye", PROMPT_FAREWELL},
        {"See you later", PROMPT_FAREWELL},
        {"Bye", PROMPT_FAREWELL},
        
        // Completions
        {"The cat sat on the", PROMPT_COMPLETION},
        {"Once upon a", PROMPT_COMPLETION},
        {"To be or not to", PROMPT_COMPLETION},
        
        // Acknowledgments
        {"Thank you", PROMPT_ACKNOWLEDGMENT},
        {"I see", PROMPT_ACKNOWLEDGMENT},
        {"Got it", PROMPT_ACKNOWLEDGMENT},
        
        // Statements
        {"The sky is blue", PROMPT_STATEMENT},
        {"I like pizza", PROMPT_STATEMENT},
        {"Dogs are animals", PROMPT_STATEMENT},
        
        {NULL, PROMPT_UNKNOWN}
    };
    
    int all_passed = 1;
    for (int i = 0; test_cases[i].input != NULL; i++) {
        PromptType result = classify_prompt(test_cases[i].input);
        if (result != test_cases[i].expected) {
            printf("    Failed: '%s' expected %s, got %s\n", 
                   test_cases[i].input,
                   prompt_type_to_string(test_cases[i].expected),
                   prompt_type_to_string(result));
            all_passed = 0;
        }
    }
    
    return all_passed;
}

// Test question classification
int test_question_classification() {
    struct {
        const char* input;
        QuestionType expected;
    } test_cases[] = {
        {"What is your name?", QUESTION_WHAT},
        {"How does this work?", QUESTION_HOW},
        {"Why is the sky blue?", QUESTION_WHY},
        {"When will it rain?", QUESTION_WHEN},
        {"Where is the library?", QUESTION_WHERE},
        {"Who are you?", QUESTION_WHO},
        {"Which one is better?", QUESTION_WHICH},
        {"Is this correct?", QUESTION_YES_NO},
        {"Are you ready?", QUESTION_YES_NO},
        {"Can you help?", QUESTION_YES_NO},
        {"Do you understand?", QUESTION_YES_NO},
        {"Tell me something", QUESTION_UNKNOWN},  // Not a question
        {NULL, QUESTION_UNKNOWN}
    };
    
    int all_passed = 1;
    for (int i = 0; test_cases[i].input != NULL; i++) {
        QuestionType result = classify_question(test_cases[i].input);
        if (result != test_cases[i].expected) {
            printf("    Failed: '%s' expected %s, got %s\n", 
                   test_cases[i].input,
                   question_type_to_string(test_cases[i].expected),
                   question_type_to_string(result));
            all_passed = 0;
        }
    }
    
    return all_passed;
}

// Test topic detection
int test_topic_detection() {
    struct {
        const char* input;
        TopicType expected;
    } test_cases[] = {
        {"Calculate 5 plus 3", TOPIC_MATH},
        {"What is 10 times 20?", TOPIC_MATH},
        {"Add these numbers together", TOPIC_MATH},
        
        {"What is photosynthesis?", TOPIC_SCIENCE},
        {"Explain quantum mechanics", TOPIC_SCIENCE},
        {"How does gravity work?", TOPIC_SCIENCE},
        
        {"How does a computer work?", TOPIC_TECHNOLOGY},
        {"What is artificial intelligence?", TOPIC_TECHNOLOGY},
        {"Explain algorithms", TOPIC_TECHNOLOGY},
        
        {"What is the meaning of life?", TOPIC_PHILOSOPHY},
        {"Does free will exist?", TOPIC_PHILOSOPHY},
        {"What is consciousness?", TOPIC_PHILOSOPHY},
        
        {"List three colors", TOPIC_INSTRUCTION},
        {"Tell me about dogs", TOPIC_INSTRUCTION},
        {"Show me examples", TOPIC_INSTRUCTION},
        
        {"Create a story", TOPIC_CREATIVE},
        {"Imagine a world", TOPIC_CREATIVE},
        {"Describe a sunset", TOPIC_CREATIVE},
        
        {"Hello, how are you?", TOPIC_CASUAL},
        {"Nice weather today", TOPIC_CASUAL},
        
        {NULL, TOPIC_UNKNOWN}
    };
    
    int all_passed = 1;
    for (int i = 0; test_cases[i].input != NULL; i++) {
        TopicType result = detect_topic(test_cases[i].input);
        if (result != test_cases[i].expected) {
            printf("    Failed: '%s' expected %s, got %s\n", 
                   test_cases[i].input,
                   topic_type_to_string(test_cases[i].expected),
                   topic_type_to_string(result));
            all_passed = 0;
        }
    }
    
    return all_passed;
}

// Test entity extraction
int test_entity_extraction() {
    struct {
        const char* input;
        int expected_count;
        EntityType expected_types[5];
        const char* expected_values[5];
    } test_cases[] = {
        {
            "Calculate 5 plus 3",
            3,
            {ENTITY_NUMBER, ENTITY_OPERATOR, ENTITY_NUMBER},
            {"5", "plus", "3"}
        },
        {
            "What is two times three?",
            3,
            {ENTITY_NUMBER, ENTITY_OPERATOR, ENTITY_NUMBER},
            {"2", "times", "3"}
        },
        {
            "List three colors",
            2,
            {ENTITY_NUMBER, ENTITY_CATEGORY},
            {"3", "colors"}
        },
        {
            "Add 10 and 20 together",
            3,
            {ENTITY_OPERATOR, ENTITY_NUMBER, ENTITY_NUMBER},
            {"add", "10", "20"}
        },
        {
            "Give me some examples",
            1,
            {ENTITY_QUANTITY},
            {"some"}
        },
        {NULL, 0, {}, {}}
    };
    
    int all_passed = 1;
    Entity entities[MAX_ENTITIES];
    
    for (int i = 0; test_cases[i].input != NULL; i++) {
        int count = extract_entities(test_cases[i].input, entities, MAX_ENTITIES);
        
        if (count != test_cases[i].expected_count) {
            printf("    Failed: '%s' expected %d entities, got %d\n", 
                   test_cases[i].input, test_cases[i].expected_count, count);
            all_passed = 0;
            continue;
        }
        
        // Check each entity
        for (int j = 0; j < count; j++) {
            if (entities[j].type != test_cases[i].expected_types[j] ||
                strcmp(entities[j].value, test_cases[i].expected_values[j]) != 0) {
                printf("    Failed: '%s' entity %d mismatch\n", 
                       test_cases[i].input, j);
                all_passed = 0;
            }
        }
    }
    
    return all_passed;
}

// Test text metrics
int test_text_metrics() {
    struct {
        const char* input;
        int expected_words;
        int expected_sentences;
    } test_cases[] = {
        {"Hello world", 2, 1},
        {"This is a test.", 4, 1},
        {"First sentence. Second sentence.", 4, 2},
        {"Question? Answer!", 2, 2},
        {"One", 1, 1},
        {"", 0, 0},
        {"Multiple   spaces   between   words", 4, 1},
        {NULL, 0, 0}
    };
    
    int all_passed = 1;
    for (int i = 0; test_cases[i].input != NULL; i++) {
        int words = count_words(test_cases[i].input);
        int sentences = count_sentences(test_cases[i].input);
        
        if (words != test_cases[i].expected_words) {
            printf("    Failed: '%s' expected %d words, got %d\n", 
                   test_cases[i].input, test_cases[i].expected_words, words);
            all_passed = 0;
        }
        
        if (sentences != test_cases[i].expected_sentences) {
            printf("    Failed: '%s' expected %d sentences, got %d\n", 
                   test_cases[i].input, test_cases[i].expected_sentences, sentences);
            all_passed = 0;
        }
    }
    
    return all_passed;
}

// Test coherence analysis
int test_coherence_analysis() {
    struct {
        const char* context;
        const char* candidate;
        float min_score;  // Minimum expected overall score
    } test_cases[] = {
        {"The cat", "sat", 0.5},        // Good continuation
        {"The cat", "flew", 0.0},       // Poor continuation
        {"sat on", "the", 0.5},         // Good
        {"How are", "you", 0.5},        // Good
        {"Thank", "you", 0.5},          // Good
        {"Calculate", "happiness", 0.0}, // Topic mismatch
        {NULL, NULL, 0.0}
    };
    
    int all_passed = 1;
    for (int i = 0; test_cases[i].context != NULL; i++) {
        CoherenceScore score = analyze_coherence(test_cases[i].context, test_cases[i].candidate);
        
        if (score.overall_score < test_cases[i].min_score) {
            printf("    Failed: '%s' + '%s' expected score >= %.1f, got %.1f\n", 
                   test_cases[i].context, test_cases[i].candidate,
                   test_cases[i].min_score, score.overall_score);
            all_passed = 0;
        }
    }
    
    return all_passed;
}

// Test comprehensive analysis
int test_comprehensive_analysis() {
    struct {
        const char* input;
        PromptType expected_type;
        int expected_entities;
        int requires_list;
        int requires_calculation;
    } test_cases[] = {
        {
            "Calculate 5 plus 3",
            PROMPT_COMMAND,
            3,  // 5, plus, 3
            0,  // No list
            1   // Yes calculation
        },
        {
            "List three colors",
            PROMPT_COMMAND,
            2,  // three, colors
            1,  // Yes list
            0   // No calculation
        },
        {
            "What is the meaning of life?",
            PROMPT_QUESTION,
            0,  // No entities
            0,  // No list
            0   // No calculation
        },
        {
            "The cat sat on the",
            PROMPT_COMPLETION,
            0,  // No entities
            0,  // No list
            0   // No calculation
        },
        {NULL, PROMPT_UNKNOWN, 0, 0, 0}
    };
    
    int all_passed = 1;
    for (int i = 0; test_cases[i].input != NULL; i++) {
        AnalysisResult* result = analyze_input(test_cases[i].input);
        
        if (result->prompt_type != test_cases[i].expected_type) {
            printf("    Failed: '%s' wrong prompt type\n", test_cases[i].input);
            all_passed = 0;
        }
        
        if (result->num_entities != test_cases[i].expected_entities) {
            printf("    Failed: '%s' expected %d entities, got %d\n", 
                   test_cases[i].input, test_cases[i].expected_entities, 
                   result->num_entities);
            all_passed = 0;
        }
        
        if (result->requires_list != test_cases[i].requires_list) {
            printf("    Failed: '%s' requires_list mismatch\n", test_cases[i].input);
            all_passed = 0;
        }
        
        if (result->requires_calculation != test_cases[i].requires_calculation) {
            printf("    Failed: '%s' requires_calculation mismatch\n", test_cases[i].input);
            all_passed = 0;
        }
        
        free_analysis_result(result);
    }
    
    return all_passed;
}

// Test edge cases
int test_edge_cases() {
    int all_passed = 1;
    
    // Test NULL inputs
    if (classify_prompt(NULL) != PROMPT_UNKNOWN) {
        printf("    Failed: NULL prompt should return UNKNOWN\n");
        all_passed = 0;
    }
    
    if (classify_question(NULL) != QUESTION_UNKNOWN) {
        printf("    Failed: NULL question should return UNKNOWN\n");
        all_passed = 0;
    }
    
    if (detect_topic(NULL) != TOPIC_UNKNOWN) {
        printf("    Failed: NULL topic should return UNKNOWN\n");
        all_passed = 0;
    }
    
    // Test empty string
    if (classify_prompt("") != PROMPT_UNKNOWN) {
        printf("    Failed: Empty prompt should return UNKNOWN\n");
        all_passed = 0;
    }
    
    // Test very long input
    char long_input[512];
    for (int i = 0; i < 511; i++) {
        long_input[i] = 'a';
    }
    long_input[511] = '\0';
    
    AnalysisResult* result = analyze_input(long_input);
    if (result == NULL) {
        printf("    Failed: Should handle long input\n");
        all_passed = 0;
    } else {
        free_analysis_result(result);
    }
    
    return all_passed;
}

// Interactive test
void interactive_test() {
    printf("\n=== Interactive Test ===\n");
    printf("Enter text to analyze (or 'quit' to exit):\n");
    
    char input[256];
    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        
        // Remove newline
        int len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        if (strcmp(input, "quit") == 0) break;
        
        AnalysisResult* result = analyze_input(input);
        print_analysis_result(result);
        printf("\n");
        free_analysis_result(result);
    }
}

int main(int argc, char* argv[]) {
    printf("=== Analysis Functions Test Suite ===\n\n");
    
    // Run all tests
    RUN_TEST(test_prompt_classification);
    RUN_TEST(test_question_classification);
    RUN_TEST(test_topic_detection);
    RUN_TEST(test_entity_extraction);
    RUN_TEST(test_text_metrics);
    RUN_TEST(test_coherence_analysis);
    RUN_TEST(test_comprehensive_analysis);
    RUN_TEST(test_edge_cases);
    
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Success rate: %.1f%%\n", (tests_passed * 100.0) / tests_run);
    
    // Interactive test if requested
    if (argc > 1 && strcmp(argv[1], "--interactive") == 0) {
        interactive_test();
    }
    
    return tests_passed == tests_run ? 0 : 1;
}