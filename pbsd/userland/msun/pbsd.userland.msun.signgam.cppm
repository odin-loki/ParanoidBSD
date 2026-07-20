module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.signgam;

/// signgam from hbsd/src/lib/msun/src/s_signgam.c
export namespace pbsd::userland::msun {

inline int signgam_val() noexcept { return 1; }

} // namespace pbsd::userland::msun
