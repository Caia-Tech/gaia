#include "transformer_attention.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

// Initialize transformer layer
TransformerLayer* create_transformer_layer(void) {
    TransformerLayer* layer = calloc(1, sizeof(TransformerLayer));
    if (!layer) return NULL;
    
    layer->num_heads = NUM_HEADS;
    layer->hidden_dim = HIDDEN_DIM;
    layer->layer_norm_epsilon = 1e-5;
    
    // Initialize layer norm parameters
    for (int i = 0; i < HIDDEN_DIM; i++) {
        layer->layer_norm_gamma[i] = 1.0;
        layer->layer_norm_beta[i] = 0.0;
    }
    
    // Initialize attention heads
    for (int i = 0; i < NUM_HEADS; i++) {
        layer->heads[i].head_index = i;
        layer->heads[i].type = (AttentionType)(i % 4);
        layer->heads[i].importance_weight = 1.0 / NUM_HEADS;
    }
    
    init_transformer_weights(layer);
    
    return layer;
}

// Initialize weight matrices with small random values
void init_transformer_weights(TransformerLayer* layer) {
    srand(time(NULL));
    float scale = sqrt(2.0 / HIDDEN_DIM);
    
    for (int i = 0; i < HIDDEN_DIM; i++) {
        for (int j = 0; j < HIDDEN_DIM; j++) {
            layer->query_weights[i][j] = ((float)rand() / RAND_MAX - 0.5) * scale;
            layer->key_weights[i][j] = ((float)rand() / RAND_MAX - 0.5) * scale;
            layer->value_weights[i][j] = ((float)rand() / RAND_MAX - 0.5) * scale;
            layer->output_weights[i][j] = ((float)rand() / RAND_MAX - 0.5) * scale;
        }
    }
}

// Tokenize input for attention processing
int tokenize_for_attention(const char* input, Token* tokens, int max_tokens) {
    if (!input || !tokens) return 0;
    
    char temp[1024];
    strncpy(temp, input, 1023);
    temp[1023] = '\0';
    
    int count = 0;
    char* word = strtok(temp, " \t\n.,!?;:");
    
    while (word && count < max_tokens) {
        strncpy(tokens[count].word, word, 49);
        tokens[count].word[49] = '\0';
        tokens[count].position = count;
        count++;
        word = strtok(NULL, " \t\n.,!?;:");
    }
    
    return count;
}

// Create simple embeddings (in practice, would use learned embeddings)
void create_embeddings(Token* tokens, int num_tokens) {
    for (int i = 0; i < num_tokens; i++) {
        // Simple hash-based embedding
        unsigned int hash = 5381;
        for (int j = 0; tokens[i].word[j]; j++) {
            hash = ((hash << 5) + hash) + tokens[i].word[j];
        }
        
        // Convert to embedding vector
        for (int d = 0; d < HIDDEN_DIM; d++) {
            tokens[i].embedding[d] = sin(hash * (d + 1) * 0.001) * 0.1;
        }
    }
}

// Add sinusoidal position encoding
void add_position_encoding(Token* tokens, int num_tokens) {
    for (int pos = 0; pos < num_tokens; pos++) {
        for (int i = 0; i < HIDDEN_DIM; i++) {
            float angle = pos / pow(10000.0, (2.0 * (i / 2)) / HIDDEN_DIM);
            if (i % 2 == 0) {
                tokens[pos].embedding[i] += sin(angle) * 0.1;
            } else {
                tokens[pos].embedding[i] += cos(angle) * 0.1;
            }
        }
    }
}

// Matrix multiplication: result = a × b
void matrix_multiply(float* a, float* b, float* result, int m, int n, int p) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            float sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += a[i * n + k] * b[k * p + j];
            }
            result[i * p + j] = sum;
        }
    }
}

// Softmax function
void softmax(float* scores, int length) {
    float max_score = scores[0];
    for (int i = 1; i < length; i++) {
        if (scores[i] > max_score) max_score = scores[i];
    }
    
    float sum = 0.0;
    for (int i = 0; i < length; i++) {
        scores[i] = exp(scores[i] - max_score);
        sum += scores[i];
    }
    
    for (int i = 0; i < length; i++) {
        scores[i] /= sum;
    }
}

