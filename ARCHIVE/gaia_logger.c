#include "gaia_logger.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

// Global logger instance
LoggerConfig* g_logger = NULL;

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

// Log level names and colors
static const char* log_level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char* log_level_colors[] = {
    COLOR_CYAN, COLOR_WHITE, COLOR_GREEN, COLOR_YELLOW, COLOR_RED, COLOR_MAGENTA
};

// Initialize logger
void logger_init(LogLevel min_level, const char* log_file_path) {
    if (g_logger) {
        logger_shutdown();
    }
    
    g_logger = calloc(1, sizeof(LoggerConfig));
    if (!g_logger) {
        fprintf(stderr, "Failed to allocate logger\n");
        return;
    }
    
    g_logger->min_level = min_level;
    g_logger->enable_colors = 1;
    g_logger->enable_timestamps = 1;
    g_logger->enable_location = 1;
    g_logger->enable_performance = 1;
    
    if (log_file_path) {
        g_logger->log_file = fopen(log_file_path, "a");
        if (!g_logger->log_file) {
            fprintf(stderr, "Failed to open log file: %s\n", log_file_path);
        }
    }
    
    LOG_INFO("GAIA Logger initialized (level: %s)", log_level_names[min_level]);
}

// Shutdown logger
void logger_shutdown(void) {
    if (!g_logger) return;
    
    logger_print_stats();
    
    if (g_logger->log_file) {
        fclose(g_logger->log_file);
    }
    
    free(g_logger);
    g_logger = NULL;
}

// Get current timestamp
static void get_timestamp(char* buffer, size_t size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    struct tm* tm_info = localtime(&tv.tv_sec);
    size_t len = strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
    
    snprintf(buffer + len, size - len, ".%03d", (int)(tv.tv_usec / 1000));
}

// Core logging function
void log_message(LogLevel level, const char* file, int line, const char* func,
                const char* format, ...) {
    if (!g_logger || level < g_logger->min_level) return;
    
    g_logger->total_logs[level]++;
    if (level == LOG_ERROR) g_logger->total_errors++;
    if (level == LOG_WARN) g_logger->total_warnings++;
    
    char timestamp[32] = "";
    if (g_logger->enable_timestamps) {
        get_timestamp(timestamp, sizeof(timestamp));
    }
    
    // Extract filename from path
    const char* filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;
    
    // Format message
    va_list args;
    char message[1024];
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Console output
    FILE* out = (level >= LOG_ERROR) ? stderr : stdout;
    
    if (g_logger->enable_colors && isatty(fileno(out))) {
        fprintf(out, "%s", log_level_colors[level]);
    }
    
    if (g_logger->enable_timestamps) {
        fprintf(out, "[%s] ", timestamp);
    }
    
    fprintf(out, "[%s] ", log_level_names[level]);
    
    if (g_logger->enable_location) {
        fprintf(out, "[%s:%d %s] ", filename, line, func);
    }
    
    fprintf(out, "%s", message);
    
    if (g_logger->enable_colors && isatty(fileno(out))) {
        fprintf(out, COLOR_RESET);
    }
    
    fprintf(out, "\n");
    fflush(out);
    
    // File output (without colors)
    if (g_logger->log_file) {
        if (g_logger->enable_timestamps) {
            fprintf(g_logger->log_file, "[%s] ", timestamp);
        }
        fprintf(g_logger->log_file, "[%s] ", log_level_names[level]);
        if (g_logger->enable_location) {
            fprintf(g_logger->log_file, "[%s:%d %s] ", filename, line, func);
        }
        fprintf(g_logger->log_file, "%s\n", message);
        fflush(g_logger->log_file);
    }
}

// Performance tracking
void perf_begin(const char* operation) {
    if (!g_logger || !g_logger->enable_performance) return;
    if (g_logger->perf_depth >= 100) return;
    
    PerfMetric* metric = &g_logger->perf_stack[g_logger->perf_depth++];
    metric->operation = operation;
    metric->start_time = clock();
    metric->end_time = 0;
    metric->duration_ms = 0;
    
    LOG_TRACE("PERF BEGIN: %s", operation);
}

double perf_end(const char* operation) {
    if (!g_logger || !g_logger->enable_performance || g_logger->perf_depth == 0) {
        return 0.0;
    }
    
    clock_t end_time = clock();
    
    // Find matching operation
    for (int i = g_logger->perf_depth - 1; i >= 0; i--) {
        PerfMetric* metric = &g_logger->perf_stack[i];
        if (metric->operation && strcmp(metric->operation, operation) == 0) {
            metric->end_time = end_time;
            metric->duration_ms = ((double)(end_time - metric->start_time) / CLOCKS_PER_SEC) * 1000.0;
            
            LOG_TRACE("PERF END: %s (%.2f ms)", operation, metric->duration_ms);
            
            // Remove from stack
            if (i < g_logger->perf_depth - 1) {
                memmove(&g_logger->perf_stack[i], &g_logger->perf_stack[i + 1],
                       (g_logger->perf_depth - i - 1) * sizeof(PerfMetric));
            }
            g_logger->perf_depth--;
            
            return metric->duration_ms;
        }
    }
    
    LOG_WARN("PERF END: No matching BEGIN for '%s'", operation);
    return 0.0;
}

