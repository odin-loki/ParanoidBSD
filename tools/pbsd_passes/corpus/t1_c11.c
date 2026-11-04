void f(char * restrict p, _Bool b) {
  _Static_assert(1, "x");
  (void)p;
  (void)b;
}
