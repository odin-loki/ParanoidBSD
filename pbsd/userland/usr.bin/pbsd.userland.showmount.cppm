module;

export module pbsd.userland.showmount;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/showmount/showmount.c — RPC mount listing flags.
export namespace pbsd::userland::usr_bin::showmount {

enum class ShowFlag : unsigned {
    None = 0,
    Dump = 1U << 0,
    Exports = 1U << 1,
    ParsableExports = 1U << 2,
};

[[nodiscard]] inline ShowFlag operator|(ShowFlag a, ShowFlag b) noexcept {
    return static_cast<ShowFlag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] inline Result<ShowFlag> parse_show_option(char opt) noexcept {
    switch (opt) {
    case 'd':
        return result_ok(ShowFlag::Dump);
    case 'e':
        return result_ok(ShowFlag::Exports);
    case 'p':
        return result_ok(ShowFlag::ParsableExports);
    default:
        return result_err<ShowFlag>(Status::Invalid);
    }
}

} // namespace pbsd::userland::usr_bin::showmount
