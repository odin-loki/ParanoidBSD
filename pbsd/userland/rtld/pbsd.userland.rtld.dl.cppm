module;

export module pbsd.userland.rtld.dl;

import pbsd.core;

/// dlopen/dlsym concepts from hbsd/src/libexec/rtld-elf/rtld.c
export namespace pbsd::userland::rtld {

enum class DlMode : unsigned {
    Lazy = 0x001,
    Now = 0x002,
    Local = 0x004,
    Global = 0x010,
};

struct DlHandle {
    unsigned id{0};
    const char* path{nullptr};
};

[[nodiscard]] inline Status dlopen(DlHandle& out, const char* path, DlMode mode) noexcept {
    out.id = 1;
    out.path = path;
    (void)mode;
    return Status::NotImplemented;
}

[[nodiscard]] inline Status dlsym(void*& sym, const DlHandle& handle,
                                    const char* name) noexcept {
    (void)handle;
    (void)name;
    sym = nullptr;
    return Status::NotFound;
}

[[nodiscard]] inline Status dlclose(DlHandle& handle) noexcept {
    handle = DlHandle{};
    return Status::Ok;
}

} // namespace pbsd::userland::rtld
