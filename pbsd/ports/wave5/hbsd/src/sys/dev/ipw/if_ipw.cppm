export module pbsd.port.wave5.hbsd.src.sys.dev.ipw.if_ipw;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ipw/if_ipw.c
// void if_ipw_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ipw/if_ipw.c wave=wave5 loc=2677
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ipw::if_ipw {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ipw::if_ipw
