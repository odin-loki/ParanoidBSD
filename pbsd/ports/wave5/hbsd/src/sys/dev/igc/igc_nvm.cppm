export module pbsd.port.wave5.hbsd.src.sys.dev.igc.igc_nvm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/igc/igc_nvm.c
// void igc_nvm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/igc/igc_nvm.c wave=wave5 loc=800
export namespace pbsd::port::wave5::hbsd::src::sys::dev::igc::igc_nvm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::igc::igc_nvm
