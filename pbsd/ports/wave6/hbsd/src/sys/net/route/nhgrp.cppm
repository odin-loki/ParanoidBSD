export module pbsd.port.wave6.hbsd.src.sys.net.route.nhgrp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/net/route/nhgrp.c
// void nhgrp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/net/route/nhgrp.c wave=wave6 loc=345
export namespace pbsd::port::wave6::hbsd::src::sys::net::route::nhgrp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::net::route::nhgrp
