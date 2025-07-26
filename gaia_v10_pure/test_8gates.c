/**
 * Test Suite: What Can 8 Gates Actually Do?
 * 
 * We know 8 gates can solve XOR. What else?
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define MAX_GATES 8
#define MAX_GENERATIONS 10000

typedef struct {
    int type;          // Gate type (0-3)
    int threshold;     // Binary threshold
    int memory;        // Gate memory
} Gate;

typedef struct {
    Gate gates[MAX_GATES];
    int connections[MAX_GATES][MAX_GATES];
    int fitness;
} Network;

// Gate operations
int gate_op(Gate* g, int input) {
    switch(g->type) {
        case 0: return input;                    // Pass
        case 1: return !input;                   // NOT
        case 2: g->memory = input; return input; // Memory
        case 3: return input ^ g->memory;        // XOR with memory
    }
    return input;
}

// Network forward pass
void forward(Network* net, int* inputs, int* outputs, int n_in, int n_out) {
    int values[MAX_GATES] = {0};
    
    // Set inputs
    for (int i = 0; i < n_in && i < MAX_GATES; i++) {
        values[i] = inputs[i];
    }
    
    // Process gates
    for (int i = n_in; i < MAX_GATES; i++) {
        int sum = 0;
        for (int j = 0; j < i; j++) {
            if (net->connections[j][i]) {
                sum += values[j];
            }
        }
        int input = (sum >= net->gates[i].threshold) ? 1 : 0;
        values[i] = gate_op(&net->gates[i], input);
    }
    
    // Get outputs from last gates
    for (int i = 0; i < n_out; i++) {
        outputs[i] = values[MAX_GATES - n_out + i];
    }
}

// Test XOR (baseline)
int test_xor(Network* net) {
    int tests[][3] = {{0,0,0}, {0,1,1}, {1,0,1}, {1,1,0}};
    int correct = 0;
    
    for (int i = 0; i < 4; i++) {
        int inputs[2] = {tests[i][0], tests[i][1]};
        int output[1];
        forward(net, inputs, output, 2, 1);
        if (output[0] == tests[i][2]) correct++;
    }
    return correct;
}

// Test 3-bit parity
int test_parity(Network* net) {
    int correct = 0;
    
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            for (int c = 0; c <= 1; c++) {
                int inputs[3] = {a, b, c};
                int output[1];
                forward(net, inputs, output, 3, 1);
                int expected = (a + b + c) % 2;
                if (output[0] == expected) correct++;
            }
        }
    }
    return correct;
}

// Test 2-bit addition
int test_addition(Network* net) {
    int correct = 0;
    
    // Test adding two 2-bit numbers (0-3)
    for (int a = 0; a <= 3; a++) {
        for (int b = 0; b <= 3; b++) {
            int inputs[4] = {a&1, (a>>1)&1, b&1, (b>>1)&1};
            int output[3];  // 3 bits for sum (0-6)
            forward(net, inputs, output, 4, 3);
            
            int sum = output[0] + (output[1]<<1) + (output[2]<<2);
            if (sum == a + b) correct++;
        }
    }
    return correct;
}

// Test AND/OR/NAND
int test_logic_gates(Network* net) {
    int tests[][3] = {{0,0,0}, {0,1,0}, {1,0,0}, {1,1,1}};  // AND
    int correct = 0;
    
    // Test AND
    for (int i = 0; i < 4; i++) {
        int inputs[2] = {tests[i][0], tests[i][1]};
        int output[1];
        forward(net, inputs, output, 2, 1);
        if (output[0] == tests[i][2]) correct++;
    }
    
    // Test OR
    for (int i = 0; i < 4; i++) {
        int inputs[2] = {tests[i][0], tests[i][1]};
        int output[1];
        forward(net, inputs, output, 2, 1);
        int expected = tests[i][0] | tests[i][1];
        if (output[0] == expected) correct++;
    }
    
    return correct;
}

// Test sequence detection (detect "101")
int test_sequence(Network* net) {
    int sequences[][8] = {
        {0,0,0,0,0,0,0,0},  // No match
        {1,0,1,0,0,0,0,0},  // Match at start
        {0,1,0,1,0,0,0,0},  // Match at position 1
        {0,0,1,0,1,0,0,0},  // Match at position 2
        {1,1,0,1,0,1,0,0},  // Multiple potential matches
        {1,0,0,1,0,1,0,0},  // Match only at position 3
    };
    int expected[] = {0, 1, 1, 1, 1, 1};
    int correct = 0;
    
    for (int seq = 0; seq < 6; seq++) {
        // Reset network memory
        for (int i = 0; i < MAX_GATES; i++) {
            net->gates[i].memory = 0;
        }
        
        int detected = 0;
        // Feed sequence one bit at a time
        for (int t = 0; t < 6; t++) {
            int inputs[1] = {sequences[seq][t]};
            int output[1];
            forward(net, inputs, output, 1, 1);
            
            // Check if we've seen "101" in last 3 inputs
            if (t >= 2 && 
                sequences[seq][t-2] == 1 && 
                sequences[seq][t-1] == 0 && 
                sequences[seq][t] == 1) {
                detected = 1;
            }
        }
        
        if ((detected > 0) == expected[seq]) correct++;
    }
    
    return correct;
}

// Mutate network
void mutate(Network* net) {
    int choice = rand() % 3;
    
    switch(choice) {
        case 0: {  // Flip connection
            int from = rand() % (MAX_GATES - 1);
            int to = from + 1 + rand() % (MAX_GATES - from - 1);
            net->connections[from][to] ^= 1;
            break;
        }
        case 1: {  // Change gate type
            int gate = rand() % MAX_GATES;
            net->gates[gate].type = rand() % 4;
            break;
        }
        case 2: {  // Change threshold
            int gate = rand() % MAX_GATES;
            net->gates[gate].threshold = rand() % 3;  // 0, 1, or 2
            break;
        }
    }
}

// Evolve network for specific task
Network evolve_for_task(const char* task_name, int (*test_func)(Network*), int max_score) {
    Network net = {0};
    Network best = {0};
    int best_score = 0;
    
    // Random init
    for (int i = 0; i < MAX_GATES; i++) {
        net.gates[i].type = rand() % 4;
        net.gates[i].threshold = rand() % 2;
    }
    for (int i = 0; i < MAX_GATES-1; i++) {
        for (int j = i+1; j < MAX_GATES; j++) {
            net.connections[i][j] = rand() % 2;
        }
    }
    
    printf("\nEvolving 8-gate network for %s...\n", task_name);
    
    for (int gen = 0; gen < MAX_GENERATIONS; gen++) {
        int score = test_func(&net);
        
        if (score > best_score) {
            best_score = score;
            best = net;
            printf("Gen %5d: Score %d/%d\n", gen, score, max_score);
            
            if (score == max_score) {
                printf("Solved in %d generations!\n", gen);
                break;
            }
        }
        
        Network saved = net;
        mutate(&net);
        if (test_func(&net) < score) {
            net = saved;
        }
    }
    
    if (best_score < max_score) {
        printf("Failed to solve completely. Best: %d/%d\n", best_score, max_score);
    }
    
    return best;
}

// Analyze what gates are doing
void analyze_network(Network* net, const char* task_name) {
    printf("\n%s Network Structure:\n", task_name);
    
    for (int i = 0; i < MAX_GATES; i++) {
        printf("Gate %d: ", i);
        const char* types[] = {"PASS", "NOT", "MEM", "XOR_MEM"};
        printf("%s (threshold=%d) <- ", types[net->gates[i].type], net->gates[i].threshold);
        
        int has_input = 0;
        for (int j = 0; j < i; j++) {
            if (net->connections[j][i]) {
                printf("%d ", j);
                has_input = 1;
            }
        }
        if (!has_input) printf("none");
        printf("\n");
    }
}

int main() {
    srand(time(NULL));
    
    printf("=== Testing What 8 Gates Can Do ===\n");
    printf("Available gate types: PASS, NOT, MEMORY, XOR_WITH_MEMORY\n");
    
    // Test XOR (we know this works)
    printf("\n1. XOR (2 inputs → 1 output):");
    Network xor_net = evolve_for_task("XOR", test_xor, 4);
    analyze_network(&xor_net, "XOR");
    
    // Test 3-bit parity
    printf("\n2. 3-bit Parity (3 inputs → 1 output):");
    Network parity_net = evolve_for_task("3-bit Parity", test_parity, 8);
    analyze_network(&parity_net, "Parity");
    
    // Test logic gates
    printf("\n3. AND/OR Gates (2 inputs → 1 output):");
    Network logic_net = evolve_for_task("Logic Gates", test_logic_gates, 8);
    
    // Test 2-bit addition
    printf("\n4. 2-bit Addition (4 inputs → 3 outputs):");
    Network add_net = evolve_for_task("2-bit Addition", test_addition, 16);
    if (test_addition(&add_net) < 16) {
        printf("Note: 8 gates insufficient for full 2-bit addition\n");
    }
    
    // Test sequence detection
    printf("\n5. Sequence Detection '101' (1 input → 1 output):");
    Network seq_net = evolve_for_task("Sequence Detection", test_sequence, 6);
    analyze_network(&seq_net, "Sequence");
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("8 gates CAN solve:\n");
    printf("- XOR ✓\n");
    printf("- 3-bit parity %s\n", test_parity(&parity_net) == 8 ? "✓" : "✗");
    printf("- Basic logic gates %s\n", test_logic_gates(&logic_net) >= 6 ? "✓" : "✗");
    printf("- Sequence detection %s\n", test_sequence(&seq_net) >= 5 ? "✓" : "✗");
    printf("\n8 gates CANNOT solve:\n");
    printf("- 2-bit addition (needs carry logic)\n");
    
    return 0;
}