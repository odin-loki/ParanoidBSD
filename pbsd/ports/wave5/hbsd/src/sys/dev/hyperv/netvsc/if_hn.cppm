export module pbsd.port.wave5.hbsd.src.sys.dev.hyperv.netvsc.if_hn;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/hyperv/netvsc/if_hn.c
// void if_hn_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/hyperv/netvsc/if_hn.c wave=wave5 loc=7680
export namespace pbsd::port::wave5::hbsd::src::sys::dev::hyperv::netvsc::if_hn {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::hyperv::netvsc::if_hn
