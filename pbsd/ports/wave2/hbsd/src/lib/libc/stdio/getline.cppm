export module pbsd.port.wave2.hbsd.src.lib.libc.stdio.getline;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdio/getline.c
// void getline_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdio/getline.c wave=wave2 loc=37
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::getline {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdio::getline
