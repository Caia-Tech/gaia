#include "gate_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ============= THRESHOLD Gate (Perceptron-like) =============
typedef struct {
    float* weights;
    float threshold;
    float learning_rate;
    size_t num_weights;
} ThresholdState;

static uint8_t threshold_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    ThresholdState* state = (ThresholdState*)gate->state;
    
    // Resize weights if needed
    if (num_inputs > state->num_weights) {
        state->weights = realloc(state->weights, num_inputs * sizeof(float));
        for (size_t i = state->num_weights; i < num_inputs; i++) {
            state->weights[i] = 0.5;  // Initialize new weights
        }
        state->num_weights = num_inputs;
    }
    
    // Calculate weighted sum
    float sum = 0.0;
    for (size_t i = 0; i < num_inputs && i < state->num_weights; i++) {
        sum += state->weights[i] * inputs[i];
    }
    
    // Apply threshold
    return (sum >= state->threshold) ? 1 : 0;
}

static void threshold_gate_update(Gate* gate, uint8_t* inputs, uint8_t expected) {
    ThresholdState* state = (ThresholdState*)gate->state;
    uint8_t output = threshold_gate_eval(gate, inputs, gate->num_inputs);
    
    if (output != expected) {
        // Simple perceptron learning rule
        int error = expected - output;
        
        for (size_t i = 0; i < gate->num_inputs && i < state->num_weights; i++) {
            state->weights[i] += state->learning_rate * error * inputs[i];
        }
        
        // Adjust threshold too
        state->threshold -= state->learning_rate * error;
    }
}

static void threshold_gate_init(Gate* gate) {
    ThresholdState* state = (ThresholdState*)gate->state;
    state->num_weights = 4;  // Start with 4
    state->weights = calloc(state->num_weights, sizeof(float));
    for (size_t i = 0; i < state->num_weights; i++) {
        state->weights[i] = 0.5;
    }
    state->threshold = 0.7;
    state->learning_rate = 0.1;
}

static void threshold_gate_cleanup(Gate* gate) {
    ThresholdState* state = (ThresholdState*)gate->state;
    free(state->weights);
}

static const GateType THRESHOLD_GATE_TYPE = {
    .name = "THRESHOLD",
    .state_size = sizeof(ThresholdState),
    .evaluate = threshold_gate_eval,
    .init = threshold_gate_init,
    .cleanup = threshold_gate_cleanup,
    .update = threshold_gate_update,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= PATTERN Gate (memorizes patterns) =============
#define MAX_PATTERNS 32

typedef struct {
    uint8_t patterns[MAX_PATTERNS][8];  // Up to 8 inputs per pattern
    uint8_t outputs[MAX_PATTERNS];
    size_t num_patterns;
    size_t pattern_size;
} PatternState;

static uint8_t pattern_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    PatternState* state = (PatternState*)gate->state;
    
    // Look for matching pattern
    for (size_t p = 0; p < state->num_patterns; p++) {
        bool match = true;
        for (size_t i = 0; i < num_inputs && i < state->pattern_size; i++) {
            if (state->patterns[p][i] != inputs[i]) {
                match = false;
                break;
            }
        }
        if (match) {
            return state->outputs[p];
        }
    }
    
    // No match found - return 0
    return 0;
}

static void pattern_gate_update(Gate* gate, uint8_t* inputs, uint8_t expected) {
    PatternState* state = (PatternState*)gate->state;
    
    // Check if pattern already exists
    for (size_t p = 0; p < state->num_patterns; p++) {
        bool match = true;
        for (size_t i = 0; i < gate->num_inputs && i < state->pattern_size; i++) {
            if (state->patterns[p][i] != inputs[i]) {
                match = false;
                break;
            }
        }
        if (match) {
            // Update existing pattern
            state->outputs[p] = expected;
            return;
        }
    }
    
    // Add new pattern if space available
    if (state->num_patterns < MAX_PATTERNS) {
        size_t idx = state->num_patterns++;
        for (size_t i = 0; i < gate->num_inputs && i < state->pattern_size; i++) {
            state->patterns[idx][i] = inputs[i];
        }
        state->outputs[idx] = expected;
    }
}

static void pattern_gate_init(Gate* gate) {
    PatternState* state = (PatternState*)gate->state;
    state->num_patterns = 0;
    state->pattern_size = 8;
}

