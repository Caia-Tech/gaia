#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>
#include "gate_types.h"
#include "function_registry.h"
#include "gaia_functions.h"
#include "analysis_functions.h"
#include "experiment_logger.h"

// Forward declarations
char* handle_function_call(const char* input);

#define HASH_SIZE 2097152     // 2M buckets (8x larger for 100-token patterns)
#define MAX_WORD_LENGTH 50
#define MAX_INPUT_LENGTH 1024
#define CONTEXT_SIZE 100      // 100-token context window!
#define PAD_TOKEN "[PAD]"
#define MAX_SUPERPOSITION 5   // Maximum states to maintain
#define SUPERPOSITION_THRESHOLD 0.8  // Similarity threshold to trigger superposition

// Feature flags
static int use_superposition = 0;  // Set to 1 to enable superposition mode
static int debug_superposition = 0;  // Set to 1 for superposition debug output
static int use_analysis = 1;  // Set to 1 to enable V6 analysis features

// Enhanced pattern structure for 100-token context
typedef struct Pattern {
    char context[CONTEXT_SIZE][MAX_WORD_LENGTH];  // 100 words of context
    char next[MAX_WORD_LENGTH];                   // Next word
    int context_length;                           // Actual context length
    int count;
    Gate* gate;
    struct Pattern* collision_next;
} Pattern;

// Chat system with more tracking
typedef struct {
    Pattern* patterns[HASH_SIZE];
    int total_patterns;
    int total_words;
    int patterns_by_length[CONTEXT_SIZE + 1];  // Track patterns of each context length
    int hash_collisions;                       // Track collision performance
    int pattern_lookups;                       // Track lookup performance
} ChatSystem;

// Enhanced hash function for 100-token context
uint32_t compute_pattern_address(char context[][MAX_WORD_LENGTH], int context_length) {
    uint64_t hash = 5381;  // Use 64-bit for better distribution
    
    for (int i = 0; i < context_length; i++) {
        for (int j = 0; context[i][j] != '\0'; j++) {
            hash = ((hash << 5) + hash) + (unsigned char)context[i][j];
        }
        hash = ((hash << 5) + hash) + '|';  // Word separator
    }
    
    return (uint32_t)(hash % HASH_SIZE);
}

// Initialize the chat system
ChatSystem* init_chat_system() {
    ChatSystem* system = calloc(1, sizeof(ChatSystem));
    if (!system) {
        printf("Failed to allocate chat system\n");
        return NULL;
    }
    
    // Initialize hash table
    for (int i = 0; i < HASH_SIZE; i++) {
        system->patterns[i] = NULL;
    }
    
    printf("Chat system initialized with %d hash buckets\n", HASH_SIZE);
    return system;
}

// Tokenize input into words
int tokenize_input(const char* input, char tokens[][MAX_WORD_LENGTH], int max_tokens) {
    int token_count = 0;
    char temp[MAX_INPUT_LENGTH];
    strncpy(temp, input, MAX_INPUT_LENGTH - 1);
    temp[MAX_INPUT_LENGTH - 1] = '\0';
    
    char* token = strtok(temp, " \t\n\r.,!?;:");
    while (token && token_count < max_tokens) {
        // Convert to lowercase
        for (int i = 0; token[i]; i++) {
            token[i] = tolower(token[i]);
        }
        
        strncpy(tokens[token_count], token, MAX_WORD_LENGTH - 1);
        tokens[token_count][MAX_WORD_LENGTH - 1] = '\0';
        token_count++;
        
        token = strtok(NULL, " \t\n\r.,!?;:");
    }
    
    return token_count;
}

// Enhanced pattern storage with collision handling
void store_pattern(ChatSystem* system, char context[][MAX_WORD_LENGTH], int context_length, const char* next) {
    system->pattern_lookups++;
    
    uint32_t address = compute_pattern_address(context, context_length);
    Pattern* current = system->patterns[address];
    
    // Search for existing pattern in collision chain
    while (current) {
        if (current->context_length == context_length) {
            int match = 1;
            for (int i = 0; i < context_length; i++) {
                if (strcmp(current->context[i], context[i]) != 0) {
                    match = 0;
                    break;
                }
            }
            if (match && strcmp(current->next, next) == 0) {
                current->count++;
                return;  // Pattern already exists, incremented count
            }
        }
        current = current->collision_next;
    }
    
    // Create new pattern
    Pattern* pattern = calloc(1, sizeof(Pattern));
    if (!pattern) return;
    
    pattern->context_length = context_length;
    for (int i = 0; i < context_length; i++) {
        strncpy(pattern->context[i], context[i], MAX_WORD_LENGTH - 1);
        pattern->context[i][MAX_WORD_LENGTH - 1] = '\0';
    }
    strncpy(pattern->next, next, MAX_WORD_LENGTH - 1);
    pattern->next[MAX_WORD_LENGTH - 1] = '\0';
    pattern->count = 1;
    
    // Insert at head of collision chain
    if (system->patterns[address]) {
        system->hash_collisions++;
    }
    pattern->collision_next = system->patterns[address];
    system->patterns[address] = pattern;
    
    system->total_patterns++;
    system->patterns_by_length[context_length]++;
}

