import pbsd.lib.libc.posix1e.b0320;
#include <cstdio>
int main() {
  std::printf("cxx=%zu\n", sizeof(pbsd::lib_libc_posix1e::b0320::acl_t_struct));
  return 0;
}
