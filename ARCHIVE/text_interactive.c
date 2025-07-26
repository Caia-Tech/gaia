#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include "gate_types.h"

#define HASH_SIZE 65536
#define MAX_WORD_LENGTH 50
#define MAX_INPUT_LENGTH 1024

// Pattern stored at computed address
typedef struct Pattern {
    char word1[MAX_WORD_LENGTH];
    char word2[MAX_WORD_LENGTH];
    char next[MAX_WORD_LENGTH];
    int count;
    Gate* gate;
    struct Pattern* collision_next;
} Pattern;

// Interactive text system
typedef struct {
    Pattern* patterns[HASH_SIZE];
    int total_patterns;
    int total_words;
    char context_word1[MAX_WORD_LENGTH];
    char context_word2[MAX_WORD_LENGTH];
} InteractiveSystem;

// Hash function
uint32_t compute_pattern_address(const char* w1, const char* w2) {
    uint32_t hash = 5381;
    for (int i = 0; w1[i]; i++) {
        hash = ((hash << 5) + hash) + w1[i];
    }
    hash = ((hash << 5) + hash) + ' ';
    for (int i = 0; w2[i]; i++) {
        hash = ((hash << 5) + hash) + w2[i];
    }
    return hash % HASH_SIZE;
}

// Create system
InteractiveSystem* create_interactive_system() {
    return calloc(1, sizeof(InteractiveSystem));
}

// Learn pattern
void learn_pattern(InteractiveSystem* sys, const char* w1, const char* w2, const char* next) {
    uint32_t addr = compute_pattern_address(w1, w2);
    Pattern* p = sys->patterns[addr];
    Pattern* prev = NULL;
    
    while (p) {
        if (strcmp(p->word1, w1) == 0 && 
            strcmp(p->word2, w2) == 0 &&
            strcmp(p->next, next) == 0) {
            p->count++;
            return;
        }
        prev = p;
        p = p->collision_next;
    }
    
    Pattern* new_p = calloc(1, sizeof(Pattern));
    strcpy(new_p->word1, w1);
    strcpy(new_p->word2, w2);
    strcpy(new_p->next, next);
    new_p->count = 1;
    new_p->gate = gate_create("THRESHOLD");
    
    if (prev) {
        prev->collision_next = new_p;
    } else {
        sys->patterns[addr] = new_p;
    }
    sys->total_patterns++;
}

// Process input text
void process_input(InteractiveSystem* sys, const char* input) {
    char buffer[MAX_INPUT_LENGTH];
    strcpy(buffer, input);
    
    // Convert to lowercase
    for (int i = 0; buffer[i]; i++) {
        buffer[i] = tolower(buffer[i]);
    }
    
    // Tokenize and learn
    char* words[100];
    int word_count = 0;
    
    char* token = strtok(buffer, " \t\n\r.,!?;:");
    while (token && word_count < 100) {
        words[word_count++] = token;
        token = strtok(NULL, " \t\n\r.,!?;:");
    }
    
    // Learn trigrams
    for (int i = 0; i < word_count - 2; i++) {
        learn_pattern(sys, words[i], words[i+1], words[i+2]);
    }
    
    // Update context from last words
    if (word_count >= 2) {
        strcpy(sys->context_word1, words[word_count-2]);
        strcpy(sys->context_word2, words[word_count-1]);
    } else if (word_count == 1) {
        strcpy(sys->context_word1, sys->context_word2);
        strcpy(sys->context_word2, words[0]);
    }
    
    sys->total_words += word_count;
}

// Generate response
void generate_response(InteractiveSystem* sys, int max_words) {
    char w1[MAX_WORD_LENGTH], w2[MAX_WORD_LENGTH];
    strcpy(w1, sys->context_word1);
    strcpy(w2, sys->context_word2);
    
    printf("\ngaia: ");
    
    // If no context, provide a starter response
    if (strlen(w1) == 0 || strlen(w2) == 0) {
        printf("I need more context to generate a response. Try typing a longer sentence!\n");
        return;
    }
    
    for (int i = 0; i < max_words; i++) {
        uint32_t addr = compute_pattern_address(w1, w2);
        Pattern* p = sys->patterns[addr];
        Pattern* best = NULL;
        int max_count = 0;
        
        while (p) {
            if (strcmp(p->word1, w1) == 0 && strcmp(p->word2, w2) == 0) {
                if (p->count > max_count) {
                    max_count = p->count;
                    best = p;
                }
            }
            p = p->collision_next;
        }
        
        if (!best) {
            // No pattern found - try single word context
            if (i == 0) {
                printf("I don't have enough patterns yet. Keep talking to teach me!\n");
            }
            break;
        }
        
        printf("%s ", best->next);
        strcpy(w1, w2);
        strcpy(w2, best->next);
    }
    
    printf("\n");
    
    // Update context
    strcpy(sys->context_word1, w1);
    strcpy(sys->context_word2, w2);
}

