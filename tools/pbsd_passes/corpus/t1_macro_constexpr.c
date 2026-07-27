#define SIZE 42
#define WRAPPED (99)
#define CASTED ((size_t)0)
#define GREETING "hi"
#define FEATURE 1
#if FEATURE
int use_feature(void);
#endif
#define SKIPME (1+2)

int main(void) {
  return SIZE + WRAPPED + (int)CASTED;
}
