#ifndef GAIA_LOGGER_H
#define GAIA_LOGGER_H

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

// Log levels
typedef enum {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARN = 3,
    LOG_ERROR = 4,
    LOG_FATAL = 5
} LogLevel;

// Error codes
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

// Performance metrics
typedef struct {
    const char* operation;
    clock_t start_time;
    clock_t end_time;
    double duration_ms;
} PerfMetric;

// Logger configuration
typedef struct {
    LogLevel min_level;
    FILE* log_file;
    int enable_colors;
    int enable_timestamps;
    int enable_location;
    int enable_performance;
    
    // Performance tracking
    PerfMetric perf_stack[100];
    int perf_depth;
    
    // Statistics
    long total_logs[6]; // One per log level
    long total_errors;
    long total_warnings;
} LoggerConfig;

// Global logger instance
extern LoggerConfig* g_logger;

// Initialize logger
void logger_init(LogLevel min_level, const char* log_file_path);
void logger_shutdown(void);

// Core logging functions
void log_message(LogLevel level, const char* file, int line, const char* func, 
                const char* format, ...);

// Convenience macros
#define LOG_TRACE(...) log_message(LOG_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_DEBUG(...) log_message(LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...)  log_message(LOG_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARN(...)  log_message(LOG_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...) log_message(LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_FATAL(...) log_message(LOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

// Error handling macros
#define CHECK_NULL(ptr, err_code) do { \
    if (!(ptr)) { \
        LOG_ERROR("NULL pointer check failed: " #ptr); \
        return err_code; \
    } \
} while(0)

#define CHECK_NULL_RETURN(ptr) do { \
    if (!(ptr)) { \
        LOG_ERROR("NULL pointer check failed: " #ptr); \
        return NULL; \
    } \
} while(0)

#define CHECK_ERROR(expr) do { \
    GaiaError err = (expr); \
    if (err != GAIA_OK) { \
        LOG_ERROR("Error %d in: " #expr, err); \
        return err; \
    } \
} while(0)

#define SAFE_FREE(ptr) do { \
    if (ptr) { \
        free(ptr); \
        ptr = NULL; \
    } \
} while(0)

// Performance tracking
void perf_begin(const char* operation);
double perf_end(const char* operation);
void perf_report(void);

// Convenience macros for performance
#define PERF_BEGIN(op) perf_begin(op)
#define PERF_END(op) perf_end(op)

// Error string conversion
const char* gaia_error_string(GaiaError err);

// Execution tracing
void trace_enter(const char* func, const char* format, ...);
void trace_exit(const char* func, const char* format, ...);

#define TRACE_ENTER(...) trace_enter(__func__, __VA_ARGS__)
#define TRACE_EXIT(...) trace_exit(__func__, __VA_ARGS__)

// Memory tracking
void* gaia_malloc(size_t size, const char* file, int line);
void* gaia_calloc(size_t nmemb, size_t size, const char* file, int line);
void gaia_free(void* ptr, const char* file, int line);

#ifdef GAIA_TRACK_MEMORY
    #define malloc(size) gaia_malloc(size, __FILE__, __LINE__)
    #define calloc(n, size) gaia_calloc(n, size, __FILE__, __LINE__)
    #define free(ptr) gaia_free(ptr, __FILE__, __LINE__)
#endif

// Statistics
void logger_print_stats(void);

#endif // GAIA_LOGGER_H