void perf_report(void) {
    if (!g_logger || !g_logger->enable_performance) return;
    
    LOG_INFO("=== Performance Report ===");
    LOG_INFO("Unclosed operations: %d", g_logger->perf_depth);
    
    for (int i = 0; i < g_logger->perf_depth; i++) {
        PerfMetric* metric = &g_logger->perf_stack[i];
        LOG_WARN("  Unclosed: %s (started but not ended)", metric->operation);
    }
}

// Error string conversion
const char* gaia_error_string(GaiaError err) {
    switch (err) {
        case GAIA_OK: return "Success";
        case GAIA_ERR_NULL_POINTER: return "Null pointer";
        case GAIA_ERR_OUT_OF_MEMORY: return "Out of memory";
        case GAIA_ERR_INVALID_INPUT: return "Invalid input";
        case GAIA_ERR_FILE_NOT_FOUND: return "File not found";
        case GAIA_ERR_WORKFLOW_FAILED: return "Workflow failed";
        case GAIA_ERR_ATTENTION_FAILED: return "Attention mechanism failed";
        case GAIA_ERR_REFINEMENT_FAILED: return "Refinement failed";
        case GAIA_ERR_SYNTHESIS_FAILED: return "Synthesis failed";
        case GAIA_ERR_CALCULATION_FAILED: return "Calculation failed";
        case GAIA_ERR_TIMEOUT: return "Operation timed out";
        case GAIA_ERR_UNKNOWN: return "Unknown error";
        default: return "Undefined error";
    }
}

// Execution tracing
void trace_enter(const char* func, const char* format, ...) {
    if (!g_logger || g_logger->min_level > LOG_TRACE) return;
    
    char message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    LOG_TRACE(">>> ENTER %s: %s", func, message);
}

void trace_exit(const char* func, const char* format, ...) {
    if (!g_logger || g_logger->min_level > LOG_TRACE) return;
    
    char message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    LOG_TRACE("<<< EXIT %s: %s", func, message);
}

// Memory tracking
static long g_total_allocations = 0;
static long g_total_deallocations = 0;
static long g_current_memory = 0;
static long g_peak_memory = 0;

void* gaia_malloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (ptr) {
        g_total_allocations++;
        g_current_memory += size;
        if (g_current_memory > g_peak_memory) {
            g_peak_memory = g_current_memory;
        }
        LOG_TRACE("MALLOC %zu bytes at %s:%d -> %p", size, file, line, ptr);
    } else {
        LOG_ERROR("MALLOC FAILED for %zu bytes at %s:%d", size, file, line);
    }
    return ptr;
}

void* gaia_calloc(size_t nmemb, size_t size, const char* file, int line) {
    void* ptr = calloc(nmemb, size);
    if (ptr) {
        g_total_allocations++;
        g_current_memory += nmemb * size;
        if (g_current_memory > g_peak_memory) {
            g_peak_memory = g_current_memory;
        }
        LOG_TRACE("CALLOC %zu x %zu bytes at %s:%d -> %p", nmemb, size, file, line, ptr);
    } else {
        LOG_ERROR("CALLOC FAILED for %zu x %zu bytes at %s:%d", nmemb, size, file, line);
    }
    return ptr;
}

void gaia_free(void* ptr, const char* file, int line) {
    if (ptr) {
        g_total_deallocations++;
        LOG_TRACE("FREE %p at %s:%d", ptr, file, line);
        free(ptr);
    }
}

// Statistics
void logger_print_stats(void) {
    if (!g_logger) return;
    
    LOG_INFO("=== Logger Statistics ===");
    LOG_INFO("Log messages by level:");
    for (int i = 0; i < 6; i++) {
        if (g_logger->total_logs[i] > 0) {
            LOG_INFO("  %s: %ld", log_level_names[i], g_logger->total_logs[i]);
        }
    }
    LOG_INFO("Total errors: %ld", g_logger->total_errors);
    LOG_INFO("Total warnings: %ld", g_logger->total_warnings);
    
    if (g_logger->enable_performance) {
        perf_report();
    }
    
#ifdef GAIA_TRACK_MEMORY
    LOG_INFO("=== Memory Statistics ===");
    LOG_INFO("Total allocations: %ld", g_total_allocations);
    LOG_INFO("Total deallocations: %ld", g_total_deallocations);
    LOG_INFO("Current memory: %ld bytes", g_current_memory);
    LOG_INFO("Peak memory: %ld bytes", g_peak_memory);
    LOG_INFO("Leaked allocations: %ld", g_total_allocations - g_total_deallocations);
#endif
}