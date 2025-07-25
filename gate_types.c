#include "gate_types.h"
#include <stdio.h>
#include <string.h>

// Gate registry implementation
#define MAX_GATE_TYPES 64

static struct {
    const char* name;
    const GateType* type;
} gate_registry[MAX_GATE_TYPES];

static size_t num_registered_types = 0;
static uint32_t next_gate_id = 1;

void gate_registry_init(void) {
    num_registered_types = 0;
    next_gate_id = 1;
}

void gate_registry_cleanup(void) {
    num_registered_types = 0;
}

bool gate_registry_register(const char* name, const GateType* type) {
    if (num_registered_types >= MAX_GATE_TYPES) {
        return false;
    }
    
    // Check for duplicates
    for (size_t i = 0; i < num_registered_types; i++) {
        if (strcmp(gate_registry[i].name, name) == 0) {
            return false;  // Already registered
        }
    }
    
    gate_registry[num_registered_types].name = name;
    gate_registry[num_registered_types].type = type;
    num_registered_types++;
    
    return true;
}

const GateType* gate_registry_get(const char* name) {
    for (size_t i = 0; i < num_registered_types; i++) {
        if (strcmp(gate_registry[i].name, name) == 0) {
            return gate_registry[i].type;
        }
    }
    return NULL;
}

void gate_registry_list(void) {
    printf("Registered gate types:\n");
    for (size_t i = 0; i < num_registered_types; i++) {
        printf("  - %s (state_size: %zu)\n", 
               gate_registry[i].name, 
               gate_registry[i].type->state_size);
    }
}

// Gate management implementation
Gate* gate_create(const char* type_name) {
    const GateType* type = gate_registry_get(type_name);
    if (!type) {
        return NULL;
    }
    
    Gate* gate = calloc(1, sizeof(Gate));
    if (!gate) {
        return NULL;
    }
    
    gate->type = type;
    gate->id = next_gate_id++;
    
    // Allocate state if needed
    if (type->state_size > 0) {
        gate->state = calloc(1, type->state_size);
        if (!gate->state) {
            free(gate);
            return NULL;
        }
    }
    
    // Initialize with 4 input slots
    gate->max_inputs = 4;
    gate->inputs = calloc(gate->max_inputs, sizeof(Gate*));
    if (!gate->inputs) {
        free(gate->state);
        free(gate);
        return NULL;
    }
    
    // Call type-specific initialization
    if (type->init) {
        type->init(gate);
    }
    
    return gate;
}

void gate_destroy(Gate* gate) {
    if (!gate) return;
    
    // Call type-specific cleanup
    if (gate->type->cleanup) {
        gate->type->cleanup(gate);
    }
    
    free(gate->inputs);
    free(gate->state);
    free(gate);
}

void gate_connect(Gate* gate, Gate* input) {
    if (!gate || !input) return;
    
    // Check if already connected
    for (size_t i = 0; i < gate->num_inputs; i++) {
        if (gate->inputs[i] == input) {
            return;  // Already connected
        }
    }
    
    // Resize if needed
    if (gate->num_inputs >= gate->max_inputs) {
        size_t new_max = gate->max_inputs * 2;
        Gate** new_inputs = realloc(gate->inputs, new_max * sizeof(Gate*));
        if (!new_inputs) return;
        
        gate->inputs = new_inputs;
        gate->max_inputs = new_max;
    }
    
    gate->inputs[gate->num_inputs++] = input;
}

void gate_disconnect(Gate* gate, Gate* input) {
    if (!gate || !input) return;
    
    for (size_t i = 0; i < gate->num_inputs; i++) {
        if (gate->inputs[i] == input) {
            // Shift remaining inputs
            for (size_t j = i; j < gate->num_inputs - 1; j++) {
                gate->inputs[j] = gate->inputs[j + 1];
            }
            gate->num_inputs--;
            return;
        }
    }
}

uint8_t gate_evaluate(Gate* gate) {
    if (!gate) return 0;
    
    // Prevent infinite loops in circular connections
    if (gate->evaluated_this_cycle) {
        return gate->last_output;
    }
    
    gate->evaluated_this_cycle = true;
    
    // Collect input values
    uint8_t input_values[gate->num_inputs];
    for (size_t i = 0; i < gate->num_inputs; i++) {
        input_values[i] = gate_evaluate(gate->inputs[i]);
    }
    
    // Evaluate using type-specific function
    gate->last_output = gate->type->evaluate(gate, input_values, gate->num_inputs);
    
    return gate->last_output;
}

void gate_reset(Gate* gate) {
    if (!gate) return;
    
    gate->evaluated_this_cycle = false;
    gate->last_output = 0;
    
    // Reset all connected inputs
    for (size_t i = 0; i < gate->num_inputs; i++) {
        gate_reset(gate->inputs[i]);
    }
}

void gate_print_info(Gate* gate) {
    if (!gate) return;
    
    printf("Gate %u (%s):\n", gate->id, gate->type->name);
    printf("  Inputs: %zu\n", gate->num_inputs);
    printf("  Last output: %u\n", gate->last_output);
}

void gate_print_connections(Gate* gate) {
    if (!gate) return;
    
    printf("Gate %u connections:\n", gate->id);
    for (size_t i = 0; i < gate->num_inputs; i++) {
        printf("  Input %zu: Gate %u (%s)\n", 
               i, 
               gate->inputs[i]->id,
               gate->inputs[i]->type->name);
    }
}