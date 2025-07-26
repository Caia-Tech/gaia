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
static int use_workflows = 1;  // V7 feature flag
static int debug_workflows = 0;

// Pattern structure (same as V6)
typedef struct Pattern {
    char context[CONTEXT_SIZE][MAX_WORD_LENGTH];
    char next[MAX_WORD_LENGTH];
    int context_length;
    int count;
    Gate* gate;
    struct Pattern* collision_next;
} Pattern;

// Chat system (same as V6)
typedef struct {
    Pattern* patterns[HASH_SIZE];
    int total_patterns;
    int total_words;
    int patterns_by_length[CONTEXT_SIZE + 1];
    int hash_collisions;
    int pattern_lookups;
} ChatSystem;

// Hash function (same as V6)
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
    ChatSystem* system = calloc(1, sizeof(ChatSystem));
    if (!system) {
        printf("Failed to allocate chat system\n");
        return NULL;
    }
    
    for (int i = 0; i < HASH_SIZE; i++) {
        system->patterns[i] = NULL;
    }
    
    printf("Chat system initialized with %d hash buckets\n", HASH_SIZE);
    return system;
}

// Tokenize input (same as V6)
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

// Store pattern (same as V6)
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
        current = current->collision_next;
    }
    
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
    
    if (system->patterns[address]) {
        system->hash_collisions++;
    }
    pattern->collision_next = system->patterns[address];
    system->patterns[address] = pattern;
    
    system->total_patterns++;
    system->patterns_by_length[context_length]++;
}

// Find next word (simplified from V6)
char* find_next_word(ChatSystem* system, char context[][MAX_WORD_LENGTH], int context_length) {
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
            if (match) {
                char* result = malloc(strlen(current->next) + 1);
                strcpy(result, current->next);
                return result;
            }
        }
        current = current->collision_next;
    }
    
    return NULL;
}

// V7: Generate response for a specific workflow step
char* generate_response_for_step(void* system_ptr, ReasoningStep* step) {
    ChatSystem* system = (ChatSystem*)system_ptr;
    static char response[512];
    
    switch (step->type) {
        case STEP_DECOMPOSE:
            // Already handled by workflow engine
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
            
            // Try function call first for calculations
            char* calc_result = handle_function_call(query_input);
            if (calc_result) {
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
            
        default:
            break;
    }
    
    return strdup("Processing...");
}

// V7: Enhanced function call handler (from V6)
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
    
    for (int i = 0; i < analysis->num_entities; i++) {
        if (analysis->entities[i].type == ENTITY_NUMBER) {
            numbers[num_count++] = atoi(analysis->entities[i].value);
        } else if (analysis->entities[i].type == ENTITY_OPERATOR) {
            strcpy(operators[op_count++], analysis->entities[i].value);
        }
    }
    
    free_analysis_result(analysis);
    
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
                return strdup("Error: division by zero");
            }
        } else {
            return NULL;
        }
        
        return format_calculation_response(op, a, b, result);
    }
    
    return NULL;
}

// V7: Generate response with dynamic workflows
void generate_response_v7(ChatSystem* system, const char* input) {
    if (!input || strlen(input) == 0) {
        printf("Please provide some input.\n");
        return;
    }
    
    printf("GAIA V7: ");
    
    // V7: Use dynamic workflows for complex queries
    if (use_workflows) {
        // Create workflow
        WorkflowState* workflow = create_workflow();
        
        // Decompose query into steps
        int num_steps = decompose_query(workflow, input);
        
        if (debug_workflows) {
            printf("\n[Workflow] Decomposed into %d steps\n", num_steps);
        }
        
        // Execute workflow
        workflow->current_step = 0;
        while (workflow->current_step < workflow->num_steps) {
            ReasoningStep* step = &workflow->steps[workflow->current_step];
            
            if (debug_workflows) {
                printf("[Step %d] %s\n", workflow->current_step + 1, step->description);
            }
            
            // Skip if already completed
            if (step->completed) {
                workflow->current_step++;
                continue;
            }
            
            // Generate response for this step
            char* step_response = generate_response_for_step(system, step);
            if (step_response) {
                strncpy(step->output, step_response, 511);
                step->output[511] = '\0';
                free(step_response);
                step->completed = 1;
                step->confidence = 0.8;  // Default confidence
                
                if (debug_workflows && strlen(step->output) > 0) {
                    printf("  Output: %s\n", step->output);
                }
            }
            
            workflow->current_step++;
        }
        
        // Synthesize final response
        char* final_response = synthesize_results(workflow);
        
        if (debug_workflows) {
            print_workflow_state(workflow);
        }
        
        // Check if we have a valid response
        if (strlen(final_response) > 0) {
            printf("%s\n", final_response);
        } else {
            printf("Processing complete.\n");
        }
        
        // Log experiment
        log_experiment(EXP_DISCOVERY, "V7 Workflow execution", input, final_response,
                      "Dynamic workflow processing", workflow->overall_confidence, 1);
        
        destroy_workflow(workflow);
        return;
    }
    
    // Fallback to V6 behavior if workflows disabled
    AnalysisResult* analysis = analyze_input(input);
    
    if (analysis->prompt_type == PROMPT_GREETING) {
        printf("Hello! How can I help you today?\n");
        free_analysis_result(analysis);
        return;
    } else if (analysis->prompt_type == PROMPT_FAREWELL) {
        printf("Goodbye! Take care!\n");
        free_analysis_result(analysis);
        return;
    }
    
    free_analysis_result(analysis);
    
    // Try function call
    char* function_result = handle_function_call(input);
    if (function_result) {
        printf("%s\n", function_result);
        free(function_result);
        return;
    }
    
    // Basic pattern matching
    char tokens[CONTEXT_SIZE][MAX_WORD_LENGTH];
    int token_count = tokenize_input(input, tokens, CONTEXT_SIZE);
    
    if (token_count == 0) {
        printf("I need some words to work with.\n");
        return;
    }
    
    printf("Processing your request...\n");
}

