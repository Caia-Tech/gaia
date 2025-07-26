/**
 * Simple calculation functions for GAIA V9
 * Just the basics that actually work
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "gaia_simple.h"

// Try to parse and calculate from input
int try_calculate(const char* input, double* result) {
    char clean_input[MAX_STRING];
    strncpy(clean_input, input, MAX_STRING - 1);
    
    // Convert to lowercase for easier parsing
    for (int i = 0; clean_input[i]; i++) {
        clean_input[i] = tolower(clean_input[i]);
    }
    
    // Look for calculation patterns
    double num1, num2;
    char operation[20];
    
    // Pattern: "X plus Y", "X minus Y", etc.
    if (sscanf(clean_input, "%lf plus %lf", &num1, &num2) == 2) {
        *result = num1 + num2;
        return 1;
    }
    
    if (sscanf(clean_input, "%lf minus %lf", &num1, &num2) == 2) {
        *result = num1 - num2;
        return 1;
    }
    
    if (sscanf(clean_input, "%lf times %lf", &num1, &num2) == 2 ||
        sscanf(clean_input, "%lf multiplied by %lf", &num1, &num2) == 2) {
        *result = num1 * num2;
        return 1;
    }
    
    if (sscanf(clean_input, "%lf divided by %lf", &num1, &num2) == 2) {
        if (num2 == 0) {
            return 0; // Division by zero
        }
        *result = num1 / num2;
        return 1;
    }
    
    // Pattern: "what is X + Y"
    if (strstr(clean_input, "what is") || strstr(clean_input, "what's")) {
        char* expr = strstr(clean_input, "is");
        if (!expr) expr = strstr(clean_input, "'s");
        if (expr) {
            expr += 2; // Skip "is" or "'s"
            while (*expr == ' ') expr++;
            
            // Try basic operators
            if (sscanf(expr, "%lf + %lf", &num1, &num2) == 2) {
                *result = num1 + num2;
                return 1;
            }
            if (sscanf(expr, "%lf - %lf", &num1, &num2) == 2) {
                *result = num1 - num2;
                return 1;
            }
            if (sscanf(expr, "%lf * %lf", &num1, &num2) == 2) {
                *result = num1 * num2;
                return 1;
            }
            if (sscanf(expr, "%lf / %lf", &num1, &num2) == 2 && num2 != 0) {
                *result = num1 / num2;
                return 1;
            }
        }
    }
    
    // Check for prime number questions
    if (strstr(clean_input, "prime")) {
        int num;
        if (sscanf(clean_input, "is %d a prime", &num) == 1 ||
            sscanf(clean_input, "is %d prime", &num) == 1) {
            
            if (num < 2) {
                *result = 0;
                return 2; // Special return for prime check
            }
            
            for (int i = 2; i * i <= num; i++) {
                if (num % i == 0) {
                    *result = 0;
                    return 2;
                }
            }
            
            *result = 1;
            return 2; // Special return for prime check
        }
    }
    
    return 0; // Not a calculation
}