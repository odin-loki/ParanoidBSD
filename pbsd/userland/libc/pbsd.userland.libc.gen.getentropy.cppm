module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getentropy;

export import pbsd.core;

/// getentropy from hbsd/src/lib/libc/gen/getentropy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getentropy_validate(std::size_t len) noexcept {
    if (len == 0 || len > 256) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline void getentropy_fill(std::uint8_t* buf, std::size_t len,
                                            std::uint8_t seed) noexcept {
    for (std::size_t i = 0; i < len; ++i) {
        buf[i] = static_cast<std::uint8_t>(seed + static_cast<std::uint8_t>(i));
    }
}

} // namespace pbsd::userland::libc
