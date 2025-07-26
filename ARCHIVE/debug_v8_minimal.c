#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dynamic_workflows.h"
#include "analysis_functions.h"
#include "explanations.h"
#include "function_registry.h"
#include "gaia_functions.h"

// Minimal test to debug V8 workflow issue

int main() {
    printf("=== V8 Minimal Debug Test ===\n");
    
    // Initialize required systems
    printf("1. Initializing function registry...\n");
    function_registry_init();
    register_gaia_functions();
    
    printf("2. Creating workflow...\n");
    WorkflowState* workflow = create_workflow();
    if (!workflow) {
        printf("ERROR: Failed to create workflow\n");
        return 1;
    }
    
    printf("3. Testing query decomposition...\n");
    const char* query = "What is 2 plus 2?";
    printf("   Query: %s\n", query);
    
    int num_steps = decompose_query(workflow, query);
    printf("   Decomposed into %d steps\n", num_steps);
    
    // Print the steps
    for (int i = 0; i < num_steps; i++) {
        ReasoningStep* step = &workflow->steps[i];
        printf("   Step %d: %s (%s)\n", i + 1, 
               step->description,
               step->type == STEP_DECOMPOSE ? "DECOMPOSE" :
               step->type == STEP_ANALYZE ? "ANALYZE" :
               step->type == STEP_EXECUTE ? "EXECUTE" :
               step->type == STEP_EVALUATE ? "EVALUATE" :
               step->type == STEP_SYNTHESIZE ? "SYNTHESIZE" : "UNKNOWN");
    }
    
    printf("4. Testing workflow execution...\n");
    workflow->current_step = 0;
    
    // Try to execute first step
    if (num_steps > 0) {
        ReasoningStep* step = &workflow->steps[0];
        printf("   Executing step 1: %s\n", step->description);
        
        // This is where it might crash - let's be careful
        if (strlen(step->input) > 0) {
            printf("   Step input: %s\n", step->input);
        }
        
        // Try to analyze
        AnalysisResult* analysis = analyze_input(query);
        if (analysis) {
            printf("   Analysis successful\n");
            free_analysis_result(analysis);
        } else {
            printf("   Analysis returned NULL\n");
        }
    }
    
    printf("5. Cleaning up...\n");
    destroy_workflow(workflow);
    function_registry_cleanup();
    
    printf("Test completed successfully!\n");
    return 0;
}