// Compute attention for a single head
void compute_single_head_attention(AttentionContext* ctx, int head_idx) {
    AttentionHead* head = &ctx->layer->heads[head_idx];
    int num_tokens = ctx->num_tokens;
    
    // Temporary arrays for Q, K, V
    float queries[MAX_SEQ_LENGTH][HEAD_DIM];
    float keys[MAX_SEQ_LENGTH][HEAD_DIM];
    float values[MAX_SEQ_LENGTH][HEAD_DIM];
    
    // Project to head dimension
    int head_start = head_idx * HEAD_DIM;
    for (int i = 0; i < num_tokens; i++) {
        for (int j = 0; j < HEAD_DIM; j++) {
            queries[i][j] = 0;
            keys[i][j] = 0;
            values[i][j] = 0;
            
            // Simple projection (in practice would use learned weights)
            for (int k = 0; k < HIDDEN_DIM; k++) {
                float emb = ctx->tokens[i].embedding[k];
                queries[i][j] += emb * ctx->layer->query_weights[k][head_start + j];
                keys[i][j] += emb * ctx->layer->key_weights[k][head_start + j];
                values[i][j] += emb * ctx->layer->value_weights[k][head_start + j];
            }
        }
    }
    
    // Compute attention scores
    float scale = 1.0 / sqrt(HEAD_DIM);
    for (int i = 0; i < num_tokens; i++) {
        for (int j = 0; j < num_tokens; j++) {
            float score = 0.0;
            for (int k = 0; k < HEAD_DIM; k++) {
                score += queries[i][k] * keys[j][k];
            }
            head->attention_scores[i][j] = score * scale;
        }
        
        // Apply softmax to get attention weights
        softmax(head->attention_scores[i], num_tokens);
    }
    
    // Apply attention to values
    for (int i = 0; i < num_tokens; i++) {
        for (int j = 0; j < HEAD_DIM; j++) {
            float sum = 0.0;
            for (int k = 0; k < num_tokens; k++) {
                sum += head->attention_scores[i][k] * values[k][j];
            }
            ctx->output[i][head_start + j] = sum;
        }
    }
}

// Apply multi-head attention
void apply_multi_head_attention(AttentionContext* ctx) {
    // Compute attention for each head
    for (int h = 0; h < NUM_HEADS; h++) {
        compute_single_head_attention(ctx, h);
    }
    
    // Apply output projection (simplified)
    for (int i = 0; i < ctx->num_tokens; i++) {
        float temp[HIDDEN_DIM];
        memcpy(temp, ctx->output[i], sizeof(float) * HIDDEN_DIM);
        
        for (int j = 0; j < HIDDEN_DIM; j++) {
            float sum = 0.0;
            for (int k = 0; k < HIDDEN_DIM; k++) {
                sum += temp[k] * ctx->layer->output_weights[k][j];
            }
            ctx->output[i][j] = sum;
        }
        
        // Add residual connection
        for (int j = 0; j < HIDDEN_DIM; j++) {
            ctx->output[i][j] += ctx->tokens[i].embedding[j];
        }
    }
    
    // Apply layer normalization
    for (int i = 0; i < ctx->num_tokens; i++) {
        layer_norm(ctx->output[i], ctx->layer->layer_norm_gamma, 
                  ctx->layer->layer_norm_beta, ctx->layer->layer_norm_epsilon, 
                  HIDDEN_DIM);
    }
}

// Layer normalization
void layer_norm(float* input, float* gamma, float* beta, float epsilon, int dim) {
    // Calculate mean
    float mean = 0.0;
    for (int i = 0; i < dim; i++) {
        mean += input[i];
    }
    mean /= dim;
    
    // Calculate variance
    float variance = 0.0;
    for (int i = 0; i < dim; i++) {
        float diff = input[i] - mean;
        variance += diff * diff;
    }
    variance /= dim;
    
    // Normalize and scale
    float std_dev = sqrt(variance + epsilon);
    for (int i = 0; i < dim; i++) {
        input[i] = gamma[i] * (input[i] - mean) / std_dev + beta[i];
    }
}

