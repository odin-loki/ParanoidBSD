export module pbsd.port.wave6.hbsd.src.sys.netgraph.ng_tag;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/ng_tag.c
// void ng_tag_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/ng_tag.c wave=wave6 loc=717
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_tag {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::ng_tag
