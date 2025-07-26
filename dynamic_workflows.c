#include "dynamic_workflows.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// Helper function for lowercase conversion
static void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Create a new workflow state
WorkflowState* create_workflow(void) {
    WorkflowState* workflow = calloc(1, sizeof(WorkflowState));
    if (!workflow) return NULL;
    
    // Initialize working memory
    workflow->memory_used = 0;
    strcpy(workflow->working_memory, "");
    
    // Initialize context stack
    workflow->context_depth = 0;
    
    // Initialize metrics
    workflow->overall_confidence = 0.0;
    workflow->coherence_score = 0;
    workflow->total_iterations = 0;
    workflow->backtrack_count = 0;
    
    return workflow;
}

// Destroy workflow and free memory
void destroy_workflow(WorkflowState* workflow) {
    if (workflow) {
        free(workflow);
    }
}

// Decompose complex query into steps
int decompose_query(WorkflowState* workflow, const char* input) {
    if (!workflow || !input) return 0;
    
    // Use V6 analysis to understand the query
    AnalysisResult* analysis = analyze_input(input);
    if (!analysis) return 0;
    
    workflow->num_steps = 0;
    
    // Create lowercase version for checking
    char lower_input[512];
    strncpy(lower_input, input, 511);
    lower_input[511] = '\0';
    to_lowercase(lower_input);
    
    // Check for multi-part questions (contains "and", commas, multiple "?")
    int question_marks = 0;
    int has_and = 0;
    int has_comma = 0;
    
    for (int i = 0; input[i]; i++) {
        if (input[i] == '?') question_marks++;
        if (input[i] == ',') has_comma++;
    }
    if (strstr(input, " and ")) has_and = 1;
    
    // Check if query explicitly mentions "also" or "and" for multi-part
    int has_also = strstr(input, "also") != NULL || strstr(input, "Also") != NULL;
    
    // Multi-part decomposition
    if (question_marks > 1 || (has_and && analysis->prompt_type == PROMPT_QUESTION) || has_also || has_comma > 1) {
        // Handle comma-separated calculations first
        if (has_comma > 0 && strstr(input, "Calculate")) {
            char temp[512];
            strncpy(temp, input, 511);
            temp[511] = '\0';
            
            // Skip "Calculate" prefix
            char* calc_start = strstr(temp, "Calculate");
            if (calc_start) {
                calc_start += 9; // Skip "Calculate"
                while (*calc_start == ' ') calc_start++;
                
                char* part = strtok(calc_start, ",");
                while (part && workflow->num_steps < MAX_REASONING_STEPS - 2) {
                    if (strlen(part) > 2) {
                        ReasoningStep* step = &workflow->steps[workflow->num_steps];
                        step->type = STEP_EXECUTE;
                        snprintf(step->description, 255, "Calculate part %d", workflow->num_steps + 1);
                        strncpy(step->input, part, 511);
                        workflow->num_steps++;
                    }
                    part = strtok(NULL, ",");
                }
            }
        }
        // Handle question marks
        else if (question_marks > 1) {
            char temp[512];
            strncpy(temp, input, 511);
            temp[511] = '\0';
            
            char* part = strtok(temp, "?");
            while (part && workflow->num_steps < MAX_REASONING_STEPS - 2) {
                if (strlen(part) > 3) {  // Skip empty parts
                    ReasoningStep* step = &workflow->steps[workflow->num_steps];
                    step->type = STEP_ANALYZE;
                    snprintf(step->description, 255, "Answer sub-question %d", workflow->num_steps + 1);
                    snprintf(step->input, 511, "%s?", part);
                    step->confidence = 0.0;
                    step->completed = 0;
                    workflow->num_steps++;
                }
                part = strtok(NULL, "?");
            }
        }
        // Handle "and" separator
        else if (has_and) {
            // Split by "and"
            char temp[512];
            strncpy(temp, input, 511);
            temp[511] = '\0';
            
            char* part = strstr(temp, " and ");
            if (part) {
                *part = '\0';
                // First part
                ReasoningStep* step = &workflow->steps[workflow->num_steps];
                step->type = STEP_EXECUTE;
                strcpy(step->description, "Process first part");
                strncpy(step->input, temp, 511);
                workflow->num_steps++;
                
                // Second part
                part += 5; // Skip " and "
                step = &workflow->steps[workflow->num_steps];
                step->type = STEP_EXECUTE;
                strcpy(step->description, "Process second part");
                strncpy(step->input, part, 511);
                workflow->num_steps++;
            }
        }
    }
    // List-based decomposition
    else if (analysis->requires_list) {
        ReasoningStep* step = &workflow->steps[workflow->num_steps];
        step->type = STEP_DECOMPOSE;
        strcpy(step->description, "Identify list requirements");
        strcpy(step->input, input);
        workflow->num_steps++;
        
        // Add execution step
        step = &workflow->steps[workflow->num_steps];
        step->type = STEP_EXECUTE;
        strcpy(step->description, "Generate list items");
        workflow->num_steps++;
    }
    // Calculation decomposition
    else if (analysis->requires_calculation) {
        // Step 1: Extract numbers and operations
        ReasoningStep* step = &workflow->steps[workflow->num_steps];
        step->type = STEP_ANALYZE;
        strcpy(step->description, "Extract mathematical elements");
        strcpy(step->input, input);
        workflow->num_steps++;
        
        // Step 2: Perform calculation
        step = &workflow->steps[workflow->num_steps];
        step->type = STEP_EXECUTE;
        strcpy(step->description, "Perform calculation");
        strcpy(step->input, input);  // Pass original input
        workflow->num_steps++;
        
        // Step 3: Verify result
        step = &workflow->steps[workflow->num_steps];
        step->type = STEP_EVALUATE;
        strcpy(step->description, "Verify calculation result");
        workflow->num_steps++;
    }
    // Complex reasoning decomposition (check early)
    else if (strstr(lower_input, "how") || strstr(lower_input, "why") || strstr(lower_input, "explain") ||
             strstr(lower_input, "what is") || strstr(lower_input, "what does")) {
        // Step 1: Understand the concept
        ReasoningStep* step = &workflow->steps[workflow->num_steps];
        step->type = STEP_ANALYZE;
        strcpy(step->description, "Understand core concept");
        strcpy(step->input, input);
        workflow->num_steps++;
        
        // Step 2: Generate explanation
        step = &workflow->steps[workflow->num_steps];
        step->type = STEP_EXECUTE;
        strcpy(step->description, "Generate detailed explanation");
        strcpy(step->input, input);  // Pass original input
        workflow->num_steps++;
        
        // Step 3: Simplify if needed
        step = &workflow->steps[workflow->num_steps];
        step->type = STEP_EVALUATE;
        strcpy(step->description, "Check explanation clarity");
        workflow->num_steps++;
    }
    else {
        // Simple single-step query
        ReasoningStep* step = &workflow->steps[workflow->num_steps];
        step->type = STEP_EXECUTE;
        strcpy(step->description, "Process query directly");
        strcpy(step->input, input);
        workflow->num_steps++;
    }
    
    // Always add synthesis step at the end
    if (workflow->num_steps > 1) {
        ReasoningStep* step = &workflow->steps[workflow->num_steps];
        step->type = STEP_SYNTHESIZE;
        strcpy(step->description, "Combine results into final response");
        workflow->num_steps++;
    }
    
    // Add completion step
    ReasoningStep* step = &workflow->steps[workflow->num_steps];
    step->type = STEP_COMPLETE;
    strcpy(step->description, "Finalize response");
    workflow->num_steps++;
    
    workflow->current_step = 0;
    free_analysis_result(analysis);
    
    return workflow->num_steps;
}

