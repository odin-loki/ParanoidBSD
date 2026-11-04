module;

export module pbsd.pkg.verify;

import pbsd.core;
import pbsd.pkg.manifest;
import pbsd.pkg.signature;

/// Burst 13 — package digest/signature verification scaffold.
export namespace pbsd::pkg::verify {

enum class Check : unsigned char {
    Digest = 1,
    Signature = 2,
    Manifest = 4,
};

inline constexpr unsigned kMaxManifestEntries = 64;

struct ManifestBundle {
    unsigned entry_count{0};
    manifest::Entry entries[kMaxManifestEntries]{};
};

struct SignedBundle {
    ManifestBundle manifest{};
    signature::SignatureBlock sig{};
};

[[nodiscard]] inline Status verify_digest(char const* digest_hex) noexcept {
    if (digest_hex == nullptr || digest_hex[0] == '\0') {
        return Status::Invalid;
    }
    unsigned len = 0;
    for (unsigned i = 0; digest_hex[i] != '\0'; ++i) {
        const char c = digest_hex[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return Status::Protocol;
        }
        ++len;
    }
    if (len != 64) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status verify_manifest(ManifestBundle const& m) noexcept {
    if (m.entry_count == 0) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < m.entry_count; ++i) {
        const Status s = manifest::validate_entry(m.entries[i]);
        if (s != Status::Ok) {
            return s;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline Status verify_all(ManifestBundle const& m,
                                       signature::SignatureBlock const& sig,
                                       unsigned checks) noexcept {
    if ((checks & static_cast<unsigned>(Check::Manifest)) != 0) {
        const Status s = verify_manifest(m);
        if (s != Status::Ok) {
            return s;
        }
    }
    if ((checks & static_cast<unsigned>(Check::Signature)) != 0) {
        const Status s = signature::validate_block(sig);
        if (s != Status::Ok) {
            return s;
        }
    }
    return Status::Ok;
}

} // namespace pbsd::pkg::verify
