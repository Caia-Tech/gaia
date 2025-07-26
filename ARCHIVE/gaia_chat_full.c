#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include "gate_types.h"

#define HASH_SIZE 65536
#define MAX_WORD_LENGTH 50
#define MAX_INPUT_LENGTH 1024

// Pattern structure
typedef struct Pattern {
    char word1[MAX_WORD_LENGTH];
    char word2[MAX_WORD_LENGTH];
    char next[MAX_WORD_LENGTH];
    int count;
    Gate* gate;
    struct Pattern* collision_next;
} Pattern;

// Chat system
typedef struct {
    Pattern* patterns[HASH_SIZE];
    int total_patterns;
    int total_words;
} ChatSystem;

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
ChatSystem* create_chat_system() {
    return calloc(1, sizeof(ChatSystem));
}

// Learn pattern
void learn_pattern(ChatSystem* sys, const char* w1, const char* w2, const char* next) {
    uint32_t addr = compute_pattern_address(w1, w2);
    Pattern* p = sys->patterns[addr];
    
    // Find existing pattern
    while (p) {
        if (strcmp(p->word1, w1) == 0 && 
            strcmp(p->word2, w2) == 0 &&
            strcmp(p->next, next) == 0) {
            p->count++;
            // Increment count to strengthen this pattern
            // Gate activation logic could go here
            return;
        }
        if (p->collision_next == NULL) break;
        p = p->collision_next;
    }
    
    // Create new pattern
    Pattern* new_p = malloc(sizeof(Pattern));
    strncpy(new_p->word1, w1, MAX_WORD_LENGTH-1);
    strncpy(new_p->word2, w2, MAX_WORD_LENGTH-1);
    strncpy(new_p->next, next, MAX_WORD_LENGTH-1);
    new_p->count = 1;
    new_p->gate = gate_create("THRESHOLD");
    new_p->collision_next = NULL;
    
    if (p) {
        p->collision_next = new_p;
    } else {
        sys->patterns[addr] = new_p;
    }
    sys->total_patterns++;
}

// Process text
void process_text(ChatSystem* sys, const char* text) {
    char words[100][MAX_WORD_LENGTH] = {0};
    int word_count = 0;
    
    // Tokenize
    char* copy = strdup(text);
    char* token = strtok(copy, " \t\n,.!?;:");
    
    while (token && word_count < 100) {
        // Convert to lowercase
        for (int i = 0; token[i]; i++) {
            token[i] = tolower(token[i]);
        }
        strncpy(words[word_count++], token, MAX_WORD_LENGTH-1);
        token = strtok(NULL, " \t\n,.!?;:");
    }
    
    // Learn trigrams
    for (int i = 0; i < word_count - 2; i++) {
        learn_pattern(sys, words[i], words[i+1], words[i+2]);
    }
    
    sys->total_words += word_count;
    free(copy);
}

// Train from file
void train_from_file(ChatSystem* sys, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Warning: Could not open %s\n", filename);
        return;
    }
    
    printf("Training from %s...\n", filename);
    char line[1024];
    int lines = 0;
    
    while (fgets(line, sizeof(line), f)) {
        process_text(sys, line);
        lines++;
        if (lines % 10 == 0) {
            printf("\rProcessed %d lines, %d patterns", lines, sys->total_patterns);
            fflush(stdout);
        }
    }
    
    printf("\nTraining complete: %d patterns\n", sys->total_patterns);
    fclose(f);
}

// Recursively train from all files in directory
void train_from_directory(ChatSystem* sys, const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return;
    
    struct dirent* entry;
    char full_path[512];
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                train_from_directory(sys, full_path);
            } else if (strstr(entry->d_name, ".txt")) {
                train_from_file(sys, full_path);
            }
        }
    }
    
    closedir(dir);
}

// Find best continuation
char* find_best_continuation(ChatSystem* sys, const char* w1, const char* w2) {
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
    
    return best ? best->next : NULL;
}

// Generate response
void generate_response(ChatSystem* sys, const char* input, char* output, int max_len) {
    // Tokenize input
    char words[100][MAX_WORD_LENGTH] = {0};
    int word_count = 0;
    
    char* copy = strdup(input);
    char* token = strtok(copy, " \t\n,.!?;:");
    
    while (token && word_count < 100) {
        for (int i = 0; token[i]; i++) {
            token[i] = tolower(token[i]);
        }
        strncpy(words[word_count++], token, MAX_WORD_LENGTH-1);
        token = strtok(NULL, " \t\n,.!?;:");
    }
    
    // Start response with last two words of input
    strcpy(output, "");
    char w1[MAX_WORD_LENGTH] = "";
    char w2[MAX_WORD_LENGTH] = "";
    
    if (word_count >= 2) {
        strcpy(w1, words[word_count-2]);
        strcpy(w2, words[word_count-1]);
    } else if (word_count == 1) {
        strcpy(w2, words[0]);
    }
    
    // Generate up to 20 words
    int generated = 0;
    while (generated < 20 && strlen(output) < max_len - MAX_WORD_LENGTH) {
        char* next = find_best_continuation(sys, w1, w2);
        if (!next) {
            // Try single word continuation
            next = find_best_continuation(sys, w2, "");
            if (!next) {
                // Try any continuation from w2
                for (int i = 0; i < HASH_SIZE; i++) {
                    Pattern* p = sys->patterns[i];
                    while (p) {
                        if (strcmp(p->word1, w2) == 0) {
                            next = p->next;
                            break;
                        }
                        p = p->collision_next;
                    }
                    if (next) break;
                }
            }
        }
        
        if (!next) break;
        
        if (strlen(output) > 0) strcat(output, " ");
        strcat(output, next);
        
        strcpy(w1, w2);
        strcpy(w2, next);
        generated++;
    }
    
    free(copy);
}

// Chat loop
void chat_loop(ChatSystem* sys) {
    char input[MAX_INPUT_LENGTH];
    char response[MAX_INPUT_LENGTH];
    
    printf("\n=== gaia Chat (Full Training) ===\n");
    printf("Type 'quit' to exit\n\n");
    
    while (1) {
        printf("You: ");
        if (!fgets(input, sizeof(input), stdin)) break;
        
        // Remove newline
        int len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        if (strcmp(input, "quit") == 0) break;
        
        if (strlen(input) > 0) {
            // Generate response
            generate_response(sys, input, response, MAX_INPUT_LENGTH);
            printf("gaia: %s\n", response);
            
            // Learn from interaction
            process_text(sys, input);
        }
    }
    
    printf("\nGoodbye!\n");
}

int main() {
    printf("gaia Chat System - Full Training Edition\n");
    printf("========================================\n\n");
    
    // Initialize gates
    gate_registry_init();
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    ChatSystem* sys = create_chat_system();
    
    // Train on ALL datasets
    printf("Training on all datasets...\n");
    train_from_directory(sys, "datasets");
    
    printf("\nTotal patterns learned: %d\n", sys->total_patterns);
    printf("Ready for chat!\n");
    
    // Start chat
    chat_loop(sys);
    
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
    return 0;
}