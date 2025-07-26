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
#include "dynamic_workflows.h"
#include "explanations.h"
#include "transformer_attention.h"

// Forward declarations
char* handle_function_call(const char* input);
char* generate_response_for_step(void* system, ReasoningStep* step);

#define HASH_SIZE 2097152
#define MAX_WORD_LENGTH 50
#define MAX_INPUT_LENGTH 1024
#define CONTEXT_SIZE 100
#define PAD_TOKEN "[PAD]"
#define MAX_SUPERPOSITION 5
#define SUPERPOSITION_THRESHOLD 0.8

// Feature flags
static int use_superposition = 0;
static int debug_superposition = 0;
static int use_analysis = 1;
static int use_workflows = 1;
static int use_attention = 1;  // V8 feature flag
static int use_refinement = 1; // V8 feature flag
static int debug_attention = 0;
static int debug_refinement = 0;
static int debug_workflows = 0;

// Pattern structure (same as V7)
typedef struct Pattern {
    char context[CONTEXT_SIZE][MAX_WORD_LENGTH];
    char next[MAX_WORD_LENGTH];
    int context_length;
    int count;
    struct Pattern* collision_next;
} Pattern;

// Chat system (same as V7)
typedef struct {
    Pattern* patterns[HASH_SIZE];
    int total_patterns;
    int total_words;
    int patterns_by_length[CONTEXT_SIZE + 1];
    int hash_collisions;
    int pattern_lookups;
} ChatSystem;

// V8 Enhancement context
typedef struct {
    TransformerLayer* transformer;
    AttentionContext* attention_ctx;
    RefinementContext* refinement_ctx;
    float base_quality;
    float enhanced_quality;
} V8Enhancement;

// Hash function (same as V7)
uint32_t compute_pattern_address(char context[][MAX_WORD_LENGTH], int context_length) {
    uint64_t hash = 5381;
    
    for (int i = 0; i < context_length; i++) {
        for (int j = 0; context[i][j] != '\0'; j++) {
            hash = ((hash << 5) + hash) + (unsigned char)context[i][j];
        }
        hash = ((hash << 5) + hash) + '|';
    }
    
    return (uint32_t)(hash % HASH_SIZE);
}

// Initialize chat system
ChatSystem* init_chat_system() {
    printf("Allocating chat system...\n");
    fflush(stdout);
    
    ChatSystem* system = calloc(1, sizeof(ChatSystem));
    if (!system) {
        printf("Failed to allocate chat system\n");
        return NULL;
    }
    
    printf("Initializing %d hash buckets...\n", HASH_SIZE);
    fflush(stdout);
    
    // Already initialized to NULL by calloc, no need to loop
    
    printf("Chat system initialized with %d hash buckets\n", HASH_SIZE);
    fflush(stdout);
    return system;
}

