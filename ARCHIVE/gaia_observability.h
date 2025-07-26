#ifndef GAIA_OBSERVABILITY_H
#define GAIA_OBSERVABILITY_H

#include "gaia_logger.h"
#include <stdint.h>

// Metrics types
typedef enum {
    METRIC_COUNTER,
    METRIC_GAUGE,
    METRIC_HISTOGRAM,
    METRIC_TIMER
} MetricType;

// Metric structure
typedef struct {
    char name[128];
    MetricType type;
    double value;
    double min;
    double max;
    double sum;
    long count;
    double buckets[10]; // For histogram
    int bucket_counts[10];
} Metric;

// Observability context
typedef struct {
    Metric metrics[1000];
    int num_metrics;
    
    // Traces
    char trace_buffer[10000];
    int trace_offset;
    
    // Events
    struct {
        char message[256];
        LogLevel level;
        time_t timestamp;
    } events[1000];
    int num_events;
    
    // Request tracking
    char request_id[64];
    time_t request_start;
    time_t request_end;
    
    // Component health
    struct {
        const char* name;
        int healthy;
        char last_error[256];
        time_t last_check;
    } components[50];
    int num_components;
} ObservabilityContext;

// Global observability instance
extern ObservabilityContext* g_observability;

// Initialize observability
void observability_init(void);
void observability_shutdown(void);

// Metrics API
void metric_increment(const char* name, double value);
void metric_gauge_set(const char* name, double value);
void metric_timer_start(const char* name);
void metric_timer_end(const char* name);
void metric_histogram_observe(const char* name, double value);

// Request tracking
void request_begin(const char* request_id);
void request_end(void);
void request_add_context(const char* key, const char* value);

// Component health
void health_register_component(const char* name);
void health_report_status(const char* component, int healthy, const char* error);
int health_check_all(void);

// Event tracking
void event_record(LogLevel level, const char* format, ...);

// Execution flow tracking
void flow_enter(const char* component, const char* operation);
void flow_exit(const char* component, const char* operation, int success);

// V8-specific observability
typedef struct {
    // Workflow metrics
    int total_workflows;
    int successful_workflows;
    int failed_workflows;
    double avg_workflow_time_ms;
    
    // Attention metrics
    int attention_computations;
    double avg_attention_confidence;
    double total_attention_time_ms;
    
    // Refinement metrics
    int refinement_attempts;
    int refinement_successes;
    double avg_quality_improvement;
    double avg_refinement_iterations;
    
    // Response metrics
    int total_responses;
    int empty_responses;
    int error_responses;
    double avg_response_length;
    double avg_response_time_ms;
    
    // Quality metrics
    double avg_coherence_score;
    double avg_relevance_score;
    double avg_completeness_score;
    double avg_grammar_score;
    double avg_overall_quality;
} V8Metrics;

// V8 observability functions
void v8_metrics_init(void);
void v8_metrics_workflow_complete(int success, double duration_ms);
void v8_metrics_attention_complete(double confidence, double duration_ms);
void v8_metrics_refinement_complete(int success, double quality_delta, int iterations);
void v8_metrics_response_complete(const char* response, double duration_ms);
void v8_metrics_quality_scores(double coherence, double relevance, 
                              double completeness, double grammar, double overall);
void v8_metrics_report(void);

// Diagnostic dumps
void dump_workflow_state(void* workflow);
void dump_attention_state(void* attention_ctx);
void dump_refinement_state(void* refinement_ctx);
void dump_system_state(void);

// Performance profiling
typedef struct {
    const char* name;
    uint64_t total_calls;
    double total_time_ms;
    double min_time_ms;
    double max_time_ms;
    double avg_time_ms;
} ProfileData;

void profile_function_enter(const char* func_name);
void profile_function_exit(const char* func_name);
void profile_report(void);

// Convenience macros
#define METRIC_INC(name) metric_increment(name, 1.0)
#define METRIC_DEC(name) metric_increment(name, -1.0)
#define METRIC_SET(name, val) metric_gauge_set(name, val)
#define METRIC_TIME(name, code) do { \
    metric_timer_start(name); \
    code; \
    metric_timer_end(name); \
} while(0)

#define PROFILE_FUNC() \
    profile_function_enter(__func__); \
    struct ProfileCleanup { \
        ~ProfileCleanup() { profile_function_exit(__func__); } \
    } _profile_cleanup

// Export functions
void export_metrics_json(const char* filename);
void export_traces_json(const char* filename);
void export_health_json(const char* filename);

#endif // GAIA_OBSERVABILITY_H