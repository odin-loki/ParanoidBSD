module;

#include <cstdint>

export module pbsd.kde.frameworks.kcoreaddons.random;

import pbsd.core;

/// Wave 3 — random string charset logic (from KRandom::randomString).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/randomness/krandom.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::random {

inline constexpr unsigned kCharsetBase = 48;
inline constexpr unsigned kCharsetSize = 62;
inline constexpr unsigned kMaxStringLen = 256;

[[nodiscard]] inline char charset_at(unsigned index) noexcept {
    unsigned r = index % kCharsetSize;
    r += kCharsetBase;
    if (r > 57) {
        r += 7;
    }
    if (r > 90) {
        r += 6;
    }
    return static_cast<char>(r);
}

[[nodiscard]] inline Status random_string(char* out, unsigned length,
                                          std::uint32_t (*next)(void*), void* ctx) noexcept {
    if (out == nullptr) {
        return Status::Invalid;
    }
    if (length == 0 || length >= kMaxStringLen) {
        return length == 0 ? Status::Ok : Status::Invalid;
    }
    if (next == nullptr) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < length; ++i) {
        const std::uint32_t v = next(ctx);
        out[i] = charset_at(static_cast<unsigned>(v));
    }
    out[length] = '\0';
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/randomness/krandom.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::random
