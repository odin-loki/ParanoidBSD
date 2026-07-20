export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_gif;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_gif.c
// void ng_gif_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_gif.c wave=wave6 loc=594
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_gif {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_gif
