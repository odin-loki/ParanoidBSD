module;
#include <cstdint>

export module pbsd.bifrost.svm;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/amd/vmcb.h — SVM intercept bitmap.
export namespace pbsd::bifrost::svm {

inline constexpr unsigned kInterceptCr = 0;
inline constexpr unsigned kInterceptDr = 1;
inline constexpr unsigned kInterceptExc = 2;
inline constexpr unsigned kInterceptCtrl1 = 3;
inline constexpr unsigned kInterceptCtrl2 = 4;

inline constexpr std::uint32_t kIntcptIntr = 1u << 0;
inline constexpr std::uint32_t kIntcptNmi = 1u << 1;
inline constexpr std::uint32_t kIntcptCpuid = 1u << 18;
inline constexpr std::uint32_t kIntcptHlt = 1u << 24;
inline constexpr std::uint32_t kIntcptInvlpg = 1u << 25;
inline constexpr std::uint32_t kIntcptIo = 1u << 27;
inline constexpr std::uint32_t kIntcptMsr = 1u << 28;

inline constexpr std::uint32_t kIntcpt2Vmrun = 1u << 0;
inline constexpr std::uint32_t kIntcpt2Vmcall = 1u << 1;
inline constexpr std::uint32_t kIntcpt2Npt = 1u << 2;

[[nodiscard]] inline Status validate_intercept_index(unsigned idx) noexcept {
    return idx <= kInterceptCtrl2 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::svm
