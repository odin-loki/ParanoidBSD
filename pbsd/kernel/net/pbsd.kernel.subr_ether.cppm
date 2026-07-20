module;
#include <cstdint>

export module pbsd.kernel.subr_ether;

import pbsd.core;
import pbsd.kernel.ether;

/// PROVENANCE: hbsd/src/sys/net/if_ethersubr.c — Ethernet subr helpers.
export namespace pbsd::kernel::subr_ether {

[[nodiscard]] inline Status validate(ether::MacAddr const& ea) noexcept {
    return ether::validate_addr(ea);
}

[[nodiscard]] inline bool is_bcast(ether::MacAddr const& ea) noexcept {
    return ether::is_broadcast(ea);
}

[[nodiscard]] inline unsigned frame_size(unsigned payload) noexcept {
    return ether::frame_len(payload);
}

} // namespace pbsd::kernel::subr_ether
