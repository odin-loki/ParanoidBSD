export module pbsd.port.wave2.hbsd.src.lib.libc.gen.opendir;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/opendir.c
// void opendir_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/opendir.c wave=wave2 loc=46
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::opendir {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::opendir