// Multi-step lookahead for better word selection
typedef struct {
    char word[MAX_WORD_LENGTH];
    int found_continuations;
    float coherence_score;
    float total_score;
} WordCandidate;

int find_word_candidates(ChatSystem* system, char context[][MAX_WORD_LENGTH], int context_length, WordCandidate* candidates, int max_candidates) {
    int candidate_count = 0;
    uint32_t address = compute_pattern_address(context, context_length);
    Pattern* current = system->patterns[address];
    
    while (current && candidate_count < max_candidates) {
        if (current->context_length == context_length) {
            int match = 1;
            for (int i = 0; i < context_length; i++) {
                if (strcmp(current->context[i], context[i]) != 0) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                strncpy(candidates[candidate_count].word, current->next, MAX_WORD_LENGTH - 1);
                candidates[candidate_count].word[MAX_WORD_LENGTH - 1] = '\0';
                candidates[candidate_count].found_continuations = 0;
                candidates[candidate_count].coherence_score = 0.0;
                candidates[candidate_count].total_score = 0.0;
                candidate_count++;
            }
        }
        current = current->collision_next;
    }
    
    return candidate_count;
}

// Check continuations for lookahead
void check_continuations(ChatSystem* system, char context[][MAX_WORD_LENGTH], int context_length, const char* candidate_word, WordCandidate* candidate) {
    // Create new context with candidate word
    char new_context[CONTEXT_SIZE][MAX_WORD_LENGTH];
    int new_length = context_length + 1;
    if (new_length > CONTEXT_SIZE) {
        // Shift context window
        for (int i = 0; i < CONTEXT_SIZE - 1; i++) {
            strcpy(new_context[i], context[i + 1]);
        }
        strcpy(new_context[CONTEXT_SIZE - 1], candidate_word);
        new_length = CONTEXT_SIZE;
    } else {
        for (int i = 0; i < context_length; i++) {
            strcpy(new_context[i], context[i]);
        }
        strcpy(new_context[context_length], candidate_word);
    }
    
    // Count how many continuations this leads to
    uint32_t address = compute_pattern_address(new_context, new_length);
    Pattern* current = system->patterns[address];
    
    while (current) {
        if (current->context_length == new_length) {
            int match = 1;
            for (int i = 0; i < new_length; i++) {
                if (strcmp(current->context[i], new_context[i]) != 0) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                candidate->found_continuations++;
            }
        }
        current = current->collision_next;
    }
}

// Calculate coherence score using V6 analysis
void calculate_coherence_score(char context[][MAX_WORD_LENGTH], int context_length, WordCandidate* candidate) {
    if (!use_analysis) {
        candidate->coherence_score = 0.5;  // Default neutral score
        return;
    }
    
    // Build context string
    char context_str[1024] = "";
    for (int i = 0; i < context_length && i < 10; i++) {  // Use last 10 words for coherence
        if (i > 0) strcat(context_str, " ");
        strcat(context_str, context[i]);
    }
    
    // Use analysis functions to score coherence
    CoherenceScore score = analyze_coherence(context_str, candidate->word);
    candidate->coherence_score = score.overall_score;
    
    // Log coherence experiment
    log_coherence_experiment(context_str, candidate->word, 
                           score.semantic_similarity, score.grammatical_fit,
                           score.topic_consistency, score.overall_score);
}

// Enhanced superposition states
typedef struct {
    char word[MAX_WORD_LENGTH];
    float probability;
    int continuation_count;
    float coherence_score;
} SuperpositionState;

