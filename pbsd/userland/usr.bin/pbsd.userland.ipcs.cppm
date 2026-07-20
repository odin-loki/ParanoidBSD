module;

export module pbsd.userland.ipcs;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/ipcs/ipcs.c — IPC listing format flags.
export namespace pbsd::userland::usr_bin::ipcs {

enum class ListFlag : unsigned {
    None = 0,
    MessageQueues = 1U << 0,
    Semaphores = 1U << 1,
    SharedMem = 1U << 2,
    All = MessageQueues | Semaphores | SharedMem,
};

[[nodiscard]] inline ListFlag operator|(ListFlag a, ListFlag b) noexcept {
    return static_cast<ListFlag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] inline bool has_flag(ListFlag set, ListFlag bit) noexcept {
    return (static_cast<unsigned>(set) & static_cast<unsigned>(bit)) != 0U;
}

[[nodiscard]] inline Result<ListFlag> parse_list_option(char opt) noexcept {
    switch (opt) {
    case 'q':
        return result_ok(ListFlag::MessageQueues);
    case 's':
        return result_ok(ListFlag::Semaphores);
    case 'm':
        return result_ok(ListFlag::SharedMem);
    case 'a':
        return result_ok(ListFlag::All);
    default:
        return result_err<ListFlag>(Status::Invalid);
    }
}

} // namespace pbsd::userland::usr_bin::ipcs