// Analyze response quality
ResponseAnalysis* analyze_response(const char* query, const char* response) {
    ResponseAnalysis* analysis = calloc(1, sizeof(ResponseAnalysis));
    if (!analysis) return NULL;
    
    analysis->coherence_score = calculate_coherence(response);
    analysis->relevance_score = calculate_relevance(query, response);
    analysis->completeness_score = calculate_completeness(query, response);
    analysis->grammar_score = calculate_grammar_score(response);
    
    // Calculate overall quality
    analysis->overall_quality = (analysis->coherence_score * 0.25 +
                                analysis->relevance_score * 0.35 +
                                analysis->completeness_score * 0.25 +
                                analysis->grammar_score * 0.15);
    
    identify_issues(analysis, query, response);
    
    return analysis;
}

// Calculate coherence score
float calculate_coherence(const char* response) {
    if (!response || strlen(response) == 0) return 0.0;
    
    float score = 0.8; // Base score
    
    // Check for repetition
    if (strstr(response, response) != response) {
        score -= 0.2; // Penalize obvious repetition
    }
    
    // Check sentence structure
    int sentence_count = 0;
    int word_count = 0;
    for (int i = 0; response[i]; i++) {
        if (response[i] == '.' || response[i] == '!' || response[i] == '?') {
            sentence_count++;
        }
        if (response[i] == ' ') word_count++;
    }
    
    if (sentence_count == 0) score -= 0.1;
    if (word_count < 3) score -= 0.2;
    
    return fmax(0.0, fmin(1.0, score));
}

// Calculate relevance to query
float calculate_relevance(const char* query, const char* response) {
    if (!query || !response) return 0.0;
    
    // Convert to lowercase for comparison
    char query_lower[512], response_lower[1024];
    strncpy(query_lower, query, 511);
    strncpy(response_lower, response, 1023);
    
    for (int i = 0; query_lower[i]; i++) {
        query_lower[i] = tolower(query_lower[i]);
    }
    for (int i = 0; response_lower[i]; i++) {
        response_lower[i] = tolower(response_lower[i]);
    }
    
    // Count keyword matches
    int matches = 0;
    int total_words = 0;
    
    char* word = strtok(query_lower, " \t\n.,!?;:");
    while (word) {
        if (strlen(word) > 2) { // Skip short words
            if (strstr(response_lower, word)) {
                matches++;
            }
            total_words++;
        }
        word = strtok(NULL, " \t\n.,!?;:");
    }
    
    return total_words > 0 ? (float)matches / total_words : 0.5;
}

// Calculate completeness
float calculate_completeness(const char* query, const char* response) {
    // Check if response addresses all parts of query
    float score = 0.7; // Base score
    
    // Check for question marks in query (multi-part)
    int question_count = 0;
    for (int i = 0; query[i]; i++) {
        if (query[i] == '?') question_count++;
    }
    
    if (question_count > 1) {
        // Multi-part question - check if response is long enough
        if (strlen(response) < 50) score -= 0.3;
    }
    
    // Check response length relative to query
    float length_ratio = (float)strlen(response) / (strlen(query) + 1);
    if (length_ratio < 0.5) score -= 0.2;
    if (length_ratio > 10) score -= 0.1; // Too verbose
    
    return fmax(0.0, fmin(1.0, score));
}

// Basic grammar scoring
float calculate_grammar_score(const char* response) {
    float score = 0.9; // Base score
    
    // Check capitalization
    if (response[0] && !isupper(response[0])) score -= 0.1;
    
    // Check for basic punctuation
    int len = strlen(response);
    if (len > 0) {
        char last = response[len - 1];
        if (last != '.' && last != '!' && last != '?') {
            score -= 0.1;
        }
    }
    
    // Check for double spaces
    if (strstr(response, "  ")) score -= 0.05;
    
    return fmax(0.0, fmin(1.0, score));
}

