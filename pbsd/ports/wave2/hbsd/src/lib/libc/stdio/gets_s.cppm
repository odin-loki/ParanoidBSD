export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.gets_s;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/gets_s.c
// void gets_s_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/gets_s.c wave=wave2 loc=100
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::gets_s {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::gets_s
