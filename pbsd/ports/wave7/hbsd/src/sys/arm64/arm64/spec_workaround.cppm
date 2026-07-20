export module pbsd.port.wave7.hbsd.src.sys.arm64.arm64.spec_workaround;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/arm64/spec_workaround.c
// void spec_workaround_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/arm64/spec_workaround.c wave=wave7 loc=166
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::spec_workaround {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::spec_workaround
