#include <stdlib.h>
register int x;
void *p = NULL;
char *msg = "hi";
typedef unsigned int u32;
int main(void) {
  int *q = malloc(4);
  free(q);
  return 0;
}
