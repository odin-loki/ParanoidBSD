export module pbsd.userland.casper;

export import pbsd.handles;
import pbsd.core;

/// libcasper core from hbsd/src/lib/libcasper/libcasper/libcasper.c
export namespace pbsd::userland::casper {

inline constexpr unsigned kCasperNoUniq = 0x00000001u;
inline constexpr int kCapChannelMagic = 0x0cac8a31;

inline constexpr unsigned kNvFlagNoUnique = 0x01u;

struct ChannelObject {
    int sock{-1};
    int pd{-1};
    int flags{0};
    int magic{0};

    static void release(ChannelObject* p) noexcept {
        if (p != nullptr) {
            p->sock = -1;
            p->pd = -1;
            p->flags = 0;
            p->magic = 0;
        }
    }
};

using ChannelHandle = UniqueHandle<ChannelObject>;

struct ChannelView {
    int fd{-1};
    unsigned flags{0};
};

[[nodiscard]] inline bool fd_is_valid(int fd) noexcept { return fd >= 0; }

[[nodiscard]] inline unsigned channel_nvlist_flags(const ChannelObject* chan) noexcept {
    unsigned flags = 0;
    if (chan != nullptr && (chan->flags & static_cast<int>(kCasperNoUniq)) != 0) {
        flags |= kNvFlagNoUnique;
    }
    return flags;
}

[[nodiscard]] inline bool channel_valid(const ChannelObject* chan) noexcept {
    return chan != nullptr && chan->magic == kCapChannelMagic &&
           fd_is_valid(chan->sock);
}

[[nodiscard]] inline Result<ChannelHandle>
wrap_channel(int sock, int flags, LineageTree& tree,
             CapabilityRights rights) noexcept {
    if (!fd_is_valid(sock)) {
        return {Status::Invalid, ChannelHandle{}};
    }
    if ((flags & ~static_cast<int>(kCasperNoUniq)) != 0) {
        return {Status::Invalid, ChannelHandle{}};
    }
    const LineageId id = tree.create_root();
    if (id == kInvalidLineage) {
        return {Status::NoMemory, ChannelHandle{}};
    }
    auto* obj = new ChannelObject{};
    obj->sock = sock;
    obj->pd = -1;
    obj->flags = flags;
    obj->magic = kCapChannelMagic;
    return {Status::Ok, ChannelHandle{obj, rights, id}};
}

[[nodiscard]] inline int channel_flags(const ChannelHandle& ch) noexcept {
    if (!ch.valid() || !channel_valid(ch.peek())) {
        return 0;
    }
    return ch.peek()->flags;
}

[[nodiscard]] inline Result<ChannelHandle>
open_service(LineageTree& tree, CapabilityRights rights,
             const char* service) noexcept {
    if (service == nullptr || service[0] == '\0') {
        return {Status::Invalid, ChannelHandle{}};
    }
    if (check_grant(CapabilityRights::Read | CapabilityRights::Write, rights) !=
        Status::Ok) {
        return {Status::Denied, ChannelHandle{}};
    }
    return wrap_channel(-1, 0, tree, rights);
}

[[nodiscard]] inline Result<ChannelHandle>
limit_channel(ChannelHandle&& parent, CapabilityRights child_rights,
              LineageTree& tree) noexcept {
    if (!parent.valid() || !channel_valid(parent.peek())) {
        return {Status::Invalid, ChannelHandle{}};
    }
    auto grant = parent.grant(child_rights, tree);
    if (!grant.has_value()) {
        return {grant.status, ChannelHandle{}};
    }
    const auto borrowed = grant.value;
    return {Status::Ok,
            ChannelHandle{borrowed.peek(), borrowed.rights(), borrowed.lineage()}};
}

[[nodiscard]] inline ChannelView view(const ChannelHandle& ch) noexcept {
    if (!ch.valid() || !channel_valid(ch.peek())) {
        return {};
    }
    return ChannelView{ch.peek()->sock, static_cast<unsigned>(ch.peek()->flags)};
}

[[nodiscard]] inline int unwrap_sock(ChannelObject* chan, int* flags_out) noexcept {
    if (!channel_valid(chan)) {
        return -1;
    }
    const int sock = chan->sock;
    if (flags_out != nullptr) {
        *flags_out = chan->flags;
    }
    chan->magic = 0;
    return sock;
}

[[nodiscard]] inline Status validate_service_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return Status::Invalid;
    }
    if (name[0] != 's' || name[1] != 'y' || name[2] != 's' ||
        name[3] != 't' || name[4] != 'e' || name[5] != 'm' ||
        name[6] != '.') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::casper
