#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Basic gate operations
uint8_t gate_and(uint8_t a, uint8_t b) {
    return a & b;
}

uint8_t gate_or(uint8_t a, uint8_t b) {
    return a | b;
}

uint8_t gate_xor(uint8_t a, uint8_t b) {
    return a ^ b;
}

uint8_t gate_not(uint8_t a) {
    return !a;
}

// A simple binary circuit with 3 gates
typedef struct {
    uint8_t gate1_result;
    uint8_t gate2_result;
    uint8_t final_result;
} Circuit;

// Process inputs through a simple circuit
Circuit process_circuit(uint8_t input_a, uint8_t input_b) {
    Circuit c;
    
    // Gate 1: AND gate
    c.gate1_result = gate_and(input_a, input_b);
    
    // Gate 2: XOR gate
    c.gate2_result = gate_xor(input_a, input_b);
    
    // Final gate: OR the results
    c.final_result = gate_or(c.gate1_result, c.gate2_result);
    
    return c;
}

// Print binary representation
void print_binary(uint8_t value) {
    printf("%d", value ? 1 : 0);
}

int main() {
    printf("gaia Binary Gates Demo\n");
    printf("========================\n\n");
    
    // Test all input combinations
    printf("A B | AND XOR | Final\n");
    printf("----+--------+------\n");
    
    for (uint8_t a = 0; a <= 1; a++) {
        for (uint8_t b = 0; b <= 1; b++) {
            Circuit result = process_circuit(a, b);
            
            print_binary(a);
            printf(" ");
            print_binary(b);
            printf(" |  ");
            print_binary(result.gate1_result);
            printf("   ");
            print_binary(result.gate2_result);
            printf("  |   ");
            print_binary(result.final_result);
            printf("\n");
        }
    }
    
    return 0;
}