// Generate superposition states
int generate_superposition_states(ChatSystem* system, char context[][MAX_WORD_LENGTH], int context_length, SuperpositionState* states, int max_states) {
    WordCandidate candidates[100];
    int candidate_count = find_word_candidates(system, context, context_length, candidates, 100);
    
    if (candidate_count == 0) return 0;
    
    // Score each candidate
    for (int i = 0; i < candidate_count; i++) {
        check_continuations(system, context, context_length, candidates[i].word, &candidates[i]);
        calculate_coherence_score(context, context_length, &candidates[i]);
        candidates[i].total_score = candidates[i].found_continuations * 0.6 + candidates[i].coherence_score * 0.4;
    }
    
    // Sort by total score
    for (int i = 0; i < candidate_count - 1; i++) {
        for (int j = i + 1; j < candidate_count; j++) {
            if (candidates[j].total_score > candidates[i].total_score) {
                WordCandidate temp = candidates[i];
                candidates[i] = candidates[j];
                candidates[j] = temp;
            }
        }
    }
    
    // Check if top candidates are similar enough for superposition
    if (candidate_count < 2) return 0;
    
    float score_diff = candidates[0].total_score - candidates[1].total_score;
    if (score_diff > (1.0 - SUPERPOSITION_THRESHOLD)) {
        return 0;  // Clear winner, no superposition needed
    }
    
    // Create superposition states
    int state_count = 0;
    float total_score = 0.0;
    
    for (int i = 0; i < candidate_count && i < max_states; i++) {
        if (candidates[i].total_score > 0.1) {  // Minimum threshold
            strcpy(states[state_count].word, candidates[i].word);
            states[state_count].continuation_count = candidates[i].found_continuations;
            states[state_count].coherence_score = candidates[i].coherence_score;
            total_score += candidates[i].total_score;
            state_count++;
        }
    }
    
    // Normalize probabilities
    for (int i = 0; i < state_count; i++) {
        float raw_score = states[i].continuation_count * 0.6 + states[i].coherence_score * 0.4;
        states[i].probability = raw_score / total_score;
    }
    
    return state_count;
}

// Choose word from superposition collapse
char* collapse_superposition(SuperpositionState* states, int state_count) {
    if (state_count == 0) return NULL;
    
    // Add randomness with microsecond precision
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    srand(ts.tv_sec * 1000000 + ts.tv_nsec / 1000 + getpid());
    
    float random_val = (float)rand() / RAND_MAX;
    float cumulative = 0.0;
    
    for (int i = 0; i < state_count; i++) {
        cumulative += states[i].probability;
        if (random_val <= cumulative) {
            // Log superposition experiment
            float probs[MAX_SUPERPOSITION];
            for (int j = 0; j < state_count && j < MAX_SUPERPOSITION; j++) {
                probs[j] = states[j].probability;
            }
            log_superposition_experiment("", state_count, probs, states[i].word);
            return states[i].word;
        }
    }
    
    return states[0].word;  // Fallback to first state
}

// V6 Enhanced word finding with analysis integration
char* find_next_word_v6(ChatSystem* system, char context[][MAX_WORD_LENGTH], int context_length, const char* original_input) {
    system->pattern_lookups++;
    
    // Use V6 analysis to understand the input better
    if (use_analysis && original_input) {
        AnalysisResult* analysis = analyze_input(original_input);
        
        if (debug_superposition) {
            printf("V6 Analysis: %s prompt, topic: %s, requires_calculation: %s\n",
                   prompt_type_to_string(analysis->prompt_type),
                   topic_type_to_string(analysis->topic),
                   analysis->requires_calculation ? "yes" : "no");
        }
        
        // If it requires calculation, try function calls first
        if (analysis->requires_calculation) {
            for (int i = 0; i < analysis->num_entities; i++) {
                if (analysis->entities[i].type == ENTITY_OPERATOR) {
                    // This is a calculation, try to handle with functions
                    char* function_result = handle_function_call(original_input);
                    if (function_result) {
                        free_analysis_result(analysis);
                        return function_result;
                    }
                    break;
                }
            }
        }
        
        free_analysis_result(analysis);
    }
    
    // Try superposition first if enabled
    if (use_superposition) {
        SuperpositionState states[MAX_SUPERPOSITION];
        int state_count = generate_superposition_states(system, context, context_length, states, MAX_SUPERPOSITION);
        
        if (state_count > 1) {
            if (debug_superposition) {
                printf("Superposition with %d states:\n", state_count);
                for (int i = 0; i < state_count; i++) {
                    printf("  State %d: '%s' (prob=%.3f, cont=%d, coh=%.3f)\n", 
                           i, states[i].word, states[i].probability, 
                           states[i].continuation_count, states[i].coherence_score);
                }
            }
            
            char* result = collapse_superposition(states, state_count);
            if (result) {
                char* final_result = malloc(strlen(result) + 1);
                strcpy(final_result, result);
                return final_result;
            }
        }
    }
    
    // Standard multi-step lookahead as fallback
    WordCandidate candidates[100];
    int candidate_count = find_word_candidates(system, context, context_length, candidates, 100);
    
    if (candidate_count == 0) return NULL;
    
    // Score candidates with V6 coherence analysis
    for (int i = 0; i < candidate_count; i++) {
        check_continuations(system, context, context_length, candidates[i].word, &candidates[i]);
        calculate_coherence_score(context, context_length, &candidates[i]);
        candidates[i].total_score = candidates[i].found_continuations * 0.6 + candidates[i].coherence_score * 0.4;
    }
    
    // Find best candidate
    int best_idx = 0;
    for (int i = 1; i < candidate_count; i++) {
        if (candidates[i].total_score > candidates[best_idx].total_score) {
            best_idx = i;
        }
    }
    
    char* result = malloc(strlen(candidates[best_idx].word) + 1);
    strcpy(result, candidates[best_idx].word);
    return result;
}

