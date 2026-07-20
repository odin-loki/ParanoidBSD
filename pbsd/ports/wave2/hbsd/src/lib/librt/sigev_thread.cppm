export module pbsd.port.wave2.hbsd.src.lib.librt.sigev_thread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/librt/sigev_thread.c
// void sigev_thread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/librt/sigev_thread.c wave=wave2 loc=466
export namespace pbsd::port::wave2::hbsd::src::lib::librt::sigev_thread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::librt::sigev_thread
