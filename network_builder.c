#include "gate_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NETWORK_GATES 1000
#define MAX_LINE_LENGTH 256

typedef struct {
    Gate* gates[MAX_NETWORK_GATES];
    size_t num_gates;
    char* names[MAX_NETWORK_GATES];  // Gate names for reference
} Network;

// Create a new network
Network* network_create() {
    Network* net = calloc(1, sizeof(Network));
    return net;
}

// Add a gate to the network
int network_add_gate(Network* net, const char* name, const char* type) {
    if (!net || net->num_gates >= MAX_NETWORK_GATES) {
        return -1;
    }
    
    Gate* gate = gate_create(type);
    if (!gate) {
        return -1;
    }
    
    net->gates[net->num_gates] = gate;
    net->names[net->num_gates] = strdup(name);
    net->num_gates++;
    
    return net->num_gates - 1;  // Return index
}

// Find gate by name
int network_find_gate(Network* net, const char* name) {
    for (size_t i = 0; i < net->num_gates; i++) {
        if (strcmp(net->names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

// Connect two gates by name
bool network_connect(Network* net, const char* from_name, const char* to_name) {
    int from_idx = network_find_gate(net, from_name);
    int to_idx = network_find_gate(net, to_name);
    
    if (from_idx < 0 || to_idx < 0) {
        return false;
    }
    
    gate_connect(net->gates[to_idx], net->gates[from_idx]);
    return true;
}

// Save network to file
bool network_save(Network* net, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return false;
    
    // Header
    fprintf(f, "# gaia Network Configuration\n");
    fprintf(f, "# Format: GATE name type\n");
    fprintf(f, "#         CONNECT from_gate to_gate\n\n");
    
    // Write gates
    for (size_t i = 0; i < net->num_gates; i++) {
        fprintf(f, "GATE %s %s\n", net->names[i], net->gates[i]->type->name);
    }
    
    fprintf(f, "\n");
    
    // Write connections
    for (size_t i = 0; i < net->num_gates; i++) {
        Gate* gate = net->gates[i];
        for (size_t j = 0; j < gate->num_inputs; j++) {
            // Find name of input gate
            for (size_t k = 0; k < net->num_gates; k++) {
                if (net->gates[k] == gate->inputs[j]) {
                    fprintf(f, "CONNECT %s %s\n", net->names[k], net->names[i]);
                    break;
                }
            }
        }
    }
    
    fclose(f);
    return true;
}

// Load network from file
Network* network_load(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return NULL;
    
    Network* net = network_create();
    char line[MAX_LINE_LENGTH];
    
    while (fgets(line, MAX_LINE_LENGTH, f)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;
        
        char command[32], arg1[64], arg2[64];
        if (sscanf(line, "%s %s %s", command, arg1, arg2) >= 2) {
            if (strcmp(command, "GATE") == 0) {
                network_add_gate(net, arg1, arg2);
            } else if (strcmp(command, "CONNECT") == 0) {
                network_connect(net, arg1, arg2);
            }
        }
    }
    
    fclose(f);
    return net;
}

// Evaluate specific gate in network
uint8_t network_evaluate_gate(Network* net, const char* gate_name) {
    int idx = network_find_gate(net, gate_name);
    if (idx < 0) return 0;
    
    return gate_evaluate(net->gates[idx]);
}

// Reset all gates in network
void network_reset(Network* net) {
    for (size_t i = 0; i < net->num_gates; i++) {
        gate_reset(net->gates[i]);
    }
}

// Set input value (for constant gates)
void network_set_input(Network* net, const char* gate_name, uint8_t value) {
    int idx = network_find_gate(net, gate_name);
    if (idx < 0) return;
    
    // Replace with appropriate constant gate
    Gate* old_gate = net->gates[idx];
    Gate* new_gate = gate_create(value ? "ONE" : "ZERO");
    
    // Preserve connections
    for (size_t i = 0; i < net->num_gates; i++) {
        Gate* g = net->gates[i];
        for (size_t j = 0; j < g->num_inputs; j++) {
            if (g->inputs[j] == old_gate) {
                g->inputs[j] = new_gate;
            }
        }
    }
    
    gate_destroy(old_gate);
    net->gates[idx] = new_gate;
}

// Print network structure
void network_print(Network* net) {
    printf("Network structure:\n");
    printf("Gates (%zu):\n", net->num_gates);
    for (size_t i = 0; i < net->num_gates; i++) {
        printf("  %s (%s)\n", net->names[i], net->gates[i]->type->name);
    }
    
    printf("\nConnections:\n");
    for (size_t i = 0; i < net->num_gates; i++) {
        Gate* gate = net->gates[i];
        if (gate->num_inputs > 0) {
            printf("  %s <- ", net->names[i]);
            for (size_t j = 0; j < gate->num_inputs; j++) {
                // Find name of input
                for (size_t k = 0; k < net->num_gates; k++) {
                    if (net->gates[k] == gate->inputs[j]) {
                        printf("%s ", net->names[k]);
                        break;
                    }
                }
            }
            printf("\n");
        }
    }
}

// Clean up network
void network_destroy(Network* net) {
    if (!net) return;
    
    for (size_t i = 0; i < net->num_gates; i++) {
        gate_destroy(net->gates[i]);
        free(net->names[i]);
    }
    
    free(net);
}