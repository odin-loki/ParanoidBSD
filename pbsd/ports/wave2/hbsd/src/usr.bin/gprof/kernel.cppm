export module pbsd.port.wave2.hbsd.src.usr_bin.gprof.kernel;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/gprof/kernel.c
// void kernel_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/gprof/kernel.c wave=wave2 loc=62
export namespace pbsd::port::wave2::hbsd::src::usr_bin::gprof::kernel {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::gprof::kernel
