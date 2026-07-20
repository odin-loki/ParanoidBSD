export module pbsd.port.wave2.hbsd.src.lib.libsdp.session;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libsdp/session.c
// void session_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libsdp/session.c wave=wave2 loc=203
export namespace pbsd::port::wave2::hbsd::src::lib::libsdp::session {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libsdp::session
