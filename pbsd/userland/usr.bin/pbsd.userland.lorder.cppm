module;
#include <cstddef>

export module pbsd.userland.lorder;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lorder/lorder.c — link-order parsing (logic-only).
export namespace pbsd::userland::usr_bin::lorder {

struct SymbolRef {
    const char* object{nullptr};
    const char* symbol{nullptr};
};

[[nodiscard]] inline int compare_symbols(const char* a, const char* b) noexcept {
    return hosted::cstrcmp(a, b);
}

[[nodiscard]] inline Result<SymbolRef> parse_line(const char* line, char* obj_buf,
                                                  std::size_t obj_len, char* sym_buf,
                                                  std::size_t sym_len) noexcept {
    if (line == nullptr || obj_buf == nullptr || sym_buf == nullptr) {
        return result_err<SymbolRef>(Status::Invalid);
    }
    std::size_t o = 0;
    const char* p = line;
    while (*p != '\0' && *p != ' ' && *p != '\t') {
        if (o + 1 >= obj_len) {
            return result_err<SymbolRef>(Status::NoMemory);
        }
        obj_buf[o++] = *p++;
    }
    obj_buf[o] = '\0';
    while (*p == ' ' || *p == '\t') {
        ++p;
    }
    std::size_t s = 0;
    while (*p != '\0' && *p != '\n') {
        if (s + 1 >= sym_len) {
            return result_err<SymbolRef>(Status::NoMemory);
        }
        sym_buf[s++] = *p++;
    }
    sym_buf[s] = '\0';
    if (obj_buf[0] == '\0' || sym_buf[0] == '\0') {
        return result_err<SymbolRef>(Status::Invalid);
    }
    SymbolRef ref{};
    ref.object = obj_buf;
    ref.symbol = sym_buf;
    return result_ok(ref);
}

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv,
                                            int& optind_out) noexcept {
    if (argv == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    optind_out = 1;
    if (argc < 2) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(0);
}

} // namespace pbsd::userland::usr_bin::lorder
