/**
 * Memory management for GAIA V9 Simple
 * Handles persistence and retrieval of learned information
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gaia_simple.h"

#define MAX_RESPONSE 512

// Initialize memory bank
void init_memory(MemoryBank* memory) {
    if (!memory) return;
    
    memory->pattern_count = 0;
    memory->fact_count = 0;
    memory->user_fact_count = 0;
    
    // Clear arrays
    memset(memory->patterns, 0, sizeof(memory->patterns));
    memset(memory->facts, 0, sizeof(memory->facts));
    memset(memory->user_facts, 0, sizeof(memory->user_facts));
}

// Load memory from file
void load_memory(MemoryBank* memory, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Starting with fresh memory.\n");
        return;
    }
    
    // Read counts
    fread(&memory->pattern_count, sizeof(int), 1, file);
    fread(&memory->fact_count, sizeof(int), 1, file);
    fread(&memory->user_fact_count, sizeof(int), 1, file);
    
    // Read data
    fread(memory->patterns, sizeof(Pattern), memory->pattern_count, file);
    fread(memory->facts, sizeof(Fact), memory->fact_count, file);
    fread(memory->user_facts, sizeof(memory->user_facts[0]), memory->user_fact_count, file);
    
    fclose(file);
    
    printf("Loaded %d patterns and %d facts from memory.\n", 
           memory->pattern_count, memory->fact_count);
}

// Save memory to file
void save_memory(MemoryBank* memory, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Warning: Could not save memory.\n");
        return;
    }
    
    // Write counts
    fwrite(&memory->pattern_count, sizeof(int), 1, file);
    fwrite(&memory->fact_count, sizeof(int), 1, file);
    fwrite(&memory->user_fact_count, sizeof(int), 1, file);
    
    // Write data
    fwrite(memory->patterns, sizeof(Pattern), memory->pattern_count, file);
    fwrite(memory->facts, sizeof(Fact), memory->fact_count, file);
    fwrite(memory->user_facts, sizeof(memory->user_facts[0]), memory->user_fact_count, file);
    
    fclose(file);
    
    printf("Saved %d patterns and %d facts to memory.\n", 
           memory->pattern_count, memory->fact_count);
}

// Cleanup (currently just a placeholder)
void cleanup_memory(MemoryBank* memory) {
    // Nothing to free with static allocation
    // This is here for future dynamic memory needs
}

// Store a fact
void store_fact(const char* subject, const char* fact, MemoryBank* memory) {
    if (memory->fact_count >= MAX_FACTS) {
        // Overwrite oldest fact
        memory->fact_count = MAX_FACTS - 1;
    }
    
    Fact* new_fact = &memory->facts[memory->fact_count];
    strncpy(new_fact->subject, subject, MAX_STRING - 1);
    strncpy(new_fact->fact, fact, MAX_STRING - 1);
    new_fact->learned_at = time(NULL);
    
    memory->fact_count++;
}

// Find a relevant fact
char* find_relevant_fact(const char* input, MemoryBank* memory) {
    static char response[MAX_RESPONSE];
    
    // Simple keyword matching for now
    for (int i = 0; i < memory->fact_count; i++) {
        if (strstr(input, memory->facts[i].subject)) {
            sprintf(response, "%s is %s", 
                    memory->facts[i].subject, 
                    memory->facts[i].fact);
            return response;
        }
    }
    
    // Check user facts
    for (int i = 0; i < memory->user_fact_count; i++) {
        if (strstr(input, "favorite") || strstr(input, "my")) {
            if (strstr(memory->user_facts[i], input)) {
                return memory->user_facts[i];
            }
        }
    }
    
    return NULL;
}

// Find similar patterns
char* find_similar_pattern(const char* input, MemoryBank* memory) {
    // Simple implementation - exact match for now
    // TODO: Add fuzzy matching
    for (int i = memory->pattern_count - 1; i >= 0; i--) {
        if (strstr(input, memory->patterns[i].input) ||
            strstr(memory->patterns[i].input, input)) {
            memory->patterns[i].use_count++;
            return memory->patterns[i].response;
        }
    }
    
    return NULL;
}