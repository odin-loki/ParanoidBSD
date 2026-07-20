export module pbsd.port.wave9.hbsd.src.contrib.ofed.libcxgb4.dev;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ofed/libcxgb4/dev.c
// void dev_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ofed/libcxgb4/dev.c wave=wave9 loc=584
export namespace pbsd::port::wave9::hbsd::src::contrib::ofed::libcxgb4::dev {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ofed::libcxgb4::dev
