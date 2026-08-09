#include <cstdio>
#include <cstring>
import pbsd.lib.libc.gen.b0255;
namespace P = pbsd::lib_libc_gen::b0255;
extern "C" char *ref_basename(char *path);
int main(void) {
    unsigned char b1[320], b2[320];
    std::memset(b1, 0x7f, sizeof b1);
    std::memset(b2, 0x7f, sizeof b2);
    char *p1 = (char*)(b1+32);
    char *p2 = (char*)(b2+32);
    std::strcpy(p1, "foo/bar");
    std::strcpy(p2, "foo/bar");
    char *a = P::basename(p1);
    char *b = ref_basename(p2);
    std::printf("a=%s off=%td\n", a, a-p1);
    std::printf("b=%s off=%td\n", b, b-p2);
    std::printf("memcmp=%d\n", std::memcmp(b1,b2,sizeof b1));
    for (int i=32;i<50;i++) std::printf("%02x ", b1[i]); std::printf("\n");
    for (int i=32;i<50;i++) std::printf("%02x ", b2[i]); std::printf("\n");
    return 0;
}
