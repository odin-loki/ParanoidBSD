module;

#include <cstdlib>

export module pbsd.userland.jail;

export import pbsd.handles;
import pbsd.core;

/// libjail-shaped port: jail parameters tracked under lineage.
export namespace pbsd::userland::jail {

enum class ParamFlag : unsigned {
    RawValue = static_cast<unsigned>(0x01),
    Bool     = static_cast<unsigned>(0x02),
    NoBool   = static_cast<unsigned>(0x04),
    JailSys  = static_cast<unsigned>(0x08),
    KeyValue = static_cast<unsigned>(0x10),
};

inline constexpr unsigned kJailErrMsgLen = 1024;

struct JailParam {
    const char* name{nullptr};
    const void* value{nullptr};
    unsigned    value_len{0};
    unsigned    elem_len{0};
    int         ctl_type{0};
    int         struct_type{0};
    ParamFlag   flags{ParamFlag::KeyValue};
};

struct JailObject {
    int jid{-1};

    static void release(JailObject* p) noexcept {
        if (p != nullptr) {
            p->jid = -1;
        }
    }
};

using JailHandle = UniqueHandle<JailObject>;

[[nodiscard]] inline Result<JailHandle>
create_jail(LineageTree& tree, CapabilityRights rights) noexcept {
    if (!has_right(rights, CapabilityRights::Write)) {
        return {Status::Denied, JailHandle{}};
    }
    const LineageId id = tree.create_root();
    if (id == kInvalidLineage) {
        return {Status::NoMemory, JailHandle{}};
    }
    return {Status::Ok, JailHandle{new JailObject{}, rights, id}};
}

[[nodiscard]] inline Status set_params(JailHandle& jail, JailParam* params,
                                     unsigned count) noexcept {
    if (!jail.valid() || params == nullptr || count == 0) {
        return Status::Invalid;
    }
    if (!jail.has_right(CapabilityRights::Write)) {
        return Status::Denied;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_jailed() noexcept { return false; }

namespace detail {

[[nodiscard]] inline bool parse_numeric_jid(const char* name, int& jid_out) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return false;
    }
    char* end = nullptr;
    const unsigned long val = std::strtoul(name, &end, 10);
    if (end == name || (end != nullptr && *end != '\0')) {
        return false;
    }
    if (val > static_cast<unsigned long>(0x7fffffff)) {
        return false;
    }
    jid_out = static_cast<int>(val);
    return true;
}

} // namespace detail

[[nodiscard]] inline Result<int> get_jid(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return {Status::Invalid, -1};
    }
    int jid = 0;
    if (detail::parse_numeric_jid(name, jid)) {
        // jid 0 always exists (host system) — skip kernel lookup.
        if (jid == 0) {
            return {Status::Ok, 0};
        }
        return {Status::Ok, jid};
    }
    return {Status::NotImplemented, -1};
}

[[nodiscard]] inline Status get_name(int jid, char* buf, unsigned buflen) noexcept {
    if (buf == nullptr || buflen == 0 || jid < 0) {
        return Status::Invalid;
    }
    buf[0] = '\0';
    return Status::NotImplemented;
}

} // namespace pbsd::userland::jail
