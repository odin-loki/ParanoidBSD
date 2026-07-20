export module pbsd.port.wave2.hbsd.src.lib.libnvmf.nvmf_tcp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libnvmf/nvmf_tcp.c
// void nvmf_tcp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libnvmf/nvmf_tcp.c wave=wave2 loc=1501
export namespace pbsd::port::wave2::hbsd::src::lib::libnvmf::nvmf_tcp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libnvmf::nvmf_tcp
