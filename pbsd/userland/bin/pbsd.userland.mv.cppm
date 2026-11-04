module;
#include <cstddef>

export module pbsd.userland.mv;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/mv/mv.c — option parsing and destination path helpers.
export namespace pbsd::userland::bin::mv {

struct Options {
    bool force{false};
    bool no_clobber{false};
    bool interactive{false};
    bool no_target_dereference{false}; // -h
    bool verbose{false};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'f':
                opt.force = true;
                opt.interactive = false;
                opt.no_clobber = false;
                break;
            case 'h':
                opt.no_target_dereference = true;
                break;
            case 'i':
                opt.interactive = true;
                opt.force = false;
                opt.no_clobber = false;
                break;
            case 'n':
                opt.no_clobber = true;
                opt.force = false;
                opt.interactive = false;
                break;
            case 'v':
                opt.verbose = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }
    if (argc - i < 2) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline std::size_t basename_component_length(const char* base) noexcept {
    if (base == nullptr) {
        return 0;
    }
    std::size_t len = 0;
    while (base[len] != '\0' && base[len] != '/') {
        ++len;
    }
    return len;
}

[[nodiscard]] inline const char* trailing_basename(const char* path) noexcept {
    if (path == nullptr) {
        return nullptr;
    }
    std::size_t len = hosted::cstrlen(path);
    while (len > 1 && path[len - 1] == '/') {
        --len;
    }
    const char* end = path + len;
    const char* base = end;
    while (base > path && base[-1] != '/') {
        --base;
    }
    return base;
}

[[nodiscard]] inline bool basename_matches(const char* path, const char* name) noexcept {
    const char* base = trailing_basename(path);
    const std::size_t blen = basename_component_length(base);
    const std::size_t nlen = hosted::cstrlen(name);
    if (blen != nlen) {
        return false;
    }
    for (std::size_t i = 0; i < blen; ++i) {
        if (base[i] != name[i]) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool join_into_dir(char* dest, std::size_t dest_len,
                                        const char* dir, const char* source) noexcept {
    if (dest == nullptr || dir == nullptr || source == nullptr) {
        return false;
    }
    const char* base = trailing_basename(source);
    std::size_t dlen = hosted::cstrlen(dir);
    std::size_t blen = basename_component_length(base);
    std::size_t need = dlen + 1 + blen + 1;
    if (dlen > 0 && dir[dlen - 1] == '/') {
        need = dlen + blen + 1;
    }
    if (need > dest_len) {
        return false;
    }
    std::size_t pos = 0;
    for (std::size_t i = 0; i < dlen; ++i) {
        dest[pos++] = dir[i];
    }
    if (dlen == 0 || dir[dlen - 1] != '/') {
        dest[pos++] = '/';
    }
    for (std::size_t i = 0; i < blen; ++i) {
        dest[pos++] = base[i];
    }
    dest[pos] = '\0';
    return true;
}

[[nodiscard]] inline bool target_is_directory(int argc, int arg_index,
                                              bool target_is_dir) noexcept {
    if (argc - arg_index > 2) {
        return true;
    }
    return target_is_dir;
}

} // namespace pbsd::userland::bin::mv
