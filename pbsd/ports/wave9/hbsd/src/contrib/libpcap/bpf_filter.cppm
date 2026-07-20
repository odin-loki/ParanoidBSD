export module pbsd.port.wave9.hbsd.src.contrib.libpcap.bpf_filter;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/bpf_filter.c
// void bpf_filter_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/bpf_filter.c wave=wave9 loc=544
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::bpf_filter {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::bpf_filter
