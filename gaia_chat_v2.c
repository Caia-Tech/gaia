#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include "gate_types.h"

#define HASH_SIZE 262144  // Increased 4x for more patterns
#define MAX_WORD_LENGTH 50
#define MAX_INPUT_LENGTH 1024
#define CONTEXT_SIZE 10   // Our new context window!
#define PAD_TOKEN "[PAD]"

// Enhanced pattern structure for 10-token context
typedef struct Pattern {
    char context[CONTEXT_SIZE][MAX_WORD_LENGTH];  // 10 words of context
    char next[MAX_WORD_LENGTH];                   // Next word
    int context_length;                            // Actual context length (for shorter sequences)
    int count;
    Gate* gate;
    struct Pattern* collision_next;
} Pattern;

// Chat system remains similar
typedef struct {
    Pattern* patterns[HASH_SIZE];
    int total_patterns;
    int total_words;
    int patterns_by_length[CONTEXT_SIZE + 1];  // Track patterns of each context length
} ChatSystem;

// Enhanced hash function for multi-word context
uint32_t compute_pattern_address(char context[][MAX_WORD_LENGTH], int context_length) {
    uint32_t hash = 5381;
    
    for (int i = 0; i < context_length; i++) {
        for (int j = 0; context[i][j]; j++) {
            hash = ((hash << 5) + hash) + context[i][j];
        }
        hash = ((hash << 5) + hash) + ' ';
    }
    
    return hash % HASH_SIZE;
}

// Create system
ChatSystem* create_chat_system() {
    return calloc(1, sizeof(ChatSystem));
}

// Check if two contexts match
int contexts_match(char ctx1[][MAX_WORD_LENGTH], char ctx2[][MAX_WORD_LENGTH], int len) {
    for (int i = 0; i < len; i++) {
        if (strcmp(ctx1[i], ctx2[i]) != 0) {
            return 0;
        }
    }
    return 1;
}

// Learn pattern with variable-length context
void learn_pattern(ChatSystem* sys, char context[][MAX_WORD_LENGTH], int context_length, const char* next) {
    if (context_length < 1 || context_length > CONTEXT_SIZE) return;
    
    uint32_t addr = compute_pattern_address(context, context_length);
    Pattern* p = sys->patterns[addr];
    
    // Find existing pattern
    while (p) {
        if (p->context_length == context_length &&
            contexts_match(p->context, context, context_length) &&
            strcmp(p->next, next) == 0) {
            p->count++;
            return;
        }
        if (p->collision_next == NULL) break;
        p = p->collision_next;
    }
    
    // Create new pattern
    Pattern* new_p = malloc(sizeof(Pattern));
    
    // Copy context
    for (int i = 0; i < context_length; i++) {
        strncpy(new_p->context[i], context[i], MAX_WORD_LENGTH-1);
    }
    // Pad remaining slots
    for (int i = context_length; i < CONTEXT_SIZE; i++) {
        strcpy(new_p->context[i], PAD_TOKEN);
    }
    
    strncpy(new_p->next, next, MAX_WORD_LENGTH-1);
    new_p->context_length = context_length;
    new_p->count = 1;
    new_p->gate = gate_create("THRESHOLD");
    new_p->collision_next = NULL;
    
    if (p) {
        p->collision_next = new_p;
    } else {
        sys->patterns[addr] = new_p;
    }
    
    sys->total_patterns++;
    sys->patterns_by_length[context_length]++;
}

// Process text with sliding window
void process_text(ChatSystem* sys, const char* text) {
    char words[200][MAX_WORD_LENGTH] = {0};  // Increased buffer
    int word_count = 0;
    
    // Tokenize
    char* copy = strdup(text);
    char* token = strtok(copy, " \t\n,.!?;:");
    
    while (token && word_count < 200) {
        // Convert to lowercase
        for (int i = 0; token[i]; i++) {
            token[i] = tolower(token[i]);
        }
        strncpy(words[word_count++], token, MAX_WORD_LENGTH-1);
        token = strtok(NULL, " \t\n,.!?;:");
    }
    
    // Learn patterns with varying context sizes (minimum 3 tokens)
    for (int i = 0; i < word_count; i++) {
        // Try different context lengths (from 3 to CONTEXT_SIZE)
        for (int ctx_len = 3; ctx_len <= CONTEXT_SIZE && i + ctx_len < word_count; ctx_len++) {
            char context[CONTEXT_SIZE][MAX_WORD_LENGTH];
            
            // Build context
            for (int j = 0; j < ctx_len; j++) {
                strcpy(context[j], words[i + j]);
            }
            
            // Learn pattern
            if (i + ctx_len < word_count) {
                learn_pattern(sys, context, ctx_len, words[i + ctx_len]);
            }
        }
    }
    
    sys->total_words += word_count;
    free(copy);
}

