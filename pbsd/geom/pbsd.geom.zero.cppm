module;
#include <cstdint>

export module pbsd.geom.zero;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/zero/g_zero.c — GEOM ZERO null provider.
export namespace pbsd::geom::zero {

inline constexpr char kClassName[] = "ZERO";

struct Config {
    bool clear_on_read{true};
    unsigned char fill_byte{};
};

[[nodiscard]] inline Status validate_fill_byte(int byte) noexcept {
    if (byte < 0 || byte > 255) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_config(Config const& cfg) noexcept {
    return validate_fill_byte(static_cast<int>(cfg.fill_byte));
}

/// Fill a buffer of `len` bytes with the configured fill byte.
[[nodiscard]] inline Status fill_buffer(unsigned char* buf, unsigned len,
                                        Config const& cfg) noexcept {
    if (buf == nullptr || len == 0) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < len; ++i) {
        buf[i] = cfg.fill_byte;
    }
    return Status::Ok;
}

struct Stats {
    unsigned long long reads{};
    unsigned long long read_bytes{};
};

} // namespace pbsd::geom::zero
