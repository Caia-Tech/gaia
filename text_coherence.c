#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "gate_types.h"

#define MAX_CONTEXT_LENGTH 10
#define MAX_WORD_LENGTH 50
#define MAX_ASSOCIATIONS 100

// Word association structure
typedef struct {
    char word[MAX_WORD_LENGTH];
    char next_words[10][MAX_WORD_LENGTH];
    int next_counts[10];
    int total_nexts;
} WordAssociation;

// Context window
typedef struct {
    char words[MAX_CONTEXT_LENGTH][MAX_WORD_LENGTH];
    int length;
    int position;
} ContextWindow;

// Coherent processor
typedef struct {
    WordAssociation associations[MAX_ASSOCIATIONS];
    int association_count;
    ContextWindow context;
    Gate* coherence_checker;
    Gate* topic_tracker;
} CoherentProcessor;

// Initialize
CoherentProcessor* create_coherent_processor() {
    CoherentProcessor* proc = calloc(1, sizeof(CoherentProcessor));
    proc->coherence_checker = gate_create("PATTERN");
    proc->topic_tracker = gate_create("MEMORY_BANK");
    return proc;
}

// Update context window
void update_context(CoherentProcessor* proc, const char* word) {
    // Shift window if full
    if (proc->context.length >= MAX_CONTEXT_LENGTH) {
        for (int i = 0; i < MAX_CONTEXT_LENGTH - 1; i++) {
            strcpy(proc->context.words[i], proc->context.words[i + 1]);
        }
        strcpy(proc->context.words[MAX_CONTEXT_LENGTH - 1], word);
    } else {
        strcpy(proc->context.words[proc->context.length++], word);
    }
}

// Learn word associations (bigrams)
void learn_association(CoherentProcessor* proc, const char* word1, const char* word2) {
    // Find or create association
    WordAssociation* assoc = NULL;
    for (int i = 0; i < proc->association_count; i++) {
        if (strcmp(proc->associations[i].word, word1) == 0) {
            assoc = &proc->associations[i];
            break;
        }
    }
    
    if (!assoc && proc->association_count < MAX_ASSOCIATIONS) {
        assoc = &proc->associations[proc->association_count++];
        strcpy(assoc->word, word1);
        assoc->total_nexts = 0;
    }
    
    if (assoc) {
        // Update next word counts
        int found = 0;
        for (int i = 0; i < 10 && i < assoc->total_nexts; i++) {
            if (strcmp(assoc->next_words[i], word2) == 0) {
                assoc->next_counts[i]++;
                found = 1;
                break;
            }
        }
        
        if (!found && assoc->total_nexts < 10) {
            strcpy(assoc->next_words[assoc->total_nexts], word2);
            assoc->next_counts[assoc->total_nexts] = 1;
            assoc->total_nexts++;
        }
    }
}

// Get most likely next word
const char* predict_next_word(CoherentProcessor* proc, const char* current_word) {
    for (int i = 0; i < proc->association_count; i++) {
        if (strcmp(proc->associations[i].word, current_word) == 0) {
            // Find most frequent next word
            int max_count = 0;
            int max_idx = 0;
            for (int j = 0; j < proc->associations[i].total_nexts; j++) {
                if (proc->associations[i].next_counts[j] > max_count) {
                    max_count = proc->associations[i].next_counts[j];
                    max_idx = j;
                }
            }
            if (max_count > 0) {
                return proc->associations[i].next_words[max_idx];
            }
        }
    }
    return NULL;
}

// Check coherence score (0-100)
int check_coherence(CoherentProcessor* proc) {
    if (proc->context.length < 2) return 100;
    
    int coherent_transitions = 0;
    int total_transitions = proc->context.length - 1;
    
    // Check each word pair
    for (int i = 0; i < proc->context.length - 1; i++) {
        const char* predicted = predict_next_word(proc, proc->context.words[i]);
        if (predicted && strcmp(predicted, proc->context.words[i + 1]) == 0) {
            coherent_transitions++;
        }
    }
    
    return (coherent_transitions * 100) / total_transitions;
}

// Detect topic from context
const char* detect_topic(CoherentProcessor* proc) {
    // Simple topic detection based on keywords
    int tech_words = 0;
    int question_words = 0;
    
    for (int i = 0; i < proc->context.length; i++) {
        if (strcmp(proc->context.words[i], "gaia") == 0 ||
            strcmp(proc->context.words[i], "gates") == 0 ||
            strcmp(proc->context.words[i], "binary") == 0 ||
            strcmp(proc->context.words[i], "logic") == 0) {
            tech_words++;
        }
        if (strcmp(proc->context.words[i], "what") == 0 ||
            strcmp(proc->context.words[i], "how") == 0 ||
            strcmp(proc->context.words[i], "why") == 0) {
            question_words++;
        }
    }
    
    if (question_words > 0 && tech_words > 0) return "technical_question";
    if (tech_words > 2) return "technical_discussion";
    if (question_words > 0) return "general_question";
    return "general";
}

