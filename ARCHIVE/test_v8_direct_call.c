#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Directly test the V8 functions

// Declare the types we need
typedef struct Pattern {
    char context[100][50];
    char next[50];
    int context_length;
    int count;
    struct Pattern* collision_next;
} Pattern;

typedef struct {
    Pattern* patterns[2097152];
    int total_patterns;
    int total_words;
    int patterns_by_length[101];
    int hash_collisions;
    int pattern_lookups;
} ChatSystem;

// External functions
extern void function_registry_init(void);
extern void register_gaia_functions(void);
extern void init_experiment_logger(void);
extern ChatSystem* init_chat_system(void);
extern void generate_response_v8(ChatSystem* system, const char* input);
extern void function_registry_cleanup(void);
extern void cleanup_experiment_logger(void);

int main() {
    printf("=== Direct V8 Test ===\n");
    
    // Initialize
    function_registry_init();
    register_gaia_functions();
    init_experiment_logger();
    
    ChatSystem* system = init_chat_system();
    if (!system) {
        printf("Failed to create system\n");
        return 1;
    }
    
    // Test queries
    const char* queries[] = {
        "What is 2 plus 2?",
        "What is factorial of 5?",
        "Is 17 prime?",
        "Explain addition",
        NULL
    };
    
    for (int i = 0; queries[i]; i++) {
        printf("\nQuery %d: %s\n", i + 1, queries[i]);
        printf("Response: ");
        fflush(stdout);
        
        generate_response_v8(system, queries[i]);
    }
    
    // Cleanup
    function_registry_cleanup();
    cleanup_experiment_logger();
    
    printf("\nTest completed\n");
    return 0;
}