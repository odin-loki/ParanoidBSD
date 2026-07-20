export module pbsd.port.wave2.hbsd.src.lib.libc.include.block_abi;

module;
// Header bridge — replace #include of hbsd/src/lib/libc/include/block_abi.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/include/block_abi.h wave=wave2 loc=61
export namespace pbsd::port::wave2::hbsd::src::lib::libc::include::block_abi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::include::block_abi
