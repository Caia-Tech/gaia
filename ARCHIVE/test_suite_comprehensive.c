#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_RESPONSE_LEN 1024
#define MAX_PROMPTS 200

// Test categories
typedef enum {
    CAT_MATH,
    CAT_SCIENCE,
    CAT_PHILOSOPHY,
    CAT_AMBIGUOUS,
    CAT_CONVERSATIONAL,
    CAT_KNOWLEDGE,
    CAT_SEQUENCE,
    CAT_INCOMPLETE,
    CAT_CONTEXTUAL,
    CAT_CREATIVE,
    CAT_TECHNICAL,
    CAT_LOGICAL,
    CAT_TEMPORAL,
    CAT_SPATIAL,
    CAT_EMOTIONAL,
    CAT_COUNT
} TestCategory;

const char* category_names[] = {
    "Mathematics",
    "Science",
    "Philosophy",
    "Ambiguous",
    "Conversational",
    "Knowledge",
    "Sequence",
    "Incomplete",
    "Contextual",
    "Creative",
    "Technical",
    "Logical",
    "Temporal",
    "Spatial",
    "Emotional"
};

typedef struct {
    const char* prompt;
    TestCategory category;
    const char* expected_keywords[5];  // Keywords to look for in response
    int requires_function;  // 1 if this should trigger a function call
} TestCase;

