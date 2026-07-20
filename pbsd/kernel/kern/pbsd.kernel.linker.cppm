module;
#include <cstdint>

export module pbsd.kernel.linker;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/kern_linker.c — kld module table scaffold.
export namespace pbsd::kernel::linker {

inline constexpr unsigned kMaxModules = 256;

struct Module {
    const char* name{};
    std::uint32_t id{};
    bool loaded{false};
};

struct Table {
    Module mods[kMaxModules]{};
    unsigned count{};
};

[[nodiscard]] inline Status load(Table& t, const char* name, std::uint32_t id) noexcept {
    if (name == nullptr || name[0] == '\0' || id == 0) {
        return Status::Invalid;
    }
    if (t.count >= kMaxModules) {
        return Status::Busy;
    }
    t.mods[t.count++] = Module{name, id, true};
    return Status::Ok;
}

[[nodiscard]] inline Status unload(Table& t, std::uint32_t id) noexcept {
    for (unsigned i = 0; i < t.count; ++i) {
        if (t.mods[i].id == id && t.mods[i].loaded) {
            t.mods[i].loaded = false;
            return Status::Ok;
        }
    }
    return Status::NotFound;
}

} // namespace pbsd::kernel::linker
