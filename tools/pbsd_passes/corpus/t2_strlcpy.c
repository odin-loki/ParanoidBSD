#include <string.h>

void copy_path(char pathbuf[256], const char *src) {
  strlcpy(pathbuf, src, sizeof(pathbuf));
}
