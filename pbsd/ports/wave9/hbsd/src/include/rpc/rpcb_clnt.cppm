export module pbsd.port.wave9.hbsd.src.include.rpc.rpcb_clnt;

module;
// Header bridge — replace #include of hbsd/src/include/rpc/rpcb_clnt.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/include/rpc/rpcb_clnt.h wave=wave9 loc=84
export namespace pbsd::port::wave9::hbsd::src::include::rpc::rpcb_clnt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::include::rpc::rpcb_clnt
