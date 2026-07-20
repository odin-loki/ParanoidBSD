module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.crc;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/crc32_libkern.c
export namespace pbsd::stand::crc {

inline constexpr unsigned kPolynomial = 0xEDB88320u;
[[nodiscard]] inline unsigned step(unsigned crc, unsigned byte) noexcept {
    return (crc >> 8) ^ byte;
}

} // namespace pbsd::stand::crc
