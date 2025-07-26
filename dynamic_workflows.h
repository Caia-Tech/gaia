#ifndef DYNAMIC_WORKFLOWS_H
#define DYNAMIC_WORKFLOWS_H

#include "analysis_functions.h"

// Configuration
#define MAX_REASONING_STEPS 20
#define MAX_CONTEXT_DEPTH 10
#define WORKING_MEMORY_SIZE 4096
#define MAX_ITERATIONS 10
#define CONFIDENCE_THRESHOLD 0.7

// Reasoning step types
typedef enum {
    STEP_DECOMPOSE,      // Break down complex query
    STEP_ANALYZE,        // Analyze sub-problem  
    STEP_EXECUTE,        // Execute action
    STEP_EVALUATE,       // Evaluate result
    STEP_SYNTHESIZE,     // Combine results
    STEP_BACKTRACK,      // Revise previous step
    STEP_COMPLETE        // Task completed
} StepType;

// Individual reasoning step
typedef struct {
    StepType type;
    char description[256];
    char input[512];
    char output[512];
    float confidence;
    int completed;
    int iteration_count;
} ReasoningStep;

// Context frame for nested reasoning
typedef struct {
    char context_name[128];
    char local_memory[1024];
    int pattern_filter[100];  // Which patterns are relevant
    float relevance_threshold;
} ContextFrame;

// Workflow state
typedef struct {
    // Reasoning steps
    ReasoningStep steps[MAX_REASONING_STEPS];
    int num_steps;
    int current_step;
    
    // Working memory
    char working_memory[WORKING_MEMORY_SIZE];
    int memory_used;
    
    // Context stack
    ContextFrame context_stack[MAX_CONTEXT_DEPTH];
    int context_depth;
    
    // Iteration tracking
    int total_iterations;
    int backtrack_count;
    
    // Quality metrics
    float overall_confidence;
    int coherence_score;
} WorkflowState;

// Multi-head reasoning paths
typedef struct {
    char path_name[64];
    ReasoningStep steps[MAX_REASONING_STEPS];
    int num_steps;
    float confidence;
    int active;
} ReasoningPath;

// Transformer-like attention
typedef struct {
    float relevance_scores[MAX_REASONING_STEPS][MAX_REASONING_STEPS];
    int position_encoding[MAX_REASONING_STEPS];
    float layer_norm_factor;
} AttentionMechanism;

// Core workflow functions
WorkflowState* create_workflow(void);
void destroy_workflow(WorkflowState* workflow);

// Reasoning operations
int decompose_query(WorkflowState* workflow, const char* input);
int execute_reasoning_step(WorkflowState* workflow);
int evaluate_step_result(WorkflowState* workflow, int step_index);
int should_backtrack(WorkflowState* workflow);
int perform_backtrack(WorkflowState* workflow);

// Context management  
int push_context(WorkflowState* workflow, const char* context_name);
int pop_context(WorkflowState* workflow);
void set_relevance_filter(WorkflowState* workflow, float threshold);
char* get_working_memory(WorkflowState* workflow);

// Iterative refinement
int iterate_on_response(WorkflowState* workflow, const char* response);
float calculate_response_quality(const char* query, const char* response);
int needs_refinement(WorkflowState* workflow);
char* refine_response(WorkflowState* workflow, const char* response);

// Multi-step workflows
int plan_workflow(WorkflowState* workflow, AnalysisResult* analysis);
int execute_workflow(WorkflowState* workflow);
char* synthesize_results(WorkflowState* workflow);
void update_workflow_progress(WorkflowState* workflow);

// Transformer-like operations
void apply_self_attention(WorkflowState* workflow, AttentionMechanism* attention);
void encode_positions(WorkflowState* workflow, AttentionMechanism* attention);
void normalize_reasoning_paths(ReasoningPath paths[], int num_paths);
char* merge_reasoning_paths(ReasoningPath paths[], int num_paths);

// Memory consolidation
int consolidate_to_patterns(WorkflowState* workflow, void* chat_system);
int extract_key_insights(WorkflowState* workflow, char insights[][256], int max_insights);

// Debugging and logging
void print_workflow_state(WorkflowState* workflow);
void log_reasoning_step(ReasoningStep* step);
char* workflow_to_string(WorkflowState* workflow);

#endif // DYNAMIC_WORKFLOWS_H