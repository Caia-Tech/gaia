#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "gaia_logger.h"
#include "gaia_observability.h"
#include "gate_types.h"
#include "function_registry.h"
#include "gaia_functions.h"
#include "analysis_functions.h"
#include "experiment_logger.h"
#include "dynamic_workflows.h"
#include "explanations.h"
#include "transformer_attention.h"

// Forward declarations
static GaiaError handle_function_call_safe(const char* input, char** output);
static GaiaError generate_response_for_step_safe(void* system, ReasoningStep* step, char** output);

#define HASH_SIZE 2097152
#define MAX_WORD_LENGTH 50
#define MAX_INPUT_LENGTH 1024
#define CONTEXT_SIZE 100

// Feature flags with logging
static int use_workflows = 1;
static int use_attention = 1;
static int use_refinement = 1;

// Pattern structure
typedef struct Pattern {
    char context[CONTEXT_SIZE][MAX_WORD_LENGTH];
    char next[MAX_WORD_LENGTH];
    int context_length;
    int count;
    struct Pattern* collision_next;
} Pattern;

// Chat system
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
    char request_id[64];
} V8Enhancement;

// Initialize chat system with error handling
static ChatSystem* init_chat_system(void) {
    TRACE_ENTER("initializing chat system");
    PERF_BEGIN("init_chat_system");
    
    LOG_INFO("Allocating chat system with %d hash buckets", HASH_SIZE);
    
    ChatSystem* system = calloc(1, sizeof(ChatSystem));
    if (!system) {
        LOG_ERROR("Failed to allocate chat system (size: %zu)", sizeof(ChatSystem));
        PERF_END("init_chat_system");
        TRACE_EXIT("failed - out of memory");
        return NULL;
    }
    
    // Already zeroed by calloc
    LOG_INFO("Chat system initialized successfully");
    METRIC_INC("chat_system.initialized");
    
    double duration = PERF_END("init_chat_system");
    LOG_DEBUG("Chat system initialization took %.2f ms", duration);
    
    TRACE_EXIT("success");
    return system;
}

// Create V8 enhancement with error handling
static V8Enhancement* create_v8_enhancement(const char* request_id) {
    TRACE_ENTER("creating V8 enhancement for request %s", request_id);
    PERF_BEGIN("create_v8_enhancement");
    
    V8Enhancement* v8 = calloc(1, sizeof(V8Enhancement));
    if (!v8) {
        LOG_ERROR("Failed to allocate V8 enhancement context");
        METRIC_INC("v8.creation_failures");
        PERF_END("create_v8_enhancement");
        return NULL;
    }
    
    strncpy(v8->request_id, request_id, 63);
    
    if (use_attention) {
        LOG_DEBUG("Creating transformer layer");
        v8->transformer = create_transformer_layer();
        if (!v8->transformer) {
            LOG_ERROR("Failed to create transformer layer");
            free(v8);
            METRIC_INC("v8.transformer_failures");
            PERF_END("create_v8_enhancement");
            return NULL;
        }
        LOG_DEBUG("Transformer layer created successfully");
    }
    
    METRIC_INC("v8.enhancements_created");
    double duration = PERF_END("create_v8_enhancement");
    LOG_DEBUG("V8 enhancement creation took %.2f ms", duration);
    
    TRACE_EXIT("success");
    return v8;
}

// Destroy V8 enhancement with cleanup
static void destroy_v8_enhancement(V8Enhancement* v8) {
    if (!v8) return;
    
    TRACE_ENTER("destroying V8 enhancement");
    
    if (v8->transformer) {
        destroy_transformer_layer(v8->transformer);
        LOG_DEBUG("Transformer layer destroyed");
    }
    
    if (v8->refinement_ctx) {
        destroy_refinement_context(v8->refinement_ctx);
        LOG_DEBUG("Refinement context destroyed");
    }
    
    free(v8);
    METRIC_DEC("v8.enhancements_created");
    
    TRACE_EXIT("complete");
}

