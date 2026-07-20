export module pbsd.port.wave6.hbsd.src.sys.net.bpf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/bpf.c
// void bpf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/bpf.c wave=wave6 loc=3260
export namespace pbsd::port::wave6::hbsd::src::sys::net::bpf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::bpf
