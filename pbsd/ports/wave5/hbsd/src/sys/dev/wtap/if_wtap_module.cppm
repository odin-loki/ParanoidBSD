export module pbsd.port.wave5.hbsd.src.sys.dev.wtap.if_wtap_module;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/wtap/if_wtap_module.c
// void if_wtap_module_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/wtap/if_wtap_module.c wave=wave5 loc=149
export namespace pbsd::port::wave5::hbsd::src::sys::dev::wtap::if_wtap_module {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::wtap::if_wtap_module
