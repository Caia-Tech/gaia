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

// External function declarations
extern void extract_keywords(const char* input, char keywords[][50], int* count);
extern int pattern_similarity(const char* pattern1, const char* pattern2);

// Find a relevant fact
char* find_relevant_fact(const char* input, MemoryBank* memory) {
    static char response[MAX_RESPONSE];
    char input_keywords[10][50];
    int input_keyword_count;
    
    // Extract keywords from input for better matching
    extract_keywords(input, input_keywords, &input_keyword_count);
    
    // Check general facts
    for (int i = 0; i < memory->fact_count; i++) {
        // Check if input contains the subject or any keyword matches the subject
        int found = 0;
        
        // Direct substring match
        if (strstr(input, memory->facts[i].subject)) {
            found = 1;
        }
        
        // Keyword match
        if (!found) {
            for (int j = 0; j < input_keyword_count; j++) {
                if (strcasecmp(input_keywords[j], memory->facts[i].subject) == 0 ||
                    strstr(memory->facts[i].subject, input_keywords[j]) != NULL) {
                    found = 1;
                    break;
                }
            }
        }
        
        if (found) {
            sprintf(response, "%s is %s", 
                    memory->facts[i].subject, 
                    memory->facts[i].fact);
            return response;
        }
    }
    
    // Check user facts - improved matching
    for (int i = 0; i < memory->user_fact_count; i++) {
        // Extract keywords from stored user fact
        char fact_keywords[10][50];
        int fact_keyword_count;
        extract_keywords(memory->user_facts[i], fact_keywords, &fact_keyword_count);
        
        // Check if any keywords match
        int matches = 0;
        for (int j = 0; j < input_keyword_count; j++) {
            for (int k = 0; k < fact_keyword_count; k++) {
                if (strcasecmp(input_keywords[j], fact_keywords[k]) == 0) {
                    matches++;
                }
            }
            
            // Also check for "favorite", "my", etc.
            if (strstr(input, "favorite") || strstr(input, "my") || 
                strstr(input, "what") || strstr(input, "tell")) {
                if (strstr(memory->user_facts[i], "favorite") ||
                    strstr(memory->user_facts[i], "my")) {
                    matches += 2; // Give extra weight to user preference queries
                }
            }
        }
        
        // Return if we have enough keyword matches
        if (matches >= 2 || (matches >= 1 && input_keyword_count <= 3)) {
            return memory->user_facts[i];
        }
    }
    
    return NULL;
}

// Find similar patterns
char* find_similar_pattern(const char* input, MemoryBank* memory) {
    int best_similarity = 0;
    int best_index = -1;
    
    // Look for patterns with keyword similarity
    for (int i = memory->pattern_count - 1; i >= 0; i--) {
        // First check exact substring match (fastest)
        if (strstr(input, memory->patterns[i].input) ||
            strstr(memory->patterns[i].input, input)) {
            memory->patterns[i].use_count++;
            return memory->patterns[i].response;
        }
        
        // Then check pattern similarity
        int similarity = pattern_similarity(input, memory->patterns[i].input);
        
        // Prefer more recent patterns with same similarity
        if (similarity > best_similarity) {
            best_similarity = similarity;
            best_index = i;
        }
    }
    
    // Return best match if similarity is high enough
    // Require at least 2 matching keywords
    if (best_index >= 0 && best_similarity >= 2) {
        memory->patterns[best_index].use_count++;
        return memory->patterns[best_index].response;
    }
    
    return NULL;
}