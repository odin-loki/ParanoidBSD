export module pbsd.port.wave5.hbsd.src.sys.dev.mthca.mthca_allocator;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mthca/mthca_allocator.c
// void mthca_allocator_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mthca/mthca_allocator.c wave=wave5 loc=301
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_allocator {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mthca::mthca_allocator
