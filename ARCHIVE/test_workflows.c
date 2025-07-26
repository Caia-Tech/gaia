#include <stdio.h>
#include <string.h>
#include "dynamic_workflows.h"
#include "analysis_functions.h"

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

// Test macro
#define RUN_TEST(test_func) do { \
    printf("Running %s...\n", #test_func); \
    tests_run++; \
    if (test_func()) { \
        tests_passed++; \
        printf("  ✓ Passed\n\n"); \
    } else { \
        printf("  ✗ Failed\n\n"); \
    } \
} while(0)

// Test basic workflow creation
int test_workflow_creation() {
    WorkflowState* workflow = create_workflow();
    if (!workflow) return 0;
    
    int success = (workflow->num_steps == 0 &&
                   workflow->current_step == 0 &&
                   workflow->context_depth == 0 &&
                   workflow->total_iterations == 0);
    
    destroy_workflow(workflow);
    return success;
}

// Test query decomposition for simple query
int test_simple_decomposition() {
    WorkflowState* workflow = create_workflow();
    
    const char* query = "What is 2 plus 2?";
    int num_steps = decompose_query(workflow, query);
    
    printf("  Query: '%s'\n", query);
    printf("  Generated %d steps\n", num_steps);
    
    // Should generate at least 3 steps for calculation
    int success = (num_steps >= 3);
    
    // Print steps for debugging
    for (int i = 0; i < workflow->num_steps; i++) {
        printf("  Step %d: %s\n", i + 1, workflow->steps[i].description);
    }
    
    destroy_workflow(workflow);
    return success;
}

// Test multi-part question decomposition
int test_multipart_decomposition() {
    WorkflowState* workflow = create_workflow();
    
    const char* query = "What is the capital of France? What is its population?";
    int num_steps = decompose_query(workflow, query);
    
    printf("  Multi-part query: '%s'\n", query);
    printf("  Generated %d steps\n", num_steps);
    
    // Should decompose into multiple analyze steps
    int analyze_steps = 0;
    for (int i = 0; i < workflow->num_steps; i++) {
        if (workflow->steps[i].type == STEP_ANALYZE) {
            analyze_steps++;
        }
        printf("  Step %d: %s\n", i + 1, workflow->steps[i].description);
    }
    
    int success = (analyze_steps >= 2);  // At least 2 sub-questions
    
    destroy_workflow(workflow);
    return success;
}

// Test reasoning step execution
int test_step_execution() {
    WorkflowState* workflow = create_workflow();
    
    // Manually create a simple workflow
    workflow->num_steps = 3;
    
    workflow->steps[0].type = STEP_ANALYZE;
    strcpy(workflow->steps[0].description, "Analyze input");
    strcpy(workflow->steps[0].input, "Test input");
    
    workflow->steps[1].type = STEP_EXECUTE;
    strcpy(workflow->steps[1].description, "Process task");
    
    workflow->steps[2].type = STEP_COMPLETE;
    strcpy(workflow->steps[2].description, "Finish");
    
    // Execute first step
    int result = execute_reasoning_step(workflow);
    
    printf("  Executed step 1: %s\n", workflow->steps[0].output);
    printf("  Confidence: %.2f\n", workflow->steps[0].confidence);
    
    int success = (result == 1 && 
                   workflow->steps[0].completed == 1 &&
                   workflow->steps[0].confidence > 0);
    
    destroy_workflow(workflow);
    return success;
}

// Test backtracking mechanism
int test_backtracking() {
    WorkflowState* workflow = create_workflow();
    
    // Set up a scenario that needs backtracking
    workflow->num_steps = 2;
    workflow->current_step = 1;
    
    workflow->steps[0].completed = 1;
    workflow->steps[0].confidence = 0.8;
    
    workflow->steps[1].completed = 0;
    workflow->steps[1].confidence = 0.2;  // Low confidence
    workflow->steps[1].iteration_count = 3;  // Multiple attempts
    
    int should_backtrack_result = should_backtrack(workflow);
    printf("  Should backtrack: %s\n", should_backtrack_result ? "YES" : "NO");
    
    if (should_backtrack_result) {
        perform_backtrack(workflow);
        printf("  After backtrack - current step: %d\n", workflow->current_step);
        printf("  Backtrack count: %d\n", workflow->backtrack_count);
    }
    
    int success = (should_backtrack_result == 1 && 
                   workflow->current_step == 0 &&
                   workflow->backtrack_count == 1);
    
    destroy_workflow(workflow);
    return success;
}

