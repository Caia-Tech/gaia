#ifndef TRANSFORMER_ATTENTION_H
#define TRANSFORMER_ATTENTION_H

#include <stdint.h>

// Configuration
#define MAX_SEQ_LENGTH 100
#define HIDDEN_DIM 64
#define NUM_HEADS 4
#define HEAD_DIM (HIDDEN_DIM / NUM_HEADS)
#define MAX_ISSUES 10
#define MAX_SUGGESTIONS 10

// Attention types for different heads
typedef enum {
    ATTN_PATTERN,    // Focus on repeated patterns
    ATTN_SYNTAX,     // Grammar and structure
    ATTN_SEMANTIC,   // Meaning and context
    ATTN_POSITION    // Relative positions
} AttentionType;

// Token representation
typedef struct {
    char word[50];
    float embedding[HIDDEN_DIM];
    int position;
} Token;

// Attention head
typedef struct {
    int head_index;
    float attention_scores[MAX_SEQ_LENGTH][MAX_SEQ_LENGTH];
    AttentionType type;
    float importance_weight;
} AttentionHead;

// Transformer layer
typedef struct {
    // Weight matrices
    float query_weights[HIDDEN_DIM][HIDDEN_DIM];
    float key_weights[HIDDEN_DIM][HIDDEN_DIM];
    float value_weights[HIDDEN_DIM][HIDDEN_DIM];
    float output_weights[HIDDEN_DIM][HIDDEN_DIM];
    
    // Multi-head attention
    AttentionHead heads[NUM_HEADS];
    
    // Layer normalization parameters
    float layer_norm_gamma[HIDDEN_DIM];
    float layer_norm_beta[HIDDEN_DIM];
    float layer_norm_epsilon;
    
    // Dimensions
    int num_heads;
    int hidden_dim;
} TransformerLayer;

// Response quality analysis
typedef struct {
    float coherence_score;
    float relevance_score;
    float completeness_score;
    float grammar_score;
    float overall_quality;
    
    char* issues[MAX_ISSUES];
    int num_issues;
    
    char* suggestions[MAX_SUGGESTIONS];
    int num_suggestions;
} ResponseAnalysis;

// Refinement context
typedef struct {
    char* original_response;
    char* current_response;
    char* input_query;
    
    ResponseAnalysis* analysis;
    
    int iteration_count;
    float quality_threshold;
    int max_iterations;
    
    float quality_history[10];
    int history_count;
} RefinementContext;

// Attention context for processing
typedef struct {
    Token* tokens;
    int num_tokens;
    TransformerLayer* layer;
    float output[MAX_SEQ_LENGTH][HIDDEN_DIM];
} AttentionContext;

// Function declarations

// Transformer initialization
TransformerLayer* create_transformer_layer(void);
void destroy_transformer_layer(TransformerLayer* layer);
void init_transformer_weights(TransformerLayer* layer);

// Tokenization
int tokenize_for_attention(const char* input, Token* tokens, int max_tokens);
void create_embeddings(Token* tokens, int num_tokens);
void add_position_encoding(Token* tokens, int num_tokens);

// Attention computation
void compute_attention(AttentionContext* ctx);
void compute_single_head_attention(AttentionContext* ctx, int head_idx);
void apply_multi_head_attention(AttentionContext* ctx);
float* attention_output(AttentionContext* ctx, int position);

// Matrix operations
void matrix_multiply(float* a, float* b, float* result, int m, int n, int p);
void softmax(float* scores, int length);
void layer_norm(float* input, float* gamma, float* beta, float epsilon, int dim);

// Response analysis
ResponseAnalysis* analyze_response(const char* query, const char* response);
float calculate_coherence(const char* response);
float calculate_relevance(const char* query, const char* response);
float calculate_completeness(const char* query, const char* response);
float calculate_grammar_score(const char* response);
void identify_issues(ResponseAnalysis* analysis, const char* query, const char* response);

// Refinement
RefinementContext* create_refinement_context(const char* query, const char* response);
void destroy_refinement_context(RefinementContext* ctx);
char* refine_response_v8(RefinementContext* ctx);
char* apply_refinement_suggestions_v8(const char* response, ResponseAnalysis* analysis);
int should_continue_refinement(RefinementContext* ctx);

// Visualization and debugging
void print_attention_matrix(AttentionHead* head, Token* tokens, int num_tokens);
void visualize_attention_patterns(AttentionContext* ctx);
char* explain_attention_focus(AttentionHead* head, Token* tokens, int num_tokens);

// Integration with V7
char* enhance_with_attention(const char* input, const char* base_response);
float get_attention_confidence(AttentionContext* ctx);

#endif // TRANSFORMER_ATTENTION_H