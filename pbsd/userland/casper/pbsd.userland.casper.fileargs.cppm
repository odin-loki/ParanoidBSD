export module pbsd.userland.casper.fileargs;

export import pbsd.userland.casper;
import pbsd.core;

/// cap_fileargs service surface from hbsd/src/lib/libcasper/services/cap_fileargs/
export namespace pbsd::userland::casper::fileargs {

inline constexpr const char* kServiceName = "system.fileargs";

inline constexpr int FA_OPEN     = 1;
inline constexpr int FA_LSTAT    = 2;
inline constexpr int FA_REALPATH = 4;

struct FileArgsObject {
    ChannelHandle* chan{nullptr};
    int flags{0};
    unsigned mode{0};
    int operations{0};

    static void release(FileArgsObject* p) noexcept {
        if (p != nullptr) {
            p->chan = nullptr;
            p->flags = 0;
            p->mode = 0;
            p->operations = 0;
        }
    }
};

using FileArgsHandle = UniqueHandle<FileArgsObject>;

[[nodiscard]] inline bool flags_valid(int flags) noexcept {
    const int mask = FA_OPEN | FA_LSTAT | FA_REALPATH;
    return (flags & ~mask) == 0 && flags != 0;
}

[[nodiscard]] inline Result<FileArgsHandle>
init(ChannelHandle& chan, int flags, unsigned mode, int operations,
     LineageTree& tree, CapabilityRights rights) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return {Status::Invalid, FileArgsHandle{}};
    }
    if (!flags_valid(flags)) {
        return {Status::Invalid, FileArgsHandle{}};
    }
    const LineageId id = tree.create_root();
    if (id == kInvalidLineage) {
        return {Status::NoMemory, FileArgsHandle{}};
    }
    auto* obj = new FileArgsObject{};
    obj->chan = &chan;
    obj->flags = flags;
    obj->mode = mode;
    obj->operations = operations;
    return {Status::Ok, FileArgsHandle{obj, rights, id}};
}

[[nodiscard]] inline Status validate_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::casper::fileargs
