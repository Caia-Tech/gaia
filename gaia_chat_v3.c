#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include "gate_types.h"

#define HASH_SIZE 2097152     // 2M buckets (8x larger for 100-token patterns)
#define MAX_WORD_LENGTH 50
#define MAX_INPUT_LENGTH 1024
#define CONTEXT_SIZE 100      // 100-token context window!
#define PAD_TOKEN "[PAD]"

// Enhanced pattern structure for 100-token context
typedef struct Pattern {
    char context[CONTEXT_SIZE][MAX_WORD_LENGTH];  // 100 words of context
    char next[MAX_WORD_LENGTH];                   // Next word
    int context_length;                           // Actual context length
    int count;
    Gate* gate;
    struct Pattern* collision_next;
} Pattern;

// Chat system with more tracking
typedef struct {
    Pattern* patterns[HASH_SIZE];
    int total_patterns;
    int total_words;
    int patterns_by_length[CONTEXT_SIZE + 1];  // Track patterns of each context length
    int hash_collisions;                       // Track collision performance
    int pattern_lookups;                       // Track lookup performance
} ChatSystem;

// Enhanced hash function for 100-token context
uint32_t compute_pattern_address(char context[][MAX_WORD_LENGTH], int context_length) {
    uint64_t hash = 5381;  // Use 64-bit for better distribution
    
    for (int i = 0; i < context_length; i++) {
        for (int j = 0; context[i][j]; j++) {
            hash = ((hash << 5) + hash) + context[i][j];
        }
        hash = ((hash << 5) + hash) + ' ';
    }
    
    return (uint32_t)(hash % HASH_SIZE);
}

