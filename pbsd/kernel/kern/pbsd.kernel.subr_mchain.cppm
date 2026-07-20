module;

export module pbsd.kernel.subr_mchain;

import pbsd.core;
import pbsd.kernel.mchain;

/// Freestanding port of `kern/subr_mchain.c` — mbuf chain splice helpers.
export namespace pbsd::kernel::subr_mchain {

[[nodiscard]] inline Status validate_adj(mchain::ChainMeta& cm, int req_len) noexcept {
    if (req_len < 0) {
        return Status::Invalid;
    }
    if (static_cast<unsigned>(req_len) > cm.length) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status adj_head(mchain::ChainMeta& cm, int len) noexcept {
    if (validate_adj(cm, len) != Status::Ok) {
        return Status::Invalid;
    }
    cm.length -= static_cast<unsigned>(len);
    return Status::Ok;
}

[[nodiscard]] inline Status adj_tail(mchain::ChainMeta& cm, int len) noexcept {
    return adj_head(cm, len);
}

[[nodiscard]] inline Status concat(mchain::ChainMeta& dst,
                                   const mchain::ChainMeta& src) noexcept {
    if (src.length == 0) {
        return Status::Ok;
    }
    const unsigned total = dst.length + src.length;
    if (mchain::validate_length(total) != Status::Ok) {
        return Status::Denied;
    }
    dst.length = total;
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_mchain
