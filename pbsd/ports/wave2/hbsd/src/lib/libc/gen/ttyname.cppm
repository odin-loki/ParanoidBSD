export module pbsd.port.wave2.hbsd.src.lib.libc.gen.ttyname;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/ttyname.c
// void ttyname_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/ttyname.c wave=wave2 loc=111
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::ttyname {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::ttyname
