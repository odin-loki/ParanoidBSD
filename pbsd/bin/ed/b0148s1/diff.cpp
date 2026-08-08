#include <cstdio>
#include <cstring>
import pbsd.bin.ed.b0148s1;
extern "C" { void oracle_reset_batch(void); char *ref_parse_char_class(char *); }
namespace port = pbsd::bin_ed::b0148s1;
int main(void){
  const char *edge[] = {"", "]", "a]", "[a]", "[]", "[^]", "[^a]", "[[:alpha:]]", "[\n", "[a\n", "[.[.]]", "[:.:]", "[=.=]", "[^]]]", "[\xff]", "[\x80]", "x", "[[", "[[:", "[=x\n", "[.x\n", "[::\n", "[a-z]", "[\\]]", "[^].]"};
  for (auto in : edge) {
    oracle_reset_batch(); port::reset_batch();
    char rb[512], pb[512];
    memset(rb,0,512); memset(pb,0,512);
    strcpy(rb+32,in); strcpy(pb+32,in);
    char *r = ref_parse_char_class(rb+32);
    char *p = port::parse_char_class(pb+32);
    long ro = r?(r-rb-32):-1, po = p?(p-pb-32):-1;
    if (ro!=po || memcmp(rb,pb,512)) printf("FAIL %s ro=%ld po=%ld\n", in, ro, po);
  }
  printf("done\n");
}
