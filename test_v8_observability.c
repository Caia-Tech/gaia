#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gaia_logger.h"
#include "gaia_observability.h"

// Test error handling
void test_error_handling(void) {
    LOG_INFO("=== Testing Error Handling ===");
    
    // Test NULL checks
    char* result = NULL;
    void* null_ptr = NULL;
    
    // This would normally crash, but with CHECK_NULL it returns gracefully
    if (null_ptr) {
        CHECK_NULL(null_ptr, );
    }
    
    // Test error propagation
    GaiaError err = GAIA_ERR_OUT_OF_MEMORY;
    LOG_ERROR("Simulated error: %s (code: %d)", gaia_error_string(err), err);
    
    // Test safe memory operations
    char* buffer = malloc(100);
    if (!buffer) {
        LOG_ERROR("Memory allocation failed");
        METRIC_INC("test.allocation_failures");
    } else {
        LOG_DEBUG("Allocated 100 bytes at %p", buffer);
        SAFE_FREE(buffer);
        LOG_DEBUG("Freed buffer (now %p)", buffer); // Should be NULL
    }
}

// Test performance tracking
void test_performance_tracking(void) {
    LOG_INFO("=== Testing Performance Tracking ===");
    
    PERF_BEGIN("test_operation");
    
    // Simulate some work
    PERF_BEGIN("sub_operation_1");
    for (int i = 0; i < 1000000; i++) {
        // Busy work
    }
    double sub1_time = PERF_END("sub_operation_1");
    LOG_DEBUG("Sub-operation 1 took %.2f ms", sub1_time);
    
    PERF_BEGIN("sub_operation_2");
    for (int i = 0; i < 500000; i++) {
        // More busy work
    }
    double sub2_time = PERF_END("sub_operation_2");
    LOG_DEBUG("Sub-operation 2 took %.2f ms", sub2_time);
    
    double total_time = PERF_END("test_operation");
    LOG_INFO("Total operation took %.2f ms", total_time);
}

// Test metrics
void test_metrics(void) {
    LOG_INFO("=== Testing Metrics ===");
    
    // Counter metrics
    for (int i = 0; i < 10; i++) {
        METRIC_INC("test.counter");
    }
    
    // Gauge metrics
    for (int i = 0; i < 5; i++) {
        metric_gauge_set("test.gauge", i * 10.5);
    }
    
    // Timer metrics
    for (int i = 0; i < 3; i++) {
        METRIC_TIME("test.timed_operation", {
            // Simulate work
            for (int j = 0; j < 100000 * (i + 1); j++);
        });
    }
    
    LOG_INFO("Metrics test completed");
}

// Test execution tracing
void helper_function(int value) {
    TRACE_ENTER("value=%d", value);
    
    LOG_DEBUG("Helper function processing value: %d", value);
    
    if (value < 0) {
        LOG_WARN("Negative value provided: %d", value);
    }
    
    TRACE_EXIT("completed");
}

void test_tracing(void) {
    LOG_INFO("=== Testing Execution Tracing ===");
    
    TRACE_ENTER("starting trace test");
    
    for (int i = -1; i <= 1; i++) {
        helper_function(i);
    }
    
    TRACE_EXIT("trace test complete");
}

// Test request tracking
void test_request_tracking(void) {
    LOG_INFO("=== Testing Request Tracking ===");
    
    for (int i = 1; i <= 3; i++) {
        char request_id[32];
        snprintf(request_id, 31, "TEST-REQ-%d", i);
        
        request_begin(request_id);
        LOG_INFO("Processing request %s", request_id);
        
        // Simulate request processing
        PERF_BEGIN("request_processing");
        
        // Simulate different response times
        for (int j = 0; j < 100000 * i; j++);
        
        PERF_END("request_processing");
        request_end();
        
        LOG_INFO("Request %s completed", request_id);
    }
}

// Test V8 metrics
void test_v8_metrics(void) {
    LOG_INFO("=== Testing V8 Metrics ===");
    
    // Simulate workflow completions
    v8_metrics_workflow_complete(1, 125.5);  // Success
    v8_metrics_workflow_complete(1, 98.3);   // Success
    v8_metrics_workflow_complete(0, 250.0);  // Failure
    
    // Simulate attention computations
    v8_metrics_attention_complete(0.85, 15.2);
    v8_metrics_attention_complete(0.92, 12.8);
    v8_metrics_attention_complete(0.78, 18.5);
    
    // Simulate refinements
    v8_metrics_refinement_complete(1, 0.15, 3);  // Success, +0.15 quality, 3 iterations
    v8_metrics_refinement_complete(1, 0.22, 5);  // Success, +0.22 quality, 5 iterations
    v8_metrics_refinement_complete(0, 0.0, 10);  // Failed after 10 iterations
    
    // Simulate responses
    v8_metrics_response_complete("This is a test response", 150.0);
    v8_metrics_response_complete("Another response with more content here", 175.5);
    v8_metrics_response_complete("", 50.0);  // Empty response
    
    // Simulate quality scores
    v8_metrics_quality_scores(0.8, 0.85, 0.75, 0.9, 0.82);
    v8_metrics_quality_scores(0.85, 0.9, 0.8, 0.95, 0.87);
    
    // Report V8 metrics
    v8_metrics_report();
}

// Test log levels
void test_log_levels(void) {
    LOG_INFO("=== Testing Log Levels ===");
    
    LOG_TRACE("This is a TRACE message (most verbose)");
    LOG_DEBUG("This is a DEBUG message");
    LOG_INFO("This is an INFO message");
    LOG_WARN("This is a WARNING message");
    LOG_ERROR("This is an ERROR message");
    LOG_FATAL("This is a FATAL message (most severe)");
}

// Main test function
int main(int argc, char* argv[]) {
    // Initialize with debug logging
    logger_init(LOG_DEBUG, "test_observability.log");
    observability_init();
    v8_metrics_init();
    
    LOG_INFO("=== GAIA V8 Observability Test Suite ===");
    
    // Run tests
    test_log_levels();
    test_error_handling();
    test_performance_tracking();
    test_metrics();
    test_tracing();
    test_request_tracking();
    test_v8_metrics();
    
    // Export metrics
    export_metrics_json("test_metrics.json");
    
    // Print statistics
    LOG_INFO("=== Test Complete ===");
    logger_print_stats();
    
    // Cleanup
    observability_shutdown();
    logger_shutdown();
    
    printf("\nCheck test_observability.log and test_metrics.json for detailed output.\n");
    
    return 0;
}