// Comprehensive test cases
TestCase test_cases[] = {
    // Mathematics
    {"What is two plus two?", CAT_MATH, {"4", "four", NULL}, 1},
    {"What is five times three?", CAT_MATH, {"15", "fifteen", NULL}, 1},
    {"What is ten minus seven?", CAT_MATH, {"3", "three", NULL}, 0},
    {"What is pi?", CAT_MATH, {"3.14", NULL}, 1},
    {"What is the square root of 16?", CAT_MATH, {"4", "four", NULL}, 0},
    {"What is the derivative of x squared?", CAT_MATH, {"2x", "two x", NULL}, 1},
    {"Calculate 7 plus 8", CAT_MATH, {"15", "fifteen", NULL}, 0},
    {"What is 100 divided by 4?", CAT_MATH, {"25", "twenty-five", NULL}, 0},
    
    // Science
    {"What is gravity?", CAT_SCIENCE, {"force", "mass", "attraction", NULL}, 0},
    {"How fast does light travel?", CAT_SCIENCE, {"speed", "fast", "299", NULL}, 0},
    {"What is photosynthesis?", CAT_SCIENCE, {"plants", "light", "energy", NULL}, 0},
    {"What is quantum mechanics?", CAT_SCIENCE, {"quantum", "probability", "particles", NULL}, 0},
    {"What is DNA?", CAT_SCIENCE, {"genetic", "molecule", "heredity", NULL}, 0},
    {"What causes rain?", CAT_SCIENCE, {"water", "clouds", "precipitation", NULL}, 0},
    {"Why is the sky blue?", CAT_SCIENCE, {"light", "scatter", "wavelength", NULL}, 0},
    {"What is entropy?", CAT_SCIENCE, {"disorder", "thermodynamics", "energy", NULL}, 0},
    
    // Philosophy
    {"What is consciousness?", CAT_PHILOSOPHY, {"aware", "mind", "experience", NULL}, 0},
    {"What is the meaning of life?", CAT_PHILOSOPHY, {"meaning", "purpose", "existence", NULL}, 0},
    {"Is there free will?", CAT_PHILOSOPHY, {"choice", "determinism", "freedom", NULL}, 0},
    {"What is reality?", CAT_PHILOSOPHY, {"exist", "perception", "truth", NULL}, 0},
    {"What is truth?", CAT_PHILOSOPHY, {"fact", "reality", "knowledge", NULL}, 0},
    {"What is beauty?", CAT_PHILOSOPHY, {"aesthetic", "perception", "subjective", NULL}, 0},
    {"What is justice?", CAT_PHILOSOPHY, {"fair", "right", "moral", NULL}, 0},
    {"What is knowledge?", CAT_PHILOSOPHY, {"understanding", "information", "truth", NULL}, 0},
    
    // Ambiguous (should trigger superposition)
    {"The bank", CAT_AMBIGUOUS, {"river", "money", "holds", "near", NULL}, 0},
    {"The spring", CAT_AMBIGUOUS, {"flowers", "coiled", "water", "brings", NULL}, 0},
    {"The wave", CAT_AMBIGUOUS, {"crashed", "carried", "enthusiasm", NULL}, 0},
    {"Time", CAT_AMBIGUOUS, {"flies", "heals", "waits", NULL}, 0},
    {"Light", CAT_AMBIGUOUS, {"travels", "weighs", "illuminates", NULL}, 0},
    {"Fire", CAT_AMBIGUOUS, {"burns", "warm", "dangerous", NULL}, 0},
    {"The key", CAT_AMBIGUOUS, {"unlock", "important", "music", NULL}, 0},
    {"The current", CAT_AMBIGUOUS, {"flow", "electric", "present", NULL}, 0},
    
    // Conversational
    {"Hello, how are you?", CAT_CONVERSATIONAL, {"well", "fine", "good", "help", NULL}, 0},
    {"What's your name?", CAT_CONVERSATIONAL, {"gaia", "ai", "assistant", NULL}, 0},
    {"Can you help me?", CAT_CONVERSATIONAL, {"yes", "help", "assist", "sure", NULL}, 0},
    {"Tell me a story", CAT_CONVERSATIONAL, {"once", "upon", "time", "story", NULL}, 0},
    {"Thank you", CAT_CONVERSATIONAL, {"welcome", "pleasure", "glad", NULL}, 0},
    {"Good morning", CAT_CONVERSATIONAL, {"morning", "good", "day", NULL}, 0},
    {"How's the weather?", CAT_CONVERSATIONAL, {"weather", "nice", "sunny", "rain", NULL}, 0},
    {"What do you think?", CAT_CONVERSATIONAL, {"think", "believe", "opinion", NULL}, 0},
    
    // Knowledge queries
    {"What is a mammal?", CAT_KNOWLEDGE, {"animal", "warm", "milk", "fur", NULL}, 0},
    {"What are neurons?", CAT_KNOWLEDGE, {"brain", "cell", "nerve", "signal", NULL}, 0},
    {"Define democracy", CAT_KNOWLEDGE, {"government", "people", "vote", "rule", NULL}, 0},
    {"What is a computer?", CAT_KNOWLEDGE, {"machine", "calculate", "process", "data", NULL}, 0},
    {"What is the internet?", CAT_KNOWLEDGE, {"network", "connect", "global", "web", NULL}, 0},
    {"What is artificial intelligence?", CAT_KNOWLEDGE, {"machine", "learning", "intelligence", NULL}, 0},
    {"What is blockchain?", CAT_KNOWLEDGE, {"distributed", "ledger", "crypto", "chain", NULL}, 0},
    {"What is a virus?", CAT_KNOWLEDGE, {"infect", "disease", "replicate", NULL}, 0},
    
    // Sequences
    {"What comes after 5 in fibonacci?", CAT_SEQUENCE, {"8", "eight", NULL}, 1},
    {"Continue: 2, 4, 6, 8", CAT_SEQUENCE, {"10", "ten", NULL}, 0},
    {"Next in sequence: A, B, C", CAT_SEQUENCE, {"D", NULL}, 0},
    {"Complete: 1, 1, 2, 3, 5", CAT_SEQUENCE, {"8", "eight", NULL}, 0},
    {"What follows: red, orange, yellow", CAT_SEQUENCE, {"green", NULL}, 0},
    {"Continue pattern: big, small, big", CAT_SEQUENCE, {"small", NULL}, 0},
    {"Next: Monday, Tuesday", CAT_SEQUENCE, {"Wednesday", NULL}, 0},
    {"Complete: 10, 20, 30", CAT_SEQUENCE, {"40", "forty", NULL}, 0},
    
    // Incomplete sentences
    {"The cat sat on the", CAT_INCOMPLETE, {"mat", "chair", "floor", "roof", NULL}, 0},
    {"Once upon a time", CAT_INCOMPLETE, {"there", "lived", "was", "in", NULL}, 0},
    {"To be or not to", CAT_INCOMPLETE, {"be", NULL}, 0},
    {"In the beginning", CAT_INCOMPLETE, {"was", "there", "god", "created", NULL}, 0},
    {"All that glitters is not", CAT_INCOMPLETE, {"gold", NULL}, 0},
    {"A penny saved is", CAT_INCOMPLETE, {"penny", "earned", NULL}, 0},
    {"When in Rome", CAT_INCOMPLETE, {"do", "as", "romans", NULL}, 0},
    {"The early bird", CAT_INCOMPLETE, {"catches", "gets", "worm", NULL}, 0},
    
    // Contextual
    {"After the rain comes", CAT_CONTEXTUAL, {"sun", "sunshine", "rainbow", "clear", NULL}, 0},
    {"Before the storm", CAT_CONTEXTUAL, {"calm", "quiet", "clouds", "wind", NULL}, 0},
    {"In spring the flowers", CAT_CONTEXTUAL, {"bloom", "grow", "blossom", "appear", NULL}, 0},
    {"During winter we", CAT_CONTEXTUAL, {"cold", "snow", "freeze", "warm", NULL}, 0},
    {"At night the stars", CAT_CONTEXTUAL, {"shine", "appear", "twinkle", "visible", NULL}, 0},
    {"In the morning", CAT_CONTEXTUAL, {"wake", "sun", "breakfast", "rise", NULL}, 0},
    {"Under the sea", CAT_CONTEXTUAL, {"fish", "water", "ocean", "deep", NULL}, 0},
    {"Above the clouds", CAT_CONTEXTUAL, {"sky", "sun", "fly", "high", NULL}, 0},
    
    // Creative/Imaginative
    {"Describe a sunset", CAT_CREATIVE, {"orange", "red", "beautiful", "sky", NULL}, 0},
    {"What does happiness feel like?", CAT_CREATIVE, {"joy", "warm", "good", "smile", NULL}, 0},
    {"Paint a picture with words", CAT_CREATIVE, {"color", "image", "see", "beautiful", NULL}, 0},
    {"Imagine a perfect world", CAT_CREATIVE, {"peace", "happy", "no", "everyone", NULL}, 0},
    {"What color is the wind?", CAT_CREATIVE, {"invisible", "clear", "see", "color", NULL}, 0},
    {"How does music taste?", CAT_CREATIVE, {"sweet", "bitter", "taste", "sense", NULL}, 0},
    {"What shape is love?", CAT_CREATIVE, {"heart", "round", "shape", "form", NULL}, 0},
    {"Describe the sound of silence", CAT_CREATIVE, {"quiet", "nothing", "peaceful", "still", NULL}, 0},
    
    // Technical
    {"What is HTTP?", CAT_TECHNICAL, {"protocol", "web", "transfer", "hypertext", NULL}, 0},
    {"Explain TCP/IP", CAT_TECHNICAL, {"protocol", "network", "internet", "transmission", NULL}, 0},
    {"What is a database?", CAT_TECHNICAL, {"data", "store", "information", "organized", NULL}, 0},
    {"How do algorithms work?", CAT_TECHNICAL, {"steps", "process", "solve", "instructions", NULL}, 0},
    {"What is machine learning?", CAT_TECHNICAL, {"learn", "data", "pattern", "train", NULL}, 0},
    {"Define API", CAT_TECHNICAL, {"interface", "application", "programming", "connect", NULL}, 0},
    {"What is encryption?", CAT_TECHNICAL, {"secure", "code", "data", "protect", NULL}, 0},
    {"Explain cloud computing", CAT_TECHNICAL, {"server", "internet", "remote", "service", NULL}, 0},
    
    // Logical reasoning
    {"If Felix is a cat, what is Felix?", CAT_LOGICAL, {"animal", "mammal", "cat", NULL}, 1},
    {"All roses are flowers. This is a rose. What is it?", CAT_LOGICAL, {"flower", NULL}, 0},
    {"If A equals B and B equals C, what does A equal?", CAT_LOGICAL, {"C", NULL}, 0},
    {"True or false: All birds can fly", CAT_LOGICAL, {"false", "no", "not", NULL}, 0},
    {"If it's raining, the ground is wet. The ground is wet. Is it raining?", CAT_LOGICAL, {"maybe", "not necessarily", "possibly", NULL}, 0},
    {"Which is heavier: a pound of feathers or a pound of steel?", CAT_LOGICAL, {"same", "equal", "neither", NULL}, 0},
    {"If today is Monday, what day was yesterday?", CAT_LOGICAL, {"Sunday", NULL}, 0},
    {"Two plus two equals four. Four minus two equals?", CAT_LOGICAL, {"two", "2", NULL}, 0},
    
    // Temporal
    {"What time is it?", CAT_TEMPORAL, {"time", "clock", "now", NULL}, 0},
    {"When does the sun rise?", CAT_TEMPORAL, {"morning", "dawn", "early", "east", NULL}, 0},
    {"How long is a day?", CAT_TEMPORAL, {"24", "hours", "twenty-four", NULL}, 0},
    {"What comes after today?", CAT_TEMPORAL, {"tomorrow", NULL}, 0},
    {"When do leaves fall?", CAT_TEMPORAL, {"autumn", "fall", "season", NULL}, 0},
    {"How often does the moon orbit Earth?", CAT_TEMPORAL, {"month", "28", "days", NULL}, 0},
    {"What year is it?", CAT_TEMPORAL, {"year", "date", "time", NULL}, 0},
    {"When is midnight?", CAT_TEMPORAL, {"12", "night", "zero", NULL}, 0},
    
    // Spatial
    {"Where is the North Pole?", CAT_SPATIAL, {"north", "arctic", "top", "earth", NULL}, 0},
    {"What's above the sky?", CAT_SPATIAL, {"space", "stars", "universe", "cosmos", NULL}, 0},
    {"Where does the sun set?", CAT_SPATIAL, {"west", NULL}, 0},
    {"What's at the center of Earth?", CAT_SPATIAL, {"core", "hot", "molten", "center", NULL}, 0},
    {"Where do rivers flow?", CAT_SPATIAL, {"sea", "ocean", "downhill", "water", NULL}, 0},
    {"What's between Earth and the Moon?", CAT_SPATIAL, {"space", "vacuum", "nothing", "void", NULL}, 0},
    {"Where is the equator?", CAT_SPATIAL, {"middle", "center", "earth", "line", NULL}, 0},
    {"What's underground?", CAT_SPATIAL, {"soil", "rock", "roots", "earth", NULL}, 0},
    
    // Emotional
    {"What is love?", CAT_EMOTIONAL, {"feeling", "emotion", "care", "heart", NULL}, 0},
    {"Why do we cry?", CAT_EMOTIONAL, {"sad", "emotion", "tears", "feel", NULL}, 0},
    {"What makes people happy?", CAT_EMOTIONAL, {"joy", "love", "friends", "smile", NULL}, 0},
    {"What is fear?", CAT_EMOTIONAL, {"emotion", "afraid", "danger", "feeling", NULL}, 0},
    {"Why do we laugh?", CAT_EMOTIONAL, {"funny", "happy", "joy", "humor", NULL}, 0},
    {"What causes anger?", CAT_EMOTIONAL, {"frustration", "mad", "upset", "emotion", NULL}, 0},
    {"How does loneliness feel?", CAT_EMOTIONAL, {"alone", "sad", "empty", "isolated", NULL}, 0},
    {"What brings peace?", CAT_EMOTIONAL, {"calm", "quiet", "harmony", "tranquil", NULL}, 0},
    
    {NULL, 0, {NULL}, 0}  // Sentinel
};

