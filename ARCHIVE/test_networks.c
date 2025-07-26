#include "gate_types.h"
#include <stdio.h>

// External functions
void register_basic_gates(void);
void register_memory_gates(void);
void register_adaptive_gates(void);

// Network builder functions
typedef struct Network Network;
Network* network_create();
int network_add_gate(Network* net, const char* name, const char* type);
bool network_connect(Network* net, const char* from_name, const char* to_name);
bool network_save(Network* net, const char* filename);
Network* network_load(const char* filename);
uint8_t network_evaluate_gate(Network* net, const char* gate_name);
void network_reset(Network* net);
void network_set_input(Network* net, const char* gate_name, uint8_t value);
void network_print(Network* net);
void network_destroy(Network* net);

// Test XOR network from file
void test_xor_network() {
    printf("\n=== Testing XOR Network ===\n");
    
    Network* net = network_load("networks/xor_network.gaia");
    if (!net) {
        printf("Failed to load network!\n");
        return;
    }
    
    printf("Loaded XOR network:\n");
    network_print(net);
    
    printf("\nTruth table:\n");
    printf("A B | Output\n");
    printf("----+-------\n");
    
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            // Set inputs
            network_set_input(net, "input_a", a);
            network_set_input(net, "input_b", b);
            
            // Evaluate
            uint8_t output = network_evaluate_gate(net, "output");
            uint8_t expected = a ^ b;
            
            printf("%d %d | %d %s\n", a, b, output, 
                   output == expected ? "✓" : "✗");
            
            // Reset for next evaluation
            network_reset(net);
        }
    }
    
    network_destroy(net);
}

// Build and test a half-adder
void test_half_adder() {
    printf("\n=== Building Half-Adder ===\n");
    
    Network* net = network_create();
    
    // Create gates
    network_add_gate(net, "a", "ZERO");
    network_add_gate(net, "b", "ZERO");
    network_add_gate(net, "sum", "XOR");
    network_add_gate(net, "carry", "AND");
    
    // Connect
    network_connect(net, "a", "sum");
    network_connect(net, "b", "sum");
    network_connect(net, "a", "carry");
    network_connect(net, "b", "carry");
    
    // Save it
    network_save(net, "networks/half_adder.gaia");
    printf("Saved half-adder network\n");
    
    // Test
    printf("\nHalf-Adder Truth Table:\n");
    printf("A B | Sum Carry\n");
    printf("----+-----------\n");
    
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            network_set_input(net, "a", a);
            network_set_input(net, "b", b);
            
            uint8_t sum = network_evaluate_gate(net, "sum");
            uint8_t carry = network_evaluate_gate(net, "carry");
            
            printf("%d %d |  %d    %d\n", a, b, sum, carry);
            
            network_reset(net);
        }
    }
    
    network_destroy(net);
}

// Test a learning network
void test_learning_network() {
    printf("\n=== Building Learning Network ===\n");
    
    Network* net = network_create();
    
    // Create a simple pattern learner
    network_add_gate(net, "in1", "ZERO");
    network_add_gate(net, "in2", "ZERO");
    network_add_gate(net, "in3", "ZERO");
    network_add_gate(net, "learner", "THRESHOLD");
    network_add_gate(net, "memory", "PATTERN");
    
    // Connect inputs to both learner and memory
    network_connect(net, "in1", "learner");
    network_connect(net, "in2", "learner");
    network_connect(net, "in3", "learner");
    network_connect(net, "in1", "memory");
    network_connect(net, "in2", "memory");
    network_connect(net, "in3", "memory");
    
    // Save
    network_save(net, "networks/learning_demo.gaia");
    printf("Created learning network with THRESHOLD and PATTERN gates\n");
    
    network_destroy(net);
}

// Test a counter circuit
void test_counter_circuit() {
    printf("\n=== Building Counter Circuit ===\n");
    
    Network* net = network_create();
    
    // Clock and reset inputs
    network_add_gate(net, "clock", "ZERO");
    network_add_gate(net, "reset", "ZERO");
    
    // Counter and display
    network_add_gate(net, "counter", "COUNTER");
    network_add_gate(net, "led", "BUFFER");
    
    // Connections
    network_connect(net, "clock", "counter");
    network_connect(net, "reset", "counter");
    network_connect(net, "counter", "led");
    
    network_save(net, "networks/counter.gaia");
    
    printf("Testing counter:\n");
    for (int i = 0; i < 8; i++) {
        // Pulse clock
        network_set_input(net, "clock", 1);
        uint8_t count = network_evaluate_gate(net, "led");
        printf("Clock %d: LED = %d\n", i, count);
        network_reset(net);
    }
    
    network_destroy(net);
}

int main() {
    printf("gaia Network Builder Test\n");
    printf("=========================\n");
    
    // Initialize
    gate_registry_init();
    register_basic_gates();
    register_memory_gates();
    register_adaptive_gates();
    
    // Create networks directory
    system("mkdir -p networks");
    
    // Run tests
    test_xor_network();
    test_half_adder();
    test_learning_network();
    test_counter_circuit();
    
    printf("\n✓ Network builder tests complete!\n");
    printf("\nCreated network files:\n");
    system("ls -la networks/");
    
    gate_registry_cleanup();
    return 0;
}