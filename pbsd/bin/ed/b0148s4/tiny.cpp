#include <cstdio>
extern "C" char *ref_translit_text(char *, int, int, int);
int main() {
  char b[4] = "abc";
  ref_translit_text(b, 3, 'a', 'A');
  std::printf("ok %s\n", b);
  return 0;
}
