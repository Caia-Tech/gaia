#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gate_types.h"

#define MAX_WORD_LENGTH 50

// Character to gate mapping
typedef struct {
    char character;
    uint8_t bits[8];  // Binary representation
    Gate* bit_gates[8];  // One gate per bit
} CharGate;

// Word structure
typedef struct {
    CharGate* chars[MAX_WORD_LENGTH];
    size_t length;
    Gate* end_detector;  // Detects word boundary
} WordGate;

// Text processor
typedef struct {
    CharGate char_map[256];  // Full ASCII
    WordGate* current_word;
    Gate* space_detector;
    Gate* period_detector;
    size_t char_count;
} TextProcessor;

// Initialize character mapping
void init_char_map(TextProcessor* tp) {
    printf("Initializing character mappings...\n");
    
    for (int i = 0; i < 256; i++) {
        tp->char_map[i].character = (char)i;
        
        // Convert character to binary
        for (int bit = 0; bit < 8; bit++) {
            tp->char_map[i].bits[bit] = (i >> (7 - bit)) & 1;
            
            // Create gate for this bit (ZERO or ONE)
            if (tp->char_map[i].bits[bit]) {
                tp->char_map[i].bit_gates[bit] = gate_create("ONE");
            } else {
                tp->char_map[i].bit_gates[bit] = gate_create("ZERO");
            }
        }
    }
    
    // Special detectors
    tp->space_detector = gate_create("PATTERN");
    tp->period_detector = gate_create("PATTERN");
}

// Convert character to gates
CharGate* char_to_gates(TextProcessor* tp, char c) {
    return &tp->char_map[(unsigned char)c];
}

// Print character as gates
void print_char_gates(CharGate* cg) {
    printf("'%c' = [", cg->character);
    for (int i = 0; i < 8; i++) {
        printf("%d", cg->bits[i]);
        if (i < 7) printf(",");
    }
    printf("]\n");
}

// Process a string into gates
void process_string(TextProcessor* tp, const char* text) {
    printf("\nProcessing: \"%s\"\n", text);
    printf("Converting to gate patterns:\n\n");
    
    size_t len = strlen(text);
    tp->char_count = 0;
    
    for (size_t i = 0; i < len; i++) {
        CharGate* cg = char_to_gates(tp, text[i]);
        print_char_gates(cg);
        tp->char_count++;
        
        // Check for word boundaries
        if (text[i] == ' ' || text[i] == '.' || text[i] == '?' || text[i] == '!') {
            printf("  [Word boundary detected]\n");
        }
    }
    
    printf("\nTotal gates used: %zu (8 per character)\n", tp->char_count * 8);
}

// Demonstrate pattern matching
void demo_pattern_matching(TextProcessor* tp) {
    printf("\n=== Pattern Matching Demo ===\n");
    
    // Create pattern for "the"
    Gate* the_pattern = gate_create("PATTERN");
    if (the_pattern && the_pattern->type->update) {
        // Train pattern for "the"
        uint8_t the_bits[24] = {
            0,1,1,1,0,1,0,0,  // 't'
            0,1,1,0,1,0,0,0,  // 'h'
            0,1,1,0,0,1,0,1   // 'e'
        };
        the_pattern->type->update(the_pattern, the_bits, 1);
    }
    
    printf("Trained pattern for 'the'\n");
    
    // Test matching
    const char* test_words[] = {"the", "and", "cat", "the"};
    for (int i = 0; i < 4; i++) {
        printf("\nChecking '%s': ", test_words[i]);
        
        // Convert word to bits
        uint8_t word_bits[24] = {0};
        for (int j = 0; j < 3 && test_words[i][j]; j++) {
            CharGate* cg = char_to_gates(tp, test_words[i][j]);
            for (int k = 0; k < 8; k++) {
                word_bits[j * 8 + k] = cg->bits[k];
            }
        }
        
        // Check pattern
        uint8_t match = 0;
        if (the_pattern && the_pattern->type->evaluate) {
            match = the_pattern->type->evaluate(the_pattern, word_bits, 24);
        }
        printf("%s", match ? "MATCH!" : "no match");
    }
}

// Simple word accumulator
void demo_word_accumulation(TextProcessor* tp) {
    printf("\n\n=== Word Accumulation Demo ===\n");
    
    const char* sentence = "Hello world";
    printf("Processing: \"%s\"\n\n", sentence);
    
    Gate* word_buffer[MAX_WORD_LENGTH * 8];  // Gates for current word
    int buffer_pos = 0;
    
    for (size_t i = 0; i < strlen(sentence); i++) {
        char c = sentence[i];
        CharGate* cg = char_to_gates(tp, c);
        
        if (c == ' ') {
            // Word boundary - process accumulated word
            printf("Word complete: ");
            for (int j = 0; j < buffer_pos; j += 8) {
                // Reconstruct character from gates
                uint8_t byte = 0;
                for (int k = 0; k < 8; k++) {
                    if (j + k < buffer_pos) {
                        // Get the actual bit value from the gate
                        uint8_t bit_value = 0;
                        if (strcmp(word_buffer[j + k]->type->name, "ONE") == 0) {
                            bit_value = 1;
                        }
                        byte = (byte << 1) | bit_value;
                    }
                }
                printf("%c", byte);
            }
            printf(" (%d gates)\n", buffer_pos);
            buffer_pos = 0;
        } else {
            // Add character to buffer
            for (int j = 0; j < 8; j++) {
                word_buffer[buffer_pos++] = cg->bit_gates[j];
            }
        }
    }
    
    // Last word
    if (buffer_pos > 0) {
        printf("Word complete: ");
        for (int j = 0; j < buffer_pos; j += 8) {
            uint8_t byte = 0;
            for (int k = 0; k < 8; k++) {
                if (j + k < buffer_pos) {
                    byte = (byte << 1) | word_buffer[j + k]->last_output;
                }
            }
            printf("%c", byte);
        }
        printf(" (%d gates)\n", buffer_pos);
    }
}

int main() {
    printf("gaia Text Processor v0.1\n");
    printf("========================\n\n");
    
    // Initialize gate registry
    gate_registry_init();
    
    // External registration functions
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    
    // Create text processor
    TextProcessor* tp = calloc(1, sizeof(TextProcessor));
    init_char_map(tp);
    
    // Demo 1: Character encoding
    printf("=== Character Encoding Demo ===\n");
    process_string(tp, "AI");
    
    // Demo 2: Longer text
    printf("\n=== Sentence Processing Demo ===\n");
    process_string(tp, "What is gaia?");
    
    // Demo 3: Pattern matching
    demo_pattern_matching(tp);
    
    // Demo 4: Word accumulation
    demo_word_accumulation(tp);
    
    // Cleanup
    printf("\n\nText processor demonstration complete.\n");
    printf("Key insights:\n");
    printf("- Each character = 8 binary gates\n");
    printf("- Words = sequences of character gates\n");
    printf("- Pattern matching = gate configuration comparison\n");
    printf("- No matrices or transformers needed!\n");
    
    // Cleanup gates
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 8; j++) {
            if (tp->char_map[i].bit_gates[j]) {
                gate_destroy(tp->char_map[i].bit_gates[j]);
            }
        }
    }
    
    free(tp);
    gate_registry_cleanup();
    
    return 0;
}