// Enhanced function call handler with V6 analysis
char* handle_function_call(const char* input) {
    if (!input) return NULL;
    
    // Use V6 analysis to extract entities
    AnalysisResult* analysis = analyze_input(input);
    if (!analysis || !analysis->requires_calculation) {
        if (analysis) free_analysis_result(analysis);
        return NULL;
    }
    
    // Look for math operations in entities
    int numbers[10];
    char operators[10][20];
    int num_count = 0, op_count = 0;
    
    for (int i = 0; i < analysis->num_entities; i++) {
        if (analysis->entities[i].type == ENTITY_NUMBER) {
            numbers[num_count++] = atoi(analysis->entities[i].value);
        } else if (analysis->entities[i].type == ENTITY_OPERATOR) {
            strcpy(operators[op_count++], analysis->entities[i].value);
        }
    }
    
    free_analysis_result(analysis);
    
    // Handle simple two-number operations
    if (num_count >= 2 && op_count >= 1) {
        int a = numbers[0];
        int b = numbers[1];
        const char* op = operators[0];
        
        FunctionResult func_result;
        int result = 0;
        if (strcmp(op, "plus") == 0 || strcmp(op, "add") == 0) {
            func_result = call_int_int_function("add", a, b);
            if (func_result.success) result = atoi(func_result.result);
        } else if (strcmp(op, "minus") == 0 || strcmp(op, "subtract") == 0) {
            func_result = call_int_int_function("subtract", a, b);
            if (func_result.success) result = atoi(func_result.result);
        } else if (strcmp(op, "times") == 0 || strcmp(op, "multiply") == 0) {
            func_result = call_int_int_function("multiply", a, b);
            if (func_result.success) result = atoi(func_result.result);
        } else if (strcmp(op, "divide") == 0 || strcmp(op, "divided") == 0) {
            if (b != 0) {
                func_result = call_int_int_function("divide", a, b);
                if (func_result.success) result = atoi(func_result.result);
            } else {
                char* error_result = malloc(50);
                strcpy(error_result, "Error: division by zero");
                return error_result;
            }
        } else {
            return NULL;
        }
        
        // Use formatted calculation response
        return format_calculation_response(op, a, b, result);
    }
    
    return NULL;
}

