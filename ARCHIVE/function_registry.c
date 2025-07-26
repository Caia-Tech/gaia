#include "function_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global function registry
static FunctionRegistry* registry = NULL;

// Initialize the function registry
void function_registry_init(void) {
    if (registry) return;  // Already initialized
    
    registry = calloc(1, sizeof(FunctionRegistry));
    if (!registry) {
        printf("Error: Failed to allocate function registry\n");
        return;
    }
    
    printf("Function registry initialized\n");
}

// Cleanup the function registry
void function_registry_cleanup(void) {
    if (registry) {
        free(registry);
        registry = NULL;
        printf("Function registry cleaned up\n");
    }
}

// Register an (int, int) -> result function
int register_int_int_function(const char* name, IntIntFunc func, const char* description) {
    if (!registry || registry->num_functions >= MAX_FUNCTIONS) return 0;
    
    FunctionEntry* entry = &registry->functions[registry->num_functions];
    strncpy(entry->name, name, MAX_FUNCTION_NAME - 1);
    entry->name[MAX_FUNCTION_NAME - 1] = '\0';
    entry->type = FUNC_INT_INT;
    entry->function_ptr = (void*)func;
    strncpy(entry->description, description, 255);
    entry->description[255] = '\0';
    entry->active = 1;
    
    registry->num_functions++;
    printf("Registered function: %s (%s)\n", name, description);
    return 1;
}

// Register an (int) -> result function
int register_int_function(const char* name, IntFunc func, const char* description) {
    if (!registry || registry->num_functions >= MAX_FUNCTIONS) return 0;
    
    FunctionEntry* entry = &registry->functions[registry->num_functions];
    strncpy(entry->name, name, MAX_FUNCTION_NAME - 1);
    entry->name[MAX_FUNCTION_NAME - 1] = '\0';
    entry->type = FUNC_INT;
    entry->function_ptr = (void*)func;
    strncpy(entry->description, description, 255);
    entry->description[255] = '\0';
    entry->active = 1;
    
    registry->num_functions++;
    printf("Registered function: %s (%s)\n", name, description);
    return 1;
}

// Register a (int) -> double result function
int register_double_int_function(const char* name, DoubleIntFunc func, const char* description) {
    if (!registry || registry->num_functions >= MAX_FUNCTIONS) return 0;
    
    FunctionEntry* entry = &registry->functions[registry->num_functions];
    strncpy(entry->name, name, MAX_FUNCTION_NAME - 1);
    entry->name[MAX_FUNCTION_NAME - 1] = '\0';
    entry->type = FUNC_DOUBLE_INT;
    entry->function_ptr = (void*)func;
    strncpy(entry->description, description, 255);
    entry->description[255] = '\0';
    entry->active = 1;
    
    registry->num_functions++;
    printf("Registered function: %s (%s)\n", name, description);
    return 1;
}

// Register a (string) -> result function
int register_string_function(const char* name, StringFunc func, const char* description) {
    if (!registry || registry->num_functions >= MAX_FUNCTIONS) return 0;
    
    FunctionEntry* entry = &registry->functions[registry->num_functions];
    strncpy(entry->name, name, MAX_FUNCTION_NAME - 1);
    entry->name[MAX_FUNCTION_NAME - 1] = '\0';
    entry->type = FUNC_STRING;
    entry->function_ptr = (void*)func;
    strncpy(entry->description, description, 255);
    entry->description[255] = '\0';
    entry->active = 1;
    
    registry->num_functions++;
    printf("Registered function: %s (%s)\n", name, description);
    return 1;
}

// Register a () -> result function
int register_no_arg_function(const char* name, NoArgFunc func, const char* description) {
    if (!registry || registry->num_functions >= MAX_FUNCTIONS) return 0;
    
    FunctionEntry* entry = &registry->functions[registry->num_functions];
    strncpy(entry->name, name, MAX_FUNCTION_NAME - 1);
    entry->name[MAX_FUNCTION_NAME - 1] = '\0';
    entry->type = FUNC_NO_ARG;
    entry->function_ptr = (void*)func;
    strncpy(entry->description, description, 255);
    entry->description[255] = '\0';
    entry->active = 1;
    
    registry->num_functions++;
    printf("Registered function: %s (%s)\n", name, description);
    return 1;
}

// Call a function by name with typed arguments
FunctionResult call_int_int_function(const char* name, int a, int b) {
    FunctionResult result = {"", 0};
    
    if (!registry) {
        strcpy(result.result, "Error: Function registry not initialized");
        return result;
    }
    
    // Find the function
    for (int i = 0; i < registry->num_functions; i++) {
        FunctionEntry* entry = &registry->functions[i];
        if (strcmp(entry->name, name) == 0 && entry->type == FUNC_INT_INT && entry->active) {
            IntIntFunc func = (IntIntFunc)entry->function_ptr;
            return func(a, b);
        }
    }
    
    snprintf(result.result, MAX_FUNCTION_RESULT, "Error: Function '%s' not found", name);
    return result;
}

