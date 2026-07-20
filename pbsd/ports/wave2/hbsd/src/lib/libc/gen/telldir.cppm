export module pbsd.port.wave2.hbsd.src.lib.libc.gen.telldir;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/telldir.c
// void telldir_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/telldir.c wave=wave2 loc=204
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::telldir {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::telldir
