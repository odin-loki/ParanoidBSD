module;

export module pbsd.userland.rtld.symbols;

/// Public/private rtld symbol tables from hbsd/src/libexec/rtld-elf/Symbol.map
export namespace pbsd::userland::rtld {

enum class SymbolVersion : unsigned char { Fbsd10, Fbsd13, Fbsd18, Private10 };

struct ExportedSymbol {
    const char* name;
    SymbolVersion version;
};

struct VersionSet {
    SymbolVersion version;
    const char* label;
    const ExportedSymbol* symbols;
    unsigned count;
};

inline constexpr ExportedSymbol kFbsd10Symbols[] = {
    {"_rtld_error", SymbolVersion::Fbsd10},
    {"dlclose", SymbolVersion::Fbsd10},
    {"dlerror", SymbolVersion::Fbsd10},
    {"dlopen", SymbolVersion::Fbsd10},
    {"dlsym", SymbolVersion::Fbsd10},
    {"dlfunc", SymbolVersion::Fbsd10},
    {"dlvsym", SymbolVersion::Fbsd10},
    {"dladdr", SymbolVersion::Fbsd10},
    {"dllockinit", SymbolVersion::Fbsd10},
    {"dlinfo", SymbolVersion::Fbsd10},
    {"dl_iterate_phdr", SymbolVersion::Fbsd10},
    {"r_debug_state", SymbolVersion::Fbsd10},
    {"__tls_get_addr", SymbolVersion::Fbsd10},
};

inline constexpr ExportedSymbol kFbsd13Symbols[] = {
    {"fdlopen", SymbolVersion::Fbsd13},
};

inline constexpr ExportedSymbol kFbsd18Symbols[] = {
    {"rtld_get_var", SymbolVersion::Fbsd18},
    {"rtld_set_var", SymbolVersion::Fbsd18},
};

inline constexpr ExportedSymbol kPrivateSymbols[] = {
    {"_dl_iterate_phdr_locked", SymbolVersion::Private10},
    {"_rtld_thread_init", SymbolVersion::Private10},
    {"_rtld_allocate_tls", SymbolVersion::Private10},
    {"_rtld_free_tls", SymbolVersion::Private10},
    {"_rtld_atfork_pre", SymbolVersion::Private10},
    {"_rtld_atfork_post", SymbolVersion::Private10},
    {"_rtld_addr_phdr", SymbolVersion::Private10},
    {"_rtld_get_pax_flags", SymbolVersion::Private10},
    {"_rtld_get_stack_prot", SymbolVersion::Private10},
    {"_rtld_is_dlopened", SymbolVersion::Private10},
    {"_r_debug_postinit", SymbolVersion::Private10},
    {"_rtld_version__FreeBSD_version", SymbolVersion::Private10},
    {"_rtld_version_laddr_offset", SymbolVersion::Private10},
    {"_rtld_version_dlpi_tls_data", SymbolVersion::Private10},
};

inline constexpr ExportedSymbol kPublicSymbols[] = {
    {"_rtld_error", SymbolVersion::Fbsd10},
    {"dlclose", SymbolVersion::Fbsd10},
    {"dlerror", SymbolVersion::Fbsd10},
    {"dlopen", SymbolVersion::Fbsd10},
    {"dlsym", SymbolVersion::Fbsd10},
    {"dlfunc", SymbolVersion::Fbsd10},
    {"dlvsym", SymbolVersion::Fbsd10},
    {"dladdr", SymbolVersion::Fbsd10},
    {"dllockinit", SymbolVersion::Fbsd10},
    {"dlinfo", SymbolVersion::Fbsd10},
    {"dl_iterate_phdr", SymbolVersion::Fbsd10},
    {"r_debug_state", SymbolVersion::Fbsd10},
    {"__tls_get_addr", SymbolVersion::Fbsd10},
    {"fdlopen", SymbolVersion::Fbsd13},
    {"rtld_get_var", SymbolVersion::Fbsd18},
    {"rtld_set_var", SymbolVersion::Fbsd18},
};

inline constexpr VersionSet kVersionSets[] = {
    {SymbolVersion::Fbsd10, "FBSD_1.0", kFbsd10Symbols,
     static_cast<unsigned>(sizeof(kFbsd10Symbols) / sizeof(kFbsd10Symbols[0]))},
    {SymbolVersion::Fbsd13, "FBSD_1.3", kFbsd13Symbols,
     static_cast<unsigned>(sizeof(kFbsd13Symbols) / sizeof(kFbsd13Symbols[0]))},
    {SymbolVersion::Fbsd18, "FBSD_1.8", kFbsd18Symbols,
     static_cast<unsigned>(sizeof(kFbsd18Symbols) / sizeof(kFbsd18Symbols[0]))},
    {SymbolVersion::Private10, "FBSDprivate_1.0", kPrivateSymbols,
     static_cast<unsigned>(sizeof(kPrivateSymbols) / sizeof(kPrivateSymbols[0]))},
};

inline constexpr unsigned kPublicSymbolCount =
    static_cast<unsigned>(sizeof(kPublicSymbols) / sizeof(kPublicSymbols[0]));
inline constexpr unsigned kPrivateSymbolCount =
    static_cast<unsigned>(sizeof(kPrivateSymbols) / sizeof(kPrivateSymbols[0]));
inline constexpr unsigned kVersionSetCount =
    static_cast<unsigned>(sizeof(kVersionSets) / sizeof(kVersionSets[0]));

[[nodiscard]] inline bool is_exported(const char* name) noexcept {
    for (const auto& s : kPublicSymbols) {
        if (s.name == name) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline bool is_private_rtld_symbol(const char* name) noexcept {
    for (const auto& s : kPrivateSymbols) {
        if (s.name == name) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline const ExportedSymbol* lookup_public(const char* name) noexcept {
    for (const auto& s : kPublicSymbols) {
        if (s.name == name) {
            return &s;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const VersionSet* version_set(SymbolVersion v) noexcept {
    for (const auto& set : kVersionSets) {
        if (set.version == v) {
            return &set;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const char* version_label(SymbolVersion v) noexcept {
    const VersionSet* set = version_set(v);
    return set != nullptr ? set->label : nullptr;
}

} // namespace pbsd::userland::rtld
