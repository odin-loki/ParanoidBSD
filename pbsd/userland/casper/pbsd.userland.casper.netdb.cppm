export module pbsd.userland.casper.netdb;

export import pbsd.userland.casper;
import pbsd.core;

/// cap_netdb service surface from hbsd/src/lib/libcasper/services/cap_netdb/
export namespace pbsd::userland::casper::netdb {

inline constexpr const char* kServiceName = "system.netdb";

inline constexpr const char* kCmdGetProtoByName = "getprotobyname";

[[nodiscard]] inline Result<ChannelHandle>
open_netdb_service(LineageTree& tree, CapabilityRights rights) noexcept {
    return open_service(tree, rights, kServiceName);
}

[[nodiscard]] inline Status netdb_limit_protocols(ChannelHandle& chan,
                                                  const char* const* names,
                                                  unsigned nnames) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return Status::Invalid;
    }
    if (names == nullptr && nnames != 0) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < nnames; ++i) {
        if (names[i] == nullptr || names[i][0] == '\0') {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

} // namespace pbsd::userland::casper::netdb
