export module pbsd.port.wave4.hbsd.src.sys.contrib.dev.iwlwifi.mvm.offloading;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/dev/iwlwifi/mvm/offloading.c
// void offloading_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/dev/iwlwifi/mvm/offloading.c wave=wave4 loc=214
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::iwlwifi::mvm::offloading {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::dev::iwlwifi::mvm::offloading
