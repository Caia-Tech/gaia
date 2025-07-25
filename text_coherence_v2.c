#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include "gate_types.h"

#define MAX_WORD_LENGTH 50
#define MAX_TRIGRAMS 1000
#define MAX_CONTEXT 20
#define CONFIDENCE_DECAY 0.95

// Trigram association
typedef struct {
    char words[3][MAX_WORD_LENGTH];  // 3-word context
    char next_word[MAX_WORD_LENGTH];
    int frequency;
    float confidence;
    int last_seen;  // For decay
} TrigramAssociation;

// Enhanced context
typedef struct {
    char words[MAX_CONTEXT][MAX_WORD_LENGTH];
    int length;
    float attention_weights[MAX_CONTEXT];  // Weight recent words more
} EnhancedContext;

// Coherence metrics
typedef struct {
    float perplexity;
    float topic_consistency;
    float grammar_score;
    float diversity_index;
} CoherenceMetrics;

// V2 Processor
typedef struct {
    TrigramAssociation trigrams[MAX_TRIGRAMS];
    int trigram_count;
    EnhancedContext context;
    CoherenceMetrics metrics;
    int total_words_seen;
    Gate* coherence_network;
} TextProcessorV2;

// Initialize V2
TextProcessorV2* create_v2_processor() {
    TextProcessorV2* proc = calloc(1, sizeof(TextProcessorV2));
    proc->coherence_network = gate_create("ADAPTIVE_AND");
    
    // Initialize attention weights (exponential decay)
    for (int i = 0; i < MAX_CONTEXT; i++) {
        proc->context.attention_weights[i] = exp(-i * 0.1);
    }
    
    return proc;
}

// Update context with attention
void update_context_v2(TextProcessorV2* proc, const char* word) {
    // Shift if full
    if (proc->context.length >= MAX_CONTEXT) {
        for (int i = 0; i < MAX_CONTEXT - 1; i++) {
            strcpy(proc->context.words[i], proc->context.words[i + 1]);
        }
        strcpy(proc->context.words[MAX_CONTEXT - 1], word);
    } else {
        strcpy(proc->context.words[proc->context.length++], word);
    }
    
    proc->total_words_seen++;
}

// Learn trigram
void learn_trigram(TextProcessorV2* proc, const char* w1, const char* w2, 
                   const char* w3, const char* next) {
    // Search for existing trigram
    TrigramAssociation* found = NULL;
    for (int i = 0; i < proc->trigram_count; i++) {
        if (strcmp(proc->trigrams[i].words[0], w1) == 0 &&
            strcmp(proc->trigrams[i].words[1], w2) == 0 &&
            strcmp(proc->trigrams[i].words[2], w3) == 0 &&
            strcmp(proc->trigrams[i].next_word, next) == 0) {
            found = &proc->trigrams[i];
            break;
        }
    }
    
    if (found) {
        found->frequency++;
        found->confidence = 1.0 - (1.0 / found->frequency);  // Asymptotic to 1
        found->last_seen = proc->total_words_seen;
    } else if (proc->trigram_count < MAX_TRIGRAMS) {
        TrigramAssociation* tri = &proc->trigrams[proc->trigram_count++];
        strcpy(tri->words[0], w1);
        strcpy(tri->words[1], w2);
        strcpy(tri->words[2], w3);
        strcpy(tri->next_word, next);
        tri->frequency = 1;
        tri->confidence = 0.5;  // Initial confidence
        tri->last_seen = proc->total_words_seen;
    }
}

// Get best next word with confidence
typedef struct {
    char word[MAX_WORD_LENGTH];
    float score;
} WordCandidate;

WordCandidate get_best_continuation(TextProcessorV2* proc, 
                                   const char* w1, const char* w2, const char* w3) {
    WordCandidate candidates[50];
    int candidate_count = 0;
    
    // Find all matching trigrams
    for (int i = 0; i < proc->trigram_count; i++) {
        // Handle partial matches for shorter contexts
        int match = 1;
        if (strlen(w1) > 0 && strcmp(proc->trigrams[i].words[0], w1) != 0) match = 0;
        if (strlen(w2) > 0 && strcmp(proc->trigrams[i].words[1], w2) != 0) match = 0;
        if (strcmp(proc->trigrams[i].words[2], w3) != 0) match = 0;
        
        if (match) {
            
            // Apply time decay
            int age = proc->total_words_seen - proc->trigrams[i].last_seen;
            float decay = pow(CONFIDENCE_DECAY, age / 100.0);
            
            // Calculate score
            float score = proc->trigrams[i].confidence * 
                         proc->trigrams[i].frequency * 
                         decay;
            
            // Add to candidates
            if (candidate_count < 50) {
                strcpy(candidates[candidate_count].word, proc->trigrams[i].next_word);
                candidates[candidate_count].score = score;
                candidate_count++;
            }
        }
    }
    
    // Find best candidate
    WordCandidate best = {"", 0.0};
    for (int i = 0; i < candidate_count; i++) {
        if (candidates[i].score > best.score) {
            best = candidates[i];
        }
    }
    
    return best;
}

