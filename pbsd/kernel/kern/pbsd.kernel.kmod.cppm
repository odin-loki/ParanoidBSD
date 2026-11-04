module;
#include <cstdint>

export module pbsd.kernel.kmod;

import pbsd.core;

/// Freestanding port of `sys/module.h` — kld(4) module metadata and modevents.
export namespace pbsd::kernel::kmod {

inline constexpr int kMdtDepend    = 1;
inline constexpr int kMdtModule    = 2;
inline constexpr int kMdtVersion   = 3;
inline constexpr int kMdtPnpInfo   = 4;
inline constexpr int kMdtStructVersion = 1;

inline constexpr unsigned kMaxNameV1V2 = 32;
inline constexpr unsigned kMaxNameV3   = 1024;

enum class ModeEvent : int {
    Load     = 0,
    Unload   = 1,
    Shutdown = 2,
    Quiesce  = 3,
};

using ModeventHandler = int (*)(void* mod, int event, void* priv);

struct ModuleData {
    char const* name{nullptr};
    ModeventHandler evhand{nullptr};
    void*         priv{nullptr};
};

struct ModDepend {
    int minimum{};
    int preferred{};
    int maximum{};
};

struct ModVersion {
    int version{};
};

struct ModMetadata {
    int         version{kMdtStructVersion};
    int         type{};
    void const* data{nullptr};
    char const* label{nullptr};
};

[[nodiscard]] inline Status validate_name(char const* name) noexcept {
    if (name == nullptr) {
        return Status::Invalid;
    }
    unsigned len = 0;
    while (name[len] != '\0') {
        if (++len >= kMaxNameV1V2) {
            return Status::Invalid;
        }
    }
    return len == 0 ? Status::Invalid : Status::Ok;
}

[[nodiscard]] inline Status validate_depend(ModDepend const& dep) noexcept {
    if (dep.minimum < 0 || dep.preferred < dep.minimum) {
        return Status::Invalid;
    }
    if (dep.maximum != 0 && dep.maximum < dep.preferred) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_modevent(int event) noexcept {
    switch (static_cast<ModeEvent>(event)) {
    case ModeEvent::Load:
    case ModeEvent::Unload:
    case ModeEvent::Shutdown:
    case ModeEvent::Quiesce:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status version_in_range(int version, ModDepend const& dep) noexcept {
    if (validate_depend(dep) != Status::Ok) {
        return Status::Invalid;
    }
    if (version < dep.minimum) {
        return Status::Denied;
    }
    if (dep.maximum != 0 && version > dep.maximum) {
        return Status::Denied;
    }
    return Status::Ok;
}

struct ModuleStub {
    char           name[kMaxNameV1V2]{};
    ModVersion     version{};
    unsigned       refs{};
    bool           loaded{};
};

[[nodiscard]] inline Status load(ModuleStub& mod, ModuleData const& data) noexcept {
    if (mod.loaded) {
        return Status::Busy;
    }
    if (validate_name(data.name) != Status::Ok || data.evhand == nullptr) {
        return Status::Invalid;
    }
    for (unsigned i = 0; data.name[i] != '\0'; ++i) {
        mod.name[i] = data.name[i];
    }
    mod.loaded = true;
    ++mod.refs;
    return Status::Ok;
}

[[nodiscard]] inline Status unload(ModuleStub& mod) noexcept {
    if (!mod.loaded) {
        return Status::Invalid;
    }
    if (mod.refs == 0) {
        return Status::Invalid;
    }
    --mod.refs;
    if (mod.refs == 0) {
        mod.loaded = false;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kmod
