export module pbsd.port.wave4.hbsd.src.sys.contrib.rdma.krping.krping;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/rdma/krping/krping.c
// void krping_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/rdma/krping/krping.c wave=wave4 loc=2211
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::rdma::krping::krping {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::rdma::krping::krping