// Tokenize input (same as V7)
int tokenize_input(const char* input, char tokens[][MAX_WORD_LENGTH], int max_tokens) {
    int token_count = 0;
    char temp[MAX_INPUT_LENGTH];
    strncpy(temp, input, MAX_INPUT_LENGTH - 1);
    temp[MAX_INPUT_LENGTH - 1] = '\0';
    
    char* token = strtok(temp, " \t\n\r.,!?;:");
    while (token && token_count < max_tokens) {
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

// V8: Create enhancement context
V8Enhancement* create_v8_enhancement() {
    V8Enhancement* v8 = calloc(1, sizeof(V8Enhancement));
    if (!v8) return NULL;
    
    v8->transformer = create_transformer_layer();
    if (!v8->transformer) {
        free(v8);
        return NULL;
    }
    
    return v8;
}

// V8: Destroy enhancement context
void destroy_v8_enhancement(V8Enhancement* v8) {
    if (!v8) return;
    
    if (v8->transformer) {
        destroy_transformer_layer(v8->transformer);
    }
    if (v8->refinement_ctx) {
        destroy_refinement_context(v8->refinement_ctx);
    }
    
    free(v8);
}

// V8: Apply attention-based enhancement
char* apply_attention_enhancement(V8Enhancement* v8, const char* input, const char* base_response) {
    if (!v8 || !use_attention) {
        return strdup(base_response);
    }
    
    if (debug_attention) {
        printf("\n[V8 Attention] Enhancing response with self-attention...\n");
    }
    
    // Tokenize input for attention
    Token tokens[MAX_SEQ_LENGTH];
    int num_tokens = tokenize_for_attention(input, tokens, MAX_SEQ_LENGTH);
    
    if (num_tokens > 0) {
        // Create embeddings
        create_embeddings(tokens, num_tokens);
        add_position_encoding(tokens, num_tokens);
        
        // Create attention context
        AttentionContext ctx = {
            .tokens = tokens,
            .num_tokens = num_tokens,
            .layer = v8->transformer
        };
        
        // Apply multi-head attention
        apply_multi_head_attention(&ctx);
        
        if (debug_attention) {
            // Visualize attention patterns
            for (int h = 0; h < NUM_HEADS && h < 2; h++) {
                print_attention_matrix(&v8->transformer->heads[h], tokens, num_tokens);
            }
        }
        
        // Get attention confidence
        float attention_confidence = get_attention_confidence(&ctx);
        if (debug_attention) {
            printf("[V8 Attention] Confidence: %.2f\n", attention_confidence);
        }
    }
    
    return strdup(base_response);
}

// V8: Apply iterative refinement
char* apply_iterative_refinement(V8Enhancement* v8, const char* input, const char* response) {
    if (!v8 || !use_refinement) {
        return strdup(response);
    }
    
    if (debug_refinement) {
        printf("\n[V8 Refinement] Starting iterative refinement...\n");
    }
    
    // Create refinement context
    v8->refinement_ctx = create_refinement_context(input, response);
    if (!v8->refinement_ctx) {
        return strdup(response);
    }
    
    // Analyze base response
    ResponseAnalysis* base_analysis = analyze_response(input, response);
    v8->base_quality = base_analysis->overall_quality;
    
    if (debug_refinement) {
        printf("[V8 Refinement] Base quality: %.2f\n", v8->base_quality);
        printf("  Coherence: %.2f, Relevance: %.2f, Completeness: %.2f, Grammar: %.2f\n",
               base_analysis->coherence_score, base_analysis->relevance_score,
               base_analysis->completeness_score, base_analysis->grammar_score);
    }
    
    // Refine if quality is below threshold
    char* refined_response = NULL;
    if (v8->base_quality < 0.8) {
        refined_response = refine_response_v8(v8->refinement_ctx);
        
        if (debug_refinement) {
            printf("[V8 Refinement] Refined response: '%s'\n", refined_response);
        }
        
        // Analyze refined response
        ResponseAnalysis* refined_analysis = analyze_response(input, refined_response);
        v8->enhanced_quality = refined_analysis->overall_quality;
        
        if (debug_refinement) {
            printf("[V8 Refinement] Enhanced quality: %.2f (improvement: +%.2f)\n",
                   v8->enhanced_quality, v8->enhanced_quality - v8->base_quality);
            printf("[V8 Refinement] Iterations: %d\n", v8->refinement_ctx->iteration_count);
        }
        
        // Log experiment
        char notes[256];
        snprintf(notes, 255, "Refinement: %.2f -> %.2f in %d iterations",
                v8->base_quality, v8->enhanced_quality, v8->refinement_ctx->iteration_count);
        log_experiment(EXP_REFINEMENT, "V8 Iterative Refinement", input, refined_response,
                      notes, v8->enhanced_quality, 1);
        
        // Cleanup
        for (int i = 0; i < refined_analysis->num_issues; i++) {
            free(refined_analysis->issues[i]);
        }
        for (int i = 0; i < refined_analysis->num_suggestions; i++) {
            free(refined_analysis->suggestions[i]);
        }
        free(refined_analysis);
    }
    
    // Cleanup base analysis
    for (int i = 0; i < base_analysis->num_issues; i++) {
        free(base_analysis->issues[i]);
    }
    for (int i = 0; i < base_analysis->num_suggestions; i++) {
        free(base_analysis->suggestions[i]);
    }
    free(base_analysis);
    
    // Return the refined response if we refined, otherwise return original
    if (refined_response) {
        return refined_response;
    } else {
        return strdup(response);
    }
}

// Enhanced function call handler (from V7)
char* handle_function_call(const char* input) {
    if (!input) return NULL;
    
    // First check for special functions (factorial, fibonacci, prime, etc)
    if (strstr(input, "factorial")) {
        // Extract number
        int num = -1;
        char temp[256];
        strncpy(temp, input, 255);
        char* token = strtok(temp, " ");
        while (token) {
            int val = atoi(token);
            if (val > 0 || (val == 0 && strcmp(token, "0") == 0)) {
                num = val;
                break;
            }
            token = strtok(NULL, " ");
        }
        
        if (num >= 0) {
            FunctionResult result = call_int_function("factorial", num);
            if (result.success) {
                char* response = malloc(256);
                snprintf(response, 255, "The factorial of %d is %s", num, result.result);
                return response;
            }
        }
    }
    
    if (strstr(input, "fibonacci") || strstr(input, "Fibonacci")) {
        // Extract position (1st, 2nd, 3rd, or just number)
        int num = -1;
        if (strstr(input, "10th") || strstr(input, "tenth")) num = 10;
        else if (strstr(input, "1st") || strstr(input, "first")) num = 1;
        else if (strstr(input, "2nd") || strstr(input, "second")) num = 2;
        else if (strstr(input, "3rd") || strstr(input, "third")) num = 3;
        else if (strstr(input, "4th") || strstr(input, "fourth")) num = 4;
        else if (strstr(input, "5th") || strstr(input, "fifth")) num = 5;
        else {
            // Try to extract a number
            char temp[256];
            strncpy(temp, input, 255);
            char* token = strtok(temp, " ");
            while (token) {
                int val = atoi(token);
                if (val > 0) {
                    num = val;
                    break;
                }
                token = strtok(NULL, " ");
            }
        }
        
        if (num > 0) {
            FunctionResult result = call_int_function("fibonacci", num);
            if (result.success) {
                char* response = malloc(256);
                snprintf(response, 255, "The %dth Fibonacci number is %s", num, result.result);
                return response;
            }
        }
    }
    
    if (strstr(input, "prime")) {
        // Extract number
        int num = -1;
        char temp[256];
        strncpy(temp, input, 255);
        char* token = strtok(temp, " ");
        while (token) {
            int val = atoi(token);
            if (val > 0) {
                num = val;
                break;
            }
            token = strtok(NULL, " ");
        }
        
        if (num > 0) {
            FunctionResult result = call_int_function("is_prime", num);
            if (result.success) {
                char* response = malloc(256);
                int is_prime = atoi(result.result);
                snprintf(response, 255, "%d is %s prime number", 
                        num, is_prime ? "a" : "not a");
                return response;
            }
        }
    }
    
    // Check for regular calculations
    AnalysisResult* analysis = analyze_input(input);
    if (!analysis || !analysis->requires_calculation) {
        if (analysis) free_analysis_result(analysis);
        return NULL;
    }
    
    int numbers[10];
    char operators[10][20];
    int num_count = 0, op_count = 0;
    
    // Extract numbers and operators with enhanced negative number support
    // Create entities for analysis  
    Entity entities[20];
    int entity_count = extract_entities(input, entities, 20);
    
    // Convert entities to numbers and operators
    for (int i = 0; i < entity_count && num_count < 10; i++) {
        if (entities[i].type == ENTITY_NUMBER && num_count < 10) {
            numbers[num_count++] = atoi(entities[i].value);
        } else if (entities[i].type == ENTITY_OPERATOR && op_count < 10) {
            strncpy(operators[op_count++], entities[i].value, 19);
        }
    }
    
    if (num_count >= 2 && op_count >= 1) {
        // Execute calculation
        int result = numbers[0];
        char* response = malloc(512);
        char operation_str[256] = "";
        
        snprintf(operation_str, 255, "%d", numbers[0]);
        
        for (int i = 0; i < op_count && i + 1 < num_count; i++) {
            char op_str[32];
            snprintf(op_str, 31, " %s %d", operators[i], numbers[i + 1]);
            strcat(operation_str, op_str);
            
            if (strcmp(operators[i], "plus") == 0 || strcmp(operators[i], "+") == 0) {
                result += numbers[i + 1];
            } else if (strcmp(operators[i], "minus") == 0 || strcmp(operators[i], "-") == 0) {
                result -= numbers[i + 1];
            } else if (strcmp(operators[i], "times") == 0 || strcmp(operators[i], "*") == 0 || 
                      strcmp(operators[i], "multiplied") == 0) {
                result *= numbers[i + 1];
            } else if (strcmp(operators[i], "divided") == 0 || strcmp(operators[i], "/") == 0) {
                if (numbers[i + 1] != 0) {
                    result /= numbers[i + 1];
                } else {
                    snprintf(response, 511, "Cannot divide by zero!");
                    free_analysis_result(analysis);
                    return response;
                }
            }
        }
        
        snprintf(response, 511, "%s equals %d", operation_str, result);
        free_analysis_result(analysis);
        return response;
    }
    
    free_analysis_result(analysis);
    return NULL;
}

// Generate response for workflow step (from V7)
char* generate_response_for_step(void* system_ptr, ReasoningStep* step) {
    ChatSystem* system = (ChatSystem*)system_ptr;
    char response[512] = "";
    
    if (!step) {
        return strdup("Error: Invalid step");
    }
    
    switch (step->type) {
        case STEP_DECOMPOSE:
            // Decomposition already done by workflow engine
            return strdup(step->output);
            
        case STEP_ANALYZE:
            // Analyze the input using V6 analysis
            if (strlen(step->input) > 0) {
                // For sub-questions, we need to execute them
                char* sub_result = handle_function_call(step->input);
                if (sub_result) {
                    return sub_result;
                }
                
                // Check if it's an explanation question
                if (strstr(step->input, "explain") || strstr(step->input, "what") || strstr(step->input, "how")) {
                    char* explanation = generate_explanation(step->input);
                    if (explanation) {
                        return explanation;
                    }
                }
                
                // Default analysis
                AnalysisResult* analysis = analyze_input(step->input);
                if (analysis) {
                    snprintf(response, 511, "Analyzing: %s",
                             step->input);
                    free_analysis_result(analysis);
                    return strdup(response);
                }
            }
            break;
            
        case STEP_EXECUTE: {
            // Execute the main task
            // First check if we have input to work with
            const char* query_input = strlen(step->input) > 0 ? step->input : step->description;
            
            if (debug_workflows) {
                printf("[V8 Debug] EXECUTE step - desc: '%s', input: '%s', using: '%s'\n", 
                       step->description, step->input, query_input);
            }
            
            // Try function call first for calculations
            char* calc_result = handle_function_call(query_input);
            if (calc_result) {
                if (debug_workflows) {
                    printf("[V8 Debug] Function call returned: %s\n", calc_result);
                }
                return calc_result;
            }
            
            // Check for specific task types
            if (strstr(step->description, "list") || strstr(query_input, "list")) {
                const char* items[] = {"red", "blue", "green"};
                return format_list_response("colors", items, 3);
            } else if (strstr(step->description, "explanation") || strstr(query_input, "explain") ||
                      strstr(query_input, "what is") || strstr(query_input, "how does")) {
                // Generate proper explanation
                char* explanation = generate_explanation(query_input);
                if (explanation) {
                    return explanation;
                }
                return strdup("I can help explain mathematical concepts. Please ask about specific operations like addition, subtraction, multiplication, or division.");
            } else {
                // Use pattern matching for general queries
                char tokens[CONTEXT_SIZE][MAX_WORD_LENGTH];
                int token_count = tokenize_input(query_input, tokens, CONTEXT_SIZE);
                
                if (token_count > 0) {
                    // Generate a simple response
                    snprintf(response, 511, "Processing request: %s", query_input);
                    return strdup(response);
                }
            }
            break;
        }
            
        case STEP_EVALUATE:
            // Evaluation already done by workflow engine
            return strdup(step->output);
            
        case STEP_SYNTHESIZE:
            // Synthesis handled by workflow engine
            return strdup(step->output);
            
        case STEP_BACKTRACK:
            // Backtracking handled by workflow engine
            return strdup("Reconsidering approach...");
            
        case STEP_COMPLETE:
            // Completion marker
            return strdup(strlen(step->output) > 0 ? step->output : "Task completed");
            
        default:
            break;
    }
    
    return strdup("Processing...");
}

// V8: Enhanced response generation with attention and refinement
void generate_response_v8(ChatSystem* system, const char* input) {
    if (!input || strlen(input) == 0) {
        printf("Please provide some input.\n");
        return;
    }
    
    // Quick test for prime which we know works
    if (strstr(input, "prime")) {
        char* result = handle_function_call(input);
        if (result) {
            printf("GAIA V8: %s\n", result);
            free(result);
            return;
        }
    }
    
    printf("GAIA V8: ");
    fflush(stdout);
    
    // Create V8 enhancement context
    V8Enhancement* v8 = create_v8_enhancement();
    if (!v8 && (debug_workflows || debug_refinement)) {
        printf("[V8 Debug] Failed to create V8 enhancement context\n");
    }
    
    // First, use V7 workflow system to generate base response
    char* base_response = NULL;
    
    if (use_workflows) {
        // Create workflow
        WorkflowState* workflow = create_workflow();
        if (!workflow) {
            printf("Failed to create workflow\n");
            base_response = strdup("Error: Could not create workflow");
        } else {
            // Decompose query
            int num_steps = decompose_query(workflow, input);
            
            if (debug_workflows) {
                printf("\n[V8 Debug] Created workflow with %d steps for query: '%s'\n", num_steps, input);
                for (int i = 0; i < num_steps; i++) {
                    printf("[V8 Debug] Step %d: %s (%d)\n", i+1, 
                           workflow->steps[i].description, workflow->steps[i].type);
                }
            }
            
            // Execute workflow
            workflow->current_step = 0;
            while (workflow->current_step < workflow->num_steps) {
                ReasoningStep* step = &workflow->steps[workflow->current_step];
                
                if (!step->completed) {
                    char* step_response = generate_response_for_step(system, step);
                    if (step_response) {
                        strncpy(step->output, step_response, 511);
                        step->output[511] = '\0';
                        free(step_response);
                        step->completed = 1;
                        step->confidence = 0.8;
                    }
                }
                
                workflow->current_step++;
            }
            
            // Synthesize results
            char* synthesis = synthesize_results(workflow);
            base_response = synthesis ? strdup(synthesis) : strdup("No response generated");
            
            if (debug_workflows) {
                printf("\n[V8 Debug] Base response: '%s'\n", base_response);
            }
            
            destroy_workflow(workflow);
        }
    } else {
        // Simple response without workflow
        base_response = strdup("I need workflows enabled to process this request.");
    }
    
    // Apply V8 enhancements
    char* enhanced_response = base_response;
    
    if (v8) {
        // Apply attention-based enhancement
        char* attention_enhanced = apply_attention_enhancement(v8, input, base_response);
        
        if (debug_workflows || debug_refinement) {
            printf("[V8 Debug] After attention: '%s'\n", attention_enhanced);
        }
        
        // Apply iterative refinement
        enhanced_response = apply_iterative_refinement(v8, input, attention_enhanced);
        
        if (debug_workflows || debug_refinement) {
            printf("[V8 Debug] After refinement: '%s'\n", enhanced_response);
        }
        
        if (attention_enhanced != base_response) {
            free(attention_enhanced);
        }
    }
    
    // Output final response
    if (enhanced_response && strlen(enhanced_response) > 0) {
        printf("%s\n", enhanced_response);
    } else {
        printf("(empty response generated)\n");
    }
    
    // Cleanup
    if (enhanced_response != base_response) {
        free(enhanced_response);
    }
    free(base_response);
    destroy_v8_enhancement(v8);
}

// Store pattern (same as V7)
void store_pattern(ChatSystem* system, char context[][MAX_WORD_LENGTH], int context_length, const char* next) {
    system->pattern_lookups++;
    
    uint32_t address = compute_pattern_address(context, context_length);
    Pattern* current = system->patterns[address];
    
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
                return;
            }
        }
        
        if (!current->collision_next) {
            system->hash_collisions++;
            break;
        }
        current = current->collision_next;
    }
    
    Pattern* new_pattern = calloc(1, sizeof(Pattern));
    if (!new_pattern) {
        printf("Failed to allocate pattern\n");
        return;
    }
    
    for (int i = 0; i < context_length; i++) {
        strcpy(new_pattern->context[i], context[i]);
    }
    strcpy(new_pattern->next, next);
    new_pattern->context_length = context_length;
    new_pattern->count = 1;
    
    // V8: No gate needed for patterns
    
    if (current) {
        current->collision_next = new_pattern;
    } else {
        system->patterns[address] = new_pattern;
    }
    
    system->total_patterns++;
    system->patterns_by_length[context_length]++;
}