// Test result structure
typedef struct {
    int total_tests;
    int responded;
    int empty_responses;
    int function_calls;
    int keyword_matches;
    int category_scores[CAT_COUNT];
    int category_totals[CAT_COUNT];
    double avg_response_length;
    int unique_responses;
    int superposition_likely;  // Responses that vary between runs
} TestResults;

// Run a single test
char* run_single_test(const char* prompt, const char* mode_flags) {
    static char response[MAX_RESPONSE_LEN];
    char cmd[2048];
    
    // Create command
    snprintf(cmd, sizeof(cmd), "echo -e \"%s\\nquit\" | ./gaia_chat_v5 %s 2>&1 | grep \"gaia:\" | head -1 | sed 's/.*gaia: //'", 
             prompt, mode_flags ? mode_flags : "");
    
    // Run command
    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;
    
    // Read response
    response[0] = '\0';
    if (fgets(response, sizeof(response), fp)) {
        // Remove newline
        size_t len = strlen(response);
        if (len > 0 && response[len-1] == '\n') {
            response[len-1] = '\0';
        }
    }
    
    pclose(fp);
    return response;
}

// Check if response contains any keywords
int check_keywords(const char* response, const char* keywords[]) {
    if (!response || !keywords) return 0;
    
    // Convert response to lowercase for comparison
    char lower_response[MAX_RESPONSE_LEN];
    strncpy(lower_response, response, MAX_RESPONSE_LEN-1);
    lower_response[MAX_RESPONSE_LEN-1] = '\0';
    
    for (int i = 0; lower_response[i]; i++) {
        lower_response[i] = tolower(lower_response[i]);
    }
    
    // Check each keyword
    for (int i = 0; keywords[i] != NULL && i < 5; i++) {
        if (strstr(lower_response, keywords[i])) {
            return 1;
        }
    }
    
    return 0;
}

