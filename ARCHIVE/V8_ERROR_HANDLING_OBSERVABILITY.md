# GAIA V8 Error Handling and Observability

## Overview

GAIA V8 has been enhanced with comprehensive error handling and observability features to provide better reliability, debuggability, and operational insights.

## Error Handling

### Error Codes
```c
typedef enum {
    GAIA_OK = 0,
    GAIA_ERR_NULL_POINTER = -1,
    GAIA_ERR_OUT_OF_MEMORY = -2,
    GAIA_ERR_INVALID_INPUT = -3,
    GAIA_ERR_FILE_NOT_FOUND = -4,
    GAIA_ERR_WORKFLOW_FAILED = -5,
    GAIA_ERR_ATTENTION_FAILED = -6,
    GAIA_ERR_REFINEMENT_FAILED = -7,
    GAIA_ERR_SYNTHESIS_FAILED = -8,
    GAIA_ERR_CALCULATION_FAILED = -9,
    GAIA_ERR_TIMEOUT = -10,
    GAIA_ERR_UNKNOWN = -99
} GaiaError;
```

### Error Checking Macros

#### NULL Pointer Checking
```c
CHECK_NULL(ptr, err_code)     // Returns err_code if ptr is NULL
CHECK_NULL_RETURN(ptr)        // Returns NULL if ptr is NULL
```

#### Error Propagation
```c
CHECK_ERROR(expr)             // Returns error if expr != GAIA_OK
```

#### Safe Memory Management
```c
SAFE_FREE(ptr)               // Frees ptr and sets to NULL
```

### Example Usage
```c
GaiaError process_input(const char* input, char** output) {
    CHECK_NULL(input, GAIA_ERR_NULL_POINTER);
    CHECK_NULL(output, GAIA_ERR_NULL_POINTER);
    
    char* buffer = malloc(256);
    if (!buffer) {
        LOG_ERROR("Failed to allocate buffer");
        return GAIA_ERR_OUT_OF_MEMORY;
    }
    
    // Process...
    
    *output = buffer;
    return GAIA_OK;
}
```

## Logging System

### Log Levels
- **TRACE**: Most verbose, execution flow tracking
- **DEBUG**: Debugging information
- **INFO**: General information
- **WARN**: Warning conditions
- **ERROR**: Error conditions
- **FATAL**: Fatal errors

### Logging Macros
```c
LOG_TRACE("Entering function with value: %d", value);
LOG_DEBUG("Processing step %d of %d", current, total);
LOG_INFO("Request completed successfully");
LOG_WARN("Quality below threshold: %.2f", quality);
LOG_ERROR("Failed to allocate memory: %zu bytes", size);
LOG_FATAL("Critical system failure");
```

### Features
- Colored console output
- File logging
- Timestamps with millisecond precision
- Source location (file:line function)
- Log level filtering
- Statistics tracking

## Performance Tracking

### Basic Usage
```c
PERF_BEGIN("operation_name");
// ... code to measure ...
double duration_ms = PERF_END("operation_name");
LOG_INFO("Operation took %.2f ms", duration_ms);
```

### Nested Performance Tracking
```c
PERF_BEGIN("main_operation");
    PERF_BEGIN("sub_operation_1");
    // ... work ...
    PERF_END("sub_operation_1");
    
    PERF_BEGIN("sub_operation_2");
    // ... work ...
    PERF_END("sub_operation_2");
PERF_END("main_operation");
```

## Metrics System

### Metric Types
- **Counter**: Monotonically increasing value
- **Gauge**: Value that can go up or down
- **Timer**: Duration measurements
- **Histogram**: Distribution of values

### Metric Operations
```c
// Increment/decrement counters
METRIC_INC("requests.total");
METRIC_DEC("active_connections");

// Set gauge values
METRIC_SET("memory.used_mb", 1024.5);

// Time operations
METRIC_TIME("database.query", {
    // Code to time
});
```

## Execution Tracing

### Function Tracing
```c
void process_request(const char* id) {
    TRACE_ENTER("request_id=%s", id);
    
    // Function logic...
    
    TRACE_EXIT("success");
}
```

### Output Example
```
[TRACE] >>> ENTER process_request: request_id=REQ-123
[TRACE] <<< EXIT process_request: success
```

## Request Tracking

### Per-Request Context
```c
request_begin("REQ-12345");

// All operations are associated with this request
LOG_INFO("Processing user query");
METRIC_INC("requests.active");

// Process request...

request_end();
```

## V8-Specific Metrics

### Workflow Metrics
```c
v8_metrics_workflow_complete(success, duration_ms);
```

### Attention Metrics
```c
v8_metrics_attention_complete(confidence, duration_ms);
```

### Refinement Metrics
```c
v8_metrics_refinement_complete(success, quality_delta, iterations);
```

### Response Metrics
```c
v8_metrics_response_complete(response_text, duration_ms);
```

### Quality Metrics
```c
v8_metrics_quality_scores(coherence, relevance, completeness, grammar, overall);
```

## Observability Reports

