module;
#include <cstdint>

export module pbsd.bifrost.rtc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/io/rtc.c
export namespace pbsd::bifrost::rtc {

inline constexpr unsigned kPortIndex = 0x70;
inline constexpr unsigned kPortData = 0x71;

[[nodiscard]] inline Status validate_register(unsigned reg) noexcept {
    return reg <= 0x0F ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::rtc
