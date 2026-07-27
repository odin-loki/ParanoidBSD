#define SIZE 42
#define WRAPPED (99)
#define CASTED ((size_t)0)
#define GREETING "hi"
#define FEATURE 1
#if FEATURE
int use_feature(void);
#endif
#define SKIPME (1+2)
#define PAGES (32 * 1024)
#define MSG "flags"

const char *usage_msg(void) {
  /* Adjacent string concat — MSG must remain a #define */
  return "usage: [-" MSG "]\n";
}

int main(void) {
  return SIZE + WRAPPED + (int)CASTED + SKIPME + PAGES;
}
