/**
 * Fixed Gate Test - Properly tests AND/OR capabilities
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_GENERATIONS 5000

typedef struct {
    int type;       // 0=AND, 1=OR, 2=NOT, 3=XOR, 4=PASS
    int input1_src; // Which gate to get input1 from (-1 = input A, -2 = input B)
    int input2_src; // Which gate to get input2 from
} Gate;

typedef struct {
    Gate* gates;
    int num_gates;
} Network;

// Apply gate operation
int apply_gate(int type, int in1, int in2) {
    switch(type) {
        case 0: return in1 & in2;  // AND
        case 1: return in1 | in2;  // OR
        case 2: return !in1;       // NOT (ignores in2)
        case 3: return in1 ^ in2;  // XOR
        case 4: return in1;        // PASS (ignores in2)
    }
    return 0;
}

// Forward pass through network
int forward(Network* net, int a, int b) {
    int* values = malloc(net->num_gates * sizeof(int));
    
    for (int i = 0; i < net->num_gates; i++) {
        Gate* g = &net->gates[i];
        
        // Get input values
        int in1 = 0, in2 = 0;
        
        if (g->input1_src == -1) in1 = a;
        else if (g->input1_src == -2) in1 = b;
        else if (g->input1_src >= 0 && g->input1_src < i) in1 = values[g->input1_src];
        
        if (g->input2_src == -1) in2 = a;
        else if (g->input2_src == -2) in2 = b;
        else if (g->input2_src >= 0 && g->input2_src < i) in2 = values[g->input2_src];
        
        values[i] = apply_gate(g->type, in1, in2);
    }
    
    int result = values[net->num_gates - 1];
    free(values);
    return result;
}

// Test functions
int test_and(Network* net) {
    int tests[][3] = {{0,0,0}, {0,1,0}, {1,0,0}, {1,1,1}};
    int correct = 0;
    for (int i = 0; i < 4; i++) {
        if (forward(net, tests[i][0], tests[i][1]) == tests[i][2]) correct++;
    }
    return correct;
}

int test_or(Network* net) {
    int tests[][3] = {{0,0,0}, {0,1,1}, {1,0,1}, {1,1,1}};
    int correct = 0;
    for (int i = 0; i < 4; i++) {
        if (forward(net, tests[i][0], tests[i][1]) == tests[i][2]) correct++;
    }
    return correct;
}

int test_xor(Network* net) {
    int tests[][3] = {{0,0,0}, {0,1,1}, {1,0,1}, {1,1,0}};
    int correct = 0;
    for (int i = 0; i < 4; i++) {
        if (forward(net, tests[i][0], tests[i][1]) == tests[i][2]) correct++;
    }
    return correct;
}

int test_not(Network* net) {
    // Test NOT on first input
    return (forward(net, 0, 0) == 1 && forward(net, 1, 0) == 0) ? 2 : 0;
}

// Create random network
Network* create_network(int n) {
    Network* net = malloc(sizeof(Network));
    net->num_gates = n;
    net->gates = malloc(n * sizeof(Gate));
    
    for (int i = 0; i < n; i++) {
        net->gates[i].type = rand() % 5;
        
        // Set valid input sources
        if (i == 0) {
            // First gate must use inputs
            net->gates[i].input1_src = -1;  // Input A
            net->gates[i].input2_src = -2;  // Input B
        } else {
            // Later gates can use inputs or previous gates
            net->gates[i].input1_src = -(rand() % 2 + 1);  // -1 or -2
            if (rand() % 2) net->gates[i].input1_src = rand() % i;  // Or previous gate
            
            net->gates[i].input2_src = -(rand() % 2 + 1);
            if (rand() % 2) net->gates[i].input2_src = rand() % i;
        }
    }
    
    return net;
}

void mutate(Network* net) {
    int i = rand() % net->num_gates;
    int choice = rand() % 3;
    
    switch(choice) {
        case 0:  // Change gate type
            net->gates[i].type = rand() % 5;
            break;
        case 1:  // Change input1 source
            if (i == 0) {
                net->gates[i].input1_src = -(rand() % 2 + 1);
            } else {
                net->gates[i].input1_src = -(rand() % 2 + 1);
                if (rand() % 2) net->gates[i].input1_src = rand() % i;
            }
            break;
        case 2:  // Change input2 source
            if (i == 0) {
                net->gates[i].input2_src = -(rand() % 2 + 1);
            } else {
                net->gates[i].input2_src = -(rand() % 2 + 1);
                if (rand() % 2) net->gates[i].input2_src = rand() % i;
            }
            break;
    }
}

void copy_network(Network* dst, Network* src) {
    for (int i = 0; i < src->num_gates; i++) {
        dst->gates[i] = src->gates[i];
    }
}

void test_n_gates(int n) {
    printf("\n=== Testing %d Gate%s ===\n", n, n > 1 ? "s" : "");
    
    typedef struct {
        char* name;
        int (*test_func)(Network*);
        int max_score;
    } Test;
    
    Test tests[] = {
        {"AND", test_and, 4},
        {"OR", test_or, 4},
        {"XOR", test_xor, 4},
        {"NOT", test_not, 2}
    };
    
    for (int t = 0; t < 4; t++) {
        Network* net = create_network(n);
        Network* best = create_network(n);
        int best_score = 0;
        
        for (int gen = 0; gen < MAX_GENERATIONS; gen++) {
            int score = tests[t].test_func(net);
            
            if (score > best_score) {
                best_score = score;
                copy_network(best, net);
                
                if (score == tests[t].max_score) break;
            }
            
            Network* saved = create_network(n);
            copy_network(saved, net);
            
            mutate(net);
            
            if (tests[t].test_func(net) < score) {
                copy_network(net, saved);
            }
            
            free(saved->gates);
            free(saved);
        }
        
        printf("%s: %s (%d/%d)", tests[t].name,
               best_score == tests[t].max_score ? "SOLVED" : "FAILED",
               best_score, tests[t].max_score);
        
        if (best_score == tests[t].max_score && n <= 3) {
            printf(" - Config: ");
            for (int i = 0; i < n; i++) {
                Gate* g = &best->gates[i];
                const char* types[] = {"AND", "OR", "NOT", "XOR", "PASS"};
                printf("%s(", types[g->type]);
                
                if (g->input1_src == -1) printf("A");
                else if (g->input1_src == -2) printf("B");
                else printf("G%d", g->input1_src);
                
                if (g->type != 2 && g->type != 4) {  // NOT and PASS only use one input
                    printf(",");
                    if (g->input2_src == -1) printf("A");
                    else if (g->input2_src == -2) printf("B");
                    else printf("G%d", g->input2_src);
                }
                printf(") ");
            }
        }
        printf("\n");
        
        free(net->gates);
        free(net);
        free(best->gates);
        free(best);
    }
}

int main() {
    srand(time(NULL));
    
    printf("=== Fixed Gate Capability Test ===\n");
    printf("Available gates: AND, OR, NOT, XOR, PASS\n");
    
    for (int n = 1; n <= 8; n++) {
        test_n_gates(n);
    }
    
    return 0;
}