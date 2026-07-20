export module pbsd.port.wave5.hbsd.src.sys.dev.mailbox.arm.arm_doorbell;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mailbox/arm/arm_doorbell.c
// void arm_doorbell_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mailbox/arm/arm_doorbell.c wave=wave5 loc=344
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mailbox::arm::arm_doorbell {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mailbox::arm::arm_doorbell
