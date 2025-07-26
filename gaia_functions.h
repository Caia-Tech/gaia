#ifndef GAIA_FUNCTIONS_H
#define GAIA_FUNCTIONS_H

#include "function_registry.h"

// Register all GAIA functions
void register_gaia_functions(void);

// Mathematics functions
FunctionResult gaia_add(int a, int b);
FunctionResult gaia_multiply(int a, int b);
FunctionResult gaia_subtract(int a, int b);
FunctionResult gaia_divide(int a, int b);
FunctionResult gaia_fibonacci(int n);
FunctionResult gaia_calculate_pi(int precision);
FunctionResult gaia_derivative_power(int coefficient, int power);
FunctionResult gaia_factorial(int n);

// Knowledge lookup functions
FunctionResult gaia_define(const char* word);
FunctionResult gaia_explain_concept(const char* concept);
FunctionResult gaia_classify_animal(const char* creature);

// Logic and reasoning functions  
FunctionResult gaia_compare_concepts(const char* concept1, const char* concept2);
FunctionResult gaia_is_prime(int n);

// Utility functions
FunctionResult gaia_get_time(void);
FunctionResult gaia_help(void);

#endif // GAIA_FUNCTIONS_H