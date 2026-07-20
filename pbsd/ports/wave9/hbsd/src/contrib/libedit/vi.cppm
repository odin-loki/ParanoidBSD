export module pbsd.port.wave9.hbsd.src.contrib.libedit.vi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libedit/vi.c
// void vi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libedit/vi.c wave=wave9 loc=1160
export namespace pbsd::port::wave9::hbsd::src::contrib::libedit::vi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libedit::vi
