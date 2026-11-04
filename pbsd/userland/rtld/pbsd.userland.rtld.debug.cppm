module;

export module pbsd.userland.rtld.debug;

/// rtld debug flags from hbsd/src/libexec/rtld-elf/debug.c
export namespace pbsd::userland::rtld {

enum class DebugFlag : unsigned {
    None = 0,
    Bindings = 0x01,
    Symbols = 0x02,
    Relocations = 0x04,
    All = 0x07,
};

inline unsigned g_debug_flags = 0;

[[nodiscard]] inline bool debug_enabled(DebugFlag f) noexcept {
    return (g_debug_flags & static_cast<unsigned>(f)) != 0;
}

inline void set_debug_flags(unsigned flags) noexcept { g_debug_flags = flags; }

} // namespace pbsd::userland::rtld