// Show stats
void show_stats(InteractiveSystem* sys) {
    printf("\n=== System Status ===\n");
    printf("Patterns learned: %d\n", sys->total_patterns);
    printf("Words processed: %d\n", sys->total_words);
    printf("Current context: '%s %s'\n", sys->context_word1, sys->context_word2);
}

// Train from file
void train_from_file(InteractiveSystem* sys, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Cannot open file: %s\n", filename);
        return;
    }
    
    char line[MAX_INPUT_LENGTH];
    int lines = 0;
    
    printf("Training from %s...\n", filename);
    while (fgets(line, sizeof(line), f)) {
        process_input(sys, line);
        lines++;
        if (lines % 100 == 0) {
            printf("\rProcessed %d lines, %d patterns", lines, sys->total_patterns);
            fflush(stdout);
        }
    }
    
    printf("\nTraining complete: %d patterns from %d lines\n", sys->total_patterns, lines);
    fclose(f);
}

// Interactive loop
void interactive_loop(InteractiveSystem* sys) {
    char input[MAX_INPUT_LENGTH];
    
    printf("\n=== gaia Interactive Mode ===\n");
    printf("Commands:\n");
    printf("  /train <filename>  - Train from file\n");
    printf("  /stats            - Show statistics\n");
    printf("  /clear            - Clear patterns\n");
    printf("  /quit             - Exit\n");
    printf("  Any other text    - Learn and generate response\n\n");
    
    while (1) {
        printf("\nYou: ");
        if (!fgets(input, sizeof(input), stdin)) break;
        
        // Remove newline
        int len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        // Check commands
        if (strncmp(input, "/quit", 5) == 0) {
            break;
        } else if (strncmp(input, "/stats", 6) == 0) {
            show_stats(sys);
        } else if (strncmp(input, "/clear", 6) == 0) {
            // Clear all patterns
            for (int i = 0; i < HASH_SIZE; i++) {
                Pattern* p = sys->patterns[i];
                while (p) {
                    Pattern* next = p->collision_next;
                    if (p->gate) gate_destroy(p->gate);
                    free(p);
                    p = next;
                }
                sys->patterns[i] = NULL;
            }
            sys->total_patterns = 0;
            sys->total_words = 0;
            printf("Patterns cleared.\n");
        } else if (strncmp(input, "/train ", 7) == 0) {
            train_from_file(sys, input + 7);
        } else if (strlen(input) > 0) {
            // Process input and generate response
            int patterns_before = sys->total_patterns;
            process_input(sys, input);
            int new_patterns = sys->total_patterns - patterns_before;
            
            if (new_patterns > 0) {
                printf("[Learned %d new patterns]\n", new_patterns);
            }
            
            generate_response(sys, 20);
        }
    }
}

int main() {
    printf("gaia Interactive Text Processor\n");
    printf("==============================\n");
    
    // Initialize gates
    gate_registry_init();
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    InteractiveSystem* sys = create_interactive_system();
    
    // Check if corpus file exists and train on it
    FILE* corpus = fopen("datasets/high_quality_corpus.txt", "r");
    if (corpus) {
        printf("Training on high-quality corpus...\n");
        char line[MAX_INPUT_LENGTH];
        int lines = 0;
        
        while (fgets(line, sizeof(line), corpus)) {
            process_input(sys, line);
            lines++;
            if (lines % 10 == 0) {
                printf("\rProcessed %d lines, %d patterns", lines, sys->total_patterns);
                fflush(stdout);
            }
        }
        fclose(corpus);
        printf("\n");
    } else {
        // Fall back to seed texts if no corpus
        const char* seed_texts[] = {
            "Hello! I am gaia, a text processing system built with logic gates.",
            "I learn patterns from our conversation and generate responses.",
            "Logic gates process binary patterns to create coherent text.",
            "Each word activates specific gate configurations.",
            "The more we talk, the better I understand context.",
            "How are you doing today? I'm learning from our conversation.",
            "Gaia is a revolutionary system that processes text through gates.",
            "I am here to demonstrate pattern-based text generation.",
            "Ask me anything and I will learn from your questions.",
            "Hello there! Nice to meet you. How can I help?",
            NULL
        };
        
        for (int i = 0; seed_texts[i]; i++) {
            process_input(sys, seed_texts[i]);
        }
    }
    
    printf("System initialized with %d patterns.\n", sys->total_patterns);
    
    // Start interactive loop
    interactive_loop(sys);
    
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
    printf("\nGoodbye!\n");
    return 0;
}