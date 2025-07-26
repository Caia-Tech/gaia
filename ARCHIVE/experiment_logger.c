#include "experiment_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_EXPERIMENTS 1000

// Global experiment log
static ExperimentLog* experiments = NULL;
static int experiment_count = 0;
static int experiment_capacity = 0;
static FILE* log_file = NULL;

// Initialize the experiment logger
void init_experiment_logger(void) {
    experiment_capacity = MAX_EXPERIMENTS;
    experiments = calloc(experiment_capacity, sizeof(ExperimentLog));
    experiment_count = 0;
    
    // Open log file for real-time logging
    log_file = fopen("gaia_experiments.log", "a");
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "\n=== GAIA Experiment Session Started: %s", ctime(&now));
        fflush(log_file);
    }
    
    printf("Experiment logger initialized (capacity: %d)\n", experiment_capacity);
}

// Add experiment to log
void log_experiment(ExperimentType type, const char* description, 
                   const char* input, const char* output, 
                   const char* metrics, float score, int success) {
    if (!experiments || experiment_count >= experiment_capacity) {
        printf("Warning: Experiment log full, cannot log more experiments\n");
        return;
    }
    
    ExperimentLog* exp = &experiments[experiment_count];
    
    exp->timestamp = time(NULL);
    exp->type = type;
    exp->score = score;
    exp->success = success;
    
    // Safe string copying
    strncpy(exp->description, description ? description : "", 255);
    exp->description[255] = '\0';
    
    strncpy(exp->input, input ? input : "", 511);
    exp->input[511] = '\0';
    
    strncpy(exp->output, output ? output : "", 511);
    exp->output[511] = '\0';
    
    strncpy(exp->metrics, metrics ? metrics : "", 255);
    exp->metrics[255] = '\0';
    
    experiment_count++;
    
    // Real-time logging to file
    if (log_file) {
        const char* type_names[] = {"SUPERPOSITION", "COHERENCE", "LOOKAHEAD", 
                                   "ANALYSIS", "PERFORMANCE", "DISCOVERY"};
        fprintf(log_file, "[%s] %s | Score: %.3f | Success: %s\n",
                type_names[type], description, score, success ? "YES" : "NO");
        if (input && strlen(input) > 0) {
            fprintf(log_file, "  Input: %s\n", input);
        }
        if (output && strlen(output) > 0) {
            fprintf(log_file, "  Output: %s\n", output);
        }
        if (metrics && strlen(metrics) > 0) {
            fprintf(log_file, "  Metrics: %s\n", metrics);
        }
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}

// Log superposition experiment
void log_superposition_experiment(const char* input, int state_count, 
                                 float* probabilities, const char* result) {
    char metrics[256];
    snprintf(metrics, sizeof(metrics), "States: %d, Max prob: %.3f", 
             state_count, probabilities ? probabilities[0] : 0.0);
    
    char description[256];
    snprintf(description, sizeof(description), "Superposition collapse with %d states", state_count);
    
    float score = state_count > 1 ? 1.0 : 0.0; // Success if we had multiple states
    
    log_experiment(EXP_SUPERPOSITION, description, input, result, metrics, score, 1);
}

// Log coherence experiment  
void log_coherence_experiment(const char* context, const char* candidate, 
                             float semantic_sim, float grammatical_fit, 
                             float topic_consistency, float overall_score) {
    char input_str[256];
    snprintf(input_str, sizeof(input_str), "Context: '%s' + Candidate: '%s'", 
             context, candidate);
    
    char metrics[256];
    snprintf(metrics, sizeof(metrics), "Semantic: %.3f, Grammar: %.3f, Topic: %.3f", 
             semantic_sim, grammatical_fit, topic_consistency);
    
    char output_str[128];
    snprintf(output_str, sizeof(output_str), "Overall coherence: %.3f", overall_score);
    
    log_experiment(EXP_COHERENCE, "Coherence analysis", input_str, output_str, 
                  metrics, overall_score, overall_score > 0.5 ? 1 : 0);
}

// Log discovery
void log_discovery(const char* discovery, const char* details) {
    log_experiment(EXP_DISCOVERY, discovery, "", details, "", 1.0, 1);
}

// Save complete experiment log to JSON file
void save_experiment_log(const char* filename) {
    if (!experiments || experiment_count == 0) {
        printf("No experiments to save\n");
        return;
    }
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Could not open %s for writing\n", filename);
        return;
    }
    
    fprintf(file, "{\n");
    fprintf(file, "  \"session_info\": {\n");
    fprintf(file, "    \"total_experiments\": %d,\n", experiment_count);
    fprintf(file, "    \"generated_at\": \"%s\",\n", ctime(&experiments[0].timestamp));
    fprintf(file, "    \"system\": \"GAIA V6\"\n");
    fprintf(file, "  },\n");
    fprintf(file, "  \"experiments\": [\n");
    
    for (int i = 0; i < experiment_count; i++) {
        ExperimentLog* exp = &experiments[i];
        const char* type_names[] = {"superposition", "coherence", "lookahead", 
                                   "analysis", "performance", "discovery"};
        
        fprintf(file, "    {\n");
        fprintf(file, "      \"id\": %d,\n", i + 1);
        fprintf(file, "      \"timestamp\": %ld,\n", exp->timestamp);
        fprintf(file, "      \"type\": \"%s\",\n", type_names[exp->type]);
        fprintf(file, "      \"description\": \"%s\",\n", exp->description);
        fprintf(file, "      \"input\": \"%s\",\n", exp->input);
        fprintf(file, "      \"output\": \"%s\",\n", exp->output);
        fprintf(file, "      \"metrics\": \"%s\",\n", exp->metrics);
        fprintf(file, "      \"score\": %.3f,\n", exp->score);
        fprintf(file, "      \"success\": %s\n", exp->success ? "true" : "false");
        fprintf(file, "    }%s\n", i < experiment_count - 1 ? "," : "");
    }
    
    fprintf(file, "  ]\n");
    fprintf(file, "}\n");
    
    fclose(file);
    printf("Experiment log saved to %s (%d experiments)\n", filename, experiment_count);
}

