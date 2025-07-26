#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include "gate_types.h"

#define HASH_SIZE 65536  // 2^16 for fast modulo
#define MAX_WORD_LENGTH 50
#define STREAM_BUFFER_SIZE 4096

// Pattern stored at computed address
typedef struct Pattern {
    char word1[MAX_WORD_LENGTH];
    char word2[MAX_WORD_LENGTH];
    char next[MAX_WORD_LENGTH];
    int count;
    Gate* gate;
    struct Pattern* collision_next;  // Handle hash collisions
} Pattern;

// Training system using computed addresses
typedef struct {
    Pattern* patterns[HASH_SIZE];  // Hash table of patterns
    int total_patterns;
    int total_words;
    Gate* learning_rate_gate;  // Controls adaptation speed
    
    // Stream processing
    char buffer[STREAM_BUFFER_SIZE];
    int buffer_pos;
    char prev_word[MAX_WORD_LENGTH];
    char prev_prev_word[MAX_WORD_LENGTH];
} TrainingSystem;

// Compute pattern address
uint32_t compute_pattern_address(const char* w1, const char* w2) {
    uint32_t hash = 5381;
    
    // DJB2 hash
    for (int i = 0; w1[i]; i++) {
        hash = ((hash << 5) + hash) + w1[i];
    }
    hash = ((hash << 5) + hash) + ' ';  // separator
    for (int i = 0; w2[i]; i++) {
        hash = ((hash << 5) + hash) + w2[i];
    }
    
    return hash % HASH_SIZE;
}

// Create training system
TrainingSystem* create_training_system() {
    TrainingSystem* ts = calloc(1, sizeof(TrainingSystem));
    ts->learning_rate_gate = gate_create("THRESHOLD");
    return ts;
}

// Learn pattern - O(1) insertion
void learn_pattern_streaming(TrainingSystem* ts, const char* w1, const char* w2, const char* next) {
    uint32_t addr = compute_pattern_address(w1, w2);
    
    // Direct lookup at computed address
    Pattern* p = ts->patterns[addr];
    Pattern* prev = NULL;
    
    // Search collision chain
    while (p) {
        if (strcmp(p->word1, w1) == 0 && 
            strcmp(p->word2, w2) == 0 &&
            strcmp(p->next, next) == 0) {
            // Found - update count
            p->count++;
            return;
        }
        prev = p;
        p = p->collision_next;
    }
    
    // Not found - create new pattern at address
    Pattern* new_p = calloc(1, sizeof(Pattern));
    strcpy(new_p->word1, w1);
    strcpy(new_p->word2, w2);
    strcpy(new_p->next, next);
    new_p->count = 1;
    new_p->gate = gate_create("THRESHOLD");
    
    // Insert at head or in chain
    if (prev) {
        prev->collision_next = new_p;
    } else {
        ts->patterns[addr] = new_p;
    }
    
    ts->total_patterns++;
}

// Process word from stream
void process_word(TrainingSystem* ts, const char* word) {
    // Learn trigram pattern
    if (strlen(ts->prev_prev_word) > 0 && strlen(ts->prev_word) > 0) {
        learn_pattern_streaming(ts, ts->prev_prev_word, ts->prev_word, word);
    }
    
    // Shift words
    strcpy(ts->prev_prev_word, ts->prev_word);
    strcpy(ts->prev_word, word);
    ts->total_words++;
}

// Stream processing - handle any size input
void train_from_stream(TrainingSystem* ts, FILE* stream) {
    char word[MAX_WORD_LENGTH];
    int word_pos = 0;
    int ch;
    
    printf("Training from stream...\n");
    clock_t start = clock();
    
    while ((ch = fgetc(stream)) != EOF) {
        ch = tolower(ch);
        
        if (isalnum(ch) || ch == '\'' || ch == '-') {
            // Build word
            if (word_pos < MAX_WORD_LENGTH - 1) {
                word[word_pos++] = ch;
            }
        } else if (word_pos > 0) {
            // End of word
            word[word_pos] = '\0';
            process_word(ts, word);
            word_pos = 0;
            
            // Progress indicator
            if (ts->total_words % 10000 == 0) {
                printf("\rProcessed %d words, %d patterns", 
                       ts->total_words, ts->total_patterns);
                fflush(stdout);
            }
        }
    }
    
    // Process last word if any
    if (word_pos > 0) {
        word[word_pos] = '\0';
        process_word(ts, word);
    }
    
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("\n\nTraining complete:\n");
    printf("- Words processed: %d\n", ts->total_words);
    printf("- Patterns learned: %d\n", ts->total_patterns);
    printf("- Time: %.2f seconds\n", cpu_time);
    printf("- Rate: %.0f words/second\n", ts->total_words / cpu_time);
}

// Train from file
void train_from_file(TrainingSystem* ts, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Cannot open file: %s\n", filename);
        return;
    }
    
    train_from_stream(ts, f);
    fclose(f);
}

