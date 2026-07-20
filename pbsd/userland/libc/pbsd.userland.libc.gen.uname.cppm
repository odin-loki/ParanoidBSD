module;

#include <cstddef>

export module pbsd.userland.libc.gen.uname;

/// uname struct from hbsd/src/lib/libc/gen/uname.c
export namespace pbsd::userland::libc {

inline constexpr std::size_t kUnameSysnameLen = 256;
inline constexpr std::size_t kUnameNodenameLen = 256;
inline constexpr std::size_t kUnameReleaseLen = 256;
inline constexpr std::size_t kUnameVersionLen = 256;
inline constexpr std::size_t kUnameMachineLen = 256;

struct UtsName {
    char sysname[kUnameSysnameLen];
    char nodename[kUnameNodenameLen];
    char release[kUnameReleaseLen];
    char version[kUnameVersionLen];
    char machine[kUnameMachineLen];
};

inline void fill_default_uname(UtsName& out) noexcept {
    const char sys[] = "PBSD";
    const char rel[] = "0.1";
    const char ver[] = "PBSD Userland";
    const char mach[] = "generic";
    for (std::size_t i = 0; i < kUnameSysnameLen; ++i) {
        out.sysname[i] = i < sizeof(sys) - 1 ? sys[i] : '\0';
    }
    for (std::size_t i = 0; i < kUnameReleaseLen; ++i) {
        out.release[i] = i < sizeof(rel) - 1 ? rel[i] : '\0';
    }
    for (std::size_t i = 0; i < kUnameVersionLen; ++i) {
        out.version[i] = i < sizeof(ver) - 1 ? ver[i] : '\0';
    }
    for (std::size_t i = 0; i < kUnameMachineLen; ++i) {
        out.machine[i] = i < sizeof(mach) - 1 ? mach[i] : '\0';
    }
    out.nodename[0] = '\0';
}

} // namespace pbsd::userland::libc