// Generate coherent response
void generate_coherent_response(CoherentProcessor* proc, const char* input) {
    printf("\n=== Coherent Response Generation ===\n");
    
    // Parse input
    char words[100][MAX_WORD_LENGTH];
    int word_count = 0;
    char temp[1000];
    strcpy(temp, input);
    
    char* token = strtok(temp, " .,!?");
    while (token && word_count < 100) {
        strcpy(words[word_count++], token);
        update_context(proc, token);
        token = strtok(NULL, " .,!?");
    }
    
    // Learn associations from input
    for (int i = 0; i < word_count - 1; i++) {
        learn_association(proc, words[i], words[i + 1]);
    }
    
    // Check coherence and topic
    int coherence = check_coherence(proc);
    const char* topic = detect_topic(proc);
    
    printf("Context coherence: %d%%\n", coherence);
    printf("Detected topic: %s\n", topic);
    
    // Generate response based on topic and associations
    printf("\nResponse: ");
    
    if (strstr(topic, "question") && word_count > 0) {
        // Answer questions coherently
        if (strcmp(words[0], "what") == 0 && word_count > 2) {
            printf("%s is ", words[2]);
            
            // Use associations to continue
            const char* next = predict_next_word(proc, "is");
            if (next) {
                printf("%s ", next);
                
                // Continue with predictions
                for (int i = 0; i < 5; i++) {
                    next = predict_next_word(proc, next);
                    if (!next) break;
                    printf("%s ", next);
                }
            } else {
                printf("something I'm learning about through pattern associations");
            }
        } else if (strcmp(words[0], "how") == 0) {
            printf("Based on learned patterns, ");
            
            // Generate from associations
            const char* start = word_count > 2 ? words[2] : "it";
            printf("%s ", start);
            
            const char* next = predict_next_word(proc, start);
            for (int i = 0; i < 8 && next; i++) {
                printf("%s ", next);
                next = predict_next_word(proc, next);
            }
        }
    } else {
        // Generate statement based on associations
        const char* seed = word_count > 0 ? words[word_count - 1] : "gaia";
        printf("Following from '%s': ", seed);
        
        const char* current = seed;
        for (int i = 0; i < 10; i++) {
            const char* next = predict_next_word(proc, current);
            if (!next) break;
            printf("%s ", next);
            current = next;
        }
    }
    
    printf("\n");
}

// Show learned associations
void show_associations(CoherentProcessor* proc) {
    printf("\n=== Learned Word Associations ===\n");
    printf("Total associations: %d\n\n", proc->association_count);
    
    for (int i = 0; i < proc->association_count && i < 10; i++) {
        printf("'%s' -> ", proc->associations[i].word);
        for (int j = 0; j < proc->associations[i].total_nexts; j++) {
            printf("'%s'(%d) ", 
                   proc->associations[i].next_words[j],
                   proc->associations[i].next_counts[j]);
        }
        printf("\n");
    }
}

int main() {
    printf("gaia Coherent Text Processor\n");
    printf("============================\n\n");
    
    // Initialize
    gate_registry_init();
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    CoherentProcessor* proc = create_coherent_processor();
    
    // Training phase - learn coherent patterns
    printf("=== Training Phase ===\n");
    printf("Learning from coherent sentences...\n\n");
    
    const char* training[] = {
        "gaia is a logic gate based artificial intelligence system",
        "logic gates process binary patterns efficiently",
        "binary patterns represent information in gaia",
        "gaia processes text without transformers or matrices",
        "what is gaia and how does it work",
        "how does gaia achieve learning without backpropagation",
        "learning happens through gate configuration updates"
    };
    
    for (int i = 0; i < 7; i++) {
        printf("Learning: %s\n", training[i]);
        generate_coherent_response(proc, training[i]);
    }
    
    // Show what we learned
    show_associations(proc);
    
    // Interactive coherent generation
    printf("\n=== Interactive Coherent Mode ===\n");
    printf("Type text to see coherent responses (or 'quit'):\n\n");
    
    char input[1000];
    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) break;
        if (strcmp(input, "show") == 0) {
            show_associations(proc);
            continue;
        }
        
        generate_coherent_response(proc, input);
    }
    
    // Cleanup
    if (proc->coherence_checker) gate_destroy(proc->coherence_checker);
    if (proc->topic_tracker) gate_destroy(proc->topic_tracker);
    free(proc);
    
    gate_registry_cleanup();
    return 0;
}