#include <stdio.h>
#include "analysis_functions.h"

int main() {
    printf("Testing basic analysis functions...
");
    
    // Test prompt classification
    PromptType type = classify_prompt("What is 2 plus 2?");
    printf("Prompt type: %s
", prompt_type_to_string(type));
    
    // Test entity extraction
    Entity entities[10];
    int count = extract_entities("Calculate 5 plus 3", entities, 10);
    printf("Found %d entities
", count);
    for (int i = 0; i < count; i++) {
        printf("  Entity %d: '%s'
", i, entities[i].value);
    }
    
    // Test text metrics
    int words = count_words("Hello world test");
    printf("Word count: %d
", words);
    
    printf("All basic tests completed successfully\!
");
    return 0;
}
