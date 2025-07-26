#include "analysis_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Common question words
static const char* question_words[] = {
    "what", "how", "why", "when", "where", "who", "which", "is", "are", 
    "can", "could", "would", "should", "do", "does", "did", "will", NULL
};

// Common greetings
static const char* greetings[] = {
    "hello", "hi", "hey", "greetings", "good morning", "good afternoon",
    "good evening", "howdy", "welcome", NULL
};

// Common farewells
static const char* farewells[] = {
    "goodbye", "bye", "farewell", "see you", "take care", "later",
    "goodnight", "ciao", "adios", NULL
};

// Number words
static const char* number_words[] = {
    "zero", "one", "two", "three", "four", "five", "six", "seven", 
    "eight", "nine", "ten", "eleven", "twelve", "thirteen", "fourteen",
    "fifteen", "sixteen", "seventeen", "eighteen", "nineteen", "twenty",
    "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety",
    "hundred", "thousand", NULL
};

// Math operation keywords
static const char* math_operators[] = {
    "plus", "add", "minus", "subtract", "times", "multiply", "divided",
    "divide", "equals", "sum", "difference", "product", NULL
};

// Helper: Convert string to lowercase
static void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Helper: Check if string starts with prefix
static int starts_with(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

// Helper: Check if word is in list
static int word_in_list(const char* word, const char** list) {
    for (int i = 0; list[i] != NULL; i++) {
        if (strcasecmp(word, list[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Count words in text
int count_words(const char* text) {
    if (!text) return 0;
    
    int count = 0;
    int in_word = 0;
    
    for (int i = 0; text[i]; i++) {
        if (isalnum(text[i])) {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
    }
    
    return count;
}

// Count sentences (simple: count . ! ?)
int count_sentences(const char* text) {
    if (!text) return 0;
    
    int count = 0;
    for (int i = 0; text[i]; i++) {
        if (text[i] == '.' || text[i] == '!' || text[i] == '?') {
            count++;
        }
    }
    
    // If no sentence endings, but has content, count as 1
    if (count == 0 && strlen(text) > 0) {
        count = 1;
    }
    
    return count;
}

// Calculate text metrics
TextMetrics calculate_text_metrics(const char* text) {
    TextMetrics metrics = {0};
    
    if (!text) return metrics;
    
    metrics.word_count = count_words(text);
    metrics.char_count = strlen(text);
    metrics.sentence_count = count_sentences(text);
    
    // Calculate average word length
    int total_word_chars = 0;
    int word_chars = 0;
    int words_counted = 0;
    
    for (int i = 0; text[i]; i++) {
        if (isalnum(text[i])) {
            word_chars++;
        } else if (word_chars > 0) {
            total_word_chars += word_chars;
            words_counted++;
            word_chars = 0;
        }
    }
    if (word_chars > 0) {
        total_word_chars += word_chars;
        words_counted++;
    }
    
    if (words_counted > 0) {
        metrics.avg_word_length = total_word_chars / words_counted;
    }
    
    // Simple readability: shorter words and sentences = higher score
    if (metrics.sentence_count > 0 && metrics.word_count > 0) {
        float avg_words_per_sentence = (float)metrics.word_count / metrics.sentence_count;
        metrics.readability_score = 100.0 - (avg_words_per_sentence * 2.0) - (metrics.avg_word_length * 5.0);
        if (metrics.readability_score < 0) metrics.readability_score = 0;
        if (metrics.readability_score > 100) metrics.readability_score = 100;
    }
    
    return metrics;
}

// Check if word is a question word
int is_question_word(const char* word) {
    return word_in_list(word, question_words);
}

// Check if text is a greeting
int is_greeting(const char* text) {
    char lower[256];
    strncpy(lower, text, 255);
    lower[255] = '\0';
    to_lowercase(lower);
    
    for (int i = 0; greetings[i] != NULL; i++) {
        // Check if greeting word appears as whole word, not substring
        char* found = strstr(lower, greetings[i]);
        if (found) {
            int len = strlen(greetings[i]);
            // Check if it's at start and followed by space/end, or preceded by space
            if ((found == lower || *(found-1) == ' ') && 
                (found[len] == '\0' || found[len] == ' ' || found[len] == ',' || found[len] == '!')) {
                return 1;
            }
        }
    }
    return 0;
}

// Check if text is a farewell
int is_farewell(const char* text) {
    char lower[256];
    strncpy(lower, text, 255);
    lower[255] = '\0';
    to_lowercase(lower);
    
    for (int i = 0; farewells[i] != NULL; i++) {
        if (strstr(lower, farewells[i]) != NULL) {
            return 1;
        }
    }
    return 0;
}

// Classify prompt type
PromptType classify_prompt(const char* input) {
    if (!input || strlen(input) == 0) return PROMPT_UNKNOWN;
    
    // Check for greetings/farewells first
    if (is_greeting(input)) return PROMPT_GREETING;
    if (is_farewell(input)) return PROMPT_FAREWELL;
    
    // Check for question marks
    if (strchr(input, '?') != NULL) return PROMPT_QUESTION;
    
    // Check for question words at start
    char first_word[50] = {0};
    sscanf(input, "%49s", first_word);
    to_lowercase(first_word);
    
    if (is_question_word(first_word)) return PROMPT_QUESTION;
    
    // Check for commands
    if (starts_with(first_word, "calculate") || starts_with(first_word, "compute") ||
        starts_with(first_word, "list") || starts_with(first_word, "show") ||
        starts_with(first_word, "tell") || starts_with(first_word, "explain") ||
        starts_with(first_word, "define") || starts_with(first_word, "describe")) {
        return PROMPT_COMMAND;
    }
    
    // Check for acknowledgments
    if (strcasecmp(input, "thank you") == 0 || strcasecmp(input, "thanks") == 0 ||
        strcasecmp(input, "i see") == 0 || strcasecmp(input, "got it") == 0 ||
        strcasecmp(input, "okay") == 0 || strcasecmp(input, "ok") == 0) {
        return PROMPT_ACKNOWLEDGMENT;
    }
    
    // Check for incomplete sentences (ends with common incomplete markers)
    int len = strlen(input);
    if (len > 0) {
        // Remove trailing spaces
        while (len > 0 && isspace(input[len-1])) len--;
        
        // Check for words that typically need completion
        const char* incomplete_endings[] = {"the", "a", "an", "to", "of", "in", "on", "at", NULL};
        char last_word[50] = {0};
        
        // Extract last word
        int j = 0;
        for (int i = len - 1; i >= 0 && !isspace(input[i]) && j < 49; i--) {
            last_word[j++] = input[i];
        }
        last_word[j] = '\0';
        
        // Reverse the word
        for (int i = 0; i < j/2; i++) {
            char temp = last_word[i];
            last_word[i] = last_word[j-1-i];
            last_word[j-1-i] = temp;
        }
        
        to_lowercase(last_word);
        if (word_in_list(last_word, incomplete_endings)) {
            return PROMPT_COMPLETION;
        }
    }
    
    // Default to statement
    return PROMPT_STATEMENT;
}

// Classify question type
QuestionType classify_question(const char* input) {
    if (!input) return QUESTION_UNKNOWN;
    
    char lower[256];
    strncpy(lower, input, 255);
    lower[255] = '\0';
    to_lowercase(lower);
    
    // Check first word
    char first_word[50] = {0};
    sscanf(lower, "%49s", first_word);
    
    if (strcmp(first_word, "what") == 0) return QUESTION_WHAT;
    if (strcmp(first_word, "how") == 0) return QUESTION_HOW;
    if (strcmp(first_word, "why") == 0) return QUESTION_WHY;
    if (strcmp(first_word, "when") == 0) return QUESTION_WHEN;
    if (strcmp(first_word, "where") == 0) return QUESTION_WHERE;
    if (strcmp(first_word, "who") == 0) return QUESTION_WHO;
    if (strcmp(first_word, "which") == 0) return QUESTION_WHICH;
    
    // Yes/no questions typically start with is/are/can/do/does/will
    if (strcmp(first_word, "is") == 0 || strcmp(first_word, "are") == 0 ||
        strcmp(first_word, "can") == 0 || strcmp(first_word, "could") == 0 ||
        strcmp(first_word, "do") == 0 || strcmp(first_word, "does") == 0 ||
        strcmp(first_word, "did") == 0 || strcmp(first_word, "will") == 0 ||
        strcmp(first_word, "would") == 0 || strcmp(first_word, "should") == 0) {
        return QUESTION_YES_NO;
    }
    
    return QUESTION_UNKNOWN;
}

// Detect topic
TopicType detect_topic(const char* input) {
    if (!input) return TOPIC_UNKNOWN;
    
    char lower[512];
    strncpy(lower, input, 511);
    lower[511] = '\0';
    to_lowercase(lower);
    
    // Math keywords
    const char* math_words[] = {"plus", "minus", "times", "divide", "equals", 
                                "calculate", "sum", "product", "math", "number",
                                "add", "subtract", "multiply", NULL};
    for (int i = 0; math_words[i]; i++) {
        if (strstr(lower, math_words[i])) return TOPIC_MATH;
    }
    
    // Science keywords
    const char* science_words[] = {"atom", "molecule", "energy", "gravity", "physics",
                                   "chemistry", "biology", "science", "quantum",
                                   "evolution", "photosynthesis", "electron", NULL};
    for (int i = 0; science_words[i]; i++) {
        if (strstr(lower, science_words[i])) return TOPIC_SCIENCE;
    }
    
    // Technology keywords
    const char* tech_words[] = {"computer", "software", "algorithm", "internet",
                                "program", "code", "data", "network", "digital",
                                "technology", "artificial intelligence", NULL};
    for (int i = 0; tech_words[i]; i++) {
        if (strstr(lower, tech_words[i])) return TOPIC_TECHNOLOGY;
    }
    
    // Philosophy keywords
    const char* philosophy_words[] = {"meaning", "consciousness", "reality", "truth",
                                      "existence", "philosophy", "ethics", "moral",
                                      "free will", "knowledge", NULL};
    for (int i = 0; philosophy_words[i]; i++) {
        if (strstr(lower, philosophy_words[i])) return TOPIC_PHILOSOPHY;
    }
    
    // Instruction keywords
    if (strstr(lower, "list") || strstr(lower, "tell me") || 
        strstr(lower, "show me") || strstr(lower, "explain")) {
        return TOPIC_INSTRUCTION;
    }
    
    // Creative keywords
    if (strstr(lower, "story") || strstr(lower, "imagine") || 
        strstr(lower, "create") || strstr(lower, "describe")) {
        return TOPIC_CREATIVE;
    }
    
    // Default to casual
    return TOPIC_CASUAL;
}

// Convert number word to digit
const char* number_word_to_digit(const char* word) {
    static char digit[10];
    
    struct {
        const char* word;
        const char* digit;
    } conversions[] = {
        {"zero", "0"}, {"one", "1"}, {"two", "2"}, {"three", "3"},
        {"four", "4"}, {"five", "5"}, {"six", "6"}, {"seven", "7"},
        {"eight", "8"}, {"nine", "9"}, {"ten", "10"}, {"eleven", "11"},
        {"twelve", "12"}, {"thirteen", "13"}, {"fourteen", "14"},
        {"fifteen", "15"}, {"sixteen", "16"}, {"seventeen", "17"},
        {"eighteen", "18"}, {"nineteen", "19"}, {"twenty", "20"},
        {NULL, NULL}
    };
    
    for (int i = 0; conversions[i].word; i++) {
        if (strcasecmp(word, conversions[i].word) == 0) {
            strcpy(digit, conversions[i].digit);
            return digit;
        }
    }
    
    return NULL;
}

// Extract entities from input
int extract_entities(const char* input, Entity* entities, int max_entities) {
    if (!input || !entities || max_entities <= 0) return 0;
    
    int entity_count = 0;
    char word[MAX_WORD_LEN];
    int position = 0;
    
    // Tokenize input
    char* input_copy = strdup(input);
    char* token = strtok(input_copy, " \t\n,.!?;:");
    
    while (token && entity_count < max_entities) {
        strcpy(word, token);
        to_lowercase(word);
        
        // Check for numbers (digits, including negative)
        int is_number = 1;
        int start_idx = 0;
        
        // Handle negative sign
        if (token[0] == '-' && token[1] != '\0') {
            start_idx = 1;
        }
        
        // Check remaining characters are digits
        for (int i = start_idx; token[i]; i++) {
            if (!isdigit(token[i])) {
                is_number = 0;
                break;
            }
        }
        
        // Must have at least one digit
        if (is_number && strlen(token + start_idx) > 0) {
            entities[entity_count].type = ENTITY_NUMBER;
            strcpy(entities[entity_count].value, token);
            entities[entity_count].position = position;
            entity_count++;
        }
        // Check for number words
        else if (word_in_list(word, number_words)) {
            entities[entity_count].type = ENTITY_NUMBER;
            const char* digit = number_word_to_digit(word);
            if (digit) {
                strcpy(entities[entity_count].value, digit);
            } else {
                strcpy(entities[entity_count].value, token);
            }
            entities[entity_count].position = position;
            entity_count++;
        }
        // Check for operators
        else if (word_in_list(word, math_operators)) {
            entities[entity_count].type = ENTITY_OPERATOR;
            strcpy(entities[entity_count].value, word);
            entities[entity_count].position = position;
            entity_count++;
        }
        // Check for quantity words
        else if (strcmp(word, "some") == 0 || strcmp(word, "many") == 0 ||
                 strcmp(word, "few") == 0 || strcmp(word, "all") == 0) {
            entities[entity_count].type = ENTITY_QUANTITY;
            strcpy(entities[entity_count].value, word);
            entities[entity_count].position = position;
            entity_count++;
        }
        // Check for categories (simple heuristic: plural nouns after "list", "name", etc.)
        else if (position > 0 && strlen(word) > 2 && word[strlen(word)-1] == 's') {
            // Check if previous word suggests a list request
            if (position == 1 || position == 2) {  // Close to start
                entities[entity_count].type = ENTITY_CATEGORY;
                strcpy(entities[entity_count].value, word);
                entities[entity_count].position = position;
                entity_count++;
            }
        }
        
        position++;
        token = strtok(NULL, " \t\n,.!?;:");
    }
    
    free(input_copy);
    return entity_count;
}

// Analyze coherence between context and candidate
CoherenceScore analyze_coherence(const char* context, const char* candidate) {
    CoherenceScore score = {0};
    
    if (!context || !candidate) {
        return score;
    }
    
    // Simple semantic similarity based on common patterns
    // This is a placeholder - real implementation would be more sophisticated
    
    // Check if candidate is a common continuation
    struct {
        const char* context_end;
        const char* good_candidates[5];
    } patterns[] = {
        {"the cat", {"sat", "is", "was", "ran", "jumped"}},
        {"sat on", {"the", "a", "his", "her", "my"}},
        {"on the", {"floor", "mat", "chair", "table", "ground"}},
        {"how are", {"you", "things", "we", "they", NULL}},
        {"thank", {"you", "goodness", NULL}},
        {NULL, {NULL}}
    };
    
    // Extract last few words of context
    char context_end[100] = {0};
    int len = strlen(context);
    int start = len > 20 ? len - 20 : 0;
    strcpy(context_end, context + start);
    to_lowercase(context_end);
    
    char candidate_lower[MAX_WORD_LEN];
    strcpy(candidate_lower, candidate);
    to_lowercase(candidate_lower);
    
    // Check patterns
    for (int i = 0; patterns[i].context_end; i++) {
        if (strstr(context_end, patterns[i].context_end)) {
            for (int j = 0; patterns[i].good_candidates[j]; j++) {
                if (strcmp(candidate_lower, patterns[i].good_candidates[j]) == 0) {
                    score.semantic_similarity = 0.8;
                    score.grammatical_fit = 0.9;
                    break;
                }
            }
        }
    }
    
    // Basic grammatical checks
    if (strlen(context) > 0) {
        // After "the", "a", "an" we expect nouns/adjectives
        if (strstr(context_end, " the") == context_end + strlen(context_end) - 4 ||
            strstr(context_end, " a") == context_end + strlen(context_end) - 2) {
            // Candidate should not be a verb ending in 's' or 'ed'
            if (!(candidate[strlen(candidate)-1] == 's' && strlen(candidate) > 3) &&
                !(strstr(candidate, "ed") == candidate + strlen(candidate) - 2)) {
                score.grammatical_fit += 0.3;
            }
        }
    }
    
    // Topic consistency (simplified)
    TopicType context_topic = detect_topic(context);
    TopicType candidate_topic = detect_topic(candidate);
    
    if (context_topic == candidate_topic || 
        context_topic == TOPIC_UNKNOWN || 
        candidate_topic == TOPIC_UNKNOWN) {
        score.topic_consistency = 0.7;
    } else {
        score.topic_consistency = 0.3;
    }
    
    // Calculate overall score
    score.overall_score = (score.semantic_similarity * 0.4 +
                          score.grammatical_fit * 0.3 +
                          score.topic_consistency * 0.3);
    
    return score;
}

// Comprehensive input analysis
AnalysisResult* analyze_input(const char* input) {
    AnalysisResult* result = calloc(1, sizeof(AnalysisResult));
    if (!result) return NULL;
    
    // Basic classification
    result->prompt_type = classify_prompt(input);
    result->question_type = classify_question(input);
    result->topic = detect_topic(input);
    
    // Extract entities
    result->num_entities = extract_entities(input, result->entities, MAX_ENTITIES);
    
    // Calculate metrics
    TextMetrics metrics = calculate_text_metrics(input);
    result->word_count = metrics.word_count;
    result->char_count = metrics.char_count;
    result->sentence_count = metrics.sentence_count;
    
    // Set flags based on analysis
    result->is_complete = (result->prompt_type != PROMPT_COMPLETION);
    
    // Check if it requires a list
    char lower[512];
    strncpy(lower, input, 511);
    lower[511] = '\0';
    to_lowercase(lower);
    
    result->requires_list = (strstr(lower, "list") != NULL || 
                            strstr(lower, "name some") != NULL ||
                            strstr(lower, "examples") != NULL);
    
    // Check if it requires calculation
    result->requires_calculation = 0;
    for (int i = 0; i < result->num_entities; i++) {
        if (result->entities[i].type == ENTITY_OPERATOR) {
            result->requires_calculation = 1;
            break;
        }
    }
    
    // Check if ambiguous (simplified)
    result->is_ambiguous = (result->word_count <= 3 && 
                           result->prompt_type == PROMPT_STATEMENT);
    
    return result;
}

// Free analysis result
void free_analysis_result(AnalysisResult* result) {
    if (result) {
        free(result);
    }
}

// Convert enum to string for debugging
const char* prompt_type_to_string(PromptType type) {
    switch (type) {
        case PROMPT_QUESTION: return "QUESTION";
        case PROMPT_COMMAND: return "COMMAND";
        case PROMPT_STATEMENT: return "STATEMENT";
        case PROMPT_GREETING: return "GREETING";
        case PROMPT_FAREWELL: return "FAREWELL";
        case PROMPT_COMPLETION: return "COMPLETION";
        case PROMPT_ACKNOWLEDGMENT: return "ACKNOWLEDGMENT";
        default: return "UNKNOWN";
    }
}

const char* question_type_to_string(QuestionType type) {
    switch (type) {
        case QUESTION_YES_NO: return "YES_NO";
        case QUESTION_WHAT: return "WHAT";
        case QUESTION_HOW: return "HOW";
        case QUESTION_WHY: return "WHY";
        case QUESTION_WHEN: return "WHEN";
        case QUESTION_WHERE: return "WHERE";
        case QUESTION_WHO: return "WHO";
        case QUESTION_WHICH: return "WHICH";
        default: return "UNKNOWN";
    }
}

const char* topic_type_to_string(TopicType type) {
    switch (type) {
        case TOPIC_MATH: return "MATH";
        case TOPIC_SCIENCE: return "SCIENCE";
        case TOPIC_TECHNOLOGY: return "TECHNOLOGY";
        case TOPIC_PHILOSOPHY: return "PHILOSOPHY";
        case TOPIC_CASUAL: return "CASUAL";
        case TOPIC_INSTRUCTION: return "INSTRUCTION";
        case TOPIC_CREATIVE: return "CREATIVE";
        default: return "UNKNOWN";
    }
}

// Response formatting helpers
char* format_list_response(const char* category, const char* items[], int count) {
    if (!category || !items || count <= 0) return NULL;
    
    // Calculate total length needed
    int total_len = strlen("Here are ") + strlen(category) + strlen(":\n");
    for (int i = 0; i < count; i++) {
        if (items[i]) {
            total_len += strlen("  - ") + strlen(items[i]) + 1; // +1 for newline
        }
    }
    total_len += 1; // null terminator
    
    char* response = malloc(total_len);
    if (!response) return NULL;
    
    sprintf(response, "Here are %s:\n", category);
    for (int i = 0; i < count; i++) {
        if (items[i]) {
            strcat(response, "  - ");
            strcat(response, items[i]);
            strcat(response, "\n");
        }
    }
    
    return response;
}

char* format_yes_no_response(int is_yes, const char* explanation) {
    const char* answer = is_yes ? "Yes" : "No";
    int total_len = strlen(answer) + 1; // +1 for period or space
    
    if (explanation) {
        total_len += strlen(explanation) + 2; // +2 for ". " or space
    }
    total_len += 1; // null terminator
    
    char* response = malloc(total_len);
    if (!response) return NULL;
    
    strcpy(response, answer);
    if (explanation && strlen(explanation) > 0) {
        strcat(response, ". ");
        strcat(response, explanation);
    }
    
    return response;
}

char* format_number_response(int number, const char* context) {
    char number_str[20];
    sprintf(number_str, "%d", number);
    
    int total_len = strlen(number_str) + 1; // +1 for null terminator
    if (context) {
        total_len += strlen(context) + 1; // +1 for space
    }
    
    char* response = malloc(total_len);
    if (!response) return NULL;
    
    strcpy(response, number_str);
    if (context && strlen(context) > 0) {
        strcat(response, " ");
        strcat(response, context);
    }
    
    return response;
}

char* format_calculation_response(const char* operation, int num1, int num2, int result) {
    if (!operation) return NULL;
    
    char* response = malloc(100); // Should be enough for most calculations
    if (!response) return NULL;
    
    sprintf(response, "%d %s %d = %d", num1, operation, num2, result);
    return response;
}

// Print analysis result for debugging
void print_analysis_result(const AnalysisResult* result) {
    if (!result) return;
    
    printf("=== Input Analysis ===\n");
    printf("Prompt Type: %s\n", prompt_type_to_string(result->prompt_type));
    printf("Question Type: %s\n", question_type_to_string(result->question_type));
    printf("Topic: %s\n", topic_type_to_string(result->topic));
    printf("Word Count: %d\n", result->word_count);
    printf("Character Count: %d\n", result->char_count);
    printf("Sentence Count: %d\n", result->sentence_count);
    
    printf("Flags:\n");
    printf("  Complete: %s\n", result->is_complete ? "Yes" : "No");
    printf("  Requires List: %s\n", result->requires_list ? "Yes" : "No");
    printf("  Requires Calculation: %s\n", result->requires_calculation ? "Yes" : "No");
    printf("  Ambiguous: %s\n", result->is_ambiguous ? "Yes" : "No");
    
    if (result->num_entities > 0) {
        printf("Entities (%d):\n", result->num_entities);
        for (int i = 0; i < result->num_entities; i++) {
            const char* type_str;
            switch (result->entities[i].type) {
                case ENTITY_NUMBER: type_str = "NUMBER"; break;
                case ENTITY_CATEGORY: type_str = "CATEGORY"; break;
                case ENTITY_OPERATOR: type_str = "OPERATOR"; break;
                case ENTITY_QUANTITY: type_str = "QUANTITY"; break;
                default: type_str = "NAME"; break;
            }
            printf("  [%s] '%s' at position %d\n", 
                   type_str, result->entities[i].value, result->entities[i].position);
        }
    }
}