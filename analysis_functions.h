#ifndef ANALYSIS_FUNCTIONS_H
#define ANALYSIS_FUNCTIONS_H

#include <stddef.h>

// Maximum sizes
#define MAX_ENTITIES 20
#define MAX_WORDS 1000
#define MAX_WORD_LEN 50

// Prompt type classification
typedef enum {
    PROMPT_UNKNOWN = 0,
    PROMPT_QUESTION,      // "What is...?", "How do...?"
    PROMPT_COMMAND,       // "Calculate...", "List...", "Tell me..."
    PROMPT_STATEMENT,     // "The cat is...", "I think..."
    PROMPT_GREETING,      // "Hello", "Hi", "Good morning"
    PROMPT_FAREWELL,      // "Goodbye", "Bye", "See you"
    PROMPT_COMPLETION,    // "The cat sat on the..."
    PROMPT_ACKNOWLEDGMENT // "Thank you", "I see", "Got it"
} PromptType;

// Question type classification
typedef enum {
    QUESTION_UNKNOWN = 0,
    QUESTION_YES_NO,      // "Is...?", "Are...?", "Can...?"
    QUESTION_WHAT,        // "What is...?"
    QUESTION_HOW,         // "How do...?", "How many...?"
    QUESTION_WHY,         // "Why...?"
    QUESTION_WHEN,        // "When...?"
    QUESTION_WHERE,       // "Where...?"
    QUESTION_WHO,         // "Who...?"
    QUESTION_WHICH        // "Which...?"
} QuestionType;

// Topic classification
typedef enum {
    TOPIC_UNKNOWN = 0,
    TOPIC_MATH,
    TOPIC_SCIENCE,
    TOPIC_TECHNOLOGY,
    TOPIC_PHILOSOPHY,
    TOPIC_CASUAL,
    TOPIC_INSTRUCTION,
    TOPIC_CREATIVE
} TopicType;

// Entity types
typedef enum {
    ENTITY_NUMBER,
    ENTITY_CATEGORY,      // "colors", "animals", etc.
    ENTITY_NAME,          // Proper nouns
    ENTITY_OPERATOR,      // "plus", "times", "minus"
    ENTITY_QUANTITY       // "three", "some", "many"
} EntityType;

// Entity structure
typedef struct {
    EntityType type;
    char value[MAX_WORD_LEN];
    int position;         // Word position in input
} Entity;

// Analysis results structure
typedef struct {
    PromptType prompt_type;
    QuestionType question_type;
    TopicType topic;
    int num_entities;
    Entity entities[MAX_ENTITIES];
    
    // Metrics
    int word_count;
    int char_count;
    int sentence_count;
    
    // Flags
    int has_typos;
    int is_complete;
    int requires_list;
    int requires_calculation;
    int is_ambiguous;
} AnalysisResult;

// Text metrics structure
typedef struct {
    int word_count;
    int char_count;
    int sentence_count;
    int avg_word_length;
    float readability_score;  // Simple metric
} TextMetrics;

// Coherence score structure
typedef struct {
    float semantic_similarity;
    float grammatical_fit;
    float topic_consistency;
    float overall_score;
} CoherenceScore;

// Main analysis functions
PromptType classify_prompt(const char* input);
QuestionType classify_question(const char* input);
TopicType detect_topic(const char* input);
int extract_entities(const char* input, Entity* entities, int max_entities);

// Text analysis functions
TextMetrics calculate_text_metrics(const char* text);
int count_words(const char* text);
int count_sentences(const char* text);
int detect_typos(const char* text, char typos[][MAX_WORD_LEN], int max_typos);

// Coherence analysis
CoherenceScore analyze_coherence(const char* context, const char* candidate);
float calculate_semantic_similarity(const char* word1, const char* word2);
int is_grammatically_valid_continuation(const char* context, const char* next_word);

// Helper functions
int is_question_word(const char* word);
int is_greeting(const char* text);
int is_farewell(const char* text);
int contains_number(const char* word);
int extract_number_from_word(const char* word);
const char* number_word_to_digit(const char* word);

// Response formatting helpers
char* format_list_response(const char* category, const char* items[], int count);
char* format_yes_no_response(int is_yes, const char* explanation);
char* format_number_response(int number, const char* context);
char* format_calculation_response(const char* operation, int num1, int num2, int result);

// Comprehensive analysis function
AnalysisResult* analyze_input(const char* input);
void free_analysis_result(AnalysisResult* result);

// Debug/logging functions
void print_analysis_result(const AnalysisResult* result);
const char* prompt_type_to_string(PromptType type);
const char* question_type_to_string(QuestionType type);
const char* topic_type_to_string(TopicType type);

#endif // ANALYSIS_FUNCTIONS_H