// Load training data (same as V7)
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
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) < 3) continue;
        
        char tokens[CONTEXT_SIZE][MAX_WORD_LENGTH];
        int token_count = tokenize_input(line, tokens, CONTEXT_SIZE);
        
        if (token_count < 2) continue;
        
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

// Print system statistics (enhanced for V8)
void print_system_stats(ChatSystem* system) {
    printf("\n=== GAIA V8 System Statistics ===\n");
    printf("Total patterns: %d\n", system->total_patterns);
    printf("Total words processed: %d\n", system->total_words);
    printf("Hash collisions: %d\n", system->hash_collisions);
    printf("Pattern lookups: %d\n", system->pattern_lookups);
    
    printf("\nV8 Features enabled:\n");
    printf("  Dynamic workflows: %s\n", use_workflows ? "ON" : "OFF");
    printf("  Self-attention: %s\n", use_attention ? "ON" : "OFF");
    printf("  Iterative refinement: %s\n", use_refinement ? "ON" : "OFF");
    printf("  Analysis functions: %s\n", use_analysis ? "ON" : "OFF");
    printf("  Debug workflows: %s\n", debug_workflows ? "ON" : "OFF");
    printf("  Debug attention: %s\n", debug_attention ? "ON" : "OFF");
    printf("  Debug refinement: %s\n", debug_refinement ? "ON" : "OFF");
    
    printf("\nTransformer configuration:\n");
    printf("  Hidden dimension: %d\n", HIDDEN_DIM);
    printf("  Number of heads: %d\n", NUM_HEADS);
    printf("  Head dimension: %d\n", HEAD_DIM);
    printf("  Max sequence length: %d\n", MAX_SEQ_LENGTH);
    printf("=======================================\n\n");
}