// Run comprehensive test suite
void run_test_suite(const char* mode_name, const char* mode_flags, TestResults* results) {
    printf("\n=== Testing %s Mode ===\n", mode_name);
    
    // Initialize results
    memset(results, 0, sizeof(TestResults));
    
    // Track unique responses for variety analysis
    char unique_responses[MAX_PROMPTS][MAX_RESPONSE_LEN];
    int unique_count = 0;
    
    // Run each test
    for (int i = 0; test_cases[i].prompt != NULL; i++) {
        TestCase* tc = &test_cases[i];
        results->total_tests++;
        results->category_totals[tc->category]++;
        
        // Run test
        char* response = run_single_test(tc->prompt, mode_flags);
        
        if (response && strlen(response) > 0) {
            results->responded++;
            results->avg_response_length += strlen(response);
            
            // Check for function call (numeric response)
            if (tc->requires_function) {
                char* endptr;
                strtod(response, &endptr);
                if (endptr != response) {
                    results->function_calls++;
                }
            }
            
            // Check keywords
            if (check_keywords(response, tc->expected_keywords)) {
                results->keyword_matches++;
                results->category_scores[tc->category]++;
            }
            
            // Track unique responses
            int is_unique = 1;
            for (int j = 0; j < unique_count; j++) {
                if (strcmp(unique_responses[j], response) == 0) {
                    is_unique = 0;
                    break;
                }
            }
            if (is_unique && unique_count < MAX_PROMPTS) {
                strcpy(unique_responses[unique_count++], response);
            }
            
            // Show sample responses
            if (i < 10 || i % 20 == 0) {
                printf("  [%s] \"%s\" → %s\n", 
                       category_names[tc->category],
                       tc->prompt,
                       response);
            }
        } else {
            results->empty_responses++;
        }
    }
    
    results->unique_responses = unique_count;
    if (results->responded > 0) {
        results->avg_response_length /= results->responded;
    }
}

