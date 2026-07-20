export module pbsd.port.wave5.hbsd.src.sys.dev.e1000.e1000_nvm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/e1000/e1000_nvm.c
// void e1000_nvm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/e1000/e1000_nvm.c wave=wave5 loc=1390
export namespace pbsd::port::wave5::hbsd::src::sys::dev::e1000::e1000_nvm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::e1000::e1000_nvm
