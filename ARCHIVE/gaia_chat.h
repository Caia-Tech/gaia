#ifndef GAIA_CHAT_H
#define GAIA_CHAT_H

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

// Chat system structure
typedef struct {
    Pattern* patterns[HASH_SIZE];
    int total_patterns;
    int total_words;
} ChatSystem;

// Function declarations
ChatSystem* create_chat_system(void);
void learn_pattern(ChatSystem* sys, const char* w1, const char* w2, const char* next);
void process_text(ChatSystem* sys, const char* text);
void train_from_file(ChatSystem* sys, const char* filename);
char* find_best_continuation(ChatSystem* sys, const char* w1, const char* w2);
void generate_response(ChatSystem* sys, const char* input);
void chat_loop(ChatSystem* sys);

#endif // GAIA_CHAT_H