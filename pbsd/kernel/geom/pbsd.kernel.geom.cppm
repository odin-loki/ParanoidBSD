export module pbsd.kernel.geom;

export import pbsd.core;

/// Wave 4 — GEOM kernel class/provider KPI (sys/geom/geom.h).
export namespace pbsd::kernel::geom {

inline constexpr unsigned kVersion00 = 0x19950323;
inline constexpr unsigned kVersion01 = 0x20041207;
inline constexpr unsigned kVersion = kVersion01;

enum class TasteFlag : int {
    Normal = 0,
    Insist = 1,
    Transparent = 2,
};

struct ClassStub {
    const char* name{};
    unsigned version{kVersion};
    unsigned geom_count{};
};

struct ProviderStub {
    const char* name{};
    unsigned mediasize{};
    unsigned sectorsize{};
    int error{};
};

struct ConsumerStub {
    ProviderStub* provider{};
    int acw{};
    int acr{};
    int ace{};
};

[[nodiscard]] constexpr Status validate_provider(const ProviderStub& pp) noexcept {
    if (pp.sectorsize == 0 || (pp.sectorsize & (pp.sectorsize - 1)) != 0) {
        return Status::Invalid;
    }
    if (pp.mediasize < pp.sectorsize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_access(int acw, int acr, int ace) noexcept {
    if (acw < 0 || acr < 0 || ace < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned taste_flag_table_size() noexcept {
    return 3;
}

} // namespace pbsd::kernel::geom
