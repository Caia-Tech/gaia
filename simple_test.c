#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "gaia_chat.h"

// Test basic functionality
void test_basic_responses(ChatSystem* sys) {
    printf("\n=== Basic Response Tests ===\n");
    
    struct {
        char* input;
        char* description;
    } tests[] = {
        {"hi", "Greeting"},
        {"hello", "Alternative greeting"},
        {"how are you", "Status inquiry"},
        {"what is two plus two", "Math question"},
        {"thank you", "Gratitude"},
        {"what is your name", "Identity"},
        {"logic gates", "Technical topic"},
        {"goodbye", "Farewell"}
    };
    
    for (int i = 0; i < 8; i++) {
        printf("\nTest: %s\n", tests[i].description);
        printf("Input: %s\n", tests[i].input);
        printf("Response: ");
        generate_response(sys, tests[i].input);
    }
}

// Test pattern learning
void test_pattern_learning(ChatSystem* sys) {
    printf("\n=== Pattern Learning Test ===\n");
    
    // Teach a new pattern
    printf("Teaching: 'gaia rocks' -> 'absolutely'\n");
    process_text(sys, "gaia rocks absolutely");
    process_text(sys, "gaia rocks absolutely right");
    process_text(sys, "yes gaia rocks absolutely");
    
    // Test if learned
    printf("Testing learned pattern...\n");
    printf("Input: gaia rocks\n");
    printf("Response: ");
    generate_response(sys, "gaia rocks");
}

int main() {
    printf("gaia Simple Test Program\n");
    printf("=======================\n");
    
    // Initialize
    gate_registry_init();
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    ChatSystem* sys = create_chat_system();
    
    // Train on datasets
    printf("\nTraining on datasets...\n");
    train_from_file(sys, "datasets/conversational_corpus.txt");
    train_from_file(sys, "datasets/dialogue_patterns.txt");
    train_from_file(sys, "datasets/math/basic_arithmetic.txt");
    train_from_file(sys, "datasets/conversation/dialogue_structure.txt");
    
    printf("\nTotal patterns learned: %d\n", sys->total_patterns);
    
    // Run tests
    test_basic_responses(sys);
    test_pattern_learning(sys);
    
    // Cleanup
    for (int i = 0; i < HASH_SIZE; i++) {
        Pattern* p = sys->patterns[i];
        while (p) {
            Pattern* next = p->collision_next;
            if (p->gate) gate_destroy(p->gate);
            free(p);
            p = next;
        }
    }
    free(sys);
    
    gate_registry_cleanup();
    printf("\n\nTest complete!\n");
    return 0;
}