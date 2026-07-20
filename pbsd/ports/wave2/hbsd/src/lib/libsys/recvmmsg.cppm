export module pbsd.port.wave2.hbsd.src.lib.libsys.recvmmsg;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libsys/recvmmsg.c
// void recvmmsg_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libsys/recvmmsg.c wave=wave2 loc=95
export namespace pbsd::port::wave2::hbsd::src::lib::libsys::recvmmsg {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libsys::recvmmsg