// Execute current reasoning step
int execute_reasoning_step(WorkflowState* workflow) {
    if (!workflow || workflow->current_step >= workflow->num_steps) return 0;
    
    ReasoningStep* step = &workflow->steps[workflow->current_step];
    step->iteration_count++;
    workflow->total_iterations++;
    
    switch (step->type) {
        case STEP_DECOMPOSE:
            // Further decomposition if needed
            snprintf(step->output, 511, "Decomposed into %d sub-tasks", workflow->num_steps);
            step->confidence = 0.9;
            step->completed = 1;
            break;
            
        case STEP_ANALYZE:
            // Analyze the sub-problem
            if (strlen(step->input) > 0) {
                // Use V6 analysis
                AnalysisResult* analysis = analyze_input(step->input);
                snprintf(step->output, 511, "Type: %s, Topic: %s, Entities: %d",
                         prompt_type_to_string(analysis->prompt_type),
                         topic_type_to_string(analysis->topic),
                         analysis->num_entities);
                
                // Store key info in working memory
                char memo[256];
                snprintf(memo, 255, "[Step %d] %s\n", workflow->current_step, step->output);
                strcat(workflow->working_memory, memo);
                
                step->confidence = 0.8;
                step->completed = 1;
                free_analysis_result(analysis);
            }
            break;
            
        case STEP_EXECUTE:
            // Execute the actual task
            strcpy(step->output, "Executed task");  // Placeholder
            step->confidence = 0.7;
            step->completed = 1;
            break;
            
        case STEP_EVALUATE:
            // Evaluate previous step's result
            if (workflow->current_step > 0) {
                ReasoningStep* prev = &workflow->steps[workflow->current_step - 1];
                step->confidence = calculate_response_quality(prev->input, prev->output);
                snprintf(step->output, 511, "Quality score: %.2f", step->confidence);
                step->completed = 1;
            }
            break;
            
        case STEP_SYNTHESIZE:
            // Combine all results
            strcpy(step->output, "");
            for (int i = 0; i < workflow->current_step; i++) {
                if (workflow->steps[i].completed && strlen(workflow->steps[i].output) > 0 &&
                    strcmp(workflow->steps[i].output, "Processing...") != 0 &&
                    strcmp(workflow->steps[i].output, "Workflow completed") != 0 &&
                    (workflow->steps[i].type == STEP_ANALYZE || workflow->steps[i].type == STEP_EXECUTE)) {
                    
                    if (strlen(step->output) > 0) {
                        strcat(step->output, " ");
                    }
                    strcat(step->output, workflow->steps[i].output);
                }
            }
            step->confidence = 0.85;
            step->completed = 1;
            break;
            
        case STEP_BACKTRACK:
            // Go back and revise
            workflow->backtrack_count++;
            if (workflow->current_step > 0) {
                workflow->current_step--;
                workflow->steps[workflow->current_step].completed = 0;
                workflow->steps[workflow->current_step].confidence = 0.0;
            }
            return 1;  // Special return to indicate backtrack
            
        case STEP_COMPLETE:
            step->completed = 1;
            step->confidence = 1.0;
            strcpy(step->output, "Workflow completed");
            break;
    }
    
    return 1;
}

