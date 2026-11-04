export module pbsd.userland.casper.pwd;

export import pbsd.userland.casper;
import pbsd.core;

/// cap_pwd service surface from hbsd/src/lib/libcasper/services/cap_pwd/
export namespace pbsd::userland::casper::pwd {

inline constexpr const char* kServiceName = "system.pwd";

inline constexpr const char* kCmdGetPwEnt  = "getpwent";
inline constexpr const char* kCmdGetPwNam  = "getpwnam";
inline constexpr const char* kCmdGetPwUid  = "getpwuid";
inline constexpr const char* kCmdSetPwEnt  = "setpwent";
inline constexpr const char* kCmdEndPwEnt  = "endpwent";

struct PasswdBuffer {
    char* data{nullptr};
    unsigned capacity{0};

    static void release(PasswdBuffer* p) noexcept {
        if (p != nullptr) {
            p->data = nullptr;
            p->capacity = 0;
        }
    }
};

using PasswdBufferHandle = UniqueHandle<PasswdBuffer>;

[[nodiscard]] inline Status passwd_resize(PasswdBuffer& buf) noexcept {
    unsigned newcap = buf.capacity == 0 ? 1024u : buf.capacity * 2u;
    if (newcap < buf.capacity) {
        return Status::NoMemory;
    }
    buf.capacity = newcap;
    return Status::Ok;
}

[[nodiscard]] inline Status passwd_unpack_string(unsigned buf_used,
                                               unsigned field_len,
                                               unsigned* cursor) noexcept {
    if (field_len >= buf_used) {
        return Status::NoMemory;
    }
    *cursor += field_len + 1;
    if (*cursor > buf_used) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Result<ChannelHandle>
open_pwd_service(LineageTree& tree, CapabilityRights rights) noexcept {
    return open_service(tree, rights, kServiceName);
}

[[nodiscard]] inline Status pwd_limit_cmds(ChannelHandle& chan,
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

[[nodiscard]] inline Status pwd_limit_fields(ChannelHandle& chan,
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

[[nodiscard]] inline Status pwd_limit_users(ChannelHandle& chan,
                                            const char* const* names,
                                            unsigned nnames,
                                            const unsigned* uids,
                                            unsigned nuids) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return Status::Invalid;
    }
    if ((names == nullptr && nnames != 0) || (uids == nullptr && nuids != 0)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::casper::pwd
