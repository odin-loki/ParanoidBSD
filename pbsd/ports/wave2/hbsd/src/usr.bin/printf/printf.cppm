export module pbsd.port.wave2.hbsd.src.usr_bin.printf.printf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/printf/printf.c
// void printf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/printf/printf.c wave=wave2 loc=672
export namespace pbsd::port::wave2::hbsd::src::usr_bin::printf::printf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::printf::printf
