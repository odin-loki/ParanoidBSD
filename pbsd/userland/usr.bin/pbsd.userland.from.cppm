module;
#include <cstddef>

export module pbsd.userland.from;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/from/from.c — mbox From-line parsing (logic-only).
export namespace pbsd::userland::usr_bin::from {

struct Options {
    int list_count{0};
    bool show_count{false};
};

[[nodiscard]] inline bool is_mailbox_char(char c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-' || c == '@';
}

[[nodiscard]] inline bool valid_sender(const char* sender) noexcept {
    if (sender == nullptr || sender[0] == '\0') {
        return false;
    }
    for (const char* p = sender; *p != '\0'; ++p) {
        if (!is_mailbox_char(*p)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline Result<std::size_t> parse_from_header(const char* line, char* sender,
                                                           std::size_t sender_len) noexcept {
    if (line == nullptr || sender == nullptr || sender_len == 0) {
        return result_err<std::size_t>(Status::Invalid);
    }
    while (*line == ' ' || *line == '\t') {
        ++line;
    }
    if (line[0] != 'F' || line[1] != 'r' || line[2] != 'o' || line[3] != 'm' ||
        line[4] != ' ') {
        return result_err<std::size_t>(Status::Invalid);
    }
    line += 5;
    std::size_t i = 0;
    while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') {
        if (i + 1 >= sender_len) {
            return result_err<std::size_t>(Status::NoMemory);
        }
        sender[i++] = *line++;
    }
    sender[i] = '\0';
    if (!valid_sender(sender)) {
        return result_err<std::size_t>(Status::Invalid);
    }
    return result_ok(i);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    if (argc > 2) {
        return result_err<Options>(Status::Invalid);
    }
    if (argc == 2) {
        if (argv[1][0] != '-' || argv[1][1] != 'c' || argv[1][2] != '\0') {
            return result_err<Options>(Status::Invalid);
        }
        opt.show_count = true;
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::from
