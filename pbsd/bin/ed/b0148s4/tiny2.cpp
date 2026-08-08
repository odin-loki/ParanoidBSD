import pbsd.bin.ed.b0148s4;
#include <cstdio>
int main() {
  char b[4] = "abc";
  pbsd::bin_ed::b0148s4::translit_text(b, 3, 'a', 'A');
  std::printf("ok %s\n", b);
  return 0;
}
