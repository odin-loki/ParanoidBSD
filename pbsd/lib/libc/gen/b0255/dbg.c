#include <stdio.h>
#include <string.h>
extern char *ref_basename(char *path);
int main(void) {
    unsigned char b1[320], b2[320];
    memset(b1, 0x7f, sizeof b1);
    memset(b2, 0x7f, sizeof b2);
    char *p1 = (char*)(b1+32);
    char *p2 = (char*)(b2+32);
    strcpy(p1, "foo/bar");
    strcpy(p2, "foo/bar");
    char *r1 = ref_basename(p1);
    char *r2 = ref_basename(p2);
    printf("r1=%s off=%td\n", r1, r1-p1);
    printf("r2=%s off=%td\n", r2, r2-p2);
    printf("memcmp=%d\n", memcmp(b1,b2,sizeof b1));
    for (int i=32;i<50;i++) printf("%02x ", b1[i]); printf("\n");
    for (int i=32;i<50;i++) printf("%02x ", b2[i]); printf("\n");
    return 0;
}
