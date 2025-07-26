#include <stdio.h>
#include <string.h>

// Test V8 directly without interactive mode

int main() {
    // Simulate what V8 should do with "What is 2 plus 2?"
    printf("Testing V8 workflow decomposition...\n");
    
    const char* query = "What is 2 plus 2?";
    printf("Query: %s\n", query);
    
    // Expected workflow steps:
    printf("\nExpected workflow decomposition:\n");
    printf("1. DECOMPOSE: Break down query\n");
    printf("2. ANALYZE: Identify calculation needed\n");
    printf("3. EXECUTE: Calculate 2 + 2\n");
    printf("4. EVALUATE: Verify result\n");
    printf("5. SYNTHESIZE: Format response\n");
    
    printf("\nExpected final output:\n");
    printf("GAIA V8: 2 plus 2 equals 4\n");
    
    // Test the actual system
    printf("\n=== Running actual V8 test ===\n");
    
    // Use popen to test
    FILE* pipe = popen("echo 'What is 2 plus 2?' | ./gaia_chat_v8 2>&1", "r");
    if (!pipe) {
        printf("Failed to run V8\n");
        return 1;
    }
    
    char buffer[256];
    int found_response = 0;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        if (strstr(buffer, "GAIA V8:")) {
            printf("Found response: %s", buffer);
            found_response = 1;
        }
    }
    
    pclose(pipe);
    
    if (!found_response) {
        printf("ERROR: No GAIA V8 response found!\n");
        return 1;
    }
    
    return 0;
}