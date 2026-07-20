module;

export module pbsd.userland.colrm;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/colrm/colrm.c — column removal filter.
export namespace pbsd::userland::usr_bin::colrm {

inline constexpr int kTab = 8;

struct Options {
    unsigned long start{0};
    unsigned long stop{0};
};

[[nodiscard]] inline Result<Options> parse_columns(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    optind_out = i;
    switch (argc - i) {
    case 2:
        opt.stop = 1;
        break;
    case 1:
        break;
    case 0:
        return result_ok(opt);
    default:
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool valid_column_range(unsigned long start,
                                             unsigned long stop) noexcept {
    if (start == 0) {
        return false;
    }
    if (stop != 0 && start > stop) {
        return false;
    }
    return true;
}

[[nodiscard]] inline bool column_in_range(unsigned long column, unsigned long start,
                                          unsigned long stop) noexcept {
    if (column < start) {
        return false;
    }
    if (stop == 0) {
        return true;
    }
    return column <= stop;
}

} // namespace pbsd::userland::usr_bin::colrm
