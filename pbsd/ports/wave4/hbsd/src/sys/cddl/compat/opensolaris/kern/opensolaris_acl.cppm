export module pbsd.port.wave4.hbsd.src.sys.cddl.compat.opensolaris.kern.opensolaris_acl;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/cddl/compat/opensolaris/kern/opensolaris_acl.c
// void opensolaris_acl_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/cddl/compat/opensolaris/kern/opensolaris_acl.c wave=wave4 loc=217
export namespace pbsd::port::wave4::hbsd::src::sys::cddl::compat::opensolaris::kern::opensolaris_acl {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::cddl::compat::opensolaris::kern::opensolaris_acl
