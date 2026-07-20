module;

export module pbsd.userland.ipcrm;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/ipcrm/ipcrm.c — SysV IPC object type letters.
export namespace pbsd::userland::usr_bin::ipcrm {

enum class IpcKind : char { MessageQueue = 'q', Semaphore = 's', SharedMem = 'm' };

[[nodiscard]] inline Result<IpcKind> parse_kind(char flag) noexcept {
    switch (flag) {
    case 'q':
    case 'Q':
        return result_ok(IpcKind::MessageQueue);
    case 's':
    case 'S':
        return result_ok(IpcKind::Semaphore);
    case 'm':
    case 'M':
        return result_ok(IpcKind::SharedMem);
    default:
        return result_err<IpcKind>(Status::Invalid);
    }
}

[[nodiscard]] inline const char* kind_string(IpcKind k) noexcept {
    switch (k) {
    case IpcKind::MessageQueue:
        return "msqid";
    case IpcKind::Semaphore:
        return "semid";
    case IpcKind::SharedMem:
        return "shmid";
    }
    return "unknown";
}

} // namespace pbsd::userland::usr_bin::ipcrm