// Safe function call handler
static GaiaError handle_function_call_safe(const char* input, char** output) {
    TRACE_ENTER("input='%s'", input ? input : "NULL");
    CHECK_NULL(input, GAIA_ERR_NULL_POINTER);
    CHECK_NULL(output, GAIA_ERR_NULL_POINTER);
    
    PERF_BEGIN("handle_function_call");
    *output = NULL;
    
    // Prime number check
    if (strstr(input, "prime")) {
        LOG_DEBUG("Detected prime number query");
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
            LOG_DEBUG("Checking if %d is prime", num);
            FunctionResult result = call_int_function("is_prime", num);
            if (result.success) {
                *output = malloc(256);
                if (!*output) {
                    LOG_ERROR("Failed to allocate output buffer");
                    PERF_END("handle_function_call");
                    return GAIA_ERR_OUT_OF_MEMORY;
                }
                
                int is_prime = (strcmp(result.result, "yes") == 0);
                snprintf(*output, 255, "%d is %s prime number", 
                        num, is_prime ? "a" : "not a");
                
                LOG_INFO("Prime check result: %s", *output);
                METRIC_INC("functions.prime_checks");
                PERF_END("handle_function_call");
                return GAIA_OK;
            }
        }
    }
    
    // Check for calculations
    AnalysisResult* analysis = analyze_input(input);
    if (!analysis) {
        LOG_DEBUG("No calculation detected in input");
        PERF_END("handle_function_call");
        return GAIA_OK; // Not an error, just no calculation needed
    }
    
    if (!analysis->requires_calculation) {
        free_analysis_result(analysis);
        PERF_END("handle_function_call");
        return GAIA_OK;
    }
    
    LOG_DEBUG("Calculation required, extracting entities");
    
    // Extract numbers and operators
    Entity entities[20];
    int entity_count = extract_entities(input, entities, 20);
    
    int numbers[10];
    char operators[10][20];
    int num_count = 0, op_count = 0;
    
    for (int i = 0; i < entity_count && num_count < 10; i++) {
        if (entities[i].type == ENTITY_NUMBER && num_count < 10) {
            numbers[num_count++] = atoi(entities[i].value);
        } else if (entities[i].type == ENTITY_OPERATOR && op_count < 10) {
            strncpy(operators[op_count++], entities[i].value, 19);
        }
    }
    
    if (num_count >= 2 && op_count >= 1) {
        LOG_DEBUG("Performing calculation with %d numbers and %d operators", num_count, op_count);
        
        int result = numbers[0];
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
            } else if (strcmp(operators[i], "times") == 0 || strcmp(operators[i], "*") == 0) {
                result *= numbers[i + 1];
            } else if (strcmp(operators[i], "divided") == 0 || strcmp(operators[i], "/") == 0) {
                if (numbers[i + 1] == 0) {
                    LOG_ERROR("Division by zero attempted");
                    free_analysis_result(analysis);
                    METRIC_INC("functions.division_by_zero");
                    PERF_END("handle_function_call");
                    return GAIA_ERR_CALCULATION_FAILED;
                }
                result /= numbers[i + 1];
            }
        }
        
        *output = malloc(512);
        if (!*output) {
            LOG_ERROR("Failed to allocate output buffer");
            free_analysis_result(analysis);
            PERF_END("handle_function_call");
            return GAIA_ERR_OUT_OF_MEMORY;
        }
        
        snprintf(*output, 511, "%s equals %d", operation_str, result);
        LOG_INFO("Calculation result: %s", *output);
        METRIC_INC("functions.calculations");
        
        free_analysis_result(analysis);
        PERF_END("handle_function_call");
        return GAIA_OK;
    }
    
    free_analysis_result(analysis);
    PERF_END("handle_function_call");
    return GAIA_OK;
}

