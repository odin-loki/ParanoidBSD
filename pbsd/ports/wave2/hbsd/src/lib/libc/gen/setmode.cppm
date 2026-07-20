export module pbsd.port.wave2.hbsd.src.lib.libc.gen.setmode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/setmode.c
// void setmode_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/setmode.c wave=wave2 loc=482
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::setmode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::setmode
