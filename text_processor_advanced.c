#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "gate_types.h"

#define MAX_WORD_LENGTH 50
#define MAX_WORDS 1000
#define MAX_PATTERNS 100

// Word pattern structure
typedef struct {
    char word[MAX_WORD_LENGTH];
    Gate* pattern_gate;
    int frequency;
} WordPattern;

// Question pattern structure
typedef struct {
    char* question_pattern;
    char* response_template;
    Gate* detector;
} QuestionPattern;

// Advanced text processor
typedef struct {
    WordPattern word_patterns[MAX_PATTERNS];
    int pattern_count;
    QuestionPattern questions[10];
    int question_count;
    Gate* word_boundary_detector;
} AdvancedProcessor;

// Initialize processor
AdvancedProcessor* create_processor() {
    AdvancedProcessor* proc = calloc(1, sizeof(AdvancedProcessor));
    
    // Create boundary detector
    proc->word_boundary_detector = gate_create("PATTERN");
    
    // Add common question patterns
    proc->questions[0].question_pattern = "what is";
    proc->questions[0].response_template = "%s is a %s";
    proc->questions[0].detector = gate_create("PATTERN");
    
    proc->questions[1].question_pattern = "how do";
    proc->questions[1].response_template = "To %s, you need to %s";
    proc->questions[1].detector = gate_create("PATTERN");
    
    proc->question_count = 2;
    
    return proc;
}

// Convert string to gate pattern
void string_to_gates(const char* str, Gate* gates[], int* gate_count) {
    *gate_count = 0;
    for (int i = 0; i < strlen(str); i++) {
        for (int bit = 0; bit < 8; bit++) {
            uint8_t bit_value = (str[i] >> (7 - bit)) & 1;
            gates[(*gate_count)++] = gate_create(bit_value ? "ONE" : "ZERO");
        }
    }
}

// Learn word pattern
void learn_word(AdvancedProcessor* proc, const char* word) {
    // Check if already exists
    for (int i = 0; i < proc->pattern_count; i++) {
        if (strcmp(proc->word_patterns[i].word, word) == 0) {
            proc->word_patterns[i].frequency++;
            return;
        }
    }
    
    // Add new pattern
    if (proc->pattern_count < MAX_PATTERNS) {
        strcpy(proc->word_patterns[proc->pattern_count].word, word);
        proc->word_patterns[proc->pattern_count].pattern_gate = gate_create("PATTERN");
        proc->word_patterns[proc->pattern_count].frequency = 1;
        
        // Train the pattern
        if (proc->word_patterns[proc->pattern_count].pattern_gate->type->update) {
            uint8_t bits[MAX_WORD_LENGTH * 8];
            int bit_count = 0;
            
            for (int i = 0; i < strlen(word); i++) {
                for (int j = 0; j < 8; j++) {
                    bits[bit_count++] = (word[i] >> (7 - j)) & 1;
                }
            }
            
            proc->word_patterns[proc->pattern_count].pattern_gate->type->update(
                proc->word_patterns[proc->pattern_count].pattern_gate,
                bits, 1
            );
        }
        
        proc->pattern_count++;
        printf("  [LEARNED] New word: '%s' (pattern %d)\n", word, proc->pattern_count);
    }
}

// Process input text
void process_text(AdvancedProcessor* proc, const char* input) {
    printf("\n=== Processing: \"%s\" ===\n", input);
    
    char word[MAX_WORD_LENGTH];
    int word_pos = 0;
    int word_count = 0;
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    
    // Tokenize and learn
    for (int i = 0; i <= strlen(input); i++) {
        char c = tolower(input[i]);
        
        if (c == ' ' || c == '.' || c == '?' || c == '!' || c == '\0') {
            if (word_pos > 0) {
                word[word_pos] = '\0';
                strcpy(words[word_count++], word);
                learn_word(proc, word);
                word_pos = 0;
            }
        } else if (isalnum(c)) {
            word[word_pos++] = c;
        }
    }
    
    printf("\nTokens found: %d\n", word_count);
    printf("Words: ");
    for (int i = 0; i < word_count; i++) {
        printf("'%s' ", words[i]);
    }
    printf("\n");
    
    // Check for question patterns
    for (int i = 0; i < proc->question_count; i++) {
        int found = 0;
        for (int j = 0; j < word_count - 1; j++) {
            char phrase[100];
            sprintf(phrase, "%s %s", words[j], words[j+1]);
            if (strcmp(phrase, proc->questions[i].question_pattern) == 0) {
                found = 1;
                printf("\n[PATTERN DETECTED] Question type: %s\n", 
                       proc->questions[i].question_pattern);
                
                // Generate response
                if (j + 2 < word_count) {
                    printf("[RESPONSE] ");
                    if (strcmp(words[j+2], "gaia") == 0) {
                        printf("gaia is a logic gate based AI system\n");
                    } else {
                        printf("%s is something I'm still learning about\n", words[j+2]);
                    }
                }
                break;
            }
        }
    }
}

// Show learned patterns
void show_patterns(AdvancedProcessor* proc) {
    printf("\n=== Learned Word Patterns ===\n");
    printf("Total unique words: %d\n", proc->pattern_count);
    
    // Sort by frequency
    for (int i = 0; i < proc->pattern_count - 1; i++) {
        for (int j = i + 1; j < proc->pattern_count; j++) {
            if (proc->word_patterns[j].frequency > proc->word_patterns[i].frequency) {
                WordPattern temp = proc->word_patterns[i];
                proc->word_patterns[i] = proc->word_patterns[j];
                proc->word_patterns[j] = temp;
            }
        }
    }
    
    printf("\nTop words by frequency:\n");
    for (int i = 0; i < proc->pattern_count && i < 10; i++) {
        printf("  '%s': %d times\n", 
               proc->word_patterns[i].word,
               proc->word_patterns[i].frequency);
    }
}

// Interactive demo
void interactive_demo(AdvancedProcessor* proc) {
    printf("\n=== Interactive Mode ===\n");
    printf("Type sentences to process (or 'quit' to exit):\n\n");
    
    char input[1000];
    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) break;
        if (strcmp(input, "show patterns") == 0) {
            show_patterns(proc);
            continue;
        }
        
        process_text(proc, input);
    }
}

int main() {
    printf("gaia Advanced Text Processor\n");
    printf("============================\n\n");
    
    // Initialize
    gate_registry_init();
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    // Create processor
    AdvancedProcessor* proc = create_processor();
    
    // Demo with sample texts
    printf("=== Training Phase ===\n");
    process_text(proc, "What is gaia?");
    process_text(proc, "gaia is amazing");
    process_text(proc, "How do gates work?");
    process_text(proc, "The gates process binary patterns");
    
    // Show what we learned
    show_patterns(proc);
    
    // Interactive mode
    interactive_demo(proc);
    
    // Cleanup
    for (int i = 0; i < proc->pattern_count; i++) {
        if (proc->word_patterns[i].pattern_gate) {
            gate_destroy(proc->word_patterns[i].pattern_gate);
        }
    }
    for (int i = 0; i < proc->question_count; i++) {
        if (proc->questions[i].detector) {
            gate_destroy(proc->questions[i].detector);
        }
    }
    if (proc->word_boundary_detector) {
        gate_destroy(proc->word_boundary_detector);
    }
    free(proc);
    
    gate_registry_cleanup();
    
    return 0;
}