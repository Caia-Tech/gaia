#include <stdio.h>
#include <string.h>
#include "analysis_functions.h"

int main() {
    printf("=== Debug Analysis Functions ===\n");
    
    // Test prompt classification
    printf("\n1. Testing prompt classification...\n");
    PromptType type1 = classify_prompt("What is 2 plus 2?");
    printf("  'What is 2 plus 2?' -> %s\n", prompt_type_to_string(type1));
    
    PromptType type2 = classify_prompt("Is this working?");
    printf("  'Is this working?' -> %s\n", prompt_type_to_string(type2));
    
    PromptType type3 = classify_prompt("Hello");
    printf("  'Hello' -> %s\n", prompt_type_to_string(type3));
    
    // Test entity extraction
    printf("\n2. Testing entity extraction...\n");
    Entity entities[10];
    
    int count1 = extract_entities("Calculate 5 plus 3", entities, 10);
    printf("  'Calculate 5 plus 3' found %d entities\n", count1);
    for (int i = 0; i < count1; i++) {
        printf("    Entity %d: '%s'\n", i, entities[i].value);
    }
    
    int count2 = extract_entities("List three colors", entities, 10);
    printf("  'List three colors' found %d entities\n", count2);
    for (int i = 0; i < count2; i++) {
        printf("    Entity %d: '%s'\n", i, entities[i].value);
    }
    
    // Test text metrics  
    printf("\n3. Testing text metrics...\n");
    int words1 = count_words("Hello world");
    printf("  'Hello world' has %d words\n", words1);
    
    int words2 = count_words("This is a test.");
    printf("  'This is a test.' has %d words\n", words2);
    
    int sentences = count_sentences("First sentence. Second sentence.");
    printf("  'First sentence. Second sentence.' has %d sentences\n", sentences);
    
    printf("\nAll debug tests completed!\n");
    return 0;
}