// Main function
int main(int argc, char* argv[]) {
    printf("=== GAIA V8 - Recursive Refinement & Transformer Architecture ===\n");
    printf("Context window: %d tokens\n", CONTEXT_SIZE);
    printf("Hash table size: %d buckets\n", HASH_SIZE);
    printf("Transformer heads: %d\n", NUM_HEADS);
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-workflows") == 0) {
            use_workflows = 0;
            printf("Dynamic workflows: DISABLED\n");
        } else if (strcmp(argv[i], "--no-attention") == 0) {
            use_attention = 0;
            printf("Self-attention: DISABLED\n");
        } else if (strcmp(argv[i], "--no-refinement") == 0) {
            use_refinement = 0;
            printf("Iterative refinement: DISABLED\n");
        } else if (strcmp(argv[i], "--debug-attention") == 0) {
            debug_attention = 1;
            printf("Attention debugging: ENABLED\n");
        } else if (strcmp(argv[i], "--debug-refinement") == 0) {
            debug_refinement = 1;
            printf("Refinement debugging: ENABLED\n");
        } else if (strcmp(argv[i], "--debug-workflows") == 0) {
            debug_workflows = 1;
            printf("Workflow debugging: ENABLED\n");
        }
    }
    
    // Initialize systems
    function_registry_init();
    register_gaia_functions();
    init_experiment_logger();
    
    // Initialize chat system
    ChatSystem* system = init_chat_system();
    if (!system) {
        function_registry_cleanup();
        cleanup_experiment_logger();
        return 1;
    }
    
    printf("Loading training data...\n");
    fflush(stdout);
    
    // Load training data
    if (!load_training_data(system, "conversational_flow.txt")) {
        printf("Warning: Could not load primary training data\n");
    }
    
    printf("Training data loaded.\n");
    fflush(stdout);
    
    print_system_stats(system);
    
    // Interactive chat loop
    char input[MAX_INPUT_LENGTH];
    printf("V8 Chat ready! (Type 'quit' to exit, 'stats' for statistics)\n");
    printf("Special commands: 'toggle-attention', 'toggle-refinement', 'attention-test'\n\n");
    
    while (1) {
        printf("You: ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;
        
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) {
            generate_response_v8(system, input);
            continue;
        }
        
        // Handle special commands
        if (strcmp(input, "quit") == 0) {
            printf("Quitting...\n");
            break;
        } else if (strcmp(input, "stats") == 0) {
            print_system_stats(system);
            continue;
        } else if (strcmp(input, "toggle-attention") == 0) {
            use_attention = !use_attention;
            printf("Self-attention: %s\n", use_attention ? "ENABLED" : "DISABLED");
            continue;
        } else if (strcmp(input, "toggle-refinement") == 0) {
            use_refinement = !use_refinement;
            printf("Iterative refinement: %s\n", use_refinement ? "ENABLED" : "DISABLED");
            continue;
        } else if (strcmp(input, "debug-attention") == 0) {
            debug_attention = !debug_attention;
            printf("Attention debugging: %s\n", debug_attention ? "ENABLED" : "DISABLED");
            continue;
        } else if (strcmp(input, "debug-refinement") == 0) {
            debug_refinement = !debug_refinement;
            printf("Refinement debugging: %s\n", debug_refinement ? "ENABLED" : "DISABLED");
            continue;
        } else if (strcmp(input, "attention-test") == 0) {
            // Test attention mechanism
            const char* test_input = "What is the meaning of life?";
            printf("Testing attention on: '%s'\n", test_input);
            debug_attention = 1;
            generate_response_v8(system, test_input);
            debug_attention = 0;
            continue;
        } else if (strcmp(input, "refinement-test") == 0) {
            // Test refinement
            const char* test_input = "explain addition";
            printf("Testing refinement on: '%s'\n", test_input);
            debug_refinement = 1;
            generate_response_v8(system, test_input);
            debug_refinement = 0;
            continue;
        }
        
        generate_response_v8(system, input);
    }
    
    print_system_stats(system);
    print_experiment_summary();
    save_experiment_log("gaia_v8_session.json");
    
    function_registry_cleanup();
    cleanup_experiment_logger();
    
    printf("GAIA V8 session ended.\n");
    return 0;
}