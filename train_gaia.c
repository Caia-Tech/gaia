#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include "gate_types.h"

#define HASH_SIZE 65536
#define MAX_WORD_LENGTH 50

// Pattern structure
typedef struct Pattern {
    char word1[MAX_WORD_LENGTH];
    char word2[MAX_WORD_LENGTH];
    char next[MAX_WORD_LENGTH];
    int count;
    Gate* gate;
    struct Pattern* collision_next;
} Pattern;

// Training system
typedef struct {
    Pattern* patterns[HASH_SIZE];
    int total_patterns;
    int total_words;
    char prev_word[MAX_WORD_LENGTH];
    char prev_prev_word[MAX_WORD_LENGTH];
} TrainingSystem;

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

// Create training system
TrainingSystem* create_training_system() {
    return calloc(1, sizeof(TrainingSystem));
}

// Learn pattern
void learn_pattern(TrainingSystem* ts, const char* w1, const char* w2, const char* next) {
    uint32_t addr = compute_pattern_address(w1, w2);
    Pattern* p = ts->patterns[addr];
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
        ts->patterns[addr] = new_p;
    }
    ts->total_patterns++;
}

// Process word
void process_word(TrainingSystem* ts, const char* word) {
    if (strlen(ts->prev_prev_word) > 0 && strlen(ts->prev_word) > 0) {
        learn_pattern(ts, ts->prev_prev_word, ts->prev_word, word);
    }
    strcpy(ts->prev_prev_word, ts->prev_word);
    strcpy(ts->prev_word, word);
    ts->total_words++;
}

// Train from file
void train_from_file(TrainingSystem* ts, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Cannot open file: %s\n", filename);
        return;
    }
    
    char word[MAX_WORD_LENGTH];
    int word_pos = 0;
    int ch;
    
    printf("Training from %s...\n", filename);
    clock_t start = clock();
    
    while ((ch = fgetc(f)) != EOF) {
        ch = tolower(ch);
        
        if (isalnum(ch) || ch == '\'' || ch == '-') {
            if (word_pos < MAX_WORD_LENGTH - 1) {
                word[word_pos++] = ch;
            }
        } else if (word_pos > 0) {
            word[word_pos] = '\0';
            process_word(ts, word);
            word_pos = 0;
            
            if (ts->total_words % 1000 == 0) {
                printf("\rProcessed %d words, %d patterns", 
                       ts->total_words, ts->total_patterns);
                fflush(stdout);
            }
        }
    }
    
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
    
    fclose(f);
}

// Generate text
void generate_text(TrainingSystem* ts, const char* prompt, int max_words) {
    char temp[500];
    strcpy(temp, prompt);
    
    // Convert to lowercase
    for (int i = 0; temp[i]; i++) {
        temp[i] = tolower(temp[i]);
    }
    
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
    
    printf("\nPrompt: %s\n", prompt);
    printf("Response: ");
    
    if (strlen(w1) > 0) printf("%s ", w1);
    if (strlen(w2) > 0) printf("%s", w2);
    
    // Generate
    int words_generated = 0;
    for (int i = 0; i < max_words && words_generated < max_words; i++) {
        uint32_t addr = compute_pattern_address(w1, w2);
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
        words_generated++;
        
        strcpy(w1, w2);
        strcpy(w2, best->next);
    }
    printf("\n");
}

// Interactive mode
void interactive_mode(TrainingSystem* ts) {
    char input[256];
    
    printf("\n=== Interactive Mode ===\n");
    printf("Enter prompts (or 'quit' to exit):\n\n");
    
    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        
        // Remove newline
        int len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        if (strcmp(input, "quit") == 0) break;
        
        if (strlen(input) > 0) {
            generate_text(ts, input, 30);
        }
    }
}

int main(int argc, char** argv) {
    printf("gaia Training System\n");
    printf("===================\n\n");
    
    // Initialize gates
    gate_registry_init();
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    TrainingSystem* ts = create_training_system();
    
    // Train from high-quality corpus
    train_from_file(ts, "datasets/high_quality_corpus.txt");
    
    // Show some example generations
    printf("\n=== Example Generations ===\n");
    generate_text(ts, "logic gates", 20);
    generate_text(ts, "The fundamental", 20);
    generate_text(ts, "Pattern recognition", 20);
    generate_text(ts, "Machine learning", 20);
    generate_text(ts, "Binary computation", 20);
    
    // Interactive mode if requested
    if (argc > 1 && strcmp(argv[1], "-i") == 0) {
        interactive_mode(ts);
    }
    
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
    free(ts);
    
    gate_registry_cleanup();
    return 0;
}