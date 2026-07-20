export module pbsd.port.wave2.hbsd.src.lib.libc.gen.getttyent;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/getttyent.c
// void getttyent_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/getttyent.c wave=wave2 loc=312
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getttyent {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::getttyent
