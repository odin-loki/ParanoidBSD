module;
#include <cstdint>

export module pbsd.kernel.subr_uio;

import pbsd.core;
import pbsd.kernel.uio;

/// Freestanding port of `kern/subr_uio.c` helpers.
export namespace pbsd::kernel::subr_uio {

[[nodiscard]] inline Status advance_resid(uio::UioStub& u, std::int64_t n) noexcept {
    if (uio::validate_resid(u.resid) != Status::Ok || n < 0 || n > u.resid) {
        return Status::Invalid;
    }
    u.resid -= n;
    u.offset += n;
    return Status::Ok;
}

[[nodiscard]] inline Status check_rw_space(const uio::UioStub& u, std::int64_t need) noexcept {
    if (uio::validate_iovcnt(u.iovcnt) != Status::Ok) {
        return Status::Invalid;
    }
    if (need < 0 || need > u.resid) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_uio
