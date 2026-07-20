export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_rseq;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_rseq.c
// void linux_rseq_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_rseq.c wave=wave4 loc=84
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_rseq {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_rseq
