#ifndef GATE_TYPES_H
#define GATE_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// Forward declarations
typedef struct Gate Gate;
typedef struct GateType GateType;

// Gate evaluation function signature
typedef uint8_t (*GateEvalFunc)(Gate* gate, uint8_t* inputs, size_t num_inputs);

// Gate initialization function
typedef void (*GateInitFunc)(Gate* gate);

// Gate cleanup function
typedef void (*GateCleanupFunc)(Gate* gate);

// Gate state update function (for learning/adaptation)
typedef void (*GateUpdateFunc)(Gate* gate, uint8_t* inputs, uint8_t expected);

// Gate serialization functions
typedef size_t (*GateSerializeFunc)(Gate* gate, uint8_t* buffer);
typedef void (*GateDeserializeFunc)(Gate* gate, uint8_t* buffer);

// Gate type definition
struct GateType {
    const char* name;
    size_t state_size;  // Size of additional state data
    
    // Required functions
    GateEvalFunc evaluate;
    GateInitFunc init;
    
    // Optional functions
    GateCleanupFunc cleanup;
    GateUpdateFunc update;
    GateSerializeFunc serialize;
    GateDeserializeFunc deserialize;
};

// Gate instance
struct Gate {
    const GateType* type;
    uint32_t id;
    
    // Connections
    Gate** inputs;
    size_t num_inputs;
    size_t max_inputs;
    
    // State
    void* state;  // Type-specific state data
    uint8_t last_output;
    bool evaluated_this_cycle;
};

// Gate registry functions
void gate_registry_init(void);
void gate_registry_cleanup(void);
bool gate_registry_register(const char* name, const GateType* type);
const GateType* gate_registry_get(const char* name);
void gate_registry_list(void);

// Gate creation and management
Gate* gate_create(const char* type_name);
void gate_destroy(Gate* gate);
void gate_connect(Gate* gate, Gate* input);
void gate_disconnect(Gate* gate, Gate* input);
uint8_t gate_evaluate(Gate* gate);
void gate_reset(Gate* gate);

// Utility functions
void gate_print_info(Gate* gate);
void gate_print_connections(Gate* gate);

#endif // GATE_TYPES_H