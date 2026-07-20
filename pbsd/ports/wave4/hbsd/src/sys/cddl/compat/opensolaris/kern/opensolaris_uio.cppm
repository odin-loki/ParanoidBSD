export module pbsd.port.wave4.hbsd.src.sys.cddl.compat.opensolaris.kern.opensolaris_uio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/cddl/compat/opensolaris/kern/opensolaris_uio.c
// void opensolaris_uio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/cddl/compat/opensolaris/kern/opensolaris_uio.c wave=wave4 loc=91
export namespace pbsd::port::wave4::hbsd::src::sys::cddl::compat::opensolaris::kern::opensolaris_uio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::cddl::compat::opensolaris::kern::opensolaris_uio