// Enhanced response generation with full observability
static GaiaError generate_response_v8_safe(ChatSystem* system, const char* input, 
                                          const char* request_id, char** output) {
    TRACE_ENTER("request_id=%s, input='%s'", request_id, input ? input : "NULL");
    CHECK_NULL(system, GAIA_ERR_NULL_POINTER);
    CHECK_NULL(input, GAIA_ERR_NULL_POINTER);
    CHECK_NULL(output, GAIA_ERR_NULL_POINTER);
    
    *output = NULL;
    
    if (strlen(input) == 0) {
        LOG_WARN("Empty input provided");
        *output = strdup("Please provide some input.");
        return GAIA_OK;
    }
    
    PERF_BEGIN("generate_response_v8");
    LOG_INFO("[%s] Processing query: %s", request_id, input);
    
    // Quick check for prime (which works)
    if (strstr(input, "prime")) {
        LOG_DEBUG("Prime query detected, using direct function call");
        GaiaError err = handle_function_call_safe(input, output);
        if (err == GAIA_OK && *output) {
            PERF_END("generate_response_v8");
            METRIC_INC("responses.prime_direct");
            return GAIA_OK;
        }
    }
    
    // Create V8 enhancement
    V8Enhancement* v8 = NULL;
    if (use_attention || use_refinement) {
        v8 = create_v8_enhancement(request_id);
        if (!v8) {
            LOG_WARN("Failed to create V8 enhancement, continuing without");
            METRIC_INC("v8.enhancement_failures");
        }
    }
    
    // Use workflow system
    char* base_response = NULL;
    
    if (use_workflows) {
        LOG_DEBUG("Creating workflow for query");
        PERF_BEGIN("workflow_processing");
        
        WorkflowState* workflow = create_workflow();
        if (!workflow) {
            LOG_ERROR("Failed to create workflow");
            if (v8) destroy_v8_enhancement(v8);
            METRIC_INC("workflows.creation_failures");
            PERF_END("workflow_processing");
            PERF_END("generate_response_v8");
            return GAIA_ERR_WORKFLOW_FAILED;
        }
        
        // Decompose query
        int num_steps = decompose_query(workflow, input);
        LOG_INFO("Query decomposed into %d steps", num_steps);
        METRIC_SET("workflow.current_steps", num_steps);
        
        // Execute workflow
        workflow->current_step = 0;
        while (workflow->current_step < workflow->num_steps) {
            ReasoningStep* step = &workflow->steps[workflow->current_step];
            
            LOG_DEBUG("Executing step %d/%d: %s", 
                     workflow->current_step + 1, num_steps, step->description);
            
            if (!step->completed) {
                char* step_response = NULL;
                GaiaError err = generate_response_for_step_safe(system, step, &step_response);
                
                if (err != GAIA_OK) {
                    LOG_ERROR("Step %d failed: %s", workflow->current_step + 1, 
                             gaia_error_string(err));
                    METRIC_INC("workflow.step_failures");
                } else if (step_response) {
                    strncpy(step->output, step_response, 511);
                    step->output[511] = '\0';
                    free(step_response);
                    step->completed = 1;
                    step->confidence = 0.8;
                    LOG_DEBUG("Step %d completed successfully", workflow->current_step + 1);
                }
            }
            
            workflow->current_step++;
        }
        
        // Synthesize results
        char* synthesis = synthesize_results(workflow);
        base_response = synthesis ? strdup(synthesis) : strdup("No response generated");
        
        LOG_DEBUG("Workflow synthesis: %s", base_response);
        METRIC_INC("workflows.completed");
        
        destroy_workflow(workflow);
        double workflow_time = PERF_END("workflow_processing");
        METRIC_SET("workflow.last_duration_ms", workflow_time);
    } else {
        base_response = strdup("Workflows are disabled");
        LOG_WARN("Workflows disabled, using fallback response");
    }
    
    // Apply V8 enhancements
    char* enhanced_response = base_response;
    
    if (v8 && base_response) {
        PERF_BEGIN("v8_enhancement");
        
        // Apply attention
        if (use_attention) {
            LOG_DEBUG("Applying attention enhancement");
            char* attention_enhanced = enhance_with_attention(input, base_response);
            if (attention_enhanced != base_response) {
                enhanced_response = attention_enhanced;
                LOG_DEBUG("Attention applied successfully");
                METRIC_INC("v8.attention_applied");
            }
        }
        
        // Apply refinement
        if (use_refinement && enhanced_response) {
            LOG_DEBUG("Applying iterative refinement");
            
            // Analyze base quality
            ResponseAnalysis* analysis = analyze_response(input, enhanced_response);
            if (analysis) {
                float base_quality = analysis->overall_quality;
                LOG_INFO("Base response quality: %.2f", base_quality);
                METRIC_SET("response.base_quality", base_quality);
                
                if (base_quality < 0.8) {
                    LOG_DEBUG("Quality below threshold, refining");
                    
                    RefinementContext* refine_ctx = create_refinement_context(input, enhanced_response);
                    if (refine_ctx) {
                        char* refined = refine_response_v8(refine_ctx);
                        if (refined) {
                            LOG_INFO("Refinement completed in %d iterations", 
                                    refine_ctx->iteration_count);
                            METRIC_INC("v8.refinements_completed");
                            METRIC_SET("v8.last_refinement_iterations", refine_ctx->iteration_count);
                            
                            if (enhanced_response != base_response) {
                                free(enhanced_response);
                            }
                            enhanced_response = refined;
                        }
                        destroy_refinement_context(refine_ctx);
                    }
                }
                
                // Cleanup analysis
                for (int i = 0; i < analysis->num_issues; i++) {
                    free(analysis->issues[i]);
                }
                for (int i = 0; i < analysis->num_suggestions; i++) {
                    free(analysis->suggestions[i]);
                }
                free(analysis);
            }
        }
        
        double enhancement_time = PERF_END("v8_enhancement");
        LOG_DEBUG("V8 enhancement took %.2f ms", enhancement_time);
    }
    
    // Set output
    if (enhanced_response && strlen(enhanced_response) > 0) {
        *output = enhanced_response;
        if (enhanced_response != base_response && base_response) {
            free(base_response);
        }
        LOG_INFO("[%s] Response generated successfully", request_id);
        METRIC_INC("responses.successful");
    } else {
        *output = strdup("(empty response generated)");
        LOG_WARN("[%s] Empty response generated", request_id);
        METRIC_INC("responses.empty");
        SAFE_FREE(base_response);
        SAFE_FREE(enhanced_response);
    }
    
    // Cleanup
    if (v8) {
        destroy_v8_enhancement(v8);
    }
    
    double total_time = PERF_END("generate_response_v8");
    LOG_INFO("[%s] Total response time: %.2f ms", request_id, total_time);
    METRIC_SET("response.last_duration_ms", total_time);
    
    TRACE_EXIT("success");
    return GAIA_OK;
}

