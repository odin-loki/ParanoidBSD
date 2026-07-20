export module pbsd.port.wave7.hbsd.src.sys.amd64.vmm.io.vioapic;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/vmm/io/vioapic.c
// void vioapic_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/vmm/io/vioapic.c wave=wave7 loc=546
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::io::vioapic {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::io::vioapic
