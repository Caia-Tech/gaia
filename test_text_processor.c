#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "gate_types.h"

#define MAX_WORD_LENGTH 50
#define MAX_SENTENCE_LENGTH 1000

// Test results
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} TestResults;

// Initialize test results
TestResults results = {0, 0, 0};

// Test macros
#define TEST_START(name) printf("\n[TEST] %s\n", name); results.total_tests++;
#define TEST_PASS() printf("  ✓ PASS\n"); results.passed_tests++;
#define TEST_FAIL(msg) printf("  ✗ FAIL: %s\n", msg); results.failed_tests++;

// Character to binary conversion
void char_to_binary(char c, uint8_t bits[8]) {
    for (int i = 0; i < 8; i++) {
        bits[i] = (c >> (7 - i)) & 1;
    }
}

// Binary to character conversion
char binary_to_char(uint8_t bits[8]) {
    char c = 0;
    for (int i = 0; i < 8; i++) {
        c = (c << 1) | bits[i];
    }
    return c;
}

// Test character encoding/decoding
void test_character_encoding() {
    TEST_START("Character Encoding/Decoding");
    
    // Test all printable ASCII characters
    int errors = 0;
    for (char c = 32; c < 127; c++) {
        uint8_t bits[8];
        char_to_binary(c, bits);
        char decoded = binary_to_char(bits);
        
        if (decoded != c) {
            printf("  Error: '%c' != '%c'\n", c, decoded);
            errors++;
        }
    }
    
    if (errors == 0) {
        TEST_PASS();
    } else {
        TEST_FAIL("Encoding/decoding errors found");
    }
}

// Test word boundary detection
void test_word_boundaries() {
    TEST_START("Word Boundary Detection");
    
    const char* test_cases[] = {
        " ", ".", "!", "?", ",", ";", ":", "\t", "\n"
    };
    
    int errors = 0;
    for (int i = 0; i < 9; i++) {
        uint8_t bits[8];
        char_to_binary(test_cases[i][0], bits);
        
        // Check if it's a boundary character
        char c = binary_to_char(bits);
        if (c != ' ' && c != '.' && c != '!' && c != '?' && 
            c != ',' && c != ';' && c != ':' && c != '\t' && c != '\n') {
            printf("  Boundary not detected: '%c'\n", c);
            errors++;
        }
    }
    
    if (errors == 0) {
        TEST_PASS();
    } else {
        TEST_FAIL("Boundary detection errors");
    }
}

// Test gate creation for characters
void test_gate_creation() {
    TEST_START("Gate Creation for Characters");
    
    // Create gates for 'A'
    uint8_t a_bits[8];
    char_to_binary('A', a_bits);
    
    Gate* gates[8];
    int errors = 0;
    
    for (int i = 0; i < 8; i++) {
        if (a_bits[i] == 1) {
            gates[i] = gate_create("ONE");
        } else {
            gates[i] = gate_create("ZERO");
        }
        
        if (gates[i] == NULL) {
            errors++;
        }
    }
    
    // Verify gates
    if (errors == 0) {
        // Check if we can evaluate them
        for (int i = 0; i < 8; i++) {
            uint8_t output = gates[i]->type->evaluate(gates[i], NULL, 0);
            if (output != a_bits[i]) {
                printf("  Gate %d output mismatch\n", i);
                errors++;
            }
        }
    }
    
    // Cleanup
    for (int i = 0; i < 8; i++) {
        if (gates[i]) gate_destroy(gates[i]);
    }
    
    if (errors == 0) {
        TEST_PASS();
    } else {
        TEST_FAIL("Gate creation/evaluation errors");
    }
}

// Test word accumulation
void test_word_accumulation() {
    TEST_START("Word Accumulation");
    
    const char* word = "Hello";
    Gate* word_gates[MAX_WORD_LENGTH * 8];
    int gate_count = 0;
    
    // Convert word to gates
    for (int i = 0; i < strlen(word); i++) {
        uint8_t bits[8];
        char_to_binary(word[i], bits);
        
        for (int j = 0; j < 8; j++) {
            if (bits[j]) {
                word_gates[gate_count++] = gate_create("ONE");
            } else {
                word_gates[gate_count++] = gate_create("ZERO");
            }
        }
    }
    
    // Reconstruct word from gates
    char reconstructed[MAX_WORD_LENGTH] = {0};
    int char_idx = 0;
    
    for (int i = 0; i < gate_count; i += 8) {
        uint8_t bits[8];
        for (int j = 0; j < 8 && (i + j) < gate_count; j++) {
            bits[j] = word_gates[i + j]->type->evaluate(word_gates[i + j], NULL, 0);
        }
        reconstructed[char_idx++] = binary_to_char(bits);
    }
    
    // Cleanup
    for (int i = 0; i < gate_count; i++) {
        gate_destroy(word_gates[i]);
    }
    
    if (strcmp(word, reconstructed) == 0) {
        TEST_PASS();
    } else {
        printf("  Expected: '%s', Got: '%s'\n", word, reconstructed);
        TEST_FAIL("Word reconstruction failed");
    }
}

