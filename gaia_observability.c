#include "gaia_observability.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

// Global observability instance
ObservabilityContext* g_observability = NULL;

// V8 metrics instance
static V8Metrics g_v8_metrics = {0};

// Initialize observability
void observability_init(void) {
    if (g_observability) return;
    
    g_observability = calloc(1, sizeof(ObservabilityContext));
    if (!g_observability) {
        LOG_ERROR("Failed to initialize observability");
        return;
    }
    
    LOG_INFO("Observability system initialized");
}

void observability_shutdown(void) {
    if (!g_observability) return;
    
    free(g_observability);
    g_observability = NULL;
}

// Metrics implementation
static Metric* find_or_create_metric(const char* name, MetricType type) {
    if (!g_observability) return NULL;
    
    // Find existing
    for (int i = 0; i < g_observability->num_metrics; i++) {
        if (strcmp(g_observability->metrics[i].name, name) == 0) {
            return &g_observability->metrics[i];
        }
    }
    
    // Create new
    if (g_observability->num_metrics >= 1000) {
        LOG_WARN("Metric limit reached");
        return NULL;
    }
    
    Metric* m = &g_observability->metrics[g_observability->num_metrics++];
    strncpy(m->name, name, 127);
    m->type = type;
    m->min = 1e9;
    m->max = -1e9;
    
    return m;
}

void metric_increment(const char* name, double value) {
    Metric* m = find_or_create_metric(name, METRIC_COUNTER);
    if (!m) return;
    
    m->value += value;
    m->count++;
}

void metric_gauge_set(const char* name, double value) {
    Metric* m = find_or_create_metric(name, METRIC_GAUGE);
    if (!m) return;
    
    m->value = value;
    m->count++;
    if (value < m->min) m->min = value;
    if (value > m->max) m->max = value;
    m->sum += value;
}

static clock_t timer_starts[100];
static char* timer_names[100];
static int timer_count = 0;

void metric_timer_start(const char* name) {
    if (timer_count >= 100) return;
    
    timer_names[timer_count] = (char*)name;
    timer_starts[timer_count] = clock();
    timer_count++;
}

void metric_timer_end(const char* name) {
    clock_t end = clock();
    
    for (int i = timer_count - 1; i >= 0; i--) {
        if (strcmp(timer_names[i], name) == 0) {
            double duration_ms = ((double)(end - timer_starts[i]) / CLOCKS_PER_SEC) * 1000.0;
            
            Metric* m = find_or_create_metric(name, METRIC_TIMER);
            if (m) {
                m->sum += duration_ms;
                m->count++;
                if (duration_ms < m->min) m->min = duration_ms;
                if (duration_ms > m->max) m->max = duration_ms;
                m->value = m->sum / m->count; // Average
            }
            
            // Remove from stack
            if (i < timer_count - 1) {
                memmove(&timer_names[i], &timer_names[i+1], 
                       (timer_count - i - 1) * sizeof(char*));
                memmove(&timer_starts[i], &timer_starts[i+1],
                       (timer_count - i - 1) * sizeof(clock_t));
            }
            timer_count--;
            return;
        }
    }
}

// Request tracking
static time_t request_start_time = 0;

void request_begin(const char* request_id) {
    if (!g_observability) return;
    
    strncpy(g_observability->request_id, request_id, 63);
    g_observability->request_start = time(NULL);
    request_start_time = clock();
    
    LOG_DEBUG("Request %s started", request_id);
    metric_increment("requests.total", 1);
}

void request_end(void) {
    if (!g_observability) return;
    
    g_observability->request_end = time(NULL);
    double duration_ms = ((double)(clock() - request_start_time) / CLOCKS_PER_SEC) * 1000.0;
    
    LOG_DEBUG("Request %s completed in %.2f ms", g_observability->request_id, duration_ms);
    metric_timer_end("request.duration");
}

// V8 Metrics
void v8_metrics_init(void) {
    memset(&g_v8_metrics, 0, sizeof(V8Metrics));
    LOG_INFO("V8 metrics initialized");
}

