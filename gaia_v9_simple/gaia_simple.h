#ifndef GAIA_SIMPLE_H
#define GAIA_SIMPLE_H

#include <time.h>

#define MAX_PATTERNS 1000
#define MAX_FACTS 500
#define MAX_STRING 256

// Simple pattern storage
typedef struct {
    char input[MAX_STRING];
    char response[MAX_STRING];
    int use_count;
} Pattern;

// Simple fact storage
typedef struct {
    char subject[MAX_STRING];
    char fact[MAX_STRING];
    time_t learned_at;
} Fact;

// Memory bank - our simple "brain"
typedef struct {
    Pattern patterns[MAX_PATTERNS];
    int pattern_count;
    
    Fact facts[MAX_FACTS];
    int fact_count;
    
    // User preferences
    char user_facts[10][MAX_STRING];
    int user_fact_count;
} MemoryBank;

// Core functions
void process_input(const char* input, char* response, MemoryBank* memory);
void learn_from_interaction(const char* input, const char* response, MemoryBank* memory);

// Memory management
void init_memory(MemoryBank* memory);
void load_memory(MemoryBank* memory, const char* filename);
void save_memory(MemoryBank* memory, const char* filename);
void cleanup_memory(MemoryBank* memory);

// Calculation functions
int try_calculate(const char* input, double* result);

// Learning functions
void learn_fact(const char* input, MemoryBank* memory);
int extract_fact_pattern(const char* input, char* subject, char* fact);
void store_fact(const char* subject, const char* fact, MemoryBank* memory);
char* find_relevant_fact(const char* input, MemoryBank* memory);

// Pattern matching
char* find_similar_pattern(const char* input, MemoryBank* memory);

#endif // GAIA_SIMPLE_H