// Calculate perplexity
float calculate_perplexity(TextProcessorV2* proc) {
    if (proc->context.length < 4) return 0.0;
    
    float total_surprise = 0.0;
    int count = 0;
    
    for (int i = 3; i < proc->context.length; i++) {
        WordCandidate pred = get_best_continuation(proc,
            proc->context.words[i-3],
            proc->context.words[i-2],
            proc->context.words[i-1]);
        
        if (pred.score > 0 && strcmp(pred.word, proc->context.words[i]) == 0) {
            total_surprise += -log(pred.score);
        } else {
            total_surprise += 10.0;  // High surprise for unpredicted
        }
        count++;
    }
    
    return count > 0 ? exp(total_surprise / count) : 100.0;
}

// Process and learn from text
void process_text_v2(TextProcessorV2* proc, const char* input) {
    printf("\n=== Processing: \"%s\" ===\n", input);
    
    // Tokenize
    char words[200][MAX_WORD_LENGTH];
    int word_count = 0;
    char temp[1000];
    strcpy(temp, input);
    
    for (int i = 0; i < strlen(temp); i++) {
        temp[i] = tolower(temp[i]);
    }
    
    char* token = strtok(temp, " .,!?;:");
    while (token && word_count < 200) {
        strcpy(words[word_count++], token);
        update_context_v2(proc, token);
        token = strtok(NULL, " .,!?;:");
    }
    
    // Learn trigrams
    for (int i = 0; i < word_count - 3; i++) {
        learn_trigram(proc, words[i], words[i+1], words[i+2], words[i+3]);
    }
    
    // Update metrics
    proc->metrics.perplexity = calculate_perplexity(proc);
    
    printf("Words processed: %d\n", word_count);
    printf("Trigrams learned: %d\n", proc->trigram_count);
    printf("Perplexity: %.2f\n", proc->metrics.perplexity);
}

// Generate coherent response
void generate_response_v2(TextProcessorV2* proc, const char* prompt) {
    printf("\n=== Generating Response ===\n");
    printf("Prompt: %s\n", prompt);
    
    // Process prompt first
    process_text_v2(proc, prompt);
    
    // Parse prompt to get starting context
    char prompt_words[50][MAX_WORD_LENGTH];
    int prompt_count = 0;
    char temp_prompt[500];
    strcpy(temp_prompt, prompt);
    
    // Convert to lowercase
    for (int i = 0; i < strlen(temp_prompt); i++) {
        temp_prompt[i] = tolower(temp_prompt[i]);
    }
    
    char* tok = strtok(temp_prompt, " .,!?");
    while (tok && prompt_count < 50) {
        strcpy(prompt_words[prompt_count++], tok);
        tok = strtok(NULL, " .,!?");
    }
    
    if (prompt_count < 1) {
        printf("Response: Need words to start generation.\n");
        return;
    }
    
    char response[500] = "";
    char current[3][MAX_WORD_LENGTH];
    
    // Handle different prompt lengths
    if (prompt_count == 1) {
        // Single word - find common continuations
        strcpy(current[0], "");
        strcpy(current[1], "");
        strcpy(current[2], prompt_words[0]);
    } else if (prompt_count == 2) {
        strcpy(current[0], "");
        strcpy(current[1], prompt_words[0]);
        strcpy(current[2], prompt_words[1]);
    } else {
        // Use last 3 words of prompt
        strcpy(current[0], prompt_words[prompt_count - 3]);
        strcpy(current[1], prompt_words[prompt_count - 2]);
        strcpy(current[2], prompt_words[prompt_count - 1]);
    }
    
    // Don't include empty strings in response
    if (strlen(current[0]) > 0) {
        strcat(response, current[0]);
        strcat(response, " ");
    }
    if (strlen(current[1]) > 0) {
        strcat(response, current[1]);
        strcat(response, " ");
    }
    strcat(response, current[2]);
    strcat(response, " ");
    
    // Generate up to 20 more words
    for (int i = 0; i < 20; i++) {
        WordCandidate next = get_best_continuation(proc, current[0], current[1], current[2]);
        
        if (next.score == 0) {
            // No continuation found, try with just last two words
            next = get_best_continuation(proc, "", current[1], current[2]);
            if (next.score == 0) {
                // Try with just last word
                next = get_best_continuation(proc, "", "", current[2]);
                if (next.score == 0) {
                    break;  // Can't continue
                }
            }
        }
        
        strcat(response, next.word);
        strcat(response, " ");
        
        // Shift context
        strcpy(current[0], current[1]);
        strcpy(current[1], current[2]);
        strcpy(current[2], next.word);
    }
    
    printf("Response: %s\n", response);
    printf("Final perplexity: %.2f\n", proc->metrics.perplexity);
}