// Identify specific issues
void identify_issues(ResponseAnalysis* analysis, const char* query, const char* response) {
    analysis->num_issues = 0;
    analysis->num_suggestions = 0;
    
    // Check for low scores
    if (analysis->coherence_score < 0.6) {
        analysis->issues[analysis->num_issues++] = strdup("Response lacks coherence");
        analysis->suggestions[analysis->num_suggestions++] = 
            strdup("Restructure response for better flow");
    }
    
    if (analysis->relevance_score < 0.5) {
        analysis->issues[analysis->num_issues++] = strdup("Response not relevant to query");
        analysis->suggestions[analysis->num_suggestions++] = 
            strdup("Focus on addressing the specific question");
    }
    
    if (analysis->completeness_score < 0.6) {
        analysis->issues[analysis->num_issues++] = strdup("Response is incomplete");
        analysis->suggestions[analysis->num_suggestions++] = 
            strdup("Add more details to fully answer the question");
    }
    
    if (analysis->grammar_score < 0.7) {
        analysis->issues[analysis->num_issues++] = strdup("Grammar needs improvement");
        analysis->suggestions[analysis->num_suggestions++] = 
            strdup("Check punctuation and capitalization");
    }
}

// Create refinement context
RefinementContext* create_refinement_context(const char* query, const char* response) {
    RefinementContext* ctx = calloc(1, sizeof(RefinementContext));
    if (!ctx) return NULL;
    
    ctx->input_query = strdup(query);
    ctx->original_response = strdup(response);
    ctx->current_response = strdup(response);
    
    ctx->quality_threshold = 0.8;
    ctx->max_iterations = 5;
    ctx->iteration_count = 0;
    
    return ctx;
}

// Destroy refinement context
void destroy_refinement_context(RefinementContext* ctx) {
    if (!ctx) return;
    
    free(ctx->input_query);
    free(ctx->original_response);
    free(ctx->current_response);
    
    if (ctx->analysis) {
        for (int i = 0; i < ctx->analysis->num_issues; i++) {
            free(ctx->analysis->issues[i]);
        }
        for (int i = 0; i < ctx->analysis->num_suggestions; i++) {
            free(ctx->analysis->suggestions[i]);
        }
        free(ctx->analysis);
    }
    
    free(ctx);
}

// Refine response iteratively
char* refine_response_v8(RefinementContext* ctx) {
    if (!ctx) return NULL;
    
    while (should_continue_refinement(ctx)) {
        // Analyze current response
        if (ctx->analysis) {
            for (int i = 0; i < ctx->analysis->num_issues; i++) {
                free(ctx->analysis->issues[i]);
            }
            for (int i = 0; i < ctx->analysis->num_suggestions; i++) {
                free(ctx->analysis->suggestions[i]);
            }
            free(ctx->analysis);
        }
        
        ctx->analysis = analyze_response(ctx->input_query, ctx->current_response);
        
        // Store quality history
        if (ctx->history_count < 10) {
            ctx->quality_history[ctx->history_count++] = ctx->analysis->overall_quality;
        }
        
        // Apply refinements
        char* refined = apply_refinement_suggestions_v8(ctx->current_response, ctx->analysis);
        if (refined) {
            free(ctx->current_response);
            ctx->current_response = refined;
        }
        
        ctx->iteration_count++;
    }
    
    return strdup(ctx->current_response);
}

// Apply refinement suggestions
char* apply_refinement_suggestions_v8(const char* response, ResponseAnalysis* analysis) {
    char* refined = malloc(strlen(response) * 2 + 100);
    if (!refined) return NULL;
    
    strcpy(refined, response);
    
    // Apply basic refinements based on issues
    for (int i = 0; i < analysis->num_issues; i++) {
        if (strstr(analysis->issues[i], "incomplete")) {
            // Add a clarifying sentence
            strcat(refined, " Let me provide more details.");
        } else if (strstr(analysis->issues[i], "Grammar")) {
            // Fix basic grammar issues
            if (!isupper(refined[0]) && isalpha(refined[0])) {
                refined[0] = toupper(refined[0]);
            }
            
            int len = strlen(refined);
            if (len > 0 && refined[len-1] != '.' && refined[len-1] != '!' && refined[len-1] != '?') {
                strcat(refined, ".");
            }
        }
    }
    
    return refined;
}