// Print experiment summary
void print_experiment_summary(void) {
    if (!experiments || experiment_count == 0) {
        printf("No experiments logged\n");
        return;
    }
    
    printf("\n=== Experiment Summary ===\n");
    printf("Total experiments: %d\n", experiment_count);
    
    // Count by type
    int type_counts[6] = {0};
    int successes = 0;
    float total_score = 0.0;
    
    for (int i = 0; i < experiment_count; i++) {
        type_counts[experiments[i].type]++;
        if (experiments[i].success) successes++;
        total_score += experiments[i].score;
    }
    
    const char* type_names[] = {"Superposition", "Coherence", "Lookahead", 
                               "Analysis", "Performance", "Discovery"};
    
    printf("\nExperiments by type:\n");
    for (int i = 0; i < 6; i++) {
        if (type_counts[i] > 0) {
            printf("  %s: %d\n", type_names[i], type_counts[i]);
        }
    }
    
    printf("\nOverall metrics:\n");
    printf("  Success rate: %.1f%% (%d/%d)\n", 
           (float)successes * 100.0 / experiment_count, successes, experiment_count);
    printf("  Average score: %.3f\n", total_score / experiment_count);
    
    // Recent experiments
    printf("\nRecent experiments:\n");
    int start = experiment_count > 5 ? experiment_count - 5 : 0;
    for (int i = start; i < experiment_count; i++) {
        printf("  [%s] %s (score: %.3f)\n", 
               type_names[experiments[i].type], 
               experiments[i].description, 
               experiments[i].score);
    }
    
    printf("===========================\n\n");
}

// Cleanup
void cleanup_experiment_logger(void) {
    if (experiments) {
        free(experiments);
        experiments = NULL;
    }
    
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "=== Session Ended: %s\n", ctime(&now));
        fclose(log_file);
        log_file = NULL;
    }
    
    experiment_count = 0;
    experiment_capacity = 0;
}