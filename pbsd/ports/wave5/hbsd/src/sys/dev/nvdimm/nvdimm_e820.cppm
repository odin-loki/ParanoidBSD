export module pbsd.port.wave5.hbsd.src.sys.dev.nvdimm.nvdimm_e820;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/nvdimm/nvdimm_e820.c
// void nvdimm_e820_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/nvdimm/nvdimm_e820.c wave=wave5 loc=385
export namespace pbsd::port::wave5::hbsd::src::sys::dev::nvdimm::nvdimm_e820 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::nvdimm::nvdimm_e820
