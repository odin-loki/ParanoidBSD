module;
#include <cstdint>

export module pbsd.kernel.netgraph;

export import pbsd.core;

/// Wave 4 — netgraph kernel stubs (sys/netgraph/netgraph.h).
export namespace pbsd::kernel::netgraph {

inline constexpr int kAbiVersion = 12;

inline constexpr unsigned kNgInvalid      = 0x00000001;
inline constexpr unsigned kNgForceWriter  = 0x00000004;
inline constexpr unsigned kNgClosing      = 0x00000008;
inline constexpr unsigned kNgReallyDie    = 0x00000010;
inline constexpr unsigned kNgHiStack      = 0x00000020;
inline constexpr unsigned kNgType1      = 0x10000000;
inline constexpr unsigned kNgType2      = 0x20000000;
inline constexpr unsigned kNgType3      = 0x40000000;
inline constexpr unsigned kNgType4      = 0x80000000;

inline constexpr unsigned kNodesiz = 32;
inline constexpr unsigned kHooksiz = 16;

struct NodeStub {
    char          name[kNodesiz]{};
    unsigned      flags{};
    unsigned      numhooks{};
    void*         private_data{};
};

struct HookStub {
    char      name[kHooksiz]{};
    HookStub* peer{};
    NodeStub* node{};
    bool      connected{};
};

[[nodiscard]] inline unsigned node_flag_table_size() noexcept {
    return 6;
}

[[nodiscard]] constexpr bool node_is_valid(unsigned flags) noexcept {
    return (flags & kNgInvalid) == 0;
}

[[nodiscard]] constexpr Status validate_node(const NodeStub& node) noexcept {
    if (!node_is_valid(node.flags)) {
        return Status::Invalid;
    }
    if (node.flags & kNgClosing) {
        return Status::Busy;
    }
    return Status::Ok;
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

[[nodiscard]] constexpr bool abi_compatible(int module_abi) noexcept {
    return module_abi == kAbiVersion;
}

enum class MsgType : unsigned {
    Connect = 1,
    Disconnect = 2,
    Error = 3,
    Text = 4,
    Binary = 5,
    Close = 6,
};

enum class HookFlag : unsigned {
    Invalid = 0x0001,
    Queue = 0x0002,
    ForceWriter = 0x0004,
    Dead = 0x0008,
    HiStack = 0x0010,
    ToInbound = 0x0020,
};

struct MsgStub {
    MsgType type{MsgType::Text};
    unsigned argcount{};
};

[[nodiscard]] constexpr Status validate_msg(const MsgStub& msg) noexcept {
    if (msg.type == MsgType::Connect && msg.argcount < 2) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool hook_valid(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(HookFlag::Invalid)) == 0;
}

[[nodiscard]] inline unsigned msg_table_size() noexcept {
    return 6;
}

} // namespace pbsd::kernel::netgraph