// Test sentence processing
void test_sentence_processing() {
    TEST_START("Sentence Processing");
    
    const char* sentence = "Hello world!";
    int word_count = 0;
    char words[10][MAX_WORD_LENGTH] = {0};
    int current_word = 0;
    int char_in_word = 0;
    
    // Process sentence
    for (int i = 0; i < strlen(sentence); i++) {
        char c = sentence[i];
        
        if (c == ' ' || c == '!' || c == '.' || c == '?') {
            if (char_in_word > 0) {
                word_count++;
                current_word++;
                char_in_word = 0;
            }
        } else {
            words[current_word][char_in_word++] = c;
        }
    }
    
    // Handle last word
    if (char_in_word > 0) {
        word_count++;
    }
    
    if (word_count == 2 && strcmp(words[0], "Hello") == 0 && strcmp(words[1], "world") == 0) {
        TEST_PASS();
    } else {
        printf("  Word count: %d\n", word_count);
        printf("  Words: '%s', '%s'\n", words[0], words[1]);
        TEST_FAIL("Sentence parsing incorrect");
    }
}

// Test pattern matching setup
void test_pattern_setup() {
    TEST_START("Pattern Matching Setup");
    
    // Create pattern gate
    Gate* pattern = gate_create("PATTERN");
    
    if (pattern != NULL) {
        // Try to train it with "cat" pattern
        uint8_t cat_bits[24];
        char_to_binary('c', cat_bits);
        char_to_binary('a', cat_bits + 8);
        char_to_binary('t', cat_bits + 16);
        
        if (pattern->type->update) {
            pattern->type->update(pattern, cat_bits, 1);
            TEST_PASS();
        } else {
            TEST_FAIL("Pattern gate has no update function");
        }
        
        gate_destroy(pattern);
    } else {
        TEST_FAIL("Failed to create pattern gate");
    }
}

// Performance test
void test_performance() {
    TEST_START("Performance Benchmark");
    
    const int iterations = 10000;
    const char* test_string = "The quick brown fox jumps over the lazy dog";
    
    clock_t start = clock();
    
    for (int i = 0; i < iterations; i++) {
        // Convert each character to bits
        for (int j = 0; j < strlen(test_string); j++) {
            uint8_t bits[8];
            char_to_binary(test_string[j], bits);
        }
    }
    
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    double chars_per_second = (iterations * strlen(test_string)) / cpu_time;
    
    printf("  Processed %d iterations in %.3f seconds\n", iterations, cpu_time);
    printf("  Rate: %.0f characters/second\n", chars_per_second);
    
    if (chars_per_second > 1000000) {  // 1M chars/sec minimum
        TEST_PASS();
    } else {
        TEST_FAIL("Performance below threshold");
    }
}

// Test special characters
void test_special_characters() {
    TEST_START("Special Character Handling");
    
    const char* specials = "!@#$%^&*()_+-=[]{}|;':\",./<>?\n\t";
    int errors = 0;
    
    for (int i = 0; i < strlen(specials); i++) {
        uint8_t bits[8];
        char_to_binary(specials[i], bits);
        char decoded = binary_to_char(bits);
        
        if (decoded != specials[i]) {
            printf("  Failed on: '%c' (0x%02X)\n", specials[i], specials[i]);
            errors++;
        }
    }
    
    if (errors == 0) {
        TEST_PASS();
    } else {
        TEST_FAIL("Special character errors");
    }
}

// Main test runner
int main() {
    printf("=== gaia Text Processor Test Suite ===\n");
    printf("=====================================\n");
    
    // Initialize gate registry
    gate_registry_init();
    
    // External registration
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    // Run tests
    test_character_encoding();
    test_word_boundaries();
    test_gate_creation();
    test_word_accumulation();
    test_sentence_processing();
    test_pattern_setup();
    test_special_characters();
    test_performance();
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", results.total_tests);
    printf("Passed: %d\n", results.passed_tests);
    printf("Failed: %d\n", results.failed_tests);
    printf("Success rate: %.1f%%\n", 
           (results.passed_tests * 100.0) / results.total_tests);
    
    // Cleanup
    gate_registry_cleanup();
    
    return (results.failed_tests == 0) ? 0 : 1;
}