// Evaluate if a step's result is satisfactory
int evaluate_step_result(WorkflowState* workflow, int step_index) {
    if (!workflow || step_index >= workflow->num_steps) return 0;
    
    ReasoningStep* step = &workflow->steps[step_index];
    
    // Check confidence threshold
    if (step->confidence < CONFIDENCE_THRESHOLD) {
        return 0;  // Needs improvement
    }
    
    // Check output validity
    if (strlen(step->output) == 0) {
        return 0;  // Empty output
    }
    
    // Check iteration limit
    if (step->iteration_count > MAX_ITERATIONS / 2) {
        return 1;  // Accept to avoid infinite loop
    }
    
    return 1;  // Satisfactory
}

// Determine if backtracking is needed
int should_backtrack(WorkflowState* workflow) {
    if (!workflow || workflow->current_step == 0) return 0;
    
    ReasoningStep* current = &workflow->steps[workflow->current_step];
    
    // Backtrack if confidence too low after multiple attempts
    if (current->confidence < 0.3 && current->iteration_count > 2) {
        return 1;
    }
    
    // Backtrack if evaluation step found issues
    if (current->type == STEP_EVALUATE && current->confidence < CONFIDENCE_THRESHOLD) {
        return 1;
    }
    
    // Limit backtracking to prevent infinite loops
    if (workflow->backtrack_count >= MAX_ITERATIONS / 2) {
        return 0;
    }
    
    return 0;
}

