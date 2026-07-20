export module pbsd.port.wave9.hbsd.src.contrib.libpcap.bpf_dump;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/bpf_dump.c
// void bpf_dump_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/bpf_dump.c wave=wave9 loc=60
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::bpf_dump {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::bpf_dump
