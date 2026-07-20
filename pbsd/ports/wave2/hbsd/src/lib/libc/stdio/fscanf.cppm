export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.fscanf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/fscanf.c
// void fscanf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/fscanf.c wave=wave2 loc=74
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fscanf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::fscanf