// Perform backtrack operation
int perform_backtrack(WorkflowState* workflow) {
    if (!workflow || workflow->current_step == 0) return 0;
    
    workflow->backtrack_count++;
    
    // Add backtrack step
    if (workflow->num_steps < MAX_REASONING_STEPS - 1) {
        ReasoningStep* backtrack = &workflow->steps[workflow->num_steps];
        backtrack->type = STEP_BACKTRACK;
        snprintf(backtrack->description, 255, "Backtrack from step %d", workflow->current_step);
        workflow->num_steps++;
    }
    
    // Reset current step
    workflow->current_step = (workflow->current_step > 0) ? workflow->current_step - 1 : 0;
    workflow->steps[workflow->current_step].completed = 0;
    workflow->steps[workflow->current_step].iteration_count = 0;
    
    // Clear affected working memory
    char memo[256];
    snprintf(memo, 255, "[BACKTRACK] Revising step %d\n", workflow->current_step);
    strcat(workflow->working_memory, memo);
    
    return 1;
}

// Push new context onto stack
int push_context(WorkflowState* workflow, const char* context_name) {
    if (!workflow || workflow->context_depth >= MAX_CONTEXT_DEPTH - 1) return 0;
    
    ContextFrame* frame = &workflow->context_stack[workflow->context_depth];
    strncpy(frame->context_name, context_name, 127);
    frame->context_name[127] = '\0';
    frame->relevance_threshold = 0.5;
    
    workflow->context_depth++;
    return 1;
}

// Pop context from stack
int pop_context(WorkflowState* workflow) {
    if (!workflow || workflow->context_depth == 0) return 0;
    
    workflow->context_depth--;
    return 1;
}

// Get working memory contents
char* get_working_memory(WorkflowState* workflow) {
    if (!workflow) return NULL;
    return workflow->working_memory;
}

// Calculate response quality score
float calculate_response_quality(const char* query, const char* response) {
    if (!query || !response) return 0.0;
    
    float quality = 0.5;  // Base score
    
    // Length check
    int response_len = strlen(response);
    if (response_len < 3) {
        quality -= 0.3;  // Way too short
    } else if (response_len < 10) {
        quality -= 0.1;  // Short but might be okay for simple answers
    } else if (response_len > 500) {
        quality -= 0.1;  // Too long
    } else {
        quality += 0.1;  // Good length
    }
    
    // Relevance check (simple keyword matching)
    char query_lower[512], response_lower[512];
    strncpy(query_lower, query, 511);
    strncpy(response_lower, response, 511);
    to_lowercase(query_lower);
    to_lowercase(response_lower);
    
    // Count matching keywords
    char* word = strtok(query_lower, " ");
    int keyword_matches = 0;
    int total_keywords = 0;
    
    while (word) {
        total_keywords++;
        if (strlen(word) > 3 && strstr(response_lower, word)) {
            keyword_matches++;
        }
        word = strtok(NULL, " ");
    }
    
    if (total_keywords > 0) {
        quality += (float)keyword_matches / total_keywords * 0.3;
    }
    
    // Ensure quality is between 0 and 1
    if (quality < 0.0) quality = 0.0;
    if (quality > 1.0) quality = 1.0;
    
    return quality;
}

// Check if response needs refinement
int needs_refinement(WorkflowState* workflow) {
    if (!workflow) return 0;
    
    // Check overall confidence
    if (workflow->overall_confidence < CONFIDENCE_THRESHOLD) {
        return 1;
    }
    
    // Check if any critical steps have low confidence
    for (int i = 0; i < workflow->num_steps; i++) {
        if (workflow->steps[i].type == STEP_EXECUTE && 
            workflow->steps[i].confidence < CONFIDENCE_THRESHOLD) {
            return 1;
        }
    }
    
    return 0;
}

// Plan workflow based on analysis
int plan_workflow(WorkflowState* workflow, AnalysisResult* analysis) {
    if (!workflow || !analysis) return 0;
    
    // Use decompose_query for now
    return decompose_query(workflow, "");  // Input already analyzed
}

