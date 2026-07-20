module;

#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.hash.crc32;

/// CRC-32 from hbsd/src/lib/libc/net/crc32.c (polynomial 0xEDB88320)
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::uint32_t crc32(std::uint32_t crc, const void* buf,
                                         std::size_t len) noexcept {
    const auto* p = static_cast<const unsigned char*>(buf);
    crc = ~crc;
    for (std::size_t i = 0; i < len; ++i) {
        crc ^= p[i];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ (0xEDB88320u & (~(crc & 1u) + 1u));
        }
    }
    return ~crc;
}

} // namespace pbsd::userland::libc
