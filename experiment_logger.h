#ifndef EXPERIMENT_LOGGER_H
#define EXPERIMENT_LOGGER_H

#include <time.h>

// Experiment types
typedef enum {
    EXP_SUPERPOSITION,
    EXP_COHERENCE,
    EXP_LOOKAHEAD,
    EXP_ANALYSIS,
    EXP_PERFORMANCE,
    EXP_DISCOVERY
} ExperimentType;

// Experiment log entry
typedef struct {
    time_t timestamp;
    ExperimentType type;
    char description[256];
    char input[512];
    char output[512];
    char metrics[256];
    float score;
    int success;
} ExperimentLog;

// Logger functions
void init_experiment_logger(void);
void log_experiment(ExperimentType type, const char* description, 
                   const char* input, const char* output, 
                   const char* metrics, float score, int success);
void log_superposition_experiment(const char* input, int state_count, 
                                 float* probabilities, const char* result);
void log_coherence_experiment(const char* context, const char* candidate, 
                             float semantic_sim, float grammatical_fit, 
                             float topic_consistency, float overall_score);
void log_discovery(const char* discovery, const char* details);
void save_experiment_log(const char* filename);
void print_experiment_summary(void);
void cleanup_experiment_logger(void);

#endif // EXPERIMENT_LOGGER_H