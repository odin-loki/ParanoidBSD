export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.vscanf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/vscanf.c
// void vscanf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/vscanf.c wave=wave2 loc=62
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::vscanf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::vscanf
