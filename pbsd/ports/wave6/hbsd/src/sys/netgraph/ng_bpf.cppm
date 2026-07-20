export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_bpf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_bpf.c
// void ng_bpf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_bpf.c wave=wave6 loc=589
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_bpf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_bpf
