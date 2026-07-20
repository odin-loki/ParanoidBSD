export module pbsd.port.wave9.hbsd.src.contrib.llvm_project.openmp.runtime.src.kmp_stub;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/llvm-project/openmp/runtime/src/kmp_stub.cpp
// void kmp_stub_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/llvm-project/openmp/runtime/src/kmp_stub.cpp wave=wave9 loc=469
export namespace pbsd::port::wave9::hbsd::src::contrib::llvm_project::openmp::runtime::src::kmp_stub {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::llvm_project::openmp::runtime::src::kmp_stub