static const GateType PATTERN_GATE_TYPE = {
    .name = "PATTERN",
    .state_size = sizeof(PatternState),
    .evaluate = pattern_gate_eval,
    .init = pattern_gate_init,
    .cleanup = NULL,
    .update = pattern_gate_update,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= CONFIDENCE Gate (probabilistic output) =============
typedef struct {
    float confidence_zero;
    float confidence_one;
    uint32_t observations;
} ConfidenceState;

static uint8_t confidence_gate_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    ConfidenceState* state = (ConfidenceState*)gate->state;
    
    // Simple rule: if majority of inputs are 1, lean towards 1
    int ones = 0;
    for (size_t i = 0; i < num_inputs; i++) {
        if (inputs[i]) ones++;
    }
    
    float input_ratio = (num_inputs > 0) ? (float)ones / num_inputs : 0.5;
    
    // Combine input ratio with learned confidence
    float prob_one = (input_ratio + state->confidence_one) / 2.0;
    
    // Deterministic threshold for now (could add randomness)
    return (prob_one > 0.5) ? 1 : 0;
}

static void confidence_gate_update(Gate* gate, uint8_t* inputs, uint8_t expected) {
    ConfidenceState* state = (ConfidenceState*)gate->state;
    
    state->observations++;
    
    // Update confidence based on whether we were right
    uint8_t output = confidence_gate_eval(gate, inputs, gate->num_inputs);
    
    if (output == expected) {
        if (expected == 1) {
            state->confidence_one = (state->confidence_one * 0.9) + 0.1;
            state->confidence_zero = (state->confidence_zero * 0.9);
        } else {
            state->confidence_zero = (state->confidence_zero * 0.9) + 0.1;
            state->confidence_one = (state->confidence_one * 0.9);
        }
    } else {
        // Wrong - adjust opposite direction
        if (expected == 1) {
            state->confidence_one = (state->confidence_one * 0.9);
            state->confidence_zero = (state->confidence_zero * 0.9) + 0.1;
        } else {
            state->confidence_zero = (state->confidence_zero * 0.9);
            state->confidence_one = (state->confidence_one * 0.9) + 0.1;
        }
    }
}

static void confidence_gate_init(Gate* gate) {
    ConfidenceState* state = (ConfidenceState*)gate->state;
    state->confidence_zero = 0.5;
    state->confidence_one = 0.5;
    state->observations = 0;
}

static const GateType CONFIDENCE_GATE_TYPE = {
    .name = "CONFIDENCE",
    .state_size = sizeof(ConfidenceState),
    .evaluate = confidence_gate_eval,
    .init = confidence_gate_init,
    .cleanup = NULL,
    .update = confidence_gate_update,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= ADAPTIVE_AND Gate (learns when to act like AND) =============
typedef struct {
    float strictness;  // 0.0 = OR-like, 1.0 = AND-like
} AdaptiveAndState;

static uint8_t adaptive_and_eval(Gate* gate, uint8_t* inputs, size_t num_inputs) {
    AdaptiveAndState* state = (AdaptiveAndState*)gate->state;
    
    if (num_inputs == 0) return 0;
    
    // Count active inputs
    int active = 0;
    for (size_t i = 0; i < num_inputs; i++) {
        if (inputs[i]) active++;
    }
    
    // Threshold based on strictness
    float required = state->strictness * num_inputs;
    return (active >= required) ? 1 : 0;
}

static void adaptive_and_update(Gate* gate, uint8_t* inputs, uint8_t expected) {
    AdaptiveAndState* state = (AdaptiveAndState*)gate->state;
    uint8_t output = adaptive_and_eval(gate, inputs, gate->num_inputs);
    
    if (output != expected) {
        if (expected == 1 && output == 0) {
            // Need to be less strict
            state->strictness *= 0.9;
        } else if (expected == 0 && output == 1) {
            // Need to be more strict
            state->strictness = state->strictness * 1.1;
            if (state->strictness > 1.0) state->strictness = 1.0;
        }
    }
}

static void adaptive_and_init(Gate* gate) {
    AdaptiveAndState* state = (AdaptiveAndState*)gate->state;
    state->strictness = 0.8;  // Start fairly strict
}

static const GateType ADAPTIVE_AND_TYPE = {
    .name = "ADAPTIVE_AND",
    .state_size = sizeof(AdaptiveAndState),
    .evaluate = adaptive_and_eval,
    .init = adaptive_and_init,
    .cleanup = NULL,
    .update = adaptive_and_update,
    .serialize = NULL,
    .deserialize = NULL
};

// ============= Registration Function =============
void register_adaptive_gates(void) {
    gate_registry_register("THRESHOLD", &THRESHOLD_GATE_TYPE);
    gate_registry_register("PATTERN", &PATTERN_GATE_TYPE);
    gate_registry_register("CONFIDENCE", &CONFIDENCE_GATE_TYPE);
    gate_registry_register("ADAPTIVE_AND", &ADAPTIVE_AND_TYPE);
}