// Check if refinement should continue
int should_continue_refinement(RefinementContext* ctx) {
    if (ctx->iteration_count >= ctx->max_iterations) return 0;
    
    if (ctx->analysis && ctx->analysis->overall_quality >= ctx->quality_threshold) {
        return 0;
    }
    
    // Check if quality is improving
    if (ctx->history_count >= 3) {
        float recent_avg = (ctx->quality_history[ctx->history_count-1] + 
                           ctx->quality_history[ctx->history_count-2] + 
                           ctx->quality_history[ctx->history_count-3]) / 3;
        float older_avg = (ctx->quality_history[ctx->history_count-3] + 
                          ctx->quality_history[ctx->history_count-4] + 
                          ctx->quality_history[ctx->history_count-5]) / 3;
        
        if (recent_avg <= older_avg) {
            return 0; // Not improving
        }
    }
    
    return 1;
}

// Print attention matrix for visualization
void print_attention_matrix(AttentionHead* head, Token* tokens, int num_tokens) {
    printf("\nAttention Matrix (Head %d - %s):\n", head->head_index,
           head->type == ATTN_PATTERN ? "PATTERN" :
           head->type == ATTN_SYNTAX ? "SYNTAX" :
           head->type == ATTN_SEMANTIC ? "SEMANTIC" : "POSITION");
    
    // Print column headers
    printf("        ");
    for (int j = 0; j < num_tokens && j < 10; j++) {
        printf("%-8.8s ", tokens[j].word);
    }
    printf("\n");
    
    // Print rows
    for (int i = 0; i < num_tokens && i < 10; i++) {
        printf("%-8.8s", tokens[i].word);
        for (int j = 0; j < num_tokens && j < 10; j++) {
            float score = head->attention_scores[i][j];
            if (score > 0.3) {
                printf("\033[1;31m"); // Red for high attention
            } else if (score > 0.1) {
                printf("\033[1;33m"); // Yellow for medium
            }
            printf("%7.3f ", score);
            printf("\033[0m"); // Reset color
        }
        printf("\n");
    }
}

// Main enhancement function integrating with V7
char* enhance_with_attention(const char* input, const char* base_response) {
    // Create transformer layer
    TransformerLayer* layer = create_transformer_layer();
    if (!layer) return strdup(base_response);
    
    // Tokenize input
    Token tokens[MAX_SEQ_LENGTH];
    int num_tokens = tokenize_for_attention(input, tokens, MAX_SEQ_LENGTH);
    if (num_tokens == 0) {
        destroy_transformer_layer(layer);
        return strdup(base_response);
    }
    
    // Create embeddings and position encoding
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
    
    // Create refinement context
    RefinementContext* refine_ctx = create_refinement_context(input, base_response);
    if (!refine_ctx) {
        destroy_transformer_layer(layer);
        return strdup(base_response);
    }
    
    // Refine response
    char* refined_response = refine_response_v8(refine_ctx);
    
    // Cleanup
    destroy_refinement_context(refine_ctx);
    destroy_transformer_layer(layer);
    
    return refined_response ? refined_response : strdup(base_response);
}

// Get attention confidence based on attention patterns
float get_attention_confidence(AttentionContext* ctx) {
    if (!ctx || !ctx->layer) return 0.5;
    
    float confidence = 0.0;
    int num_strong_connections = 0;
    
    // Analyze attention patterns across all heads
    for (int h = 0; h < NUM_HEADS; h++) {
        AttentionHead* head = &ctx->layer->heads[h];
        
        // Count strong attention connections
        for (int i = 0; i < ctx->num_tokens; i++) {
            for (int j = 0; j < ctx->num_tokens; j++) {
                if (head->attention_scores[i][j] > 0.3) {
                    num_strong_connections++;
                }
            }
        }
    }
    
    // Calculate confidence based on pattern strength
    float avg_connections = (float)num_strong_connections / (NUM_HEADS * ctx->num_tokens * ctx->num_tokens);
    confidence = fmin(1.0, avg_connections * 2.0);
    
    return confidence;
}

// Cleanup transformer layer
void destroy_transformer_layer(TransformerLayer* layer) {
    if (layer) {
        free(layer);
    }
}