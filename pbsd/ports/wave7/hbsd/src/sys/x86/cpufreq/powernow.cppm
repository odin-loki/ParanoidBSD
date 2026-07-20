export module pbsd.port.wave7.hbsd.src.sys.x86.cpufreq.powernow;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/cpufreq/powernow.c
// void powernow_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/cpufreq/powernow.c wave=wave7 loc=967
export namespace pbsd::port::wave7::hbsd::src::sys::x86::cpufreq::powernow {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::cpufreq::powernow
