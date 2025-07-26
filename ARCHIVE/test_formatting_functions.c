#include <stdio.h>
#include <stdlib.h>
#include "analysis_functions.h"

int main() {
    printf("=== Testing V6 Response Formatting Functions ===\n\n");
    
    // Test list formatting
    printf("1. List Formatting:\n");
    const char* colors[] = {"red", "blue", "green", "yellow"};
    char* list_response = format_list_response("colors", colors, 4);
    if (list_response) {
        printf("%s", list_response);
        free(list_response);
    }
    printf("\n");
    
    // Test yes/no responses
    printf("2. Yes/No Responses:\n");
    char* yes_response = format_yes_no_response(1, "This is a valid statement");
    if (yes_response) {
        printf("Question: Is this correct? Answer: %s\n", yes_response);
        free(yes_response);
    }
    
    char* no_response = format_yes_no_response(0, "This statement is incorrect");
    if (no_response) {
        printf("Question: Is this wrong? Answer: %s\n", no_response);
        free(no_response);
    }
    printf("\n");
    
    // Test number responses
    printf("3. Number Responses:\n");
    char* num_response = format_number_response(42, "is the answer");
    if (num_response) {
        printf("The meaning of life: %s\n", num_response);
        free(num_response);
    }
    printf("\n");
    
    // Test calculation responses
    printf("4. Calculation Responses:\n");
    char* calc_response = format_calculation_response("plus", 15, 27, 42);
    if (calc_response) {
        printf("Math result: %s\n", calc_response);
        free(calc_response);
    }
    
    calc_response = format_calculation_response("times", 6, 7, 42);
    if (calc_response) {
        printf("Math result: %s\n", calc_response);
        free(calc_response);
    }
    
    printf("\n=== All formatting tests completed! ===\n");
    return 0;
}