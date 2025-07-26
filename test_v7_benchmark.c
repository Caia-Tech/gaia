#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#define NUM_ITERATIONS 10
#define MAX_QUERIES 100

typedef struct {
    const char* name;
    const char* query;
    double avg_time;
    double min_time;
    double max_time;
    int successful_runs;
} BenchmarkResult;

// Get current time in milliseconds
double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

// Run a single benchmark
double benchmark_query(const char* query) {
    char command[1024];
    double start_time, end_time;
    
    snprintf(command, sizeof(command), 
             "echo '%s' | ./gaia_chat_v7 2>&1 >/dev/null", query);
    
    start_time = get_time_ms();
    int result = system(command);
    end_time = get_time_ms();
    
    if (result != 0) return -1.0;  // Error
    
    return end_time - start_time;
}

// Calculate statistics
void calculate_stats(double* times, int count, BenchmarkResult* result) {
    if (count == 0) {
        result->avg_time = -1;
        result->min_time = -1;
        result->max_time = -1;
        return;
    }
    
    double sum = 0;
    result->min_time = times[0];
    result->max_time = times[0];
    
    for (int i = 0; i < count; i++) {
        sum += times[i];
        if (times[i] < result->min_time) result->min_time = times[i];
        if (times[i] > result->max_time) result->max_time = times[i];
    }
    
    result->avg_time = sum / count;
}

