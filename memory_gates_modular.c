#include "gate_types.h"
#include <stdio.h>
#include <string.h>

// ============= LATCH Gate (SR Latch behavior) =============
typedef struct {
    uint8_t state;
} LatchState;

static uint8_t latch_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    LatchState* state = (LatchState*)gate->state;
    
    // Expects 2 inputs: SET and RESET
    if (num_inputs >= 2) {
        uint8_t set = inputs[0];
        uint8_t reset = inputs[1];
        
        if (set && !reset) {
            state->state = 1;  // Set
        } else if (!set && reset) {
            state->state = 0;  // Reset
        }
        // If both or neither, maintain current state
    }
    
    return state->state;
}

static void latch_gate_init(Gate* gate) {
    LatchState* state = (LatchState*)gate->state;
    state->state = 0;
}

static const GateType LATCH_GATE_TYPE = {
    .name = "LATCH",
    .state_size = sizeof(LatchState),
    .evaluate = latch_gate_eval,
    .init = latch_gate_init,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= DELAY Gate (D Flip-Flop behavior) =============
typedef struct {
    uint8_t stored_value;
} DelayState;

static uint8_t delay_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    DelayState* state = (DelayState*)gate->state;
    
    uint8_t output = state->stored_value;
    
    // Store current input for next cycle
    if (num_inputs > 0) {
        state->stored_value = inputs[0];
    }
    
    return output;
}

static void delay_gate_init(Gate* gate) {
    DelayState* state = (DelayState*)gate->state;
    state->stored_value = 0;
}

static const GateType DELAY_GATE_TYPE = {
    .name = "DELAY",
    .state_size = sizeof(DelayState),
    .evaluate = delay_gate_eval,
    .init = delay_gate_init,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= COUNTER Gate =============
typedef struct {
    uint8_t count;
    uint8_t max_count;
} CounterState;

static uint8_t counter_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    CounterState* state = (CounterState*)gate->state;
    
    // Input 0: increment signal
    // Input 1: reset signal
    if (num_inputs >= 2 && inputs[1]) {
        state->count = 0;  // Reset
    } else if (num_inputs >= 1 && inputs[0]) {
        state->count = (state->count + 1) % state->max_count;
    }
    
    return state->count > 0 ? 1 : 0;  // Output 1 if count > 0
}

static void counter_gate_init(Gate* gate) {
    CounterState* state = (CounterState*)gate->state;
    state->count = 0;
    state->max_count = 16;  // Default max
}

static const GateType COUNTER_GATE_TYPE = {
    .name = "COUNTER",
    .state_size = sizeof(CounterState),
    .evaluate = counter_gate_eval,
    .init = counter_gate_init,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= MEMORY BANK Gate (8-bit memory) =============
#define MEMORY_BANK_SIZE 8

typedef struct {
    uint8_t memory[MEMORY_BANK_SIZE];
    uint8_t last_address;
} MemoryBankState;

static uint8_t memory_bank_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    MemoryBankState* state = (MemoryBankState*)gate->state;
    
    // Input 0-2: 3-bit address
    // Input 3: write enable
    // Input 4: data to write
    
    uint8_t address = 0;
    if (num_inputs >= 3) {
        address = (inputs[0] & 1) | ((inputs[1] & 1) << 1) | ((inputs[2] & 1) << 2);
        address %= MEMORY_BANK_SIZE;
    }
    
    state->last_address = address;
    
    // Write operation
    if (num_inputs >= 5 && inputs[3]) {
        state->memory[address] = inputs[4];
    }
    
    // Always read from addressed location
    return state->memory[address];
}

static void memory_bank_init(Gate* gate) {
    MemoryBankState* state = (MemoryBankState*)gate->state;
    memset(state->memory, 0, MEMORY_BANK_SIZE);
    state->last_address = 0;
}

static const GateType MEMORY_BANK_TYPE = {
    .name = "MEMORY_BANK",
    .state_size = sizeof(MemoryBankState),
    .evaluate = memory_bank_eval,
    .init = memory_bank_init,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= ACCUMULATOR Gate =============
typedef struct {
    uint8_t value;
} AccumulatorState;

static uint8_t accumulator_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    AccumulatorState* state = (AccumulatorState*)gate->state;
    
    // Input 0: value to add/subtract
    // Input 1: operation (0 = add, 1 = subtract)
    // Input 2: reset
    
    if (num_inputs >= 3 && inputs[2]) {
        state->value = 0;  // Reset
    } else if (num_inputs >= 2) {
        if (inputs[1] == 0) {
            // Add (with saturation)
            if (state->value < 255 && inputs[0]) {
                state->value++;
            }
        } else {
            // Subtract (with saturation)
            if (state->value > 0 && inputs[0]) {
                state->value--;
            }
        }
    }
    
    return state->value > 0 ? 1 : 0;
}

static void accumulator_gate_init(Gate* gate) {
    AccumulatorState* state = (AccumulatorState*)gate->state;
    state->value = 0;
}

static const GateType ACCUMULATOR_GATE_TYPE = {
    .name = "ACCUMULATOR",
    .state_size = sizeof(AccumulatorState),
    .evaluate = accumulator_gate_eval,
    .init = accumulator_gate_init,
    .cleanup = NULL,
    .update = NULL,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= Registration Function =============
void register_memory_gates(void) {
    gate_registry_register("LATCH", &LATCH_GATE_TYPE);
    gate_registry_register("DELAY", &DELAY_GATE_TYPE);
    gate_registry_register("COUNTER", &COUNTER_GATE_TYPE);
    gate_registry_register("MEMORY_BANK", &MEMORY_BANK_TYPE);
    gate_registry_register("ACCUMULATOR", &ACCUMULATOR_GATE_TYPE);
}