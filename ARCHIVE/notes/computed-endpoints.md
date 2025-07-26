# Computed API Endpoints: O(1) Route Resolution

## The Revelation

Traditional API routing searches through patterns. This is unnecessary.

```c
// Old way: O(n) search through routes
for (route in routes) {
    if (pattern_match(request.path, route.pattern)) {
        return route.handler;
    }
}

// New way: O(1) computation
address = hash(method + path);
return handlers[address];
```

## Implementation

```c
typedef void (*handler_func)(request_t*);
handler_func handlers[MAX_ENDPOINTS];

void register_endpoint(char* method, char* path, handler_func handler) {
    uint32_t addr = hash_combine(method, path);
    handlers[addr % MAX_ENDPOINTS] = handler;
}

void handle_request(request_t* req) {
    uint32_t addr = hash_combine(req->method, req->path);
    handler_func handler = handlers[addr % MAX_ENDPOINTS];
    if (handler) handler(req);
}
```

## Benefits

1. **Constant Time**: Million endpoints = same speed as one
2. **No Pattern Matching**: Direct computation to handler
3. **Deterministic**: Same input always = same handler
4. **Cache Friendly**: Sequential memory access
5. **Stateless**: No routing table to maintain

## For gaia

Each gate/network can expose computed endpoints:
- `/gate/xor/eval` → Direct gate evaluation
- `/network/train` → Training circuit activation
- `/api/v1/compute` → Computation submission

No frameworks. No routers. Just math.

## The Paradigm

Stop searching. Start computing. Every lookup is just a missing equation.