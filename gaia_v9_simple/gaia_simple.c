/**
 * GAIA V9 Simple - Back to Basics
 * A genuinely useful AI assistant in <1000 lines
 * 
 * Philosophy: Less architecture, more intelligence
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "gaia_simple.h"

#define MAX_INPUT 256
#define MAX_RESPONSE 512

// Global knowledge base (simple for now)
static MemoryBank memory;

int main() {
    printf("GAIA V9 Simple - Learning AI Assistant\n");
    printf("I learn from our conversations. Type 'quit' to exit.\n\n");
    
    // Initialize systems
    init_memory(&memory);
    load_memory(&memory, "gaia_memory.dat");
    
    char input[MAX_INPUT];
    char response[MAX_RESPONSE];
    
    while (1) {
        printf("You: ");
        if (!fgets(input, MAX_INPUT, stdin)) break;
        
        // Clean input
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;
        
        // Check for quit
        if (strcmp(input, "quit") == 0) {
            printf("GAIA: Goodbye! I'll remember our conversation.\n");
            break;
        }
        
        // Process input and generate response
        process_input(input, response, &memory);
        printf("GAIA: %s\n", response);
        
        // Learn from this interaction
        learn_from_interaction(input, response, &memory);
    }
    
    // Save what we learned
    save_memory(&memory, "gaia_memory.dat");
    cleanup_memory(&memory);
    
    return 0;
}

// Process user input and generate response
void process_input(const char* input, char* response, MemoryBank* memory) {
    // First, check if it's a calculation
    double result;
    int calc_type = try_calculate(input, &result);
    if (calc_type == 1) {
        if (result == (int)result) {
            sprintf(response, "That equals %d", (int)result);
        } else {
            sprintf(response, "That equals %.2f", result);
        }
        return;
    } else if (calc_type == 2) {
        // Prime number check
        if (result == 1) {
            sprintf(response, "Yes, that's a prime number");
        } else {
            sprintf(response, "No, that's not a prime number");
        }
        return;
    }
    
    // Check for learning intents
    if (strstr(input, "remember that") || strstr(input, "my favorite")) {
        learn_fact(input, memory);
        strcpy(response, "I'll remember that.");
        return;
    }
    
    // Check for fact teaching (X is Y pattern)
    char subject[128], fact[128];
    if (extract_fact_pattern(input, subject, fact)) {
        store_fact(subject, fact, memory);
        sprintf(response, "I've learned that %s is %s.", subject, fact);
        return;
    }
    
    // Check for questions about stored facts
    char* answer = find_relevant_fact(input, memory);
    if (answer) {
        strcpy(response, answer);
        return;
    }
    
    // Check similar patterns from past conversations
    char* pattern_response = find_similar_pattern(input, memory);
    if (pattern_response) {
        strcpy(response, pattern_response);
        return;
    }
    
    // Default response - but try to be helpful
    if (strstr(input, "?")) {
        strcpy(response, "I don't know that yet, but I'm learning. Can you tell me?");
    } else {
        strcpy(response, "Tell me more about that.");
    }
}

// Learn from the interaction
void learn_from_interaction(const char* input, const char* response, MemoryBank* memory) {
    // Store this pattern for future use
    if (memory->pattern_count < MAX_PATTERNS) {
        strcpy(memory->patterns[memory->pattern_count].input, input);
        strcpy(memory->patterns[memory->pattern_count].response, response);
        memory->patterns[memory->pattern_count].use_count = 1;
        memory->pattern_count++;
    }
}