void v8_metrics_workflow_complete(int success, double duration_ms) {
    g_v8_metrics.total_workflows++;
    if (success) {
        g_v8_metrics.successful_workflows++;
    } else {
        g_v8_metrics.failed_workflows++;
    }
    
    g_v8_metrics.avg_workflow_time_ms = 
        (g_v8_metrics.avg_workflow_time_ms * (g_v8_metrics.total_workflows - 1) + duration_ms) 
        / g_v8_metrics.total_workflows;
    
    metric_increment(success ? "v8.workflows.success" : "v8.workflows.failed", 1);
    metric_gauge_set("v8.workflows.duration_ms", duration_ms);
}

void v8_metrics_attention_complete(double confidence, double duration_ms) {
    g_v8_metrics.attention_computations++;
    g_v8_metrics.avg_attention_confidence = 
        (g_v8_metrics.avg_attention_confidence * (g_v8_metrics.attention_computations - 1) + confidence)
        / g_v8_metrics.attention_computations;
    g_v8_metrics.total_attention_time_ms += duration_ms;
    
    metric_gauge_set("v8.attention.confidence", confidence);
    metric_gauge_set("v8.attention.duration_ms", duration_ms);
}

void v8_metrics_refinement_complete(int success, double quality_delta, int iterations) {
    g_v8_metrics.refinement_attempts++;
    if (success) {
        g_v8_metrics.refinement_successes++;
        g_v8_metrics.avg_quality_improvement = 
            (g_v8_metrics.avg_quality_improvement * (g_v8_metrics.refinement_successes - 1) + quality_delta)
            / g_v8_metrics.refinement_successes;
    }
    
    g_v8_metrics.avg_refinement_iterations = 
        (g_v8_metrics.avg_refinement_iterations * (g_v8_metrics.refinement_attempts - 1) + iterations)
        / g_v8_metrics.refinement_attempts;
    
    metric_increment(success ? "v8.refinements.success" : "v8.refinements.failed", 1);
    metric_gauge_set("v8.refinements.quality_delta", quality_delta);
    metric_gauge_set("v8.refinements.iterations", iterations);
}

void v8_metrics_response_complete(const char* response, double duration_ms) {
    g_v8_metrics.total_responses++;
    
    if (!response || strlen(response) == 0) {
        g_v8_metrics.empty_responses++;
    }
    
    size_t len = response ? strlen(response) : 0;
    g_v8_metrics.avg_response_length = 
        (g_v8_metrics.avg_response_length * (g_v8_metrics.total_responses - 1) + len)
        / g_v8_metrics.total_responses;
    
    g_v8_metrics.avg_response_time_ms = 
        (g_v8_metrics.avg_response_time_ms * (g_v8_metrics.total_responses - 1) + duration_ms)
        / g_v8_metrics.total_responses;
    
    metric_increment("v8.responses.total", 1);
    metric_gauge_set("v8.responses.length", len);
    metric_gauge_set("v8.responses.duration_ms", duration_ms);
}

void v8_metrics_quality_scores(double coherence, double relevance, 
                              double completeness, double grammar, double overall) {
    static int quality_count = 0;
    quality_count++;
    
    g_v8_metrics.avg_coherence_score = 
        (g_v8_metrics.avg_coherence_score * (quality_count - 1) + coherence) / quality_count;
    g_v8_metrics.avg_relevance_score = 
        (g_v8_metrics.avg_relevance_score * (quality_count - 1) + relevance) / quality_count;
    g_v8_metrics.avg_completeness_score = 
        (g_v8_metrics.avg_completeness_score * (quality_count - 1) + completeness) / quality_count;
    g_v8_metrics.avg_grammar_score = 
        (g_v8_metrics.avg_grammar_score * (quality_count - 1) + grammar) / quality_count;
    g_v8_metrics.avg_overall_quality = 
        (g_v8_metrics.avg_overall_quality * (quality_count - 1) + overall) / quality_count;
    
    metric_gauge_set("v8.quality.coherence", coherence);
    metric_gauge_set("v8.quality.relevance", relevance);
    metric_gauge_set("v8.quality.completeness", completeness);
    metric_gauge_set("v8.quality.grammar", grammar);
    metric_gauge_set("v8.quality.overall", overall);
}

