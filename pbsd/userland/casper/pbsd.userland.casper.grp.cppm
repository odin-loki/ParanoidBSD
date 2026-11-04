export module pbsd.userland.casper.grp;

export import pbsd.userland.casper;
import pbsd.core;

/// cap_grp service surface from hbsd/src/lib/libcasper/services/cap_grp/
export namespace pbsd::userland::casper::grp {

inline constexpr const char* kServiceName = "system.grp";

inline constexpr const char* kCmdGetGrEnt  = "getgrent";
inline constexpr const char* kCmdGetGrNam  = "getgrnam";
inline constexpr const char* kCmdGetGrGid  = "getgrgid";
inline constexpr const char* kCmdSetGrEnt  = "setgrent";
inline constexpr const char* kCmdEndGrEnt  = "endgrent";

[[nodiscard]] inline Result<ChannelHandle>
open_grp_service(LineageTree& tree, CapabilityRights rights) noexcept {
    return open_service(tree, rights, kServiceName);
}

[[nodiscard]] inline Status grp_limit_cmds(ChannelHandle& chan,
                                           const char* const* cmds,
                                           unsigned ncmds) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return Status::Invalid;
    }
    if (cmds == nullptr && ncmds != 0) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < ncmds; ++i) {
        if (cmds[i] == nullptr || cmds[i][0] == '\0') {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline Status grp_limit_fields(ChannelHandle& chan,
                                             const char* const* fields,
                                             unsigned nfields) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return Status::Invalid;
    }
    if (fields == nullptr && nfields != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status grp_limit_groups(ChannelHandle& chan,
                                             const char* const* names,
                                             unsigned nnames,
                                             const unsigned* gids,
                                             unsigned ngids) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return Status::Invalid;
    }
    if ((names == nullptr && nnames != 0) || (gids == nullptr && ngids != 0)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::casper::grp
