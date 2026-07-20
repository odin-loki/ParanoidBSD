export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.vfscanf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/vfscanf.c
// void vfscanf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/vfscanf.c wave=wave2 loc=1146
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::vfscanf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::vfscanf