// Show top trigrams
void show_top_trigrams(TextProcessorV2* proc) {
    printf("\n=== Top Trigram Patterns ===\n");
    
    // Sort by frequency * confidence
    for (int i = 0; i < proc->trigram_count - 1; i++) {
        for (int j = i + 1; j < proc->trigram_count; j++) {
            float score_i = proc->trigrams[i].frequency * proc->trigrams[i].confidence;
            float score_j = proc->trigrams[j].frequency * proc->trigrams[j].confidence;
            if (score_j > score_i) {
                TrigramAssociation temp = proc->trigrams[i];
                proc->trigrams[i] = proc->trigrams[j];
                proc->trigrams[j] = temp;
            }
        }
    }
    
    for (int i = 0; i < 10 && i < proc->trigram_count; i++) {
        printf("%s %s %s -> %s (freq:%d, conf:%.2f)\n",
               proc->trigrams[i].words[0],
               proc->trigrams[i].words[1],
               proc->trigrams[i].words[2],
               proc->trigrams[i].next_word,
               proc->trigrams[i].frequency,
               proc->trigrams[i].confidence);
    }
}

int main() {
    printf("gaia Text Coherence V2 - Trigram Edition\n");
    printf("========================================\n\n");
    
    // Initialize
    gate_registry_init();
    extern void register_basic_gates(void);
    extern void register_memory_gates(void);
    extern void register_adaptive_gates(void);
    
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    TextProcessorV2* proc = create_v2_processor();
    
    // Training corpus
    const char* training[] = {
        "gaia is a revolutionary logic gate based artificial intelligence system",
        "logic gates form the fundamental building blocks of gaia",
        "gaia processes information through binary gate operations",
        "the system learns by adjusting gate configurations dynamically",
        "unlike traditional neural networks gaia uses discrete logic",
        "gate based processing enables deterministic computation",
        "gaia achieves intelligence through emergent gate behaviors",
        "binary patterns flow through interconnected logic gates",
        "learning in gaia happens without backpropagation algorithms",
        "the coherence emerges from pattern associations in gates"
    };
    
    printf("=== Training Phase ===\n");
    for (int i = 0; i < 10; i++) {
        process_text_v2(proc, training[i]);
    }
    
    show_top_trigrams(proc);
    
    // Test generation
    printf("\n=== Generation Tests ===\n");
    // First check what patterns we have for "gaia is"
    printf("\n=== Checking learned patterns ===\n");
    for (int i = 0; i < proc->trigram_count; i++) {
        if (strstr(proc->trigrams[i].words[0], "gaia") ||
            strstr(proc->trigrams[i].words[1], "gaia") ||
            strstr(proc->trigrams[i].words[2], "gaia")) {
            printf("Pattern: '%s' '%s' '%s' -> '%s'\n",
                   proc->trigrams[i].words[0],
                   proc->trigrams[i].words[1],
                   proc->trigrams[i].words[2],
                   proc->trigrams[i].next_word);
        }
    }
    
    // Also check what "gaia is" patterns exist
    printf("\nSpecific 'gaia is' patterns:\n");
    for (int i = 0; i < proc->trigram_count; i++) {
        if (strcmp(proc->trigrams[i].words[1], "gaia") == 0 &&
            strcmp(proc->trigrams[i].words[2], "is") == 0) {
            printf("Found: '%s' 'gaia' 'is' -> '%s'\n",
                   proc->trigrams[i].words[0],
                   proc->trigrams[i].next_word);
        }
    }
    
    generate_response_v2(proc, "gaia is");
    generate_response_v2(proc, "logic gates");
    generate_response_v2(proc, "the system");
    
    // Interactive mode
    printf("\n=== Interactive Mode ===\n");
    printf("Enter prompts (or 'quit'):\n\n");
    
    char input[500];
    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) break;
        if (strcmp(input, "show") == 0) {
            show_top_trigrams(proc);
            continue;
        }
        
        generate_response_v2(proc, input);
    }
    
    // Cleanup
    if (proc->coherence_network) gate_destroy(proc->coherence_network);
    free(proc);
    
    gate_registry_cleanup();
    return 0;
}