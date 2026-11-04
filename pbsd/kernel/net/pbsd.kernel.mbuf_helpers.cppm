module;
#include <cstdint>

export module pbsd.kernel.mbuf_helpers;

import pbsd.core;
import pbsd.kernel.mbuf;

/// Freestanding port of `sys/mbuf.h`.
export namespace pbsd::kernel::mbuf_helpers {

[[nodiscard]] inline Status require_pkthdr(mbuf::Flags f) noexcept {
    if (!mbuf::has_flag(f, mbuf::Flags::Pkthdr)) {
        return Status::Invalid;
    }
    return mbuf::validate_pkthdr(f);
}

[[nodiscard]] inline bool has_ext(mbuf::Flags f) noexcept {
    return mbuf::has_flag(f, mbuf::Flags::Ext);
}

} // namespace pbsd::kernel::mbuf_helpers
