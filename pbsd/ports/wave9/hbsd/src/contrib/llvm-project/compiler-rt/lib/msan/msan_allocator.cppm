export module pbsd.port.wave9.hbsd.src.contrib.llvm_project.compiler_rt.lib.msan.msan_allocator;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/llvm-project/compiler-rt/lib/msan/msan_allocator.cpp
// void msan_allocator_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/llvm-project/compiler-rt/lib/msan/msan_allocator.cpp wave=wave9 loc=453
export namespace pbsd::port::wave9::hbsd::src::contrib::llvm_project::compiler_rt::lib::msan::msan_allocator {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::llvm_project::compiler_rt::lib::msan::msan_allocator
