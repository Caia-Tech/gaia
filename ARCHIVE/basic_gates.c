#include "gate_types.h"
#include <stdio.h>

// ============= AND Gate =============
static uint8_t and_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    if (num_inputs == 0) return 0;
    
    uint8_t result = 1;
    for (size_t i = 0; i < num_inputs; i++) {
        result &= inputs[i];
        if (result == 0) break;  // Short circuit
    }
    return result;
}

static const GateType AND_GATE_TYPE = {
    .name = "AND",
    .state_size = 0,
    .evaluate = and_gate_eval,
    .init = NULL,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= OR Gate =============
static uint8_t or_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    if (num_inputs == 0) return 0;
    
    uint8_t result = 0;
    for (size_t i = 0; i < num_inputs; i++) {
        result |= inputs[i];
        if (result == 1) break;  // Short circuit
    }
    return result;
}

static const GateType OR_GATE_TYPE = {
    .name = "OR",
    .state_size = 0,
    .evaluate = or_gate_eval,
    .init = NULL,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= XOR Gate =============
static uint8_t xor_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    if (num_inputs == 0) return 0;
    
    uint8_t result = inputs[0];
    for (size_t i = 1; i < num_inputs; i++) {
        result ^= inputs[i];
    }
    return result;
}

static const GateType XOR_GATE_TYPE = {
    .name = "XOR",
    .state_size = 0,
    .evaluate = xor_gate_eval,
    .init = NULL,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= NOT Gate =============
static uint8_t not_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    // NOT gate only uses first input
    if (num_inputs == 0) return 1;
    return !inputs[0];
}

static const GateType NOT_GATE_TYPE = {
    .name = "NOT",
    .state_size = 0,
    .evaluate = not_gate_eval,
    .init = NULL,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= NAND Gate =============
static uint8_t nand_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    return !and_gate_eval(gate, inputs, num_inputs);
}

static const GateType NAND_GATE_TYPE = {
    .name = "NAND",
    .state_size = 0,
    .evaluate = nand_gate_eval,
    .init = NULL,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= NOR Gate =============
static uint8_t nor_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    return !or_gate_eval(gate, inputs, num_inputs);
}

static const GateType NOR_GATE_TYPE = {
    .name = "NOR",
    .state_size = 0,
    .evaluate = nor_gate_eval,
    .init = NULL,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= CONSTANT Gates =============
static uint8_t const_zero_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    return 0;
}

static uint8_t const_one_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    return 1;
}

static const GateType CONST_ZERO_TYPE = {
    .name = "ZERO",
    .state_size = 0,
    .evaluate = const_zero_eval,
    .init = NULL,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

static const GateType CONST_ONE_TYPE = {
    .name = "ONE",
    .state_size = 0,
    .evaluate = const_one_eval,
    .init = NULL,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= BUFFER Gate (identity) =============
static uint8_t buffer_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    if (num_inputs == 0) return 0;
    return inputs[0];  // Pass through first input
}

static const GateType BUFFER_GATE_TYPE = {
    .name = "BUFFER",
    .state_size = 0,
    .evaluate = buffer_gate_eval,
    .init = NULL,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= Registration Function =============
void register_basic_gates(void) {
    gate_registry_register("AND", &AND_GATE_TYPE);
    gate_registry_register("OR", &OR_GATE_TYPE);
    gate_registry_register("XOR", &XOR_GATE_TYPE);
    gate_registry_register("NOT", &NOT_GATE_TYPE);
    gate_registry_register("NAND", &NAND_GATE_TYPE);
    gate_registry_register("NOR", &NOR_GATE_TYPE);
    gate_registry_register("ZERO", &CONST_ZERO_TYPE);
    gate_registry_register("ONE", &CONST_ONE_TYPE);
    gate_registry_register("BUFFER", &BUFFER_GATE_TYPE);
}