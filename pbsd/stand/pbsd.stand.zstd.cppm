module;
#include <cstddef>

export module pbsd.stand.zstd;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/zstd.c
export namespace pbsd::stand::zstd {

inline constexpr unsigned kWindowLogMax = 27;
inline constexpr unsigned kFrameHeaderSize = 4;

[[nodiscard]] inline Status validate_window_log(unsigned log) noexcept {
    return log <= kWindowLogMax ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::zstd
