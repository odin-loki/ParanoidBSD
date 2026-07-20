export module pbsd.port.wave7.hbsd.src.sys.x86.cpufreq.hwpstate_intel;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/cpufreq/hwpstate_intel.c
// void hwpstate_intel_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/cpufreq/hwpstate_intel.c wave=wave7 loc=650
export namespace pbsd::port::wave7::hbsd::src::sys::x86::cpufreq::hwpstate_intel {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::cpufreq::hwpstate_intel