// Train from string (for testing)
void train_from_string(TrainingSystem* ts, const char* text) {
    FILE* stream = fmemopen((void*)text, strlen(text), "r");
    train_from_stream(ts, stream);
    fclose(stream);
}

// Generate using computed lookups
void generate_text(TrainingSystem* ts, const char* prompt, int max_words) {
    char temp[500];
    strcpy(temp, prompt);
    
    // Parse prompt
    char w1[MAX_WORD_LENGTH] = "";
    char w2[MAX_WORD_LENGTH] = "";
    
    char* token = strtok(temp, " ");
    char* prev_token = NULL;
    while (token) {
        if (prev_token) {
            strcpy(w1, w2);
            strcpy(w2, token);
        } else {
            strcpy(w2, token);
        }
        prev_token = token;
        token = strtok(NULL, " ");
    }
    
    printf("Generating from: '%s %s'\n", w1, w2);
    printf("Response: %s %s", w1, w2);
    
    // Generate
    for (int i = 0; i < max_words; i++) {
        // Compute address for lookup - O(1)
        uint32_t addr = compute_pattern_address(w1, w2);
        
        // Find best continuation
        Pattern* p = ts->patterns[addr];
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
        
        if (!best) break;
        
        printf(" %s", best->next);
        
        // Shift context
        strcpy(w1, w2);
        strcpy(w2, best->next);
    }
    printf("\n");
}

// Show memory usage
void show_stats(TrainingSystem* ts) {
    printf("\n=== Training System Stats ===\n");
    
    // Calculate memory usage
    size_t pattern_memory = ts->total_patterns * sizeof(Pattern);
    size_t table_memory = HASH_SIZE * sizeof(Pattern*);
    size_t total_memory = pattern_memory + table_memory + sizeof(TrainingSystem);
    
    printf("Memory usage:\n");
    printf("- Pattern storage: %.2f MB\n", pattern_memory / (1024.0 * 1024.0));
    printf("- Hash table: %.2f KB\n", table_memory / 1024.0);
    printf("- Total: %.2f MB\n", total_memory / (1024.0 * 1024.0));
    
    // Calculate hash efficiency
    int used_buckets = 0;
    int max_chain = 0;
    
    for (int i = 0; i < HASH_SIZE; i++) {
        if (ts->patterns[i]) {
            used_buckets++;
            int chain_len = 0;
            Pattern* p = ts->patterns[i];
            while (p) {
                chain_len++;
                p = p->collision_next;
            }
            if (chain_len > max_chain) max_chain = chain_len;
        }
    }
    
    printf("\nHash efficiency:\n");
    printf("- Buckets used: %d/%d (%.1f%%)\n", 
           used_buckets, HASH_SIZE, (used_buckets * 100.0) / HASH_SIZE);
    printf("- Max collision chain: %d\n", max_chain);
    printf("- Average chain length: %.2f\n", 
           (float)ts->total_patterns / used_buckets);
}

int main() {
    printf("gaia Text Training System\n");
    printf("=========================\n\n");
    
    // Initialize
    gate_registry_init();
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    TrainingSystem* ts = create_training_system();
    
    // Demo 1: Train from string
    printf("Demo 1: Training from text\n");
    const char* demo_text = 
        "Gaia is a revolutionary system. "
        "Gaia processes text through logic gates. "
        "Logic gates enable coherent generation. "
        "Coherent generation emerges from patterns. "
        "Patterns are learned incrementally. "
        "The system scales with data volume.";
    
    train_from_string(ts, demo_text);
    
    // Demo 2: Generate
    printf("\nDemo 2: Generation\n");
    generate_text(ts, "gaia is", 10);
    generate_text(ts, "logic gates", 10);
    generate_text(ts, "the system", 10);
    
    // Demo 3: Train from file (if available)
    printf("\nDemo 3: File training\n");
    printf("To train from file: train_from_file(ts, \"corpus.txt\")\n");
    
    // Show efficiency
    show_stats(ts);
    
    printf("\n=== Key Training Features ===\n");
    printf("1. O(1) pattern storage using computed addresses\n");
    printf("2. Streaming processing - handles any file size\n");
    printf("3. No in-memory dataset required\n");
    printf("4. Incremental learning as data arrives\n");
    printf("5. Each pattern has associated gate\n");
    printf("6. Memory efficient - only stores unique patterns\n");
    
    // Cleanup
    for (int i = 0; i < HASH_SIZE; i++) {
        Pattern* p = ts->patterns[i];
        while (p) {
            Pattern* next = p->collision_next;
            if (p->gate) gate_destroy(p->gate);
            free(p);
            p = next;
        }
    }
    if (ts->learning_rate_gate) gate_destroy(ts->learning_rate_gate);
    free(ts);
    
    gate_registry_cleanup();
    return 0;
}