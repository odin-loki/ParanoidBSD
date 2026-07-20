export module pbsd.port.wave2.hbsd.src.libexec.rpc_rusersd.rusers_proc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/rpc.rusersd/rusers_proc.c
// void rusers_proc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/rpc.rusersd/rusers_proc.c wave=wave2 loc=329
export namespace pbsd::port::wave2::hbsd::src::libexec::rpc_rusersd::rusers_proc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::rpc_rusersd::rusers_proc