// Call an (int) -> result function
FunctionResult call_int_function(const char* name, int a) {
    FunctionResult result = {"", 0};
    
    if (!registry) {
        strcpy(result.result, "Error: Function registry not initialized");
        return result;
    }
    
    // Find the function
    for (int i = 0; i < registry->num_functions; i++) {
        FunctionEntry* entry = &registry->functions[i];
        if (strcmp(entry->name, name) == 0 && entry->type == FUNC_INT && entry->active) {
            IntFunc func = (IntFunc)entry->function_ptr;
            return func(a);
        }
    }
    
    snprintf(result.result, MAX_FUNCTION_RESULT, "Error: Function '%s' not found", name);
    return result;
}

// Call a (int) -> double result function
FunctionResult call_double_int_function(const char* name, int precision) {
    FunctionResult result = {"", 0};
    
    if (!registry) {
        strcpy(result.result, "Error: Function registry not initialized");
        return result;
    }
    
    // Find the function
    for (int i = 0; i < registry->num_functions; i++) {
        FunctionEntry* entry = &registry->functions[i];
        if (strcmp(entry->name, name) == 0 && entry->type == FUNC_DOUBLE_INT && entry->active) {
            DoubleIntFunc func = (DoubleIntFunc)entry->function_ptr;
            return func(precision);
        }
    }
    
    snprintf(result.result, MAX_FUNCTION_RESULT, "Error: Function '%s' not found", name);
    return result;
}

// Call a (string) -> result function
FunctionResult call_string_function(const char* name, const char* arg) {
    FunctionResult result = {"", 0};
    
    if (!registry) {
        strcpy(result.result, "Error: Function registry not initialized");
        return result;
    }
    
    // Find the function
    for (int i = 0; i < registry->num_functions; i++) {
        FunctionEntry* entry = &registry->functions[i];
        if (strcmp(entry->name, name) == 0 && entry->type == FUNC_STRING && entry->active) {
            StringFunc func = (StringFunc)entry->function_ptr;
            return func(arg);
        }
    }
    
    snprintf(result.result, MAX_FUNCTION_RESULT, "Error: Function '%s' not found", name);
    return result;
}

// Call a () -> result function
FunctionResult call_no_arg_function(const char* name) {
    FunctionResult result = {"", 0};
    
    if (!registry) {
        strcpy(result.result, "Error: Function registry not initialized");
        return result;
    }
    
    // Find the function
    for (int i = 0; i < registry->num_functions; i++) {
        FunctionEntry* entry = &registry->functions[i];
        if (strcmp(entry->name, name) == 0 && entry->type == FUNC_NO_ARG && entry->active) {
            NoArgFunc func = (NoArgFunc)entry->function_ptr;
            return func();
        }
    }
    
    snprintf(result.result, MAX_FUNCTION_RESULT, "Error: Function '%s' not found", name);
    return result;
}

// Check if a function exists
int function_exists(const char* name) {
    if (!registry) return 0;
    
    for (int i = 0; i < registry->num_functions; i++) {
        if (strcmp(registry->functions[i].name, name) == 0 && registry->functions[i].active) {
            return 1;
        }
    }
    return 0;
}

// List all available functions
void list_functions(void) {
    if (!registry) {
        printf("Function registry not initialized\n");
        return;
    }
    
    printf("\nAvailable functions (%d total):\n", registry->num_functions);
    for (int i = 0; i < registry->num_functions; i++) {
        FunctionEntry* entry = &registry->functions[i];
        if (entry->active) {
            const char* type_name;
            switch (entry->type) {
                case FUNC_INT_INT: type_name = "(int, int)"; break;
                case FUNC_INT: type_name = "(int)"; break;
                case FUNC_DOUBLE_INT: type_name = "(precision)"; break;
                case FUNC_STRING: type_name = "(string)"; break;
                case FUNC_NO_ARG: type_name = "()"; break;
                default: type_name = "(unknown)"; break;
            }
            printf("  %s%s - %s\n", entry->name, type_name, entry->description);
        }
    }
    printf("\n");
}

// Get function description
const char* get_function_description(const char* name) {
    if (!registry) return "Function registry not initialized";
    
    for (int i = 0; i < registry->num_functions; i++) {
        if (strcmp(registry->functions[i].name, name) == 0 && registry->functions[i].active) {
            return registry->functions[i].description;
        }
    }
    return "Function not found";
}