// Test for superposition variety
void test_superposition_variety(TestResults* baseline, TestResults* super) {
    printf("\n=== Testing Superposition Variety ===\n");
    
    // Test ambiguous prompts multiple times
    const char* ambiguous_prompts[] = {
        "The bank", "The spring", "The wave", "Time", "Light", NULL
    };
    
    for (int p = 0; ambiguous_prompts[p] != NULL; p++) {
        printf("\nTesting '%s' (5 runs each):\n", ambiguous_prompts[p]);
        
        // Collect baseline responses
        printf("  Baseline: ");
        char first_baseline[MAX_RESPONSE_LEN] = "";
        int baseline_varies = 0;
        
        for (int i = 0; i < 5; i++) {
            char* response = run_single_test(ambiguous_prompts[p], "");
            if (i == 0) {
                strcpy(first_baseline, response);
            } else if (strcmp(first_baseline, response) != 0) {
                baseline_varies = 1;
            }
        }
        printf("%s\n", baseline_varies ? "VARIES" : "CONSISTENT");
        
        // Collect superposition responses
        printf("  Superposition: ");
        char first_super[MAX_RESPONSE_LEN] = "";
        int super_varies = 0;
        char* responses[5];
        
        for (int i = 0; i < 5; i++) {
            responses[i] = run_single_test(ambiguous_prompts[p], "--superposition");
            if (i == 0) {
                strcpy(first_super, responses[i]);
            } else if (strcmp(first_super, responses[i]) != 0) {
                super_varies = 1;
            }
            usleep(10000);  // 10ms delay for different seeds
        }
        
        printf("%s", super_varies ? "VARIES" : "CONSISTENT");
        if (super_varies) {
            super->superposition_likely++;
            printf(" (");
            for (int i = 0; i < 5; i++) {
                if (i > 0) printf(", ");
                printf("'%s'", strtok(responses[i], " ") ?: "");
            }
            printf(")");
        }
        printf("\n");
    }
}