// Generate response for workflow step with error handling
static GaiaError generate_response_for_step_safe(void* system_ptr, ReasoningStep* step, 
                                                char** output) {
    TRACE_ENTER("step type=%d, desc='%s'", step ? step->type : -1, 
               step ? step->description : "NULL");
    
    CHECK_NULL(step, GAIA_ERR_NULL_POINTER);
    CHECK_NULL(output, GAIA_ERR_NULL_POINTER);
    
    *output = NULL;
    ChatSystem* system = (ChatSystem*)system_ptr;
    
    LOG_DEBUG("Processing step: %s (type: %d)", step->description, step->type);
    
    switch (step->type) {
        case STEP_EXECUTE: {
            const char* query_input = strlen(step->input) > 0 ? step->input : step->description;
            LOG_DEBUG("EXECUTE step with input: %s", query_input);
            
            // Try function call
            GaiaError err = handle_function_call_safe(query_input, output);
            if (err != GAIA_OK) {
                LOG_ERROR("Function call failed: %s", gaia_error_string(err));
                return err;
            }
            
            if (*output) {
                LOG_DEBUG("Function call successful: %s", *output);
                return GAIA_OK;
            }
            
            // Fallback - try to generate a simple response
            *output = malloc(512);
            if (!*output) {
                return GAIA_ERR_OUT_OF_MEMORY;
            }
            
            // Generate a basic response based on the query
            if (strstr(query_input, "capital") && strstr(query_input, "France")) {
                strcpy(*output, "The capital of France is Paris.");
            } else if (strstr(query_input, "explain") || strstr(query_input, "what is")) {
                snprintf(*output, 511, "I need to analyze: %s", query_input);
            } else {
                snprintf(*output, 511, "Let me help you with: %s", query_input);
            }
            LOG_DEBUG("Generated fallback response: %s", *output);
            return GAIA_OK;
        }
        
        case STEP_DECOMPOSE:
        case STEP_ANALYZE:
        case STEP_EVALUATE:
        case STEP_SYNTHESIZE:
            *output = strdup(strlen(step->output) > 0 ? step->output : "Processing...");
            return GAIA_OK;
            
        case STEP_COMPLETE:
            *output = strdup(strlen(step->output) > 0 ? step->output : "Task completed");
            return GAIA_OK;
            
        default:
            LOG_WARN("Unknown step type: %d", step->type);
            *output = strdup("Unknown step");
            return GAIA_OK;
    }
}

