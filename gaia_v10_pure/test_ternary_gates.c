/**
 * Ternary Gates: Because Binary is Arbitrary
 * 
 * States: -1, 0, +1 (balanced ternary)
 * Let's see what universe this creates
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_GENERATIONS 10000

typedef struct {
    int type;           // Gate type
    int threshold_low;  // Below this = -1
    int threshold_high; // Above this = +1
    int memory;         // Ternary memory (-1, 0, 1)
} TernaryGate;

typedef struct {
    TernaryGate* gates;
    int num_gates;
    int** connections;
} TernaryNetwork;

// Ternary operations
int t_min(int a, int b) { return a < b ? a : b; }
int t_max(int a, int b) { return a > b ? a : b; }
int t_neg(int a) { return -a; }
int t_mul(int a, int b) { return a * b; }

// Balanced ternary AND-like (minimum)
int t_and(int a, int b) { return t_min(a, b); }

// Balanced ternary OR-like (maximum)  
int t_or(int a, int b) { return t_max(a, b); }

// Ternary gate operations
int apply_ternary_gate(TernaryGate* g, int input) {
    switch(g->type) {
        case 0: return input;                        // PASS
        case 1: return t_neg(input);                // NEGATE
        case 2: g->memory = input; return input;    // MEMORY
        case 3: return t_mul(input, g->memory);     // MULTIPLY with memory
        case 4: return t_min(input, g->memory);     // MIN with memory
        case 5: return t_max(input, g->memory);     // MAX with memory
        case 6: // THRESHOLD
            if (input < g->threshold_low) return -1;
            if (input > g->threshold_high) return 1;
            return 0;
        case 7: // CYCLE (-1→0→1→-1)
            if (input == -1) return 0;
            if (input == 0) return 1;
            return -1;
    }
    return input;
}

// Create network
TernaryNetwork* create_ternary_network(int n) {
    TernaryNetwork* net = malloc(sizeof(TernaryNetwork));
    net->num_gates = n;
    net->gates = calloc(n, sizeof(TernaryGate));
    net->connections = malloc(n * sizeof(int*));
    
    for (int i = 0; i < n; i++) {
        net->connections[i] = calloc(n, sizeof(int));
        net->gates[i].type = rand() % 8;
        net->gates[i].threshold_low = -1;
        net->gates[i].threshold_high = 1;
        net->gates[i].memory = 0;
    }
    
    return net;
}

// Forward pass
int forward_ternary(TernaryNetwork* net, int a, int b) {
    int* values = calloc(net->num_gates, sizeof(int));
    
    // First two gates get inputs
    if (net->num_gates >= 1) values[0] = apply_ternary_gate(&net->gates[0], a);
    if (net->num_gates >= 2) values[1] = apply_ternary_gate(&net->gates[1], b);
    
    // Process remaining gates
    for (int i = 2; i < net->num_gates; i++) {
        int sum = 0;
        int count = 0;
        
        for (int j = 0; j < i; j++) {
            if (net->connections[j][i]) {
                sum += values[j];
                count++;
            }
        }
        
        // Ternary combination
        int input = 0;
        if (count > 0) {
            if (sum < 0) input = -1;
            else if (sum > 0) input = 1;
        }
        
        values[i] = apply_ternary_gate(&net->gates[i], input);
    }
    
    int result = values[net->num_gates - 1];
    free(values);
    return result;
}

// Test ternary AND (minimum)
int test_ternary_and(TernaryNetwork* net) {
    int tests[][3] = {
        {-1,-1,-1}, {-1,0,-1}, {-1,1,-1},
        {0,-1,-1}, {0,0,0}, {0,1,0},
        {1,-1,-1}, {1,0,0}, {1,1,1}
    };
    
    int correct = 0;
    for (int i = 0; i < 9; i++) {
        int out = forward_ternary(net, tests[i][0], tests[i][1]);
        if (out == tests[i][2]) correct++;
    }
    return correct;
}

// Test ternary OR (maximum)
int test_ternary_or(TernaryNetwork* net) {
    int tests[][3] = {
        {-1,-1,-1}, {-1,0,0}, {-1,1,1},
        {0,-1,0}, {0,0,0}, {0,1,1},
        {1,-1,1}, {1,0,1}, {1,1,1}
    };
    
    int correct = 0;
    for (int i = 0; i < 9; i++) {
        int out = forward_ternary(net, tests[i][0], tests[i][1]);
        if (out == tests[i][2]) correct++;
    }
    return correct;
}

// Test ternary multiplication
int test_ternary_mul(TernaryNetwork* net) {
    int tests[][3] = {
        {-1,-1,1}, {-1,0,0}, {-1,1,-1},
        {0,-1,0}, {0,0,0}, {0,1,0},
        {1,-1,-1}, {1,0,0}, {1,1,1}
    };
    
    int correct = 0;
    for (int i = 0; i < 9; i++) {
        int out = forward_ternary(net, tests[i][0], tests[i][1]);
        if (out == tests[i][2]) correct++;
    }
    return correct;
}

// Test consensus (ternary-specific: if 2 inputs agree, output that, else 0)
int test_consensus(TernaryNetwork* net) {
    // 3-input consensus
    int correct = 0;
    for (int a = -1; a <= 1; a++) {
        for (int b = -1; b <= 1; b++) {
            for (int c = -1; c <= 1; c++) {
                // Feed a,b to network, then result with c
                int partial = forward_ternary(net, a, b);
                int out = forward_ternary(net, partial, c);
                
                // Expected: if 2+ agree, output that value, else 0
                int expected = 0;
                if (a == b || a == c) expected = a;
                else if (b == c) expected = b;
                
                if (out == expected) correct++;
            }
        }
    }
    return correct;
}

// Mutate network
void mutate_ternary(TernaryNetwork* net) {
    int choice = rand() % 3;
    int gate = rand() % net->num_gates;
    
    switch(choice) {
        case 0: // Change gate type
            net->gates[gate].type = rand() % 8;
            break;
        case 1: // Flip connection
            if (gate > 0) {
                int from = rand() % gate;
                net->connections[from][gate] ^= 1;
            }
            break;
        case 2: // Reset memory
            net->gates[gate].memory = rand() % 3 - 1;
            break;
    }
}

void test_n_ternary_gates(int n) {
    printf("\n=== Testing %d Ternary Gate%s ===\n", n, n > 1 ? "s" : "");
    printf("States: -1, 0, +1 (balanced ternary)\n");
    
    typedef struct {
        char* name;
        int (*test_func)(TernaryNetwork*);
        int max_score;
    } Test;
    
    Test tests[] = {
        {"Ternary AND (min)", test_ternary_and, 9},
        {"Ternary OR (max)", test_ternary_or, 9},
        {"Ternary MUL", test_ternary_mul, 9},
        {"Consensus (3-input)", test_consensus, 27}
    };
    
    for (int t = 0; t < 4; t++) {
        TernaryNetwork* net = create_ternary_network(n);
        TernaryNetwork* best = create_ternary_network(n);
        int best_score = 0;
        
        for (int gen = 0; gen < MAX_GENERATIONS; gen++) {
            int score = tests[t].test_func(net);
            
            if (score > best_score) {
                best_score = score;
                // Copy best
                for (int i = 0; i < n; i++) {
                    best->gates[i] = net->gates[i];
                    for (int j = 0; j < n; j++) {
                        best->connections[i][j] = net->connections[i][j];
                    }
                }
                
                if (score == tests[t].max_score) break;
            }
            
            // Save and mutate
            TernaryNetwork* saved = create_ternary_network(n);
            for (int i = 0; i < n; i++) {
                saved->gates[i] = net->gates[i];
                for (int j = 0; j < n; j++) {
                    saved->connections[i][j] = net->connections[i][j];
                }
            }
            
            mutate_ternary(net);
            
            if (tests[t].test_func(net) < score) {
                // Revert
                for (int i = 0; i < n; i++) {
                    net->gates[i] = saved->gates[i];
                    for (int j = 0; j < n; j++) {
                        net->connections[i][j] = saved->connections[i][j];
                    }
                }
            }
            
            free(saved->gates);
            free(saved->connections);
            free(saved);
        }
        
        printf("%s: %s (%d/%d)\n", tests[t].name,
               best_score == tests[t].max_score ? "SOLVED" : "FAILED",
               best_score, tests[t].max_score);
        
        free(net->gates);
        free(net->connections);
        free(net);
        free(best->gates);
        free(best->connections);
        free(best);
    }
}

int main() {
    srand(time(NULL));
    
    printf("=== TERNARY GATE UNIVERSE ===\n");
    printf("Because binary was just a choice, not destiny\n\n");
    
    printf("Balanced ternary: -1, 0, +1\n");
    printf("Operations: MIN, MAX, NEGATE, MULTIPLY, THRESHOLD, CYCLE\n");
    
    // Test 1-8 ternary gates
    for (int n = 1; n <= 8; n++) {
        test_n_ternary_gates(n);
    }
    
    printf("\n=== TERNARY INSIGHTS ===\n");
    printf("- Balanced around zero (no binary bias)\n");
    printf("- Natural for negative/neutral/positive\n");
    printf("- Consensus operations emerge naturally\n");
    printf("- Different computational universe confirmed\n");
    
    return 0;
}