export module pbsd.userland.casper.net;

export import pbsd.userland.casper;
import pbsd.core;

/// cap_net service surface from hbsd/src/lib/libcasper/services/cap_net/
export namespace pbsd::userland::casper::net {

inline constexpr const char* kServiceName = "system.net";

inline constexpr unsigned long long CAPNET_ADDR2NAME                = 0x01;
inline constexpr unsigned long long CAPNET_NAME2ADDR                = 0x02;
inline constexpr unsigned long long CAPNET_DEPRECATED_ADDR2NAME     = 0x04;
inline constexpr unsigned long long CAPNET_DEPRECATED_NAME2ADDR     = 0x08;
inline constexpr unsigned long long CAPNET_CONNECT                  = 0x10;
inline constexpr unsigned long long CAPNET_BIND                     = 0x20;
inline constexpr unsigned long long CAPNET_CONNECTDNS               = 0x40;

inline constexpr unsigned long long kCapNetMask =
    CAPNET_ADDR2NAME | CAPNET_NAME2ADDR | CAPNET_DEPRECATED_ADDR2NAME |
    CAPNET_DEPRECATED_NAME2ADDR | CAPNET_CONNECT | CAPNET_BIND |
    CAPNET_CONNECTDNS;

inline constexpr const char* kLimitBind      = "bind";
inline constexpr const char* kLimitConnect   = "connect";
inline constexpr const char* kLimitAddr2Name = "addr2name";
inline constexpr const char* kLimitName2Addr = "name2addr";

struct NetLimitObject {
    ChannelHandle* chan{nullptr};
    unsigned long long mode{0};

    static void release(NetLimitObject* p) noexcept {
        if (p != nullptr) {
            p->chan = nullptr;
            p->mode = 0;
        }
    }
};

using NetLimitHandle = UniqueHandle<NetLimitObject>;

[[nodiscard]] inline bool mode_valid(unsigned long long mode) noexcept {
    return (mode & ~kCapNetMask) == 0;
}

[[nodiscard]] inline Result<NetLimitHandle>
limit_init(ChannelHandle& chan, unsigned long long mode,
           LineageTree& tree, CapabilityRights rights) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return {Status::Invalid, NetLimitHandle{}};
    }
    if (!mode_valid(mode)) {
        return {Status::Invalid, NetLimitHandle{}};
    }
    const LineageId id = tree.create_root();
    if (id == kInvalidLineage) {
        return {Status::NoMemory, NetLimitHandle{}};
    }
    auto* obj = new NetLimitObject{};
    obj->chan = &chan;
    obj->mode = mode;
    return {Status::Ok, NetLimitHandle{obj, rights, id}};
}

[[nodiscard]] inline Result<ChannelHandle>
open_net_service(LineageTree& tree, CapabilityRights rights) noexcept {
    return open_service(tree, rights, kServiceName);
}

[[nodiscard]] inline Status limit_apply(NetLimitHandle& limit) noexcept {
    if (!limit.valid()) {
        return Status::Invalid;
    }
    if (!mode_valid(limit.peek()->mode)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::casper::net
