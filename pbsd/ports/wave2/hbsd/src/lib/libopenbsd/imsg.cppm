export module pbsd.port.wave2.hbsd.src.lib.libopenbsd.imsg;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libopenbsd/imsg.c
// void imsg_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libopenbsd/imsg.c wave=wave2 loc=302
export namespace pbsd::port::wave2::hbsd::src::lib::libopenbsd::imsg {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libopenbsd::imsg
