export module pbsd.userland.casper.dns;

export import pbsd.userland.casper;
import pbsd.core;

/// cap_dns service surface from hbsd/src/lib/libcasper/services/cap_dns/
export namespace pbsd::userland::casper::dns {

inline constexpr const char* kServiceName = "system.dns";

inline constexpr const char* kCmdGetHostByName  = "gethostbyname";
inline constexpr const char* kCmdGetHostByName2 = "gethostbyname2";
inline constexpr const char* kCmdGetHostByAddr  = "gethostbyaddr";
inline constexpr const char* kCmdGetAddrInfo    = "getaddrinfo";
inline constexpr const char* kCmdGetNameInfo    = "getnameinfo";

struct HostEntView {
    const char* name{nullptr};
    int addrtype{0};
    int length{0};
    unsigned alias_count{0};
    unsigned addr_count{0};
};

[[nodiscard]] inline Status validate_type_limit(const char* const* types,
                                              unsigned ntypes) noexcept {
    if (types == nullptr && ntypes != 0) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < ntypes; ++i) {
        if (types[i] == nullptr || types[i][0] == '\0') {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_family_limit(const int* families,
                                                unsigned nfamilies) noexcept {
    if (families == nullptr && nfamilies != 0) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < nfamilies; ++i) {
        if (families[i] != 0 && families[i] != 2 && families[i] != 10 &&
            families[i] != 28) {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline Result<ChannelHandle>
open_dns_service(LineageTree& tree, CapabilityRights rights) noexcept {
    if (validate_service_name(kServiceName) != Status::Ok) {
        return {Status::Invalid, ChannelHandle{}};
    }
    return open_service(tree, rights, kServiceName);
}

[[nodiscard]] inline Status dns_type_limit(ChannelHandle& chan,
                                           const char* const* types,
                                           unsigned ntypes) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return Status::Invalid;
    }
    return validate_type_limit(types, ntypes);
}

[[nodiscard]] inline Status dns_family_limit(ChannelHandle& chan,
                                             const int* families,
                                             unsigned nfamilies) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return Status::Invalid;
    }
    return validate_family_limit(families, nfamilies);
}

} // namespace pbsd::userland::casper::dns
