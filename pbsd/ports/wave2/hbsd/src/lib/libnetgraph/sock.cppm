export module pbsd.port.wave2.hbsd.src.lib.libnetgraph.sock;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libnetgraph/sock.c
// void sock_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libnetgraph/sock.c wave=wave2 loc=301
export namespace pbsd::port::wave2::hbsd::src::lib::libnetgraph::sock {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libnetgraph::sock
