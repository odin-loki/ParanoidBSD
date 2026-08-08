#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
extern int ref_fprintf(FILE *, const char *, ...);
int main() {
  FILE *fp = tmpfile();
  printf("%d\n", ref_fprintf(fp, "%d", 42));
  return 0;
}
