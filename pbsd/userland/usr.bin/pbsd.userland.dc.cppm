export module pbsd.userland.dc;

export import pbsd.userland.hosted;

/// Wave 2 — dc(1) option stubs from usr.bin/dc/.
export namespace pbsd::userland::dc {

inline constexpr int kDefaultIbase = 10;
inline constexpr int kDefaultObase = 10;

struct Options {
    int ibase{kDefaultIbase};
    int obase{kDefaultObase};
    bool extended{};
};

[[nodiscard]] constexpr Status validate(const Options& o) noexcept {
    if (o.ibase < 2 || o.ibase > 16) {
        return Status::Invalid;
    }
    if (o.obase < 2 || o.obase > 16) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::dc
