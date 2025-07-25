#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "gate_types.h"

#define MAX_WORD_LENGTH 50
#define MAX_PATTERNS 10000

// Simple pattern: "A B" -> "C"
typedef struct {
    char word1[MAX_WORD_LENGTH];
    char word2[MAX_WORD_LENGTH];
    char next[MAX_WORD_LENGTH];
    int count;
} Pattern;

typedef struct {
    Pattern patterns[MAX_PATTERNS];
    int pattern_count;
} SimpleProcessor;

SimpleProcessor* create_simple_processor() {
    return calloc(1, sizeof(SimpleProcessor));
}

void learn_pattern(SimpleProcessor* proc, const char* w1, const char* w2, const char* next) {
    // Check if exists
    for (int i = 0; i < proc->pattern_count; i++) {
        if (strcmp(proc->patterns[i].word1, w1) == 0 &&
            strcmp(proc->patterns[i].word2, w2) == 0 &&
            strcmp(proc->patterns[i].next, next) == 0) {
            proc->patterns[i].count++;
            return;
        }
    }
    
    // Add new
    if (proc->pattern_count < MAX_PATTERNS) {
        Pattern* p = &proc->patterns[proc->pattern_count++];
        strcpy(p->word1, w1);
        strcpy(p->word2, w2);
        strcpy(p->next, next);
        p->count = 1;
    }
}

void train_text(SimpleProcessor* proc, const char* text) {
    char temp[1000];
    strcpy(temp, text);
    
    // Lowercase
    for (int i = 0; i < strlen(temp); i++) {
        temp[i] = tolower(temp[i]);
    }
    
    // Extract words
    char words[100][MAX_WORD_LENGTH];
    int count = 0;
    
    char* token = strtok(temp, " .,!?;:");
    while (token && count < 100) {
        strcpy(words[count++], token);
        token = strtok(NULL, " .,!?;:");
    }
    
    // Learn patterns
    for (int i = 0; i < count - 2; i++) {
        learn_pattern(proc, words[i], words[i+1], words[i+2]);
    }
    
    printf("Trained on %d words, total patterns: %d\n", count, proc->pattern_count);
}

const char* find_next(SimpleProcessor* proc, const char* w1, const char* w2) {
    Pattern* best = NULL;
    int max_count = 0;
    
    for (int i = 0; i < proc->pattern_count; i++) {
        if (strcmp(proc->patterns[i].word1, w1) == 0 &&
            strcmp(proc->patterns[i].word2, w2) == 0) {
            if (proc->patterns[i].count > max_count) {
                max_count = proc->patterns[i].count;
                best = &proc->patterns[i];
            }
        }
    }
    
    return best ? best->next : NULL;
}

void generate_text(SimpleProcessor* proc, const char* prompt) {
    printf("\nPrompt: %s\n", prompt);
    
    char temp[500];
    strcpy(temp, prompt);
    for (int i = 0; i < strlen(temp); i++) {
        temp[i] = tolower(temp[i]);
    }
    
    char words[50][MAX_WORD_LENGTH];
    int count = 0;
    
    char* token = strtok(temp, " .,!?");
    while (token && count < 50) {
        strcpy(words[count++], token);
        token = strtok(NULL, " .,!?");
    }
    
    if (count < 2) {
        printf("Response: Need at least 2 words\n");
        return;
    }
    
    printf("Response: %s %s", words[count-2], words[count-1]);
    
    char w1[MAX_WORD_LENGTH], w2[MAX_WORD_LENGTH];
    strcpy(w1, words[count-2]);
    strcpy(w2, words[count-1]);
    
    // Generate
    for (int i = 0; i < 20; i++) {
        const char* next = find_next(proc, w1, w2);
        if (!next) break;
        
        printf(" %s", next);
        
        strcpy(w1, w2);
        strcpy(w2, next);
    }
    printf("\n");
}

void show_patterns_for(SimpleProcessor* proc, const char* word) {
    printf("\nPatterns containing '%s':\n", word);
    for (int i = 0; i < proc->pattern_count; i++) {
        if (strstr(proc->patterns[i].word1, word) ||
            strstr(proc->patterns[i].word2, word)) {
            printf("  '%s %s' -> '%s' (count: %d)\n",
                   proc->patterns[i].word1,
                   proc->patterns[i].word2,
                   proc->patterns[i].next,
                   proc->patterns[i].count);
        }
    }
}

int main() {
    printf("gaia Simple Coherent Text v3\n");
    printf("============================\n\n");
    
    SimpleProcessor* proc = create_simple_processor();
    
    // Training
    const char* corpus[] = {
        "gaia is a revolutionary system",
        "gaia is based on logic gates",
        "logic gates process binary patterns",
        "binary patterns represent information",
        "the system learns through adaptation",
        "gaia processes text without matrices"
    };
    
    printf("Training...\n");
    for (int i = 0; i < 6; i++) {
        train_text(proc, corpus[i]);
    }
    
    // Show what we learned
    show_patterns_for(proc, "gaia");
    show_patterns_for(proc, "logic");
    
    // Test generation
    printf("\n=== Generation Tests ===\n");
    generate_text(proc, "gaia is");
    generate_text(proc, "logic gates");
    generate_text(proc, "binary patterns");
    generate_text(proc, "the system");
    
    free(proc);
    return 0;
}