export module pbsd.userland.column;

export import pbsd.userland.hosted;

/// Wave 2 — column(1) layout stubs from usr.bin/column/.
export namespace pbsd::userland::column {

struct Options {
    unsigned width{80};
    char separator{'\t'};
    bool fill_rows{};
    bool table{};
};

[[nodiscard]] constexpr unsigned columns_for(unsigned term_width, unsigned col_width) noexcept {
    if (col_width == 0 || term_width < col_width) {
        return 1;
    }
    return term_width / col_width;
}

[[nodiscard]] constexpr Status validate(const Options& o) noexcept {
    if (o.width == 0 || o.width > 512) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::column
