export module pbsd.port.wave2.hbsd.src.lib.libnvmf.nvmf_transport;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libnvmf/nvmf_transport.c
// void nvmf_transport_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libnvmf/nvmf_transport.c wave=wave2 loc=307
export namespace pbsd::port::wave2::hbsd::src::lib::libnvmf::nvmf_transport {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libnvmf::nvmf_transport
