#undef SHADOW_STACK_SET_MARKER
#define SHADOW_STACK_SET_MARKER (1ULL << 1)
#undef SHADOW_STACK_SET_TOKEN
#define SHADOW_STACK_SET_TOKEN (1ULL << 0)
#undef page_size
#define page_size(p) ((unsigned long)(PAGE_SIZE << compound_order(p)))