void v8_metrics_report(void) {
    LOG_INFO("=== V8 Metrics Report ===");
    LOG_INFO("Workflows: %d total, %d successful, %d failed (avg time: %.2f ms)",
             g_v8_metrics.total_workflows, g_v8_metrics.successful_workflows,
             g_v8_metrics.failed_workflows, g_v8_metrics.avg_workflow_time_ms);
    LOG_INFO("Attention: %d computations (avg confidence: %.2f, total time: %.2f ms)",
             g_v8_metrics.attention_computations, g_v8_metrics.avg_attention_confidence,
             g_v8_metrics.total_attention_time_ms);
    LOG_INFO("Refinements: %d attempts, %d successful (avg improvement: %.2f, avg iterations: %.2f)",
             g_v8_metrics.refinement_attempts, g_v8_metrics.refinement_successes,
             g_v8_metrics.avg_quality_improvement, g_v8_metrics.avg_refinement_iterations);
    LOG_INFO("Responses: %d total, %d empty (avg length: %.1f, avg time: %.2f ms)",
             g_v8_metrics.total_responses, g_v8_metrics.empty_responses,
             g_v8_metrics.avg_response_length, g_v8_metrics.avg_response_time_ms);
    LOG_INFO("Quality Scores - Coherence: %.2f, Relevance: %.2f, Completeness: %.2f, Grammar: %.2f, Overall: %.2f",
             g_v8_metrics.avg_coherence_score, g_v8_metrics.avg_relevance_score,
             g_v8_metrics.avg_completeness_score, g_v8_metrics.avg_grammar_score,
             g_v8_metrics.avg_overall_quality);
}

// Export metrics as JSON
void export_metrics_json(const char* filename) {
    if (!g_observability) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        LOG_ERROR("Failed to open metrics file: %s", filename);
        return;
    }
    
    fprintf(f, "{\n  \"metrics\": [\n");
    
    for (int i = 0; i < g_observability->num_metrics; i++) {
        Metric* m = &g_observability->metrics[i];
        fprintf(f, "    {\n");
        fprintf(f, "      \"name\": \"%s\",\n", m->name);
        fprintf(f, "      \"type\": \"%s\",\n", 
               m->type == METRIC_COUNTER ? "counter" :
               m->type == METRIC_GAUGE ? "gauge" :
               m->type == METRIC_TIMER ? "timer" : "histogram");
        fprintf(f, "      \"value\": %.3f,\n", m->value);
        fprintf(f, "      \"count\": %ld,\n", m->count);
        fprintf(f, "      \"min\": %.3f,\n", m->min);
        fprintf(f, "      \"max\": %.3f,\n", m->max);
        fprintf(f, "      \"sum\": %.3f\n", m->sum);
        fprintf(f, "    }%s\n", i < g_observability->num_metrics - 1 ? "," : "");
    }
    
    fprintf(f, "  ],\n");
    fprintf(f, "  \"v8_metrics\": {\n");
    fprintf(f, "    \"workflows\": { \"total\": %d, \"successful\": %d, \"failed\": %d, \"avg_time_ms\": %.2f },\n",
           g_v8_metrics.total_workflows, g_v8_metrics.successful_workflows,
           g_v8_metrics.failed_workflows, g_v8_metrics.avg_workflow_time_ms);
    fprintf(f, "    \"responses\": { \"total\": %d, \"empty\": %d, \"avg_length\": %.1f, \"avg_time_ms\": %.2f }\n",
           g_v8_metrics.total_responses, g_v8_metrics.empty_responses,
           g_v8_metrics.avg_response_length, g_v8_metrics.avg_response_time_ms);
    fprintf(f, "  }\n}\n");
    
    fclose(f);
    LOG_INFO("Metrics exported to %s", filename);
}