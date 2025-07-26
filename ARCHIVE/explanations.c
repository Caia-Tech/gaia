#include "explanations.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// Database of explanations
static const Explanation explanations[] = {
    // Mathematical operations
    {
        "addition",
        {"add", "addition", "plus", "sum", "adding", NULL},
        "Addition is a mathematical operation that combines two or more numbers to find their total. "
        "When you add numbers, you're finding out how many you have altogether. "
        "For example, 3 + 5 = 8 means if you have 3 items and get 5 more, you'll have 8 total."
    },
    {
        "subtraction", 
        {"subtract", "subtraction", "minus", "take away", "difference", NULL},
        "Subtraction is a mathematical operation that finds the difference between numbers. "
        "When you subtract, you're taking away one amount from another. "
        "For example, 10 - 3 = 7 means if you have 10 items and remove 3, you'll have 7 left."
    },
    {
        "multiplication",
        {"multiply", "multiplication", "times", "product", "multiplying", NULL},
        "Multiplication is a mathematical operation that finds the result of adding a number to itself repeatedly. "
        "It's a shortcut for repeated addition. "
        "For example, 4 × 3 = 12 means adding 4 three times: 4 + 4 + 4 = 12."
    },
    {
        "division",
        {"divide", "division", "divided", "quotient", "dividing", NULL},
        "Division is a mathematical operation that splits a number into equal parts. "
        "When you divide, you're finding how many times one number fits into another. "
        "For example, 12 ÷ 3 = 4 means 12 items can be split into 3 equal groups of 4."
    },
    {
        "factorial",
        {"factorial", "!", NULL},
        "A factorial is the product of all positive integers up to a given number. "
        "It's written as n! and means n × (n-1) × (n-2) × ... × 1. "
        "For example, 5! = 5 × 4 × 3 × 2 × 1 = 120. By definition, 0! = 1."
    },
    {
        "fibonacci",
        {"fibonacci", "fib", NULL},
        "The Fibonacci sequence is a series where each number is the sum of the two before it. "
        "Starting with 0 and 1, the sequence goes: 0, 1, 1, 2, 3, 5, 8, 13, 21... "
        "It appears frequently in nature, from spiral shells to flower petals."
    },
    {
        "prime",
        {"prime", "primes", NULL},
        "A prime number is a natural number greater than 1 that has no positive divisors other than 1 and itself. "
        "The first few primes are 2, 3, 5, 7, 11, 13, 17, 19, 23... "
        "Prime numbers are the building blocks of all other numbers."
    },
    {
        "percentage",
        {"percent", "percentage", "%", NULL},
        "A percentage is a way of expressing a number as a fraction of 100. "
        "The symbol % means 'per hundred'. "
        "For example, 25% means 25 out of 100, which equals 0.25 or 1/4."
    },
    {
        "mathematics",
        {"math", "mathematics", "maths", NULL},
        "Mathematics is the science of numbers, quantities, shapes, and patterns. "
        "It provides tools for understanding and describing the world around us. "
        "Math includes arithmetic, algebra, geometry, calculus, and many other branches."
    },
    {
        "number",
        {"number", "numbers", "digit", NULL},
        "A number is a mathematical object used to count, measure, and label. "
        "Numbers can be whole (like 5), negative (like -3), fractions (like 1/2), "
        "or decimals (like 3.14). They form the foundation of mathematics."
    },
    {
        "zero",
        {"zero", "0", "nothing", NULL},
        "Zero is the number that represents nothing or no quantity. "
        "It's the additive identity, meaning any number plus zero equals itself. "
        "Zero is neither positive nor negative and serves as the boundary between them."
    },
    {
        "arithmetic",
        {"arithmetic", "basic math", NULL},
        "Arithmetic is the branch of mathematics dealing with basic operations: "
        "addition, subtraction, multiplication, and division. "
        "It's the foundation for all other mathematical concepts and daily calculations."
    },
    // End marker
    {NULL, {NULL}, NULL}
};

// Helper to lowercase a string
static void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Find explanation by keyword matching
const char* find_explanation(const char* topic) {
    if (!topic) return NULL;
    
    char lower_topic[256];
    strncpy(lower_topic, topic, 255);
    lower_topic[255] = '\0';
    to_lowercase(lower_topic);
    
    // Search through all explanations
    for (int i = 0; explanations[i].topic != NULL; i++) {
        // Check each keyword
        for (int j = 0; explanations[i].keywords[j] != NULL; j++) {
            if (strstr(lower_topic, explanations[i].keywords[j])) {
                return explanations[i].explanation;
            }
        }
    }
    
    return NULL;
}

// Generate explanation based on query
char* generate_explanation(const char* query) {
    if (!query) return NULL;
    
    // First try to find a matching explanation
    const char* explanation = find_explanation(query);
    if (explanation) {
        return strdup(explanation);
    }
    
    // Check for common explanation patterns
    char lower[512];
    strncpy(lower, query, 511);
    lower[511] = '\0';
    to_lowercase(lower);
    
    // "What is X" pattern
    if (strstr(lower, "what is") || strstr(lower, "what's")) {
        // Try to extract the topic
        char* topic_start = strstr(lower, "what is");
        if (topic_start) {
            topic_start += 7; // Skip "what is"
            while (*topic_start == ' ') topic_start++;
            
            // Remove trailing punctuation
            char topic[256];
            strncpy(topic, topic_start, 255);
            topic[255] = '\0';
            for (int i = strlen(topic) - 1; i >= 0; i--) {
                if (topic[i] == '?' || topic[i] == '.' || topic[i] == '!') {
                    topic[i] = '\0';
                } else {
                    break;
                }
            }
            
            explanation = find_explanation(topic);
            if (explanation) {
                return strdup(explanation);
            }
        }
    }
    
    // "How does X work" pattern
    if (strstr(lower, "how does") || strstr(lower, "how do")) {
        char* work_ptr = strstr(lower, "work");
        if (work_ptr) {
            // Extract topic between "how does" and "work"
            char* start = strstr(lower, "how does");
            if (start) {
                start += 8; // Skip "how does"
                while (*start == ' ') start++;
                
                int len = work_ptr - start;
                if (len > 0 && len < 200) {
                    char topic[256];
                    strncpy(topic, start, len);
                    topic[len] = '\0';
                    
                    explanation = find_explanation(topic);
                    if (explanation) {
                        return strdup(explanation);
                    }
                }
            }
        }
    }
    
    // "Explain X" pattern
    if (strstr(lower, "explain")) {
        char* explain_start = strstr(lower, "explain");
        explain_start += 7; // Skip "explain"
        while (*explain_start == ' ') explain_start++;
        
        explanation = find_explanation(explain_start);
        if (explanation) {
            return strdup(explanation);
        }
    }
    
    // Default generic response
    return strdup("This is a complex topic that involves understanding how different components work together to achieve a specific goal.");
}