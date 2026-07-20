export module pbsd.port.wave2.hbsd.src.lib.libc.gen.scandir;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/scandir.c
// void scandir_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/scandir.c wave=wave2 loc=252
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::scandir {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::scandir
