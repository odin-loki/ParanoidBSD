#include <cstdio>
#include <cstring>
import pbsd.lib.libc.gen.b0255;
extern "C" char *ref_basename(char *path);
namespace P = pbsd::lib_libc_gen::b0255;
int main() {
    unsigned char buf1[320], buf2[320];
    memset(buf1, 0x7f, sizeof buf1);
    memset(buf2, 0x7f, sizeof buf2);
    char *p1 = (char*)(buf1+32);
    char *p2 = (char*)(buf2+32);
    p1[0]='a'; p1[1]=0;
    p2[0]='a'; p2[1]=0;
    char *a = P::basename(p1);
    char *b = ref_basename(p2);
    printf("off a=%td b=%td\n", a-p1, b-p2);
    printf("buf1[32..40]: %02x %02x %02x %02x\n", buf1[32],buf1[33],buf1[34],buf1[35]);
    printf("buf2[32..40]: %02x %02x %02x %02x\n", buf2[32],buf2[33],buf2[34],buf2[35]);
    printf("memcmp all=%d\n", memcmp(buf1,buf2,sizeof buf1));
    return 0;
}
