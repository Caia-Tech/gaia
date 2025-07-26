#ifndef EXPLANATIONS_H
#define EXPLANATIONS_H

// Explanation structure
typedef struct {
    const char* topic;
    const char* keywords[10];
    const char* explanation;
} Explanation;

// Function declarations
char* generate_explanation(const char* query);
const char* find_explanation(const char* topic);

#endif // EXPLANATIONS_H