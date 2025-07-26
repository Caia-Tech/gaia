#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "transformer_attention.h"

// Test response analysis
void test_response_analysis() {
    printf("=== Testing Response Analysis ===\n\n");
    
    // Test cases
    struct {
        const char* query;
        const char* response;
        const char* test_name;
    } test_cases[] = {
        {
            "What is 2 plus 2?",
            "The answer is 4.",
            "Good response"
        },
        {
            "What is the capital of France?",
            "paris",
            "Missing capitalization"
        },
        {
            "Explain photosynthesis and respiration",
            "Plants make food.",
            "Incomplete response"
        },
        {
            "What is machine learning?",
            "Machine learning is when computers learn from data patterns.",
            "Decent explanation"
        },
        {NULL, NULL, NULL}
    };
    
    for (int i = 0; test_cases[i].query != NULL; i++) {
        printf("Test: %s\n", test_cases[i].test_name);
        printf("Query: %s\n", test_cases[i].query);
        printf("Response: %s\n", test_cases[i].response);
        
        ResponseAnalysis* analysis = analyze_response(test_cases[i].query, test_cases[i].response);
        
        printf("Analysis:\n");
        printf("  Overall Quality: %.2f\n", analysis->overall_quality);
        printf("  Coherence: %.2f\n", analysis->coherence_score);
        printf("  Relevance: %.2f\n", analysis->relevance_score);
        printf("  Completeness: %.2f\n", analysis->completeness_score);
        printf("  Grammar: %.2f\n", analysis->grammar_score);
        
        if (analysis->num_issues > 0) {
            printf("  Issues:\n");
            for (int j = 0; j < analysis->num_issues; j++) {
                printf("    - %s\n", analysis->issues[j]);
            }
        }
        
        if (analysis->num_suggestions > 0) {
            printf("  Suggestions:\n");
            for (int j = 0; j < analysis->num_suggestions; j++) {
                printf("    - %s\n", analysis->suggestions[j]);
            }
        }
        
        printf("\n");
        
        // Cleanup
        for (int j = 0; j < analysis->num_issues; j++) {
            free(analysis->issues[j]);
        }
        for (int j = 0; j < analysis->num_suggestions; j++) {
            free(analysis->suggestions[j]);
        }
        free(analysis);
    }
}

// Test attention mechanism
void test_attention_mechanism() {
    printf("=== Testing Attention Mechanism ===\n\n");
    
    // Create transformer layer
    TransformerLayer* layer = create_transformer_layer();
    if (!layer) {
        printf("Failed to create transformer layer\n");
        return;
    }
    
    // Test input
    const char* input = "The quick brown fox jumps over the lazy dog";
    printf("Input: %s\n\n", input);
    
    // Tokenize
    Token tokens[MAX_SEQ_LENGTH];
    int num_tokens = tokenize_for_attention(input, tokens, MAX_SEQ_LENGTH);
    printf("Tokenized into %d tokens\n", num_tokens);
    
    // Create embeddings
    create_embeddings(tokens, num_tokens);
    add_position_encoding(tokens, num_tokens);
    
    // Create attention context
    AttentionContext ctx = {
        .tokens = tokens,
        .num_tokens = num_tokens,
        .layer = layer
    };
    
    // Apply attention
    apply_multi_head_attention(&ctx);
    
    // Print attention patterns for first two heads
    for (int h = 0; h < 2 && h < NUM_HEADS; h++) {
        print_attention_matrix(&layer->heads[h], tokens, num_tokens);
    }
    
    // Get confidence
    float confidence = get_attention_confidence(&ctx);
    printf("\nAttention confidence: %.2f\n", confidence);
    
    // Cleanup
    destroy_transformer_layer(layer);
}

// Test refinement
void test_refinement() {
    printf("\n=== Testing Iterative Refinement ===\n\n");
    
    struct {
        const char* query;
        const char* initial_response;
        const char* test_name;
    } test_cases[] = {
        {
            "What is addition?",
            "addition is math",
            "Poor grammar and incomplete"
        },
        {
            "Explain gravity",
            "Gravity pulls things down.",
            "Too simple"
        },
        {NULL, NULL, NULL}
    };
    
    for (int i = 0; test_cases[i].query != NULL; i++) {
        printf("Test: %s\n", test_cases[i].test_name);
        printf("Query: %s\n", test_cases[i].query);
        printf("Initial: %s\n", test_cases[i].initial_response);
        
        // Create refinement context
        RefinementContext* ctx = create_refinement_context(
            test_cases[i].query, 
            test_cases[i].initial_response
        );
        
        // Set lower threshold for testing
        ctx->quality_threshold = 0.7;
        ctx->max_iterations = 3;
        
        // Refine
        char* refined = refine_response_v8(ctx);
        printf("Refined: %s\n", refined);
        printf("Iterations: %d\n", ctx->iteration_count);
        printf("Quality improvement: %.2f -> %.2f\n\n",
               ctx->quality_history[0],
               ctx->analysis ? ctx->analysis->overall_quality : 0);
        
        // Cleanup
        free(refined);
        destroy_refinement_context(ctx);
    }
}

int main() {
    printf("=== GAIA V8 Component Tests ===\n\n");
    
    test_response_analysis();
    test_attention_mechanism();
    test_refinement();
    
    printf("\n=== Tests Complete ===\n");
    return 0;
}