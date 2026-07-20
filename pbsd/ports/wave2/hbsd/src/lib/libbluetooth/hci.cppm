export module pbsd.port.wave2.hbsd.src.lib.libbluetooth.hci;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libbluetooth/hci.c
// void hci_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libbluetooth/hci.c wave=wave2 loc=813
export namespace pbsd::port::wave2::hbsd::src::lib::libbluetooth::hci {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libbluetooth::hci
