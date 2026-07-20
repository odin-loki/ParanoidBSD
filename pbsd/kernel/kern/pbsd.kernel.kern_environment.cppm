module;
#include <cstdint>

export module pbsd.kernel.kern_environment;

export import pbsd.core;

/// Freestanding port of `kern/kern_environment.c` — environment helpers.
export namespace pbsd::kernel::kern_environment {

inline constexpr unsigned kEnvMax = 4096;
inline constexpr unsigned kEnvHashBuckets = 256;

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len >= kEnvMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned hash_bucket(const char* name, unsigned len) noexcept {
    unsigned h = 0;
    for (unsigned i = 0; i < len && name[i] != '\0'; ++i) {
        h = h * 33 + static_cast<unsigned char>(name[i]);
    }
    return h % kEnvHashBuckets;
}

} // namespace pbsd::kernel::kern_environment
