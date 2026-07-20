module;
#include <cstdint>

export module pbsd.bifrost.vga;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/io/vga.c
export namespace pbsd::bifrost::vga {

inline constexpr unsigned kFbSize = 0xA0000;
inline constexpr unsigned kMode13Width = 320;
inline constexpr unsigned kMode13Height = 200;

[[nodiscard]] inline Status validate_resolution(unsigned w, unsigned h) noexcept {
    if (w == 0 || h == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::vga