// Print detailed results
void print_results(const char* mode_name, TestResults* results) {
    printf("\n=== %s Results Summary ===\n", mode_name);
    printf("Total tests: %d\n", results->total_tests);
    printf("Responses: %d (%.1f%%)\n", results->responded, 
           (results->responded * 100.0) / results->total_tests);
    printf("Empty responses: %d\n", results->empty_responses);
    printf("Function calls: %d\n", results->function_calls);
    printf("Keyword matches: %d (%.1f%%)\n", results->keyword_matches,
           (results->keyword_matches * 100.0) / results->responded);
    printf("Average response length: %.1f characters\n", results->avg_response_length);
    printf("Unique responses: %d\n", results->unique_responses);
    
    if (results->superposition_likely > 0) {
        printf("Superposition variety detected: %d prompts\n", results->superposition_likely);
    }
    
    printf("\nCategory Performance:\n");
    for (int i = 0; i < CAT_COUNT; i++) {
        if (results->category_totals[i] > 0) {
            printf("  %-20s: %d/%d (%.1f%%)\n", 
                   category_names[i],
                   results->category_scores[i],
                   results->category_totals[i],
                   (results->category_scores[i] * 100.0) / results->category_totals[i]);
        }
    }
}

// Compare two result sets
void compare_results(TestResults* baseline, TestResults* super) {
    printf("\n=== Comparison: Baseline vs Superposition ===\n");
    
    printf("Response rate: %.1f%% vs %.1f%% (%+.1f%%)\n",
           (baseline->responded * 100.0) / baseline->total_tests,
           (super->responded * 100.0) / super->total_tests,
           ((super->responded - baseline->responded) * 100.0) / baseline->total_tests);
    
    printf("Keyword accuracy: %.1f%% vs %.1f%% (%+.1f%%)\n",
           (baseline->keyword_matches * 100.0) / baseline->responded,
           (super->keyword_matches * 100.0) / super->responded,
           ((super->keyword_matches * 100.0) / super->responded) - 
           ((baseline->keyword_matches * 100.0) / baseline->responded));
    
    printf("Response variety: %d vs %d unique responses\n",
           baseline->unique_responses, super->unique_responses);
    
    printf("Function accuracy: %d vs %d\n",
           baseline->function_calls, super->function_calls);
    
    printf("\nCategory improvements with superposition:\n");
    for (int i = 0; i < CAT_COUNT; i++) {
        if (baseline->category_totals[i] > 0) {
            double base_pct = (baseline->category_scores[i] * 100.0) / baseline->category_totals[i];
            double super_pct = (super->category_scores[i] * 100.0) / super->category_totals[i];
            if (super_pct != base_pct) {
                printf("  %-20s: %+.1f%% (%.1f%% → %.1f%%)\n", 
                       category_names[i], super_pct - base_pct, base_pct, super_pct);
            }
        }
    }
}

int main() {
    printf("GAIA V5 Comprehensive Test Suite\n");
    printf("================================\n");
    
    // Check if gaia_chat_v5 exists
    if (system("test -f ./gaia_chat_v5") != 0) {
        printf("Error: gaia_chat_v5 not found. Please compile it first.\n");
        return 1;
    }
    
    TestResults baseline_results, super_results;
    
    // Run baseline tests
    run_test_suite("Baseline", "", &baseline_results);
    
    // Run superposition tests
    run_test_suite("Superposition", "--superposition", &super_results);
    
    // Test variety specifically
    test_superposition_variety(&baseline_results, &super_results);
    
    // Print results
    print_results("Baseline", &baseline_results);
    print_results("Superposition", &super_results);
    
    // Compare results
    compare_results(&baseline_results, &super_results);
    
    // Save results to file
    FILE* report = fopen("test_suite_report.txt", "w");
    if (report) {
        fprintf(report, "GAIA V5 Test Suite Report\n");
        fprintf(report, "Generated: %s\n", ctime(&(time_t){time(NULL)}));
        fprintf(report, "\nTest Summary:\n");
        fprintf(report, "Total test cases: %d\n", baseline_results.total_tests);
        fprintf(report, "Categories tested: %d\n", CAT_COUNT);
        fprintf(report, "\nBaseline Mode:\n");
        fprintf(report, "  Response rate: %.1f%%\n", 
                (baseline_results.responded * 100.0) / baseline_results.total_tests);
        fprintf(report, "  Keyword accuracy: %.1f%%\n",
                (baseline_results.keyword_matches * 100.0) / baseline_results.responded);
        fprintf(report, "\nSuperposition Mode:\n");
        fprintf(report, "  Response rate: %.1f%%\n", 
                (super_results.responded * 100.0) / super_results.total_tests);
        fprintf(report, "  Keyword accuracy: %.1f%%\n",
                (super_results.keyword_matches * 100.0) / super_results.responded);
        fprintf(report, "  Variety detected: %d prompts\n", super_results.superposition_likely);
        fclose(report);
        printf("\nDetailed report saved to test_suite_report.txt\n");
    }
    
    return 0;
}