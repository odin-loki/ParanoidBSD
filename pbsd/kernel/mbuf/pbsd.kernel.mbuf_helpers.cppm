module;
#include <cstdint>

export module pbsd.kernel.mbuf_helpers;

import pbsd.core;
import pbsd.kernel.mbuf;

/// PROVENANCE: hbsd/src/sys/kern/uipc_mbuf.c — mbuf chain helpers.
export namespace pbsd::kernel::mbuf_helpers {

[[nodiscard]] inline Status validate_header(mbuf::Flags f) noexcept {
    return mbuf::validate_pkthdr(f);
}

[[nodiscard]] inline bool has_ext(mbuf::Flags f) noexcept {
    return mbuf::has_flag(f, mbuf::Flags::Ext);
}

[[nodiscard]] inline bool is_packet(mbuf::Flags f) noexcept {
    return mbuf::has_flag(f, mbuf::Flags::Pkthdr);
}

[[nodiscard]] inline unsigned min_cluster() noexcept {
    return mbuf::kMlclbytes;
}

} // namespace pbsd::kernel::mbuf_helpers