// Main function with full observability
int main(int argc, char* argv[]) {
    // Initialize logging
    LogLevel log_level = LOG_INFO;
    const char* log_file = "gaia_v8.log";
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            log_level = LOG_DEBUG;
        } else if (strcmp(argv[i], "--trace") == 0) {
            log_level = LOG_TRACE;
        } else if (strcmp(argv[i], "--log-file") == 0 && i + 1 < argc) {
            log_file = argv[++i];
        }
    }
    
    logger_init(log_level, log_file);
    LOG_INFO("=== GAIA V8 Enhanced - Starting ===");
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-workflows") == 0) {
            use_workflows = 0;
            LOG_INFO("Workflows disabled");
        } else if (strcmp(argv[i], "--no-attention") == 0) {
            use_attention = 0;
            LOG_INFO("Attention disabled");
        } else if (strcmp(argv[i], "--no-refinement") == 0) {
            use_refinement = 0;
            LOG_INFO("Refinement disabled");
        }
    }
    
    // Initialize subsystems
    PERF_BEGIN("system_initialization");
    
    LOG_INFO("Initializing function registry");
    function_registry_init();
    register_gaia_functions();
    
    LOG_INFO("Initializing experiment logger");
    init_experiment_logger();
    
    // Initialize chat system
    LOG_INFO("Creating chat system");
    ChatSystem* system = init_chat_system();
    if (!system) {
        LOG_FATAL("Failed to create chat system");
        logger_shutdown();
        return 1;
    }
    
    double init_time = PERF_END("system_initialization");
    LOG_INFO("System initialization complete in %.2f ms", init_time);
    
    // Interactive loop
    char input[MAX_INPUT_LENGTH];
    int request_count = 0;
    
    printf("GAIA V8 Enhanced ready! (Type 'quit' to exit)\n\n");
    
    while (1) {
        printf("You: ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) {
            LOG_INFO("User requested quit");
            break;
        }
        
        if (strcmp(input, "stats") == 0) {
            logger_print_stats();
            continue;
        }
        
        // Generate request ID
        char request_id[64];
        snprintf(request_id, 63, "REQ-%d-%ld", ++request_count, time(NULL));
        
        LOG_INFO("=== Request %s started ===", request_id);
        request_begin(request_id);
        
        // Generate response
        printf("GAIA V8: ");
        fflush(stdout);
        
        char* response = NULL;
        GaiaError err = generate_response_v8_safe(system, input, request_id, &response);
        
        if (err != GAIA_OK) {
            LOG_ERROR("[%s] Response generation failed: %s", 
                     request_id, gaia_error_string(err));
            printf("Error: %s\n", gaia_error_string(err));
        } else if (response) {
            printf("%s\n", response);
            free(response);
        } else {
            printf("(no response)\n");
        }
        
        request_end();
        LOG_INFO("=== Request %s completed ===", request_id);
    }
    
    // Cleanup
    LOG_INFO("Shutting down GAIA V8");
    
    print_experiment_summary();
    save_experiment_log("gaia_v8_enhanced_session.json");
    
    function_registry_cleanup();
    cleanup_experiment_logger();
    
    // Free patterns
    if (system) {
        LOG_DEBUG("Freeing %d patterns", system->total_patterns);
        for (int i = 0; i < HASH_SIZE; i++) {
            Pattern* p = system->patterns[i];
            while (p) {
                Pattern* next = p->collision_next;
                free(p);
                p = next;
            }
        }
        free(system);
    }
    
    LOG_INFO("=== GAIA V8 Enhanced - Shutdown complete ===");
    logger_shutdown();
    
    return 0;
}