#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include "gaia_chat.h"

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

// Process text for learning
void process_text(ChatSystem* sys, const char* text) {
    char buffer[1024];
    strcpy(buffer, text);
    
    // Convert to lowercase
    for (int i = 0; buffer[i]; i++) {
        buffer[i] = tolower(buffer[i]);
    }
    
    // Tokenize
    char* words[100];
    int word_count = 0;
    
    char* token = strtok(buffer, " \t\n\r.,!?;:");
    while (token && word_count < 100) {
        words[word_count++] = token;
        token = strtok(NULL, " \t\n\r.,!?;:");
    }
    
    // Learn all trigrams
    for (int i = 0; i < word_count - 2; i++) {
        learn_pattern(sys, words[i], words[i+1], words[i+2]);
    }
    
    sys->total_words += word_count;
}

// Train from file
void train_from_file(ChatSystem* sys, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Cannot open file: %s\n", filename);
        return;
    }
    
    char line[1024];
    int lines = 0;
    
    printf("Training from %s...\n", filename);
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

// Find best continuation given context
char* find_best_continuation(ChatSystem* sys, const char* w1, const char* w2) {
    if (!w2 || strlen(w2) == 0) return NULL;
    
    // Try exact match first
    if (w1 && strlen(w1) > 0) {
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
        
        if (best) return best->next;
    }
    
    // Fallback: search all patterns where word2 matches either position
    Pattern* best = NULL;
    int max_count = 0;
    
    for (int i = 0; i < HASH_SIZE; i++) {
        Pattern* p = sys->patterns[i];
        while (p) {
            if (strcmp(p->word2, w2) == 0 || strcmp(p->word1, w2) == 0) {
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

// Generate response
void generate_response(ChatSystem* sys, const char* input) {
    char buffer[1024];
    strcpy(buffer, input);
    
    // Convert to lowercase
    for (int i = 0; buffer[i]; i++) {
        buffer[i] = tolower(buffer[i]);
    }
    
    // Extract last two words from input
    char* words[100];
    int word_count = 0;
    
    char* token = strtok(buffer, " \t\n\r.,!?;:");
    while (token && word_count < 100) {
        words[word_count++] = token;
        token = strtok(NULL, " \t\n\r.,!?;:");
    }
    
    if (word_count == 0) {
        printf("gaia: Hello! Type something to start our conversation.\n");
        return;
    }
    
    // Use last 1-2 words as context
    char w1[MAX_WORD_LENGTH] = "";
    char w2[MAX_WORD_LENGTH] = "";
    
    if (word_count >= 2) {
        strcpy(w1, words[word_count-2]);
        strcpy(w2, words[word_count-1]);
    } else {
        strcpy(w2, words[word_count-1]);
    }
    
    printf("gaia: ");
    
    // Generate response
    int generated = 0;
    int max_words = 25;
    
    for (int i = 0; i < max_words; i++) {
        char* next = find_best_continuation(sys, w1, w2);
        
        if (!next) {
            if (generated == 0) {
                // No patterns found - give helpful response
                if (strcmp(w2, "hi") == 0 || strcmp(w2, "hello") == 0) {
                    printf("Hello! I'm gaia, built with logic gates. How can I help you today?");
                } else if (strcmp(w2, "gaia") == 0) {
                    printf("Yes, I'm gaia - a text processing system using logic gates instead of neural networks.");
                } else {
                    printf("I don't have patterns for '%s' yet. Try asking about logic gates, patterns, or computation!", w2);
                }
            }
            break;
        }
        
        printf("%s ", next);
        generated++;
        
        // Shift context
        strcpy(w1, w2);
        strcpy(w2, next);
        
        // Stop at sentence end
        if (strstr(next, ".") || strstr(next, "!") || strstr(next, "?")) {
            break;
        }
    }
    
    printf("\n");
}

// Interactive chat
void chat_loop(ChatSystem* sys) {
    char input[1024];
    
    printf("\n=== gaia Chat ===\n");
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
            // Learn from user input (optional)
            process_text(sys, input);
            
            // Generate response
            generate_response(sys, input);
        }
    }
}