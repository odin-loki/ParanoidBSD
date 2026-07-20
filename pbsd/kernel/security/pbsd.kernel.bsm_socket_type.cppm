module;
#include <cstdint>

export module pbsd.kernel.bsm_socket_type;

import pbsd.core;

/// Freestanding port of `security/audit/bsm_socket_type.c` BSM socket mappings.
export namespace pbsd::kernel::bsm_socket_type {

inline constexpr int kNoLocalMapping = -600;

inline constexpr unsigned short kBsmDgram      = 2;
inline constexpr unsigned short kBsmStream    = 1;
inline constexpr unsigned short kBsmRaw         = 3;
inline constexpr unsigned short kBsmRdm         = 4;
inline constexpr unsigned short kBsmSeqpacket   = 5;

inline constexpr int kSockDgram      = 2;
inline constexpr int kSockStream     = 1;
inline constexpr int kSockRaw        = 3;
inline constexpr int kSockRdm        = 4;
inline constexpr int kSockSeqpacket  = 5;

struct Mapping {
    unsigned short bsm_type{};
    int            local_type{};
};

inline constexpr Mapping kTable[] = {
    {kBsmDgram,     kSockDgram},
    {kBsmStream,    kSockStream},
    {kBsmRaw,       kSockRaw},
    {kBsmRdm,       kSockRdm},
    {kBsmSeqpacket, kSockSeqpacket},
};

[[nodiscard]] inline unsigned table_size() noexcept {
    return static_cast<unsigned>(sizeof(kTable) / sizeof(kTable[0]));
}

[[nodiscard]] inline int lookup_local(unsigned short bsm_type) noexcept {
    for (unsigned i = 0; i < table_size(); ++i) {
        if (kTable[i].bsm_type == bsm_type) {
            return kTable[i].local_type;
        }
    }
    return kNoLocalMapping;
}

[[nodiscard]] inline unsigned short lookup_bsm(int local_type) noexcept {
    for (unsigned i = 0; i < table_size(); ++i) {
        if (kTable[i].local_type == local_type) {
            return kTable[i].bsm_type;
        }
    }
    return 0;
}

} // namespace pbsd::kernel::bsm_socket_type
