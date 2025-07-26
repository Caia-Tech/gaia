#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

// ============= EXPERIMENT 1: PURE GATES =============
typedef struct {
    uint8_t (*gate_func)(uint8_t, uint8_t);
    char name[16];
} Gate;

uint8_t and_gate(uint8_t a, uint8_t b) { return a & b; }
uint8_t or_gate(uint8_t a, uint8_t b) { return a | b; }
uint8_t xor_gate(uint8_t a, uint8_t b) { return a ^ b; }
uint8_t nand_gate(uint8_t a, uint8_t b) { return !(a & b); }

// ============= EXPERIMENT 2: GATES WITH MEMORY =============
typedef struct MemoryGate MemoryGate;

struct MemoryGate {
    uint8_t state;
    uint8_t memory;
    uint8_t (*process)(MemoryGate*, uint8_t, uint8_t);
};

uint8_t memory_and(MemoryGate* mg, uint8_t a, uint8_t b) {
    uint8_t result = (a & b) ^ mg->memory;  // XOR with memory
    mg->memory = mg->state;  // Remember last state
    mg->state = result;
    return result;
}

// ============= EXPERIMENT 3: SUPERPOSITION =============
typedef struct {
    float prob_zero;
    float prob_one;
    uint8_t collapsed;
    uint8_t final_value;
} Superposition;

Superposition create_superposition(uint8_t a, uint8_t b) {
    Superposition s;
    s.collapsed = 0;
    
    // Create probabilities based on inputs
    if (a == 1 && b == 1) {
        s.prob_one = 0.8;
    } else if (a == 0 && b == 0) {
        s.prob_one = 0.2;
    } else {
        s.prob_one = 0.5;
    }
    s.prob_zero = 1.0 - s.prob_one;
    
    return s;
}

uint8_t collapse(Superposition* s) {
    if (!s->collapsed) {
        // Simple collapse - could be context-dependent
        s->final_value = (s->prob_one > 0.5) ? 1 : 0;
        s->collapsed = 1;
    }
    return s->final_value;
}

// ============= EXPERIMENT 4: ADAPTIVE GATES =============
typedef struct AdaptiveGate AdaptiveGate;

struct AdaptiveGate {
    float weights[4];  // Weights for: a, b, a&b, a^b
    uint8_t (*decide)(AdaptiveGate*, uint8_t, uint8_t);
};

uint8_t adaptive_process(AdaptiveGate* ag, uint8_t a, uint8_t b) {
    float score = 0;
    score += ag->weights[0] * a;
    score += ag->weights[1] * b;
    score += ag->weights[2] * (a & b);
    score += ag->weights[3] * (a ^ b);
    
    return (score > 0.5) ? 1 : 0;
}

void adapt(AdaptiveGate* ag, uint8_t a, uint8_t b, uint8_t expected) {
    uint8_t got = adaptive_process(ag, a, b);
    if (got != expected) {
        // Simple learning: adjust weights
        float learning_rate = 0.1;
        if (expected == 1) {
            ag->weights[0] += learning_rate * a;
            ag->weights[1] += learning_rate * b;
        } else {
            ag->weights[0] -= learning_rate * a;
            ag->weights[1] -= learning_rate * b;
        }
    }
}

// ============= EXPERIMENT 5: GATE NETWORKS =============
#define MAX_NODES 10

typedef struct {
    uint8_t type;  // 0=AND, 1=OR, 2=XOR, 3=ADAPTIVE
    uint8_t inputs[2];  // Which nodes feed this one
    uint8_t value;
} Node;

typedef struct {
    Node nodes[MAX_NODES];
    uint8_t num_nodes;
} Network;

uint8_t evaluate_network(Network* net, uint8_t input_a, uint8_t input_b) {
    // Set input nodes
    net->nodes[0].value = input_a;
    net->nodes[1].value = input_b;
    
    // Evaluate each node
    for (int i = 2; i < net->num_nodes; i++) {
        uint8_t in1 = net->nodes[net->nodes[i].inputs[0]].value;
        uint8_t in2 = net->nodes[net->nodes[i].inputs[1]].value;
        
        switch(net->nodes[i].type) {
            case 0: net->nodes[i].value = in1 & in2; break;
            case 1: net->nodes[i].value = in1 | in2; break;
            case 2: net->nodes[i].value = in1 ^ in2; break;
        }
    }
    
    return net->nodes[net->num_nodes - 1].value;
}

// ============= TEST ALL EXPERIMENTS =============
int main() {
    printf("gaia: TRYING EVERYTHING!\n");
    printf("===========================\n\n");
    
    // Experiment 1: Basic gates
    printf("Experiment 1: Pure Gates\n");
    Gate gates[] = {
        {and_gate, "AND"},
        {or_gate, "OR"},
        {xor_gate, "XOR"},
        {nand_gate, "NAND"}
    };
    
    for (int g = 0; g < 4; g++) {
        printf("%s: ", gates[g].name);
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                printf("%d%d->%d ", a, b, gates[g].gate_func(a, b));
            }
        }
        printf("\n");
    }
    
    // Experiment 2: Memory gates
    printf("\nExperiment 2: Memory Gates\n");
    MemoryGate mg = {0, 0, memory_and};
    printf("Input sequence: ");
    uint8_t sequence[] = {1, 1, 0, 1, 1, 0};
    for (int i = 0; i < 6; i++) {
        uint8_t result = mg.process(&mg, sequence[i], 1);
        printf("%d->%d ", sequence[i], result);
    }
    printf("\n");
    
    // Experiment 3: Superposition
    printf("\nExperiment 3: Superposition\n");
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            Superposition s = create_superposition(a, b);
            printf("%d,%d: P(0)=%.2f P(1)=%.2f -> %d\n", 
                   a, b, s.prob_zero, s.prob_one, collapse(&s));
        }
    }
    
    // Experiment 4: Adaptive gates
    printf("\nExperiment 4: Adaptive Gate Learning XOR\n");
    AdaptiveGate ag = {{0.5, 0.5, 0.5, 0.5}, adaptive_process};
    
    // Train it
    for (int epoch = 0; epoch < 10; epoch++) {
        adapt(&ag, 0, 0, 0);  // XOR truth table
        adapt(&ag, 0, 1, 1);
        adapt(&ag, 1, 0, 1);
        adapt(&ag, 1, 1, 0);
    }
    
    // Test it
    printf("After training: ");
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            printf("%d%d->%d ", a, b, ag.decide(&ag, a, b));
        }
    }
    printf("\n");
    
    // Experiment 5: Network
    printf("\nExperiment 5: Gate Network\n");
    Network net;
    net.num_nodes = 5;
    // Node 0,1 are inputs
    // Node 2: AND of inputs
    net.nodes[2].type = 0;  // AND
    net.nodes[2].inputs[0] = 0;
    net.nodes[2].inputs[1] = 1;
    // Node 3: XOR of inputs  
    net.nodes[3].type = 2;  // XOR
    net.nodes[3].inputs[0] = 0;
    net.nodes[3].inputs[1] = 1;
    // Node 4: OR of previous results
    net.nodes[4].type = 1;  // OR
    net.nodes[4].inputs[0] = 2;
    net.nodes[4].inputs[1] = 3;
    
    printf("Network (AND + XOR fed to OR): ");
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            printf("%d%d->%d ", a, b, evaluate_network(&net, a, b));
        }
    }
    printf("\n");
    
    return 0;
}