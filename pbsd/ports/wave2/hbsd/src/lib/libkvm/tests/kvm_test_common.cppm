export module pbsd.port.wave2.hbsd.src.lib.libkvm.tests.kvm_test_common;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libkvm/tests/kvm_test_common.c
// void kvm_test_common_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libkvm/tests/kvm_test_common.c wave=wave2 loc=45
export namespace pbsd::port::wave2::hbsd::src::lib::libkvm::tests::kvm_test_common {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libkvm::tests::kvm_test_common