// Load training data (same as V6)
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

// Print system statistics (enhanced for V7)
void print_system_stats(ChatSystem* system) {
    printf("\n=== GAIA V7 System Statistics ===\n");
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
    
    printf("\nV7 Features enabled:\n");
    printf("  Dynamic workflows: %s\n", use_workflows ? "ON" : "OFF");
    printf("  Analysis functions: %s\n", use_analysis ? "ON" : "OFF");
    printf("  Superposition mode: %s\n", use_superposition ? "ON" : "OFF");
    printf("  Debug workflows: %s\n", debug_workflows ? "ON" : "OFF");
    printf("=======================================\n\n");
}

// Main function
int main(int argc, char* argv[]) {
    printf("=== GAIA V7 - Dynamic Workflows & Iterative Reasoning ===\n");
    printf("Context window: %d tokens\n", CONTEXT_SIZE);
    printf("Hash table size: %d buckets\n", HASH_SIZE);
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-workflows") == 0) {
            use_workflows = 0;
            printf("Dynamic workflows: DISABLED\n");
        } else if (strcmp(argv[i], "--debug-workflows") == 0) {
            debug_workflows = 1;
            printf("Workflow debugging: ENABLED\n");
        } else if (strcmp(argv[i], "--superposition") == 0) {
            use_superposition = 1;
            printf("Superposition mode: ENABLED\n");
        } else if (strcmp(argv[i], "--no-analysis") == 0) {
            use_analysis = 0;
            printf("Analysis functions: DISABLED\n");
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
    
    // Load training data
    if (!load_training_data(system, "conversational_flow.txt")) {
        printf("Warning: Could not load primary training data\n");
    }
    
    // Try to load additional training files
    load_training_data(system, "instruction_training.txt");
    load_training_data(system, "mathematical_data.txt");
    
    print_system_stats(system);
    
    // Interactive chat loop
    char input[MAX_INPUT_LENGTH];
    printf("V7 Chat ready! (Type 'quit' to exit, 'stats' for statistics)\n");
    printf("Special commands: 'toggle-workflows', 'toggle-debug', 'workflow-test'\n\n");
    
    while (1) {
        printf("You: ");
        if (!fgets(input, sizeof(input), stdin)) break;
        
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) {
            generate_response_v7(system, input);
            continue;
        }
        
        // Handle special commands
        if (strcmp(input, "quit") == 0) {
            break;
        } else if (strcmp(input, "stats") == 0) {
            print_system_stats(system);
            continue;
        } else if (strcmp(input, "toggle-workflows") == 0) {
            use_workflows = !use_workflows;
            printf("Dynamic workflows: %s\n", use_workflows ? "ENABLED" : "DISABLED");
            continue;
        } else if (strcmp(input, "toggle-debug") == 0) {
            debug_workflows = !debug_workflows;
            printf("Workflow debugging: %s\n", debug_workflows ? "ENABLED" : "DISABLED");
            continue;
        } else if (strcmp(input, "workflow-test") == 0) {
            // Test workflow with a complex query
            const char* test_query = "What is 15 plus 27? Also, can you explain what addition means?";
            printf("Testing workflow with: '%s'\n", test_query);
            generate_response_v7(system, test_query);
            continue;
        } else if (strcmp(input, "log-summary") == 0) {
            print_experiment_summary();
            continue;
        } else if (strcmp(input, "save-experiments") == 0) {
            save_experiment_log("gaia_v7_experiments.json");
            continue;
        }
        
        generate_response_v7(system, input);
    }
    
    print_system_stats(system);
    print_experiment_summary();
    save_experiment_log("gaia_v7_session.json");
    
    function_registry_cleanup();
    cleanup_experiment_logger();
    
    // Basic cleanup
    printf("GAIA V7 session ended.\n");
    return 0;
}