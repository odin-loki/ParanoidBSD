module;
#include <cstdint>

export module pbsd.net.netgraph;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netgraph/ng_message.h — netgraph message limits.
export namespace pbsd::net::netgraph {

inline constexpr unsigned kTypeSize = 32;
inline constexpr unsigned kHookSize = 32;
inline constexpr unsigned kNodeSize = 32;
inline constexpr unsigned kPathSize = 512;
inline constexpr unsigned kCmdStrSize = 32;
inline constexpr unsigned kTextResponse = 1024;
inline constexpr unsigned kAbiVersion = 12;

enum class MsgType : unsigned char {
    Connect = 1,
    Disconnect = 2,
    Generic = 3,
    TextStatus = 4,
    Error = 5,
    Binary = 6,
    Close = 7,
};

struct MsgHeader {
    MsgType type{MsgType::Generic};
    unsigned argcount{};
    unsigned token{};
};

[[nodiscard]] inline Status validate_name_len(unsigned len, unsigned max) noexcept {
    if (len == 0 || len >= max) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_msg(MsgHeader const& msg) noexcept {
    if (msg.type == MsgType::Connect && msg.argcount < 2) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool abi_compatible(int module_abi) noexcept {
    return module_abi == static_cast<int>(kAbiVersion);
}

} // namespace pbsd::net::netgraph
