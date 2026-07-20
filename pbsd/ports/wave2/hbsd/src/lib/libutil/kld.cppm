export module pbsd.port.wave2.hbsd.src.lib.libutil.kld;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libutil/kld.c
// void kld_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libutil/kld.c wave=wave2 loc=76
export namespace pbsd::port::wave2::hbsd::src::lib::libutil::kld {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libutil::kld
