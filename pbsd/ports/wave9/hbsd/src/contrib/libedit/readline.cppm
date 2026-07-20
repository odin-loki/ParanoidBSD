export module pbsd.port.wave9.hbsd.src.contrib.libedit.readline;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libedit/readline.c
// void readline_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libedit/readline.c wave=wave9 loc=2625
export namespace pbsd::port::wave9::hbsd::src::contrib::libedit::readline {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libedit::readline
