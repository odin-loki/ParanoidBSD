module;

#include <cstdint>

export module pbsd.userland.libc.gen.ftok;

/// ftok key composition from hbsd/src/lib/libc/gen/ftok.c (logic-only; stat in hosted layer)
export namespace pbsd::userland::libc {

using key_t = std::int32_t;

[[nodiscard]] inline key_t ftok_from_stat(std::uint64_t dev, std::uint64_t ino, int id) noexcept {
    return static_cast<key_t>((static_cast<unsigned int>(id) << 24) |
                              ((static_cast<unsigned int>(dev) & 0xffU) << 16) |
                              (static_cast<unsigned int>(ino) & 0xffffU));
}

} // namespace pbsd::userland::libc
