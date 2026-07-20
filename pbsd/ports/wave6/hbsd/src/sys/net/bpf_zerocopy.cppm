export module pbsd.port.wave6.hbsd.src.sys.net.bpf_zerocopy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/bpf_zerocopy.c
// void bpf_zerocopy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/bpf_zerocopy.c wave=wave6 loc=591
export namespace pbsd::port::wave6::hbsd::src::sys::net::bpf_zerocopy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::bpf_zerocopy
