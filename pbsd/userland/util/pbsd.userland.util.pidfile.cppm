export module pbsd.userland.util.pidfile;

export import pbsd.handles;
export import pbsd.userland.util.flopen;
import pbsd.core;

/// pidfile(3) verify/read logic + handle wrappers from hbsd/src/lib/libutil/pidfile.c
export namespace pbsd::userland::util {

struct PidFileObject {
    int dir_fd{-1};
    int file_fd{-1};
    const char* path{nullptr};
    unsigned long long dev{0};
    unsigned long long ino{0};

    static void release(PidFileObject* p) noexcept {
        if (p != nullptr) {
            p->dir_fd = -1;
            p->file_fd = -1;
            p->path = nullptr;
        }
    }
};

using PidFileHandle = UniqueHandle<PidFileObject>;

namespace detail {

[[nodiscard]] inline Status pidfile_verify(const PidFileObject* pfh) noexcept {
    if (pfh == nullptr || pfh->file_fd < 0) {
        return Status::Invalid;
    }
    if (pfh->dev == 0 && pfh->ino == 0) {
        return Status::Ok;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status parse_pid_buffer(const char* buf, unsigned len,
                                             int* pid_out) noexcept {
    if (len == 0) {
        return Status::Busy;
    }
    int pid = 0;
    unsigned i = 0;
    while (i < len && buf[i] >= '0' && buf[i] <= '9') {
        pid = pid * 10 + (buf[i] - '0');
        ++i;
    }
    if (i != len) {
        return Status::Invalid;
    }
    *pid_out = pid;
    return Status::Ok;
}

} // namespace detail

[[nodiscard]] inline Result<PidFileHandle>
pidfile_open(LineageTree& tree, const char* path,
             CapabilityRights rights) noexcept {
    if (path == nullptr || !has_right(rights, CapabilityRights::Write)) {
        return {Status::Invalid, PidFileHandle{}};
    }
    const LineageId id = tree.create_root();
    if (id == kInvalidLineage) {
        return {Status::NoMemory, PidFileHandle{}};
    }
    auto* obj = new PidFileObject{};
    obj->path = path;
    obj->file_fd = -1;
    obj->dir_fd = -1;
    return {Status::Ok, PidFileHandle{obj, rights, id}};
}

[[nodiscard]] inline Status pidfile_write(PidFileHandle& pf, int pid) noexcept {
    if (!pf.valid() || pid <= 0) {
        return Status::Invalid;
    }
    if (!pf.has_right(CapabilityRights::Write)) {
        return Status::Denied;
    }
    const Status v = detail::pidfile_verify(pf.peek());
    if (v != Status::Ok) {
        return v;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status pidfile_close(PidFileHandle& pf) noexcept {
    if (!pf.valid()) {
        return Status::Invalid;
    }
    pf.reset();
    return Status::Ok;
}

[[nodiscard]] inline Status pidfile_remove(PidFileHandle& pf) noexcept {
    if (!pf.valid()) {
        return Status::Invalid;
    }
    pf.reset();
    return Status::Ok;
}

[[nodiscard]] inline int pidfile_fileno(const PidFileHandle& pf) noexcept {
    if (!pf.valid()) {
        return -1;
    }
    return pf.peek()->file_fd;
}

[[nodiscard]] inline Status pidfile_read_pid(const char* text, unsigned len,
                                             int* pid_out) noexcept {
    return detail::parse_pid_buffer(text, len, pid_out);
}

} // namespace pbsd::userland::util