// Generate response with V6 enhancements
void generate_response_v6(ChatSystem* system, const char* input) {
    if (!input || strlen(input) == 0) {
        printf("Please provide some input.\n");
        return;
    }
    
    printf("GAIA V6: ");
    
    // V6 Analysis first
    if (use_analysis) {
        AnalysisResult* analysis = analyze_input(input);
        
        // Handle different prompt types appropriately
        if (analysis->prompt_type == PROMPT_GREETING) {
            printf("Hello! How can I help you today?\n");
            free_analysis_result(analysis);
            return;
        } else if (analysis->prompt_type == PROMPT_FAREWELL) {
            printf("Goodbye! Take care!\n");
            free_analysis_result(analysis);
            return;
        } else if (analysis->prompt_type == PROMPT_ACKNOWLEDGMENT) {
            printf("You're welcome! Is there anything else I can help with?\n");
            free_analysis_result(analysis);
            return;
        } else if (analysis->requires_list) {
            // Handle list requests with proper formatting
            const char* colors[] = {"red", "blue", "green"};
            const char* animals[] = {"cat", "dog", "bird"};
            const char* numbers[] = {"one", "two", "three"};
            
            if (strstr(input, "color")) {
                char* formatted = format_list_response("colors", colors, 3);
                if (formatted) {
                    printf("%s", formatted);
                    free(formatted);
                    free_analysis_result(analysis);
                    return;
                }
            } else if (strstr(input, "animal")) {
                char* formatted = format_list_response("animals", animals, 3);
                if (formatted) {
                    printf("%s", formatted);
                    free(formatted);
                    free_analysis_result(analysis);
                    return;
                }
            } else if (strstr(input, "number")) {
                char* formatted = format_list_response("numbers", numbers, 3);
                if (formatted) {
                    printf("%s", formatted);
                    free(formatted);
                    free_analysis_result(analysis);
                    return;
                }
            }
        }
        
        free_analysis_result(analysis);
    }
    
    // Try function call first
    char* function_result = handle_function_call(input);
    if (function_result) {
        printf("%s\n", function_result);
        free(function_result);
        return;
    }
    
    // Tokenize input
    char tokens[CONTEXT_SIZE][MAX_WORD_LENGTH];
    int token_count = tokenize_input(input, tokens, CONTEXT_SIZE);
    
    if (token_count == 0) {
        printf("I need some words to work with.\n");
        return;
    }
    
    // Generate response using pattern matching
    char context[CONTEXT_SIZE][MAX_WORD_LENGTH];
    int context_length = 0;
    
    // Build initial context from input
    for (int i = 0; i < token_count && i < CONTEXT_SIZE; i++) {
        strcpy(context[i], tokens[i]);
        context_length++;
    }
    
    // Generate up to 20 words
    int words_generated = 0;
    int max_words = 20;
    
    while (words_generated < max_words) {
        char* next_word = find_next_word_v6(system, context, context_length, input);
        
        if (!next_word) {
            if (words_generated == 0) {
                printf("I don't have enough training data for this input.\n");
            }
            break;
        }
        
        printf("%s ", next_word);
        fflush(stdout);
        
        // Update context for next iteration
        if (context_length < CONTEXT_SIZE) {
            strcpy(context[context_length], next_word);
            context_length++;
        } else {
            // Shift context window
            for (int i = 0; i < CONTEXT_SIZE - 1; i++) {
                strcpy(context[i], context[i + 1]);
            }
            strcpy(context[CONTEXT_SIZE - 1], next_word);
        }
        
        free(next_word);
        words_generated++;
        
        // Natural stopping points
        if (words_generated >= 3) {
            char last_char = context[context_length - 1][strlen(context[context_length - 1]) - 1];
            if (last_char == '.' || last_char == '!' || last_char == '?') {
                break;
            }
        }
        
        // Stop if we've generated a reasonable response
        if (words_generated >= 8 && words_generated % 4 == 0) {
            if (rand() % 3 == 0) break;  // 33% chance to stop at natural points
        }
    }
    
    printf("\n");
}

// Load training data from file
int load_training_data(ChatSystem* system, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Could not open training file: %s\n", filename);
        return 0;
    }
    
    char line[MAX_INPUT_LENGTH];
    int lines_processed = 0;
    
    printf("Loading training data from %s...\n", filename);
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) < 3) continue;  // Skip very short lines
        
        // Tokenize the line
        char tokens[CONTEXT_SIZE][MAX_WORD_LENGTH];
        int token_count = tokenize_input(line, tokens, CONTEXT_SIZE);
        
        if (token_count < 2) continue;  // Need at least 2 tokens
        
        // Create patterns of various lengths (1 to CONTEXT_SIZE)
        for (int context_len = 1; context_len < token_count && context_len <= CONTEXT_SIZE; context_len++) {
            for (int start = 0; start + context_len < token_count; start++) {
                char context[CONTEXT_SIZE][MAX_WORD_LENGTH];
                for (int i = 0; i < context_len; i++) {
                    strcpy(context[i], tokens[start + i]);
                }
                const char* next_word = tokens[start + context_len];
                
                store_pattern(system, context, context_len, next_word);
                system->total_words++;
            }
        }
        
        lines_processed++;
        if (lines_processed % 1000 == 0) {
            printf("Processed %d lines...\n", lines_processed);
        }
    }
    
    fclose(file);
    printf("Training complete: %d lines, %d patterns, %d words\n", 
           lines_processed, system->total_patterns, system->total_words);
    
    return 1;
}

