export module pbsd.port.wave2.hbsd.src.lib.libthr.arch.amd64.include.pthread_md;

module;
// Header bridge — replace #include of hbsd/src/lib/libthr/arch/amd64/include/pthread_md.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/arch/amd64/include/pthread_md.h wave=wave2 loc=62
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::arch::amd64::include::pthread_md {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::arch::amd64::include::pthread_md