// Train from file (same as before)
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

// Recursively train from directory
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

// Find best continuation with fallback
char* find_best_continuation(ChatSystem* sys, char context[][MAX_WORD_LENGTH], int context_length) {
    Pattern* best = NULL;
    int max_count = 0;
    
    // Try exact context length first, but don't go below 3 tokens
    int min_context = 3;  // Minimum meaningful context
    for (int try_len = context_length; try_len >= min_context && !best; try_len--) {
        uint32_t addr = compute_pattern_address(context + (context_length - try_len), try_len);
        Pattern* p = sys->patterns[addr];
        
        while (p) {
            if (p->context_length == try_len &&
                contexts_match(p->context, context + (context_length - try_len), try_len)) {
                if (p->count > max_count) {
                    max_count = p->count;
                    best = p;
                }
            }
            p = p->collision_next;
        }
    }
    
    return best ? best->next : NULL;
}

// Generate response with better context handling
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
    
    // Build initial context from input
    char context[CONTEXT_SIZE][MAX_WORD_LENGTH];
    int context_length = 0;
    
    // Fill context with last words of input
    int start_idx = word_count > CONTEXT_SIZE ? word_count - CONTEXT_SIZE : 0;
    for (int i = 0; i < word_count && context_length < CONTEXT_SIZE; i++) {
        strcpy(context[context_length], words[start_idx + i]);
        context_length++;
    }
    
    // Generate response
    strcpy(output, "");
    int generated = 0;
    
    while (generated < 30 && strlen(output) < max_len - MAX_WORD_LENGTH) {  // Generate more words
        char* next = find_best_continuation(sys, context, context_length);
        
        if (!next) {
            // No continuation found with minimum 3-token context
            break;
        }
        
        if (strlen(output) > 0) strcat(output, " ");
        strcat(output, next);
        
        // Shift context window
        for (int i = 0; i < CONTEXT_SIZE - 1; i++) {
            strcpy(context[i], context[i + 1]);
        }
        strcpy(context[CONTEXT_SIZE - 1], next);
        if (context_length < CONTEXT_SIZE) context_length++;
        
        generated++;
    }
    
    free(copy);
}

// Chat loop
void chat_loop(ChatSystem* sys) {
    char input[MAX_INPUT_LENGTH];
    char response[MAX_INPUT_LENGTH];
    
    printf("\n=== GAIA Chat V2 (10-token context) ===\n");
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

// Print statistics
void print_stats(ChatSystem* sys) {
    printf("\n=== Pattern Statistics ===\n");
    printf("Total patterns: %d\n", sys->total_patterns);
    printf("Patterns by context length:\n");
    for (int i = 3; i <= CONTEXT_SIZE; i++) {
        printf("  %d-token contexts: %d\n", i, sys->patterns_by_length[i]);
    }
    printf("Hash table size: %d\n", HASH_SIZE);
    
    // Calculate hash table usage
    int used_buckets = 0;
    int max_chain = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        if (sys->patterns[i]) {
            used_buckets++;
            int chain_len = 0;
            Pattern* p = sys->patterns[i];
            while (p) {
                chain_len++;
                p = p->collision_next;
            }
            if (chain_len > max_chain) max_chain = chain_len;
        }
    }
    
    printf("Hash table utilization: %.1f%%\n", (used_buckets * 100.0) / HASH_SIZE);
    printf("Longest collision chain: %d\n", max_chain);
}

int main() {
    printf("GAIA Chat System V2 - 10-Token Context Edition\n");
    printf("==============================================\n\n");
    
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
    printf("Training on all datasets with %d-token context window...\n", CONTEXT_SIZE);
    train_from_directory(sys, "datasets");
    
    print_stats(sys);
    printf("\nReady for chat!\n");
    
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