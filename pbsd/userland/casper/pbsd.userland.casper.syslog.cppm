export module pbsd.userland.casper.syslog;

export import pbsd.userland.casper;
import pbsd.core;

/// cap_syslog service surface from hbsd/src/lib/libcasper/services/cap_syslog/
export namespace pbsd::userland::casper::syslog {

inline constexpr const char* kServiceName = "system.syslog";

inline constexpr int kLogEmerg   = 0;
inline constexpr int kLogAlert   = 1;
inline constexpr int kLogCrit    = 2;
inline constexpr int kLogErr     = 3;
inline constexpr int kLogWarning = 4;
inline constexpr int kLogNotice  = 5;
inline constexpr int kLogInfo    = 6;
inline constexpr int kLogDebug   = 7;

[[nodiscard]] inline bool priority_valid(int pri) noexcept {
    return pri >= kLogEmerg && pri <= kLogDebug;
}

[[nodiscard]] inline Result<ChannelHandle>
open_syslog_service(LineageTree& tree, CapabilityRights rights) noexcept {
    return open_service(tree, rights, kServiceName);
}

[[nodiscard]] inline Status syslog_limit_priority(ChannelHandle& chan,
                                                  int min_pri,
                                                  int max_pri) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return Status::Invalid;
    }
    if (!priority_valid(min_pri) || !priority_valid(max_pri)) {
        return Status::Invalid;
    }
    if (min_pri > max_pri) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status syslog_limit_facility(ChannelHandle& chan,
                                                  int facility) noexcept {
    if (!chan.valid() || !channel_valid(chan.peek())) {
        return Status::Invalid;
    }
    if (facility < 0 || facility > 23) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::casper::syslog
