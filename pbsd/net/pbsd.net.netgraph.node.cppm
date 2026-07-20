module;
#include <cstdint>

export module pbsd.net.netgraph.node;

import pbsd.core;
import pbsd.net.netgraph;

/// PROVENANCE: hbsd/src/sys/netgraph/ng_*.h — common netgraph node types and hooks.
export namespace pbsd::net::netgraph::node {

inline constexpr char kTypeEther[] = "ether";
inline constexpr char kTypeSocket[] = "socket";
inline constexpr char kTypeIface[] = "iface";
inline constexpr char kTypeTee[] = "tee";
inline constexpr char kTypeBpf[] = "bpf";
inline constexpr char kTypePipe[] = "pipe";
inline constexpr char kTypePatch[] = "patch";
inline constexpr char kTypePppoe[] = "pppoe";

inline constexpr char kHookLower[] = "lower";
inline constexpr char kHookUpper[] = "upper";
inline constexpr char kHookLeft[] = "left";
inline constexpr char kHookRight[] = "right";
inline constexpr char kHookDebug[] = "debug";

struct NodeStub {
    char name[netgraph::kNodeSize]{};
    unsigned flags{};
    unsigned numhooks{};
    void* private_data{};
};

struct HookStub {
    char name[netgraph::kHookSize]{};
    HookStub* peer{};
    NodeStub* node{};
    bool connected{};
};

[[nodiscard]] inline Status validate_type_name(char const* type) noexcept {
    if (type == nullptr) {
        return Status::Invalid;
    }
    unsigned len = 0;
    while (type[len] != '\0') {
        if (++len >= netgraph::kTypeSize) {
            return Status::Invalid;
        }
    }
    return netgraph::validate_name_len(len, netgraph::kTypeSize);
}

[[nodiscard]] constexpr Status connect_hook(HookStub& a, HookStub& b) noexcept {
    if (a.connected || b.connected) {
        return Status::Busy;
    }
    a.peer = &b;
    b.peer = &a;
    a.connected = true;
    b.connected = true;
    return Status::Ok;
}

[[nodiscard]] constexpr Status disconnect_hook(HookStub& h) noexcept {
    if (!h.connected || h.peer == nullptr) {
        return Status::Invalid;
    }
    h.peer->connected = false;
    h.peer->peer = nullptr;
    h.connected = false;
    h.peer = nullptr;
    return Status::Ok;
}

enum class NodeFlag : unsigned {
    Invalid = 0x00000001,
    ForceWriter = 0x00000004,
    Closing = 0x00000008,
};

[[nodiscard]] constexpr bool node_is_valid(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(NodeFlag::Invalid)) == 0;
}

} // namespace pbsd::net::netgraph::node
