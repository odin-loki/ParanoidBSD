module;

#include <cstdlib>
#include <cstring>

export module pbsd.userland.util.paths;

/// getlocalbase(3) from hbsd/src/lib/libutil/getlocalbase.c
export namespace pbsd::userland::util {

inline constexpr const char* kDefaultLocalBase = "/usr/local";
inline constexpr const char* kIllegalPrefix    = "/dev/null/";

[[nodiscard]] inline const char* resolve_localbase(const char* env_value,
                                                   const char* sysctl_value,
                                                   bool sysctl_ok) noexcept {
    if (env_value != nullptr && env_value[0] != '\0') {
        return env_value;
    }
    if (sysctl_ok && sysctl_value != nullptr && sysctl_value[0] != '\0') {
        return sysctl_value;
    }
    if (sysctl_ok) {
        return kDefaultLocalBase;
    }
    return kIllegalPrefix;
}

#ifndef _WIN32
[[nodiscard]] inline const char* getlocalbase() noexcept {
    static const char* cached = nullptr;
    if (cached != nullptr) {
        return cached;
    }
    const char* env = std::getenv("LOCALBASE");
    cached = resolve_localbase(env, kDefaultLocalBase, true);
    return cached;
}
#else
[[nodiscard]] inline const char* getlocalbase() noexcept {
    return kDefaultLocalBase;
}
#endif

} // namespace pbsd::userland::util
