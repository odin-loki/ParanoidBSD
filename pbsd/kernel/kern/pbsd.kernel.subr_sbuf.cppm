module;
#include <cstdint>

export module pbsd.kernel.subr_sbuf;

import pbsd.core;
import pbsd.kernel.sbuf;

/// PROVENANCE: hbsd/src/sys/kern/subr_sbuf.c — sbuf(9) subr helpers.
export namespace pbsd::kernel::subr_sbuf {

[[nodiscard]] inline Status init_dynamic(sbuf::Softc& sb, unsigned cap) noexcept {
    if (sbuf::validate_capacity(cap) != Status::Ok) {
        return Status::Invalid;
    }
    sb.capacity = cap;
    sb.length = 0;
    sb.flags = sbuf::Flag::Dynamic;
    return Status::Ok;
}

[[nodiscard]] inline Status putc_len(sbuf::Softc& sb, unsigned n) noexcept {
    return sbuf::append_len(sb, n);
}

[[nodiscard]] inline Status finalize(sbuf::Softc& sb) noexcept {
    return sbuf::finish(sb);
}

[[nodiscard]] inline bool overflowed(sbuf::Softc const& sb) noexcept {
    return sbuf::has(sb.flags, sbuf::Flag::Overflow);
}

} // namespace pbsd::kernel::subr_sbuf
