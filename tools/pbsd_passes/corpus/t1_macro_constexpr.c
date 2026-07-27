#define SIZE 42
#define GREETING "hi"
#define FEATURE 1
#if FEATURE
int use_feature(void);
#endif
#define SKIPME (1+2)

int main(void) {
  return SIZE;
}