### Logger Statistics
```
=== Logger Statistics ===
Log messages by level:
  DEBUG: 1523
  INFO: 456
  WARN: 23
  ERROR: 5
Total errors: 5
Total warnings: 23
```

### V8 Metrics Report
```
=== V8 Metrics Report ===
Workflows: 150 total, 142 successful, 8 failed (avg time: 125.3 ms)
Attention: 450 computations (avg confidence: 0.85, total time: 6750.0 ms)
Refinements: 45 attempts, 38 successful (avg improvement: 0.18, avg iterations: 3.2)
Responses: 150 total, 3 empty (avg length: 125.4, avg time: 156.7 ms)
Quality Scores - Coherence: 0.82, Relevance: 0.87, Completeness: 0.79, Grammar: 0.91, Overall: 0.84
```

## JSON Export

### Metrics Export
```json
{
  "metrics": [
    {
      "name": "requests.total",
      "type": "counter",
      "value": 150.000,
      "count": 150,
      "min": 1000000000.000,
      "max": -1000000000.000,
      "sum": 0.000
    },
    {
      "name": "response.duration_ms",
      "type": "timer",
      "value": 156.734,
      "count": 150,
      "min": 45.230,
      "max": 523.450,
      "sum": 23510.100
    }
  ],
  "v8_metrics": {
    "workflows": { "total": 150, "successful": 142, "failed": 8, "avg_time_ms": 125.34 },
    "responses": { "total": 150, "empty": 3, "avg_length": 125.4, "avg_time_ms": 156.73 }
  }
}
```

## Best Practices

### 1. Always Check Return Values
```c
GaiaError err = generate_response_v8_safe(system, input, request_id, &output);
if (err != GAIA_OK) {
    LOG_ERROR("Response generation failed: %s", gaia_error_string(err));
    return err;
}
```

### 2. Use Appropriate Log Levels
- TRACE: Detailed execution flow
- DEBUG: Development and debugging
- INFO: Normal operations
- WARN: Recoverable issues
- ERROR: Errors that need attention
- FATAL: Unrecoverable errors

### 3. Track Performance of Critical Paths
```c
PERF_BEGIN("critical_operation");
GaiaError err = perform_critical_operation();
double duration = PERF_END("critical_operation");

if (duration > 1000.0) {
    LOG_WARN("Critical operation took %.2f ms (threshold: 1000ms)", duration);
}
```

### 4. Use Metrics for Monitoring
- Track success/failure rates
- Monitor response times
- Measure quality scores
- Watch for anomalies

### 5. Enable Tracing for Debugging
```c
// Enable trace logging for detailed flow
logger_init(LOG_TRACE, "debug_session.log");
```

## Integration Example

```c
int main() {
    // Initialize with appropriate log level
    logger_init(LOG_INFO, "gaia_v8.log");
    observability_init();
    v8_metrics_init();
    
    // Process requests
    while (running) {
        char* request_id = generate_request_id();
        request_begin(request_id);
        
        LOG_INFO("[%s] Processing request", request_id);
        
        char* response = NULL;
        GaiaError err = process_request_safe(input, &response);
        
        if (err != GAIA_OK) {
            LOG_ERROR("[%s] Request failed: %s", 
                     request_id, gaia_error_string(err));
            METRIC_INC("requests.failed");
        } else {
            LOG_INFO("[%s] Request succeeded", request_id);
            METRIC_INC("requests.successful");
        }
        
        request_end();
    }
    
    // Export metrics and stats
    export_metrics_json("metrics.json");
    v8_metrics_report();
    logger_print_stats();
    
    // Cleanup
    observability_shutdown();
    logger_shutdown();
}
```

## Benefits

1. **Better Debugging**: Detailed logs and traces make issues easier to diagnose
2. **Performance Visibility**: Know exactly where time is spent
3. **Error Recovery**: Graceful handling prevents crashes
4. **Operational Insights**: Metrics show system behavior over time
5. **Quality Tracking**: Monitor response quality metrics
6. **Memory Safety**: Tracking helps identify leaks
7. **Request Context**: Associate all operations with specific requests

## Configuration

### Environment Variables
```bash
GAIA_LOG_LEVEL=DEBUG      # Set minimum log level
GAIA_LOG_FILE=output.log  # Set log file path
GAIA_ENABLE_COLORS=0      # Disable colored output
GAIA_TRACK_MEMORY=1       # Enable memory tracking
```

### Runtime Configuration
```c
// Change log level at runtime
g_logger->min_level = LOG_DEBUG;

// Enable/disable features
g_logger->enable_timestamps = 1;
g_logger->enable_location = 1;
g_logger->enable_performance = 1;
```

## Conclusion

The enhanced error handling and observability in GAIA V8 provides:
- Robust error handling that prevents crashes
- Comprehensive logging for debugging
- Performance metrics for optimization
- Quality tracking for improvement
- Operational visibility for monitoring

This makes GAIA V8 production-ready with the ability to diagnose issues, track performance, and maintain high quality responses.