// Test context management
int test_context_management() {
    WorkflowState* workflow = create_workflow();
    
    // Push contexts
    int push1 = push_context(workflow, "Math context");
    int push2 = push_context(workflow, "Calculation subcontext");
    
    printf("  Context depth after pushes: %d\n", workflow->context_depth);
    printf("  Context 1: %s\n", workflow->context_stack[0].context_name);
    printf("  Context 2: %s\n", workflow->context_stack[1].context_name);
    
    // Pop context
    int pop1 = pop_context(workflow);
    printf("  Context depth after pop: %d\n", workflow->context_depth);
    
    int success = (push1 && push2 && pop1 && workflow->context_depth == 1);
    
    destroy_workflow(workflow);
    return success;
}

// Test quality evaluation
int test_quality_evaluation() {
    // Test various query-response pairs
    struct {
        const char* query;
        const char* response;
        float min_expected;
    } test_cases[] = {
        {"What is 2 plus 2?", "The answer is 4", 0.6},
        {"What is the capital of France?", "Paris", 0.4},
        {"Explain quantum physics", "xyz", 0.3},  // Too short
        {"Hello", "Hi there! How can I help you today?", 0.6},
        {NULL, NULL, 0.0}
    };
    
    int all_passed = 1;
    for (int i = 0; test_cases[i].query != NULL; i++) {
        float quality = calculate_response_quality(test_cases[i].query, 
                                                 test_cases[i].response);
        printf("  Query: '%s'\n", test_cases[i].query);
        printf("  Response: '%s'\n", test_cases[i].response);
        printf("  Quality: %.2f (expected >= %.2f)\n", quality, test_cases[i].min_expected);
        
        if (quality < test_cases[i].min_expected) {
            all_passed = 0;
        }
    }
    
    return all_passed;
}

// Test full workflow execution
int test_full_workflow() {
    WorkflowState* workflow = create_workflow();
    
    const char* query = "List three colors";
    printf("  Testing full workflow for: '%s'\n", query);
    
    // Decompose
    int num_steps = decompose_query(workflow, query);
    printf("  Decomposed into %d steps\n", num_steps);
    
    // Execute workflow
    int exec_result = execute_workflow(workflow);
    printf("  Execution result: %s\n", exec_result ? "SUCCESS" : "FAILED");
    
    // Get final result
    char* result = synthesize_results(workflow);
    printf("  Final result: %s\n", result ? result : "(null)");
    
    // Print final state
    print_workflow_state(workflow);
    
    int success = (exec_result == 1 && workflow->overall_confidence > 0);
    
    destroy_workflow(workflow);
    return success;
}

// Test complex multi-step workflow
int test_complex_workflow() {
    WorkflowState* workflow = create_workflow();
    
    const char* query = "What is 5 plus 3? Also, explain why addition works.";
    printf("  Testing complex workflow for: '%s'\n", query);
    
    // Decompose
    int num_steps = decompose_query(workflow, query);
    printf("  Decomposed into %d steps\n", num_steps);
    
    // Should have multiple analyze steps
    int analyze_count = 0;
    for (int i = 0; i < workflow->num_steps; i++) {
        if (workflow->steps[i].type == STEP_ANALYZE) {
            analyze_count++;
        }
    }
    
    printf("  Found %d analyze steps\n", analyze_count);
    
    int success = (num_steps > 3 && analyze_count >= 2);
    
    destroy_workflow(workflow);
    return success;
}

int main() {
    printf("=== Dynamic Workflows Test Suite ===\n\n");
    
    // Run all tests
    RUN_TEST(test_workflow_creation);
    RUN_TEST(test_simple_decomposition);
    RUN_TEST(test_multipart_decomposition);
    RUN_TEST(test_step_execution);
    RUN_TEST(test_backtracking);
    RUN_TEST(test_context_management);
    RUN_TEST(test_quality_evaluation);
    RUN_TEST(test_full_workflow);
    RUN_TEST(test_complex_workflow);
    
    printf("=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Success rate: %.1f%%\n", (tests_passed * 100.0) / tests_run);
    
    return tests_passed == tests_run ? 0 : 1;
}