// Print system statistics
void print_system_stats(ChatSystem* system) {
    printf("\n=== GAIA V6 System Statistics ===\n");
    printf("Total patterns: %d\n", system->total_patterns);
    printf("Total words processed: %d\n", system->total_words);
    printf("Hash collisions: %d\n", system->hash_collisions);
    printf("Pattern lookups: %d\n", system->pattern_lookups);
    printf("Hash efficiency: %.2f%%\n", 
           system->hash_collisions > 0 ? 
           100.0 * (1.0 - (float)system->hash_collisions / system->pattern_lookups) : 100.0);
    
    printf("\nPatterns by context length:\n");
    for (int i = 1; i <= 10 && i <= CONTEXT_SIZE; i++) {
        if (system->patterns_by_length[i] > 0) {
            printf("  %d-word context: %d patterns\n", i, system->patterns_by_length[i]);
        }
    }
    
    printf("\nV6 Features enabled:\n");
    printf("  Analysis functions: %s\n", use_analysis ? "ON" : "OFF");
    printf("  Superposition mode: %s\n", use_superposition ? "ON" : "OFF");
    printf("  Debug superposition: %s\n", debug_superposition ? "ON" : "OFF");
    printf("=======================================\n\n");
}

// Main function
int main(int argc, char* argv[]) {
    printf("=== GAIA V6 - Enhanced Analysis & Superposition ===\n");
    printf("Context window: %d tokens\n", CONTEXT_SIZE);
    printf("Hash table size: %d buckets\n", HASH_SIZE);
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--superposition") == 0) {
            use_superposition = 1;
            printf("Superposition mode: ENABLED\n");
        } else if (strcmp(argv[i], "--debug-superposition") == 0) {
            debug_superposition = 1;
            printf("Superposition debug: ENABLED\n");
        } else if (strcmp(argv[i], "--no-analysis") == 0) {
            use_analysis = 0;
            printf("Analysis functions: DISABLED\n");
        }
    }
    
    // Initialize function registry and experiment logger
    function_registry_init();
    register_gaia_functions();
    init_experiment_logger();
    
    // Initialize chat system
    ChatSystem* system = init_chat_system();
    if (!system) {
        function_registry_cleanup();
        return 1;
    }
    
    // Load training data
    if (!load_training_data(system, "conversational_flow.txt")) {
        printf("Warning: Could not load primary training data\n");
    }
    
    // Try to load additional training files
    load_training_data(system, "instruction_data.txt");
    load_training_data(system, "mathematical_data.txt");
    
    print_system_stats(system);
    
    // Interactive chat loop
    char input[MAX_INPUT_LENGTH];
    printf("V6 Chat ready! (Type 'quit' to exit, 'stats' for statistics)\n");
    printf("Special commands: 'toggle-superposition', 'toggle-analysis', 'toggle-debug'\n");
    printf("Experiment commands: 'log-summary', 'save-experiments'\n\n");
    
    while (1) {
        printf("You: ");
        if (!fgets(input, sizeof(input), stdin)) break;
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;
        
        // Handle special commands
        if (strcmp(input, "quit") == 0) {
            break;
        } else if (strcmp(input, "stats") == 0) {
            print_system_stats(system);
            continue;
        } else if (strcmp(input, "toggle-superposition") == 0) {
            use_superposition = !use_superposition;
            printf("Superposition mode: %s\n", use_superposition ? "ENABLED" : "DISABLED");
            continue;
        } else if (strcmp(input, "toggle-analysis") == 0) {
            use_analysis = !use_analysis;
            printf("Analysis functions: %s\n", use_analysis ? "ENABLED" : "DISABLED");
            continue;
        } else if (strcmp(input, "toggle-debug") == 0) {
            debug_superposition = !debug_superposition;
            printf("Superposition debug: %s\n", debug_superposition ? "ENABLED" : "DISABLED");
            continue;
        } else if (strcmp(input, "log-summary") == 0) {
            print_experiment_summary();
            continue;
        } else if (strcmp(input, "save-experiments") == 0) {
            save_experiment_log("gaia_v6_experiments.json");
            continue;
        }
        
        generate_response_v6(system, input);
    }
    
    print_system_stats(system);
    print_experiment_summary();
    save_experiment_log("gaia_v6_session.json");
    
    function_registry_cleanup();
    cleanup_experiment_logger();
    
    // Cleanup (basic - in real system would need proper cleanup)
    printf("GAIA V6 session ended.\n");
    return 0;
}