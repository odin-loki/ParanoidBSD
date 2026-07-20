export module pbsd.port.wave6.hbsd.src.sys.net.bpf_jitter;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/bpf_jitter.c
// void bpf_jitter_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/bpf_jitter.c wave=wave6 loc=120
export namespace pbsd::port::wave6::hbsd::src::sys::net::bpf_jitter {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::bpf_jitter
