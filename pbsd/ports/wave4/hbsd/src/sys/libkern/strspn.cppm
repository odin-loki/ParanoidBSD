export module pbsd.port.wave4.hbsd.src.sys.libkern.strspn;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/libkern/strspn.c
// void strspn_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/libkern/strspn.c wave=wave4 loc=71
export namespace pbsd::port::wave4::hbsd::src::sys::libkern::strspn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::libkern::strspn
