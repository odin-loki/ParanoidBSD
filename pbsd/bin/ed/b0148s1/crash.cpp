import pbsd.bin.ed.b0148s1;
#include <cstdio>
#include <cstring>
extern "C" {
extern char *ibuf; extern char *ibufp; extern int isbinary; extern int patlock;
extern int oracle_malloc_fail_at; void oracle_reset_batch(void);
char *ref_parse_char_class(char *); char *ref_extract_pattern(int);
regex_t *ref_get_compiled_pattern(void);
}
namespace port = pbsd::bin_ed::b0148s1;
void setup(const char*s){ std::strcpy(ibuf,s); ibufp=ibuf; std::memcpy(port::ibuf,ibuf,65536); port::ibufp=port::ibuf+(ibufp-ibuf); }
int main(int argc,char**argv){
  if(argc<2) return 1;
  oracle_reset_batch(); port::reset_batch();
  if(strcmp(argv[1],"pcc")==0){ char b[512]; memset(b,0,512); strcpy((char*)b+32,argv[2]); ref_parse_char_class((char*)b+32); port::parse_char_class((char*)b+32); }
  if(strcmp(argv[1],"ep")==0){ isbinary=0; port::isbinary=0; setup(argv[2]); ibufp++; port::ibufp++; ref_extract_pattern(argv[2][0]); port::extract_pattern(argv[2][0]); }
  if(strcmp(argv[1],"gcp")==0){ setup(argv[2]); ref_get_compiled_pattern(); port::get_compiled_pattern(); }
  std::printf("ok\n"); return 0;
}
