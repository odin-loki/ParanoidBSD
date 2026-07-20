module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.cpio;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/cpio.c
export namespace pbsd::stand::cpio {

inline constexpr unsigned kHeaderSize = 110;
[[nodiscard]] inline Status validate_header_size(unsigned n) noexcept {
    return n >= kHeaderSize ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::cpio
