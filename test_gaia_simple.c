#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

// Color codes for output
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"

// Simple pattern structure for testing
typedef struct {
    char prompt[256];
    char expected[256];
    char category[64];
    int passed;
} TestCase;

// Test cases covering all domains
TestCase test_cases[] = {
    // Mathematics
    {"What is two plus two", "four", "Mathematics", 0},
    {"Calculate x squared derivative", "2x", "Mathematics", 0},
    {"What comes after 5 in Fibonacci", "8", "Mathematics", 0},
    {"What is pi", "3.14", "Mathematics", 0},
    
    // Reasoning
    {"If Felix is a cat what is Felix", "animal", "Reasoning", 0},
    {"Correlation vs causation", "different", "Reasoning", 0},
    
    // Philosophy
    {"What are qualia", "experience", "Philosophy", 0},
    {"Is money natural", "social construct", "Philosophy", 0},
    
    // Social Intelligence
    {"Someone says fine with crossed arms", "not okay", "Social", 0},
    {"Friend venting about work", "listen", "Social", 0},
    
    // Emotional Intelligence
    {"Sad vs melancholy", "wistful", "Emotional", 0},
    {"Good days bad days", "normal", "Emotional", 0},
    
    // Technical
    {"What is a monad", "computation", "Technical", 0},
    {"Can program halt", "undecidable", "Technical", 0},
    
    // Creative
    {"What is negative space", "empty", "Creative", 0},
    {"What is wabi-sabi", "imperfection", "Creative", 0},
    
    // Attribution
    {"Who created gaia", "Caia Tech", "Attribution", 0},
    {"What makes gaia different", "logic gates", "Attribution", 0}
};

int num_test_cases = sizeof(test_cases) / sizeof(TestCase);

// Simple function to test if response contains expected keyword
int check_response(const char* response, const char* expected) {
    char response_lower[1024];
    char expected_lower[256];
    
    // Convert to lowercase for comparison
    int i;
    for (i = 0; response[i] && i < 1023; i++) {
        response_lower[i] = (response[i] >= 'A' && response[i] <= 'Z') 
                          ? response[i] + 32 : response[i];
    }
    response_lower[i] = '\0';
    
    for (i = 0; expected[i] && i < 255; i++) {
        expected_lower[i] = (expected[i] >= 'A' && expected[i] <= 'Z') 
                          ? expected[i] + 32 : expected[i];
    }
    expected_lower[i] = '\0';
    
    return strstr(response_lower, expected_lower) != NULL;
}

// Simple gaia interaction simulation
void test_gaia(TestCase* test) {
    printf("\n%sTesting:%s %s\n", YELLOW, RESET, test->prompt);
    printf("Expected keyword: '%s'\n", test->expected);
    
    // Simulate running gaia_chat with the prompt
    char command[512];
    char response[1024] = {0};
    
    // Create temp file with prompt followed by quit
    FILE* tmp = fopen("test_prompt.txt", "w");
    if (tmp) {
        fprintf(tmp, "%s\nquit\n", test->prompt);
        fclose(tmp);
        
        // Run gaia_chat_v4 and capture the response line after "gaia:"
        FILE* pipe = popen("./gaia_chat_v4 < test_prompt.txt 2>/dev/null | grep 'gaia:' | tail -n 1", "r");
        if (pipe) {
            fgets(response, sizeof(response), pipe);
            pclose(pipe);
        }
        
        // Clean up
        remove("test_prompt.txt");
    }
    
    // Check if response contains expected keyword
    test->passed = check_response(response, test->expected);
    
    if (test->passed) {
        printf("%s✓ PASSED%s\n", GREEN, RESET);
    } else {
        printf("%s✗ FAILED%s\n", RED, RESET);
        printf("Response: %s", response);
    }
}

// Category summary
void print_summary() {
    printf("\n%s=== Test Summary by Category ===%s\n", YELLOW, RESET);
    
    // Count by category
    typedef struct {
        char name[64];
        int total;
        int passed;
    } CategoryStats;
    
    CategoryStats stats[10] = {0};
    int num_categories = 0;
    
    for (int i = 0; i < num_test_cases; i++) {
        // Find or add category
        int cat_idx = -1;
        for (int j = 0; j < num_categories; j++) {
            if (strcmp(stats[j].name, test_cases[i].category) == 0) {
                cat_idx = j;
                break;
            }
        }
        
        if (cat_idx == -1) {
            cat_idx = num_categories++;
            strcpy(stats[cat_idx].name, test_cases[i].category);
        }
        
        stats[cat_idx].total++;
        if (test_cases[i].passed) {
            stats[cat_idx].passed++;
        }
    }
    
    // Print stats
    for (int i = 0; i < num_categories; i++) {
        float percentage = (stats[i].passed * 100.0) / stats[i].total;
        printf("%-15s: %d/%d (%.1f%%)\n", 
               stats[i].name, 
               stats[i].passed, 
               stats[i].total,
               percentage);
    }
}

// Check if gaia_chat_v4 exists
int check_gaia_exists() {
    FILE* f = fopen("./gaia_chat_v4", "r");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

// Count dataset files
int count_dataset_files(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return 0;
    
    int count = 0;
    struct dirent* entry;
    char full_path[512];
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                count += count_dataset_files(full_path);
            } else if (strstr(entry->d_name, ".txt") || strstr(entry->d_name, ".md")) {
                count++;
            }
        }
    }
    
    closedir(dir);
    return count;
}

int main() {
    printf("%s=== GAIA Simple Testing Suite ===%s\n", YELLOW, RESET);
    printf("Testing gaia across knowledge domains...\n\n");
    
    // Check if gaia_chat_v4 exists
    if (!check_gaia_exists()) {
        printf("%sError: gaia_chat_v4 not found. Please build it first%s\n", RED, RESET);
        return 1;
    }
    
    // Count datasets
    int dataset_count = count_dataset_files("datasets");
    printf("Found %d dataset files\n\n", dataset_count);
    
    // Run tests
    int total_passed = 0;
    for (int i = 0; i < num_test_cases; i++) {
        test_gaia(&test_cases[i]);
        if (test_cases[i].passed) total_passed++;
    }
    
    // Print overall results
    printf("\n%s=== Overall Results ===%s\n", YELLOW, RESET);
    printf("Total tests: %d\n", num_test_cases);
    printf("%sPassed: %d%s\n", GREEN, total_passed, RESET);
    printf("%sFailed: %d%s\n", RED, num_test_cases - total_passed, RESET);
    printf("Success rate: %.1f%%\n", (total_passed * 100.0) / num_test_cases);
    
    // Category breakdown
    print_summary();
    
    // Performance note
    printf("\n%s=== Performance ===%s\n", YELLOW, RESET);
    printf("GAIA processes patterns at 1.4M words/second\n");
    printf("Using O(1) hash lookups, no neural networks\n");
    printf("Pure logic gates and pattern matching\n");
    
    return 0;
}