// Create system
ChatSystem* create_chat_system() {
    ChatSystem* sys = calloc(1, sizeof(ChatSystem));
    printf("Allocated %.1f MB for hash table\n", 
           (HASH_SIZE * sizeof(Pattern*)) / (1024.0 * 1024.0));
    return sys;
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
    if (context_length < 2 || context_length > CONTEXT_SIZE) return;  // Minimum 2 tokens
    
    uint32_t addr = compute_pattern_address(context, context_length);
    Pattern* p = sys->patterns[addr];
    
    // Count collisions for performance tracking
    int collision_count = 0;
    
    // Find existing pattern
    while (p) {
        collision_count++;
        if (p->context_length == context_length &&
            contexts_match(p->context, context, context_length) &&
            strcmp(p->next, next) == 0) {
            p->count++;
            return;
        }
        if (p->collision_next == NULL) break;
        p = p->collision_next;
    }
    
    if (collision_count > 1) sys->hash_collisions++;
    
    // Create new pattern
    Pattern* new_p = malloc(sizeof(Pattern));
    
    // Copy context
    for (int i = 0; i < context_length; i++) {
        strncpy(new_p->context[i], context[i], MAX_WORD_LENGTH-1);
        new_p->context[i][MAX_WORD_LENGTH-1] = '\0';
    }
    // Pad remaining slots
    for (int i = context_length; i < CONTEXT_SIZE; i++) {
        strcpy(new_p->context[i], PAD_TOKEN);
    }
    
    strncpy(new_p->next, next, MAX_WORD_LENGTH-1);
    new_p->next[MAX_WORD_LENGTH-1] = '\0';
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

// Process text with sliding window up to 100 tokens
void process_text(ChatSystem* sys, const char* text) {
    char words[500][MAX_WORD_LENGTH] = {0};  // Larger buffer for longer texts
    int word_count = 0;
    
    // Tokenize
    char* copy = strdup(text);
    char* token = strtok(copy, " \t\n,.!?;:");
    
    while (token && word_count < 500) {
        // Convert to lowercase
        for (int i = 0; token[i]; i++) {
            token[i] = tolower(token[i]);
        }
        strncpy(words[word_count++], token, MAX_WORD_LENGTH-1);
        words[word_count-1][MAX_WORD_LENGTH-1] = '\0';
        token = strtok(NULL, " \t\n,.!?;:");
    }
    
    // Learn patterns with varying context sizes (2 to CONTEXT_SIZE)
    for (int i = 0; i < word_count; i++) {
        // Try multiple context lengths starting from minimum 2
        for (int ctx_len = 2; ctx_len <= CONTEXT_SIZE && i + ctx_len < word_count; ctx_len++) {
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
    char line[2048];  // Larger line buffer for 100-token contexts
    int lines = 0;
    
    while (fgets(line, sizeof(line), f)) {
        process_text(sys, line);
        lines++;
        if (lines % 5 == 0) {  // Less frequent updates for performance
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

// Multi-step lookahead structure
typedef struct {
    char word[MAX_WORD_LENGTH];
    int path_score;
    int found_continuations;
} WordCandidate;

// Find best continuation with multi-step lookahead
char* find_best_continuation(ChatSystem* sys, char context[][MAX_WORD_LENGTH], int context_length) {
    sys->pattern_lookups++;
    
    // Step 1: Find all possible next words
    WordCandidate candidates[100] = {0};  // Up to 100 candidates
    int num_candidates = 0;
    
    int min_context = 2;
    for (int try_len = context_length; try_len >= min_context && num_candidates < 100; try_len--) {
        uint32_t addr = compute_pattern_address(context + (context_length - try_len), try_len);
        Pattern* p = sys->patterns[addr];
        
        while (p && num_candidates < 100) {
            if (p->context_length == try_len &&
                contexts_match(p->context, context + (context_length - try_len), try_len)) {
                
                // Check if we already have this word as candidate
                int existing = -1;
                for (int i = 0; i < num_candidates; i++) {
                    if (strcmp(candidates[i].word, p->next) == 0) {
                        existing = i;
                        break;
                    }
                }
                
                int score = p->count * try_len;  // Prefer longer contexts
                if (existing >= 0) {
                    // Update existing candidate with better score
                    if (score > candidates[existing].path_score) {
                        candidates[existing].path_score = score;
                    }
                } else {
                    // Add new candidate
                    strncpy(candidates[num_candidates].word, p->next, MAX_WORD_LENGTH-1);
                    candidates[num_candidates].path_score = score;
                    candidates[num_candidates].found_continuations = 0;
                    num_candidates++;
                }
            }
            p = p->collision_next;
        }
    }
    
    if (num_candidates == 0) return NULL;
    
    // Step 2: For each candidate, check if it has good continuations (lookahead)
    for (int i = 0; i < num_candidates; i++) {
        // Build new context with this candidate word
        char new_context[CONTEXT_SIZE][MAX_WORD_LENGTH];
        int new_length = context_length;
        
        // Copy current context and add candidate
        for (int j = 0; j < context_length - 1; j++) {
            strcpy(new_context[j], context[j + 1]);
        }
        strcpy(new_context[context_length - 1], candidates[i].word);
        if (new_length < CONTEXT_SIZE) new_length++;
        
        // Count how many continuations this candidate has
        for (int try_len = new_length; try_len >= min_context; try_len--) {
            uint32_t addr = compute_pattern_address(new_context + (new_length - try_len), try_len);
            Pattern* p = sys->patterns[addr];
            
            while (p) {
                if (p->context_length == try_len &&
                    contexts_match(p->context, new_context + (new_length - try_len), try_len)) {
                    candidates[i].found_continuations++;
                }
                p = p->collision_next;
            }
        }
        
        // Boost score for words that have continuations (lookahead bonus)
        if (candidates[i].found_continuations > 0) {
            candidates[i].path_score += candidates[i].found_continuations * 10;
        }
    }
    
    // Step 3: Pick best candidate based on combined score
    int best_idx = 0;
    for (int i = 1; i < num_candidates; i++) {
        if (candidates[i].path_score > candidates[best_idx].path_score) {
            best_idx = i;
        }
    }
    
    // Debug disabled for testing
    // static int debug_count = 0;
    
    // Return the word from the best candidate
    static char result[MAX_WORD_LENGTH];
    strcpy(result, candidates[best_idx].word);
    return result;
}

// Generate response with 100-token context
void generate_response(ChatSystem* sys, const char* input, char* output, int max_len) {
    // Tokenize input
    char words[200][MAX_WORD_LENGTH] = {0};
    int word_count = 0;
    
    char* copy = strdup(input);
    char* token = strtok(copy, " \t\n,.!?;:");
    
    while (token && word_count < 200) {
        for (int i = 0; token[i]; i++) {
            token[i] = tolower(token[i]);
        }
        strncpy(words[word_count++], token, MAX_WORD_LENGTH-1);
        words[word_count-1][MAX_WORD_LENGTH-1] = '\0';
        token = strtok(NULL, " \t\n,.!?;:");
    }
    
    // Build initial context from input (up to 100 tokens)
    char context[CONTEXT_SIZE][MAX_WORD_LENGTH];
    int context_length = 0;
    
    // Fill context with input words
    int start_idx = word_count > CONTEXT_SIZE ? word_count - CONTEXT_SIZE : 0;
    for (int i = start_idx; i < word_count && context_length < CONTEXT_SIZE; i++) {
        strcpy(context[context_length], words[i]);
        context_length++;
    }
    
    // Generate response (up to 50 words)
    strcpy(output, "");
    int generated = 0;
    
    while (generated < 50 && strlen(output) < max_len - MAX_WORD_LENGTH) {
        char* next = find_best_continuation(sys, context, context_length);
        
        if (!next) {
            // No continuation found with minimum context
            break;
        }
        
        if (strlen(output) > 0) strcat(output, " ");
        strcat(output, next);
        
        // Shift context window less aggressively - keep more original context
        if (context_length < CONTEXT_SIZE) {
            // If we have room, just add the word
            strcpy(context[context_length], next);
            context_length++;
        } else {
            // Only shift if we're at max capacity - shift by half, not one
            int shift_amount = context_length / 2;
            for (int i = 0; i < context_length - shift_amount; i++) {
                strcpy(context[i], context[i + shift_amount]);
            }
            strcpy(context[context_length - shift_amount], next);
            context_length = context_length - shift_amount + 1;
        }
        
        generated++;
        
        // Debug: show why generation stops
        if (generated == 1) {
            // Check if we can continue from this new context
            char* next_check = find_best_continuation(sys, context, context_length);
            if (!next_check) {
                // printf("DEBUG: No continuation after first word\n");
            }
        }
    }
    
    free(copy);
}

// Chat loop
void chat_loop(ChatSystem* sys) {
    char input[MAX_INPUT_LENGTH];
    char response[2048];  // Larger response buffer
    
    printf("\n=== GAIA Chat V3 (100-token context) ===\n");
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
            generate_response(sys, input, response, sizeof(response));
            printf("gaia: %s\n", response);
            
            // Learn from interaction
            process_text(sys, input);
        }
    }
    
    printf("\nGoodbye!\n");
}

// Print comprehensive statistics
void print_stats(ChatSystem* sys) {
    printf("\n=== GAIA V3 Pattern Statistics ===\n");
    printf("Total patterns: %d\n", sys->total_patterns);
    printf("Hash table size: %d buckets\n", HASH_SIZE);
    printf("Hash collisions: %d\n", sys->hash_collisions);
    printf("Pattern lookups: %d\n", sys->pattern_lookups);
    
    printf("\nPatterns by context length:\n");
    int total_meaningful = 0;
    for (int i = 3; i <= CONTEXT_SIZE; i++) {
        if (sys->patterns_by_length[i] > 0) {
            printf("  %d-token contexts: %d\n", i, sys->patterns_by_length[i]);
            total_meaningful += sys->patterns_by_length[i];
        }
    }
    
    // Calculate memory usage
    size_t pattern_memory = sys->total_patterns * sizeof(Pattern);
    size_t hash_memory = HASH_SIZE * sizeof(Pattern*);
    size_t total_memory = pattern_memory + hash_memory;
    
    printf("\nMemory usage:\n");
    printf("  Patterns: %.1f MB\n", pattern_memory / (1024.0 * 1024.0));
    printf("  Hash table: %.1f MB\n", hash_memory / (1024.0 * 1024.0));
    printf("  Total: %.1f MB\n", total_memory / (1024.0 * 1024.0));
    
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
    
    printf("\nHash table performance:\n");
    printf("  Utilization: %.1f%%\n", (used_buckets * 100.0) / HASH_SIZE);
    printf("  Longest chain: %d\n", max_chain);
    printf("  Avg patterns per bucket: %.1f\n", sys->total_patterns / (float)used_buckets);
}

int main() {
    printf("GAIA Chat System V3 - 100-Token Context Edition\n");
    printf("===============================================\n\n");
    
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
    printf("\nReady for chat with 100-token context!\n");
    
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