int main() {
    printf("=== GAIA V7 Performance Benchmark ===\n");
    printf("Running %d iterations per query...\n\n", NUM_ITERATIONS);
    
    // Define benchmark queries
    BenchmarkResult benchmarks[] = {
        // Simple queries
        {"Simple addition", "What is 5 plus 3?", 0, 0, 0, 0},
        {"Simple subtraction", "What is 10 minus 4?", 0, 0, 0, 0},
        {"Simple multiplication", "What is 6 times 7?", 0, 0, 0, 0},
        {"Simple division", "What is 20 divided by 4?", 0, 0, 0, 0},
        
        // Complex queries
        {"Multi-part calculation", "What is 5 plus 3? What is 10 minus 2?", 0, 0, 0, 0},
        {"Calculation with explanation", "What is 15 plus 27? Also, explain addition.", 0, 0, 0, 0},
        {"Three-part question", "What is 2 times 3? What is 12 divided by 4? What is 10 plus 5?", 0, 0, 0, 0},
        
        // Workflow-intensive
        {"Complex workflow", "List three colors and explain why they are primary", 0, 0, 0, 0},
        {"Nested reasoning", "Calculate factorial of 5 and explain what factorial means", 0, 0, 0, 0},
        
        // Edge cases
        {"Empty input", "", 0, 0, 0, 0},
        {"Very long input", "What is 1 plus 1 plus 1 plus 1 plus 1 plus 1 plus 1 plus 1 plus 1 plus 1?", 0, 0, 0, 0},
        {"Error case", "What is 5 divided by 0?", 0, 0, 0, 0},
        
        // Function calls
        {"Factorial function", "What is factorial of 7?", 0, 0, 0, 0},
        {"Prime check", "Is 23 a prime number?", 0, 0, 0, 0},
        
        // Pattern matching
        {"Greeting", "Hello there!", 0, 0, 0, 0},
        {"Farewell", "Goodbye!", 0, 0, 0, 0},
        
        // End marker
        {NULL, NULL, 0, 0, 0, 0}
    };
    
    // Warm-up run
    printf("Warming up...\n");
    benchmark_query("What is 1 plus 1?");
    
    // Run benchmarks
    int total_benchmarks = 0;
    for (int i = 0; benchmarks[i].name != NULL; i++) {
        total_benchmarks++;
    }
    
    printf("\nRunning %d benchmarks...\n\n", total_benchmarks);
    
    for (int i = 0; benchmarks[i].name != NULL; i++) {
        printf("Benchmarking: %s\n", benchmarks[i].name);
        printf("  Query: %.50s%s\n", benchmarks[i].query, 
               strlen(benchmarks[i].query) > 50 ? "..." : "");
        
        double times[NUM_ITERATIONS];
        int successful = 0;
        
        // Run multiple iterations
        for (int j = 0; j < NUM_ITERATIONS; j++) {
            double time = benchmark_query(benchmarks[i].query);
            if (time > 0) {
                times[successful++] = time;
            }
            
            // Progress indicator
            printf("\r  Progress: %d/%d", j + 1, NUM_ITERATIONS);
            fflush(stdout);
        }
        
        printf("\r  Completed: %d/%d successful runs\n", successful, NUM_ITERATIONS);
        
        benchmarks[i].successful_runs = successful;
        calculate_stats(times, successful, &benchmarks[i]);
        
        if (successful > 0) {
            printf("  Average: %.2f ms\n", benchmarks[i].avg_time);
            printf("  Min: %.2f ms, Max: %.2f ms\n", 
                   benchmarks[i].min_time, benchmarks[i].max_time);
        } else {
            printf("  All runs failed!\n");
        }
        printf("\n");
    }
    
    // Generate summary report
    printf("\n=== PERFORMANCE SUMMARY ===\n\n");
    
    // Find fastest and slowest
    double fastest_time = 999999;
    double slowest_time = 0;
    const char* fastest_query = NULL;
    const char* slowest_query = NULL;
    
    for (int i = 0; benchmarks[i].name != NULL; i++) {
        if (benchmarks[i].successful_runs > 0) {
            if (benchmarks[i].avg_time < fastest_time) {
                fastest_time = benchmarks[i].avg_time;
                fastest_query = benchmarks[i].name;
            }
            if (benchmarks[i].avg_time > slowest_time) {
                slowest_time = benchmarks[i].avg_time;
                slowest_query = benchmarks[i].name;
            }
        }
    }
    
    printf("Fastest query: %s (%.2f ms)\n", fastest_query, fastest_time);
    printf("Slowest query: %s (%.2f ms)\n", slowest_query, slowest_time);
    printf("Performance ratio: %.2fx\n\n", slowest_time / fastest_time);
    
    // Category analysis
    printf("=== CATEGORY ANALYSIS ===\n\n");
    
    double simple_total = 0, complex_total = 0, workflow_total = 0;
    int simple_count = 0, complex_count = 0, workflow_count = 0;
    
    for (int i = 0; benchmarks[i].name != NULL; i++) {
        if (benchmarks[i].successful_runs > 0) {
            if (strstr(benchmarks[i].name, "Simple")) {
                simple_total += benchmarks[i].avg_time;
                simple_count++;
            } else if (strstr(benchmarks[i].name, "Multi-part") || 
                      strstr(benchmarks[i].name, "Three-part")) {
                complex_total += benchmarks[i].avg_time;
                complex_count++;
            } else if (strstr(benchmarks[i].name, "workflow") || 
                      strstr(benchmarks[i].name, "reasoning")) {
                workflow_total += benchmarks[i].avg_time;
                workflow_count++;
            }
        }
    }
    
    if (simple_count > 0) {
        printf("Simple queries average: %.2f ms\n", simple_total / simple_count);
    }
    if (complex_count > 0) {
        printf("Multi-part queries average: %.2f ms\n", complex_total / complex_count);
    }
    if (workflow_count > 0) {
        printf("Workflow queries average: %.2f ms\n", workflow_total / workflow_count);
    }
    
    // Performance thresholds
    printf("\n=== PERFORMANCE THRESHOLDS ===\n\n");
    
    int under_100ms = 0, under_500ms = 0, under_1000ms = 0, over_1000ms = 0;
    
    for (int i = 0; benchmarks[i].name != NULL; i++) {
        if (benchmarks[i].successful_runs > 0) {
            if (benchmarks[i].avg_time < 100) under_100ms++;
            else if (benchmarks[i].avg_time < 500) under_500ms++;
            else if (benchmarks[i].avg_time < 1000) under_1000ms++;
            else over_1000ms++;
        }
    }
    
    printf("Under 100ms: %d queries\n", under_100ms);
    printf("100-500ms: %d queries\n", under_500ms);
    printf("500-1000ms: %d queries\n", under_1000ms);
    printf("Over 1000ms: %d queries\n", over_1000ms);
    
    // Detailed results table
    printf("\n=== DETAILED RESULTS TABLE ===\n\n");
    printf("%-40s %10s %10s %10s %10s\n", 
           "Query", "Avg (ms)", "Min (ms)", "Max (ms)", "Success");
    printf("%-40s %10s %10s %10s %10s\n", 
           "-----", "--------", "--------", "--------", "-------");
    
    for (int i = 0; benchmarks[i].name != NULL; i++) {
        if (benchmarks[i].successful_runs > 0) {
            printf("%-40s %10.2f %10.2f %10.2f %9d%%\n",
                   benchmarks[i].name,
                   benchmarks[i].avg_time,
                   benchmarks[i].min_time,
                   benchmarks[i].max_time,
                   (benchmarks[i].successful_runs * 100) / NUM_ITERATIONS);
        } else {
            printf("%-40s %10s %10s %10s %9d%%\n",
                   benchmarks[i].name,
                   "FAILED", "FAILED", "FAILED", 0);
        }
    }
    
    // Workflow vs Non-workflow comparison
    printf("\n=== WORKFLOW PERFORMANCE IMPACT ===\n");
    
    // Test with workflows disabled
    printf("\nTesting with workflows disabled...\n");
    system("echo 'toggle-workflows' | ./gaia_chat_v7 >/dev/null 2>&1");
    
    double no_workflow_time = benchmark_query("What is 15 plus 27? Also, explain addition.");
    
    // Test with workflows enabled
    system("echo 'toggle-workflows' | ./gaia_chat_v7 >/dev/null 2>&1");
    
    double workflow_time = benchmark_query("What is 15 plus 27? Also, explain addition.");
    
    if (no_workflow_time > 0 && workflow_time > 0) {
        printf("Multi-part query with workflows: %.2f ms\n", workflow_time);
        printf("Multi-part query without workflows: %.2f ms\n", no_workflow_time);
        printf("Workflow overhead: %.2f ms (%.1f%%)\n", 
               workflow_time - no_workflow_time,
               ((workflow_time - no_workflow_time) / no_workflow_time) * 100);
    }
    
    return 0;
}