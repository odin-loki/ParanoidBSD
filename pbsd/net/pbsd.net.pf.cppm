module;
#include <cstdint>

export module pbsd.net.pf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/pfvar.h — PF scrub/state flags (subset).
export namespace pbsd::net::pf {

inline constexpr char kReservedAnchor[] = "_pf";

enum class ScrubFlag : unsigned char {
    Valid = 0x01,
};

enum class StateFlag : unsigned int {
    None       = 0,
    Floating   = 0x01,
    Divert     = 0x02,
    NoSync     = 0x04,
    Pflow      = 0x08,
    Block      = 0x10,
    Table      = 0x20,
    Translation = 0x40,
};

[[nodiscard]] inline Status validate_scrub(unsigned flags) noexcept {
    if (flags != 0 && (flags & static_cast<unsigned>(ScrubFlag::Valid)) == 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::pf
