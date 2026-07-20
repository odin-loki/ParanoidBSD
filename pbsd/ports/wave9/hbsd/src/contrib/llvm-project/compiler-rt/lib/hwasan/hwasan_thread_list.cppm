export module pbsd.port.wave9.hbsd.src.contrib.llvm_project.compiler_rt.lib.hwasan.hwasan_thread_list;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/llvm-project/compiler-rt/lib/hwasan/hwasan_thread_list.cpp
// void hwasan_thread_list_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/llvm-project/compiler-rt/lib/hwasan/hwasan_thread_list.cpp wave=wave9 loc=29
export namespace pbsd::port::wave9::hbsd::src::contrib::llvm_project::compiler_rt::lib::hwasan::hwasan_thread_list {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::llvm_project::compiler_rt::lib::hwasan::hwasan_thread_list
