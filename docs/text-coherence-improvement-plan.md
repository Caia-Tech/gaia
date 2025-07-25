# Text Coherence Improvement Plan

## Current State
- Basic word associations (bigrams)
- Simple context window
- Pattern-based responses
- 77% coherence achieved

## Improvement Areas

### 1. Enhanced Association Learning (Week 1)
**Goal**: Move from bigrams to trigrams and context-aware associations

```c
typedef struct {
    char context[3][MAX_WORD_LENGTH];  // trigram
    char next_word[MAX_WORD_LENGTH];
    int frequency;
    float confidence;
} TrigramAssociation;
```

**Implementation**:
- Store 3-word contexts instead of pairs
- Weight associations by recency
- Track confidence scores
- Implement forgetting for old patterns

### 2. Semantic Clustering (Week 2)
**Goal**: Group related words to maintain topic consistency

```c
typedef struct {
    char topic[50];
    char words[100][MAX_WORD_LENGTH];
    int word_count;
    Gate* cluster_gate;
} SemanticCluster;
```

**Features**:
- Auto-cluster words that appear together
- Topic persistence across responses
- Smooth topic transitions
- Prevent jarring subject changes

### 3. Grammar Rule Gates (Week 3)
**Goal**: Ensure grammatical correctness

```c
typedef struct {
    Gate* subject_verb_agreement;
    Gate* tense_consistency;
    Gate* article_usage;
    Gate* punctuation_rules;
} GrammarGates;
```

**Rules to implement**:
- Subject-verb agreement
- Consistent tense
- Proper article usage (a/an/the)
- Sentence structure patterns

### 4. Response Diversity (Week 4)
**Goal**: Avoid repetitive outputs

```c
typedef struct {
    char recent_responses[10][MAX_RESPONSE_LENGTH];
    int response_count;
    Gate* diversity_enforcer;
} ResponseHistory;
```

**Mechanisms**:
- Track recent outputs
- Penalize repetition
- Encourage synonym usage
- Vary sentence structures

### 5. Conversation Memory (Week 5)
**Goal**: Remember context across multiple exchanges

```c
typedef struct {
    char entities[50][MAX_WORD_LENGTH];  // Named entities
    char facts[50][MAX_FACT_LENGTH];     // Stated facts
    int conversation_turns;
    Gate* memory_consolidation;
} ConversationMemory;
```

**Features**:
- Entity tracking
- Fact consistency
- Reference resolution
- Long-term context

## Testing Strategy

### Coherence Metrics
1. **Perplexity Score**: How surprised the model is by next word
2. **Topic Drift**: Measure of staying on subject
3. **Grammar Score**: Rule violations per sentence
4. **Diversity Index**: Vocabulary richness
5. **Context Retention**: Information preservation

### Test Cases
```
Input: "What is gaia?"
Expected coherence: >90%
Topic consistency: HIGH
Grammar score: 100%

Input: "Tell me about logic gates"
Expected flow: Natural progression
Topic drift: <10%
Information density: MEDIUM
```

### Benchmark Targets
- Coherence: >90% (from current 77%)
- Response time: <100ms
- Memory usage: <10MB
- Grammar accuracy: >95%
- Topic consistency: >85%

## Implementation Order
1. Start with trigram associations (biggest impact)
2. Add grammar gates (ensures quality)
3. Implement semantic clustering (topic coherence)
4. Add conversation memory (context awareness)
5. Finally, diversity mechanisms (polish)

## Iteration Process
1. Implement feature
2. Run coherence tests
3. Measure improvement
4. Tune parameters
5. Validate with users
6. Document findings

## Success Criteria
- Sustained conversations feel natural
- Topics flow logically
- Grammar remains correct
- Responses vary appropriately
- Context is preserved
- No transformer-like complexity added

This plan maintains the gate-based approach while achieving coherence through structural improvements rather than statistical models.