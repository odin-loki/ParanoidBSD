#include <string.h>

int is_empty(const char *s) {
  return strlen(s) == 0;
}

int is_nonempty(const char *s) {
  return strlen(s) != 0;
}
