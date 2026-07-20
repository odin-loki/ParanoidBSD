module;
#include <cstdint>

export module pbsd.stand.env;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/common/env.c — loader environment variable names.
export namespace pbsd::stand::env {

inline constexpr unsigned kMaxName = 64;
inline constexpr unsigned kMaxValue = 256;

enum class Var : unsigned char {
    BootPath = 0,
    KernelPath = 1,
    ModulePath = 2,
    RootDev = 3,
    Console = 4,
    VfsRoot = 5,
};

struct VarEntry {
    Var         id{};
    const char* name{};
};

inline constexpr VarEntry kVarTable[] = {
    {Var::BootPath, "bootpath"},
    {Var::KernelPath, "kernel_path"},
    {Var::ModulePath, "module_path"},
    {Var::RootDev, "rootdev"},
    {Var::Console, "console"},
    {Var::VfsRoot, "vfs.root.mountfrom"},
};

[[nodiscard]] inline constexpr std::size_t var_table_size() noexcept {
    return sizeof(kVarTable) / sizeof(kVarTable[0]);
}

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len > kMaxName) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::env
