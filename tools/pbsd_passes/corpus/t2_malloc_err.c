#include <stdlib.h>
#include <err.h>
int main(void) {
  char *buf;
  if ((buf = malloc(64)) == NULL)
    err(1, "malloc");
  buf[0] = 'x';
  free(buf);
  return 0;
}
