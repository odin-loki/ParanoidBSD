export module pbsd.port.wave9.hbsd.src.contrib.llvm_project.clang.lib.basic.nosanitizelist;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/llvm-project/clang/lib/Basic/NoSanitizeList.cpp
// void NoSanitizeList_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/llvm-project/clang/lib/Basic/NoSanitizeList.cpp wave=wave9 loc=74
export namespace pbsd::port::wave9::hbsd::src::contrib::llvm_project::clang::lib::basic::nosanitizelist {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::llvm_project::clang::lib::basic::nosanitizelist