// Execute entire workflow
int execute_workflow(WorkflowState* workflow) {
    if (!workflow) return 0;
    
    workflow->current_step = 0;
    
    while (workflow->current_step < workflow->num_steps) {
        ReasoningStep* current = &workflow->steps[workflow->current_step];
        
        // Skip completed steps
        if (current->completed) {
            workflow->current_step++;
            continue;
        }
        
        // Execute current step
        int result = execute_reasoning_step(workflow);
        if (!result) {
            return 0;  // Execution failed
        }
        
        // Evaluate result
        if (!evaluate_step_result(workflow, workflow->current_step)) {
            if (should_backtrack(workflow)) {
                perform_backtrack(workflow);
                continue;  // Retry from earlier step
            }
        }
        
        // Update overall confidence
        float total_confidence = 0.0;
        int completed_steps = 0;
        for (int i = 0; i <= workflow->current_step; i++) {
            if (workflow->steps[i].completed) {
                total_confidence += workflow->steps[i].confidence;
                completed_steps++;
            }
        }
        if (completed_steps > 0) {
            workflow->overall_confidence = total_confidence / completed_steps;
        }
        
        workflow->current_step++;
    }
    
    return 1;  // Workflow completed
}

// Synthesize results from all steps
char* synthesize_results(WorkflowState* workflow) {
    if (!workflow) return NULL;
    
    static char final_response[2048];
    strcpy(final_response, "");
    
    // First check if we have a synthesis step with output
    for (int i = 0; i < workflow->num_steps; i++) {
        if (workflow->steps[i].type == STEP_SYNTHESIZE && 
            workflow->steps[i].completed &&
            strlen(workflow->steps[i].output) > 0 &&
            strcmp(workflow->steps[i].output, "Processing...") != 0) {
            
            strcpy(final_response, workflow->steps[i].output);
            return final_response;
        }
    }
    
    // Otherwise, collect all meaningful outputs
    int has_output = 0;
    for (int i = 0; i < workflow->num_steps; i++) {
        ReasoningStep* step = &workflow->steps[i];
        
        if (step->completed && strlen(step->output) > 0 &&
            strcmp(step->output, "Processing...") != 0 &&
            strcmp(step->output, "Workflow completed") != 0 &&
            step->type != STEP_COMPLETE) {
            
            // Add spacing between outputs
            if (has_output && strlen(final_response) > 0) {
                strcat(final_response, " ");
            }
            
            // For ANALYZE steps that are sub-questions, include the output
            if (step->type == STEP_ANALYZE || step->type == STEP_EXECUTE) {
                strcat(final_response, step->output);
                has_output = 1;
            }
        }
    }
    
    return final_response;
}

// Print workflow state for debugging
void print_workflow_state(WorkflowState* workflow) {
    if (!workflow) return;
    
    printf("\n=== Workflow State ===\n");
    printf("Total steps: %d\n", workflow->num_steps);
    printf("Current step: %d\n", workflow->current_step);
    printf("Iterations: %d\n", workflow->total_iterations);
    printf("Backtracks: %d\n", workflow->backtrack_count);
    printf("Overall confidence: %.2f\n", workflow->overall_confidence);
    
    printf("\nSteps:\n");
    for (int i = 0; i < workflow->num_steps; i++) {
        ReasoningStep* step = &workflow->steps[i];
        const char* type_names[] = {"DECOMPOSE", "ANALYZE", "EXECUTE", 
                                   "EVALUATE", "SYNTHESIZE", "BACKTRACK", "COMPLETE"};
        
        printf("%d. [%s] %s\n", i + 1, type_names[step->type], step->description);
        if (step->completed) {
            printf("   Status: COMPLETED (confidence: %.2f)\n", step->confidence);
            if (strlen(step->output) > 0) {
                printf("   Output: %s\n", step->output);
            }
        } else {
            printf("   Status: PENDING\n");
        }
    }
    
    printf("\nWorking Memory:\n%s\n", workflow->working_memory);
    printf("=====================\n\n");
}

