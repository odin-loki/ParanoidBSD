export module pbsd.port.wave5.hbsd.src.sys.dev.exca.exca;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/exca/exca.c
// void exca_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/exca/exca.c wave=wave5 loc=933
export namespace pbsd::port::wave5::hbsd::src::sys::dev::exca::exca {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::exca::exca
