export module pbsd.port.wave5.hbsd.src.sys.dev.nvmf.host.nvmf_sim;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/nvmf/host/nvmf_sim.c
// void nvmf_sim_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/nvmf/host/nvmf_sim.c wave=wave5 loc=349
export namespace pbsd::port::wave5::hbsd::src::sys::dev::nvmf::host::nvmf_sim {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::nvmf::host::nvmf_sim
