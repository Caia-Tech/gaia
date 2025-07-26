/**
 * Learning functions for GAIA V9 Simple
 * Extract and store knowledge from conversations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "gaia_simple.h"

// Learn a user-specific fact
void learn_fact(const char* input, MemoryBank* memory) {
    if (memory->user_fact_count >= 10) return;
    
    // Store the whole statement about the user
    strncpy(memory->user_facts[memory->user_fact_count], input, MAX_STRING - 1);
    memory->user_fact_count++;
}

// Extract fact from "X is Y" pattern
int extract_fact_pattern(const char* input, char* subject, char* fact) {
    // Look for " is " pattern
    const char* is_pos = strstr(input, " is ");
    if (!is_pos) return 0;
    
    // Extract subject (before "is")
    int subject_len = is_pos - input;
    if (subject_len <= 0 || subject_len >= 128) return 0;
    
    strncpy(subject, input, subject_len);
    subject[subject_len] = '\0';
    
    // Skip " is "
    const char* fact_start = is_pos + 4;
    
    // Extract fact (after "is")
    strncpy(fact, fact_start, 127);
    fact[127] = '\0';
    
    // Clean up whitespace
    while (*subject == ' ') memmove(subject, subject + 1, strlen(subject));
    while (*fact == ' ') memmove(fact, fact + 1, strlen(fact));
    
    // Remove trailing punctuation
    int fact_len = strlen(fact);
    if (fact_len > 0 && (fact[fact_len-1] == '.' || fact[fact_len-1] == '!')) {
        fact[fact_len-1] = '\0';
    }
    
    return 1;
}

// Simple word extraction for better pattern matching
void extract_keywords(const char* input, char keywords[][50], int* count) {
    char temp[MAX_STRING];
    strncpy(temp, input, MAX_STRING - 1);
    
    *count = 0;
    char* token = strtok(temp, " .,?!");
    
    while (token && *count < 10) {
        // Skip common words
        if (strcmp(token, "the") != 0 && 
            strcmp(token, "is") != 0 &&
            strcmp(token, "a") != 0 &&
            strcmp(token, "an") != 0 &&
            strlen(token) > 2) {
            
            strcpy(keywords[*count], token);
            (*count)++;
        }
        token = strtok(NULL, " .,?!");
    }
}

// Score pattern similarity (simple version)
int pattern_similarity(const char* pattern1, const char* pattern2) {
    char keywords1[10][50], keywords2[10][50];
    int count1, count2;
    
    extract_keywords(pattern1, keywords1, &count1);
    extract_keywords(pattern2, keywords2, &count2);
    
    int matches = 0;
    for (int i = 0; i < count1; i++) {
        for (int j = 0; j < count2; j++) {
            if (strcasecmp(keywords1[i], keywords2[j]) == 0) {
                matches++;
            }
        }
    }
    
    return matches;
}