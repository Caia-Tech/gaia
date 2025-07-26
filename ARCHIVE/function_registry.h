#ifndef FUNCTION_REGISTRY_H
#define FUNCTION_REGISTRY_H

#include <stdint.h>

// Maximum function name length and result length
#define MAX_FUNCTION_NAME 64
#define MAX_FUNCTION_RESULT 512
#define MAX_FUNCTIONS 100

// Function result structure
typedef struct {
    char result[MAX_FUNCTION_RESULT];
    int success;  // 1 if function executed successfully, 0 if failed
} FunctionResult;

// Function pointer types for different signatures
typedef FunctionResult (*IntIntFunc)(int a, int b);           // add(2, 3)
typedef FunctionResult (*IntFunc)(int a);                     // fibonacci(5)
typedef FunctionResult (*DoubleIntFunc)(int precision);       // calculate_pi(10)
typedef FunctionResult (*StringFunc)(const char* word);       // define("monad")
typedef FunctionResult (*NoArgFunc)(void);                    // get_current_time()

// Function types
typedef enum {
    FUNC_INT_INT,      // (int, int) -> result
    FUNC_INT,          // (int) -> result  
    FUNC_DOUBLE_INT,   // (int) -> double result
    FUNC_STRING,       // (string) -> result
    FUNC_NO_ARG        // () -> result
} FunctionType;

// Function registry entry
typedef struct {
    char name[MAX_FUNCTION_NAME];
    FunctionType type;
    void* function_ptr;
    char description[256];
    int active;  // 1 if function is available, 0 if disabled
} FunctionEntry;

// Function registry system
typedef struct {
    FunctionEntry functions[MAX_FUNCTIONS];
    int num_functions;
} FunctionRegistry;

// Core function registry operations
void function_registry_init(void);
void function_registry_cleanup(void);

// Register functions of different types
int register_int_int_function(const char* name, IntIntFunc func, const char* description);
int register_int_function(const char* name, IntFunc func, const char* description);
int register_double_int_function(const char* name, DoubleIntFunc func, const char* description);
int register_string_function(const char* name, StringFunc func, const char* description);
int register_no_arg_function(const char* name, NoArgFunc func, const char* description);

// Function execution
FunctionResult call_function(const char* name, const char* args);
FunctionResult call_int_int_function(const char* name, int a, int b);
FunctionResult call_int_function(const char* name, int a);
FunctionResult call_double_int_function(const char* name, int precision);
FunctionResult call_string_function(const char* name, const char* arg);
FunctionResult call_no_arg_function(const char* name);

// Function discovery
int function_exists(const char* name);
void list_functions(void);
const char* get_function_description(const char* name);

#endif // FUNCTION_REGISTRY_H