#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate Wave 9 burst-9 net/fs/geom/zfs/kde/theme C++23 modules."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd"

MODULES: dict[str, str] = {
    "net/pbsd.net.stf.cppm": """module;
#include <cstdint>

export module pbsd.net.stf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_stf.h — 6to4/STF tunnel ioctls.
export namespace pbsd::net::stf {

enum class Ioctl : int {
    Sv4Net = 1,
    Gv4Net = 2,
    Sbr    = 3,
};

struct V4Args {
    unsigned src{};
    unsigned braddr{};
    int v4_prefixlen{};
};

[[nodiscard]] inline Status validate_prefixlen(int len) noexcept {
    if (len < 0 || len > 32) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_args(V4Args const& a) noexcept {
    return validate_prefixlen(a.v4_prefixlen);
}

} // namespace pbsd::net::stf
""",
    "net/pbsd.net.wg.cppm": """module;
#include <cstddef>
#include <cstdint>

export module pbsd.net.wg;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/wg/if_wg.h — WireGuard key and ioctl constants.
export namespace pbsd::net::wg {

inline constexpr std::size_t kKeySize = 32;
inline constexpr unsigned kAllowedIpRemoveMe = 0x0001;
inline constexpr unsigned kAllowedIpValidFlags = kAllowedIpRemoveMe;

enum class Ioctl : unsigned int {
    Set = 210,
    Get = 211,
};

struct DataIo {
    char name[16]{};
    void* data{};
    std::size_t size{};
};

[[nodiscard]] inline Status validate_key_len(std::size_t len) noexcept {
    if (len != kKeySize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_allowedip_flags(unsigned flags) noexcept {
    if ((flags & ~kAllowedIpValidFlags) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::wg
""",
    "net/pbsd.net.pfsync.cppm": """module;
#include <cstdint>

export module pbsd.net.pfsync;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/pfvar.h, netinet/in.h — PFSYNC protocol constants.
export namespace pbsd::net::pfsync {

inline constexpr unsigned char kIpProto = 240;
inline constexpr unsigned kGroupAddr = 0xe00000f0; // 224.0.0.240
inline constexpr unsigned kModVer = 1;
inline constexpr unsigned kMinVer = 1;
inline constexpr unsigned kMaxVer = 1;

enum class Action : unsigned char {
    Inserts = 0,
    InsAck  = 1,
    UpdC    = 2,
    UpdAck  = 3,
    DelC    = 4,
    DelAck  = 5,
    Clear   = 6,
    ClearAck = 7,
};

[[nodiscard]] inline Status validate_version(unsigned ver) noexcept {
    if (ver < kMinVer || ver > kMaxVer) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_action(Action a) noexcept {
    switch (a) {
    case Action::Inserts:
    case Action::InsAck:
    case Action::UpdC:
    case Action::UpdAck:
    case Action::DelC:
    case Action::DelAck:
    case Action::Clear:
    case Action::ClearAck:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::net::pfsync
""",
    "fs/pbsd.fs.isofs.cppm": """module;
#include <cstdint>

export module pbsd.fs.isofs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/isofs/cd9660/iso.h — ISO9660 primary volume descriptor.
export namespace pbsd::fs::isofs {

inline constexpr unsigned kSectorSize = 2048;
inline constexpr unsigned char kVdPrimary = 1;
inline constexpr unsigned char kVdSupplementary = 2;
inline constexpr unsigned char kVdTerminator = 255;

struct VolumeDesc {
    unsigned char type{kVdPrimary};
    char standard_id[5]{'C', 'D', '0', '0', '1'};
    unsigned char version{1};
};

[[nodiscard]] inline Status validate(const VolumeDesc& v) noexcept {
    if (v.version == 0) {
        return Status::Invalid;
    }
    if (v.standard_id[0] != 'C' || v.standard_id[1] != 'D') {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::isofs
""",
    "fs/pbsd.fs.autofs.cppm": """module;
#include <cstdint>

export module pbsd.fs.autofs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/autofs/autofs.h — autofs mount map types.
export namespace pbsd::fs::autofs {

inline constexpr unsigned kMaxPathLen = 1024;
inline constexpr unsigned kMaxKeyLen = 256;

enum class MapType : unsigned char {
    Indirect = 0,
    Direct   = 1,
    Master   = 2,
};

enum class Request : unsigned char {
    Mount   = 0,
    Unmount = 1,
};

[[nodiscard]] inline Status validate_map_type(MapType t) noexcept {
    switch (t) {
    case MapType::Indirect:
    case MapType::Direct:
    case MapType::Master:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::autofs
""",
    "geom/pbsd.geom.cache.cppm": """module;
#include <cstdint>

export module pbsd.geom.cache;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/cache/g_cache.h — GEOM CACHE metadata.
export namespace pbsd::geom::cache {

inline constexpr unsigned kVersion = 1;
inline constexpr unsigned kBuckets = 8;

enum class Type : unsigned char {
    Manual    = 0,
    Automatic = 1,
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    switch (t) {
    case Type::Manual:
    case Type::Automatic:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline unsigned bucket(unsigned bno) noexcept {
    return bno & (kBuckets - 1);
}

} // namespace pbsd::geom::cache
""",
    "geom/pbsd.geom.shsec.cppm": """module;
#include <cstdint>

export module pbsd.geom.shsec;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/shsec/g_shsec.h — GEOM shared-secret metadata.
export namespace pbsd::geom::shsec {

inline constexpr unsigned kVersion = 0;
inline constexpr unsigned kMagicLen = 16;

enum class State : unsigned char {
    New    = 0,
    Active = 1,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::New:
    case State::Active:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::geom::shsec
""",
    "geom/pbsd.geom.virstor.cppm": """module;
#include <cstdint>

export module pbsd.geom.virstor;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/virstor/g_virstor.h — GEOM VIRSTOR metadata.
export namespace pbsd::geom::virstor {

inline constexpr unsigned kVersion = 1;

enum class State : unsigned char {
    New    = 0,
    Active = 1,
    Full   = 2,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::New:
    case State::Active:
    case State::Full:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_chunk_size(unsigned bytes) noexcept {
    if (bytes < 512 || (bytes % 512) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::virstor
""",
    "zfs/pbsd.zfs.zap.cppm": """module;
#include <cstdint>

export module pbsd.zfs.zap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zap.h — ZAP name/value limits.
export namespace pbsd::zfs::zap {

inline constexpr unsigned kMaxNameLen = 256;
inline constexpr unsigned kMaxValueLen = 8192;
inline constexpr unsigned kChunkGap = 24;

enum class Type : unsigned char {
    Micro = 0,
    Fat   = 1,
};

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len >= kMaxNameLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_value_len(unsigned len) noexcept {
    if (len > kMaxValueLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::zap
""",
    "zfs/pbsd.zfs.zil.cppm": """module;
#include <cstdint>

export module pbsd.zfs.zil;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zil.h — ZIL block types.
export namespace pbsd::zfs::zil {

inline constexpr unsigned kHeaderSize = 128;
inline constexpr unsigned kMaxBlockSize = 131072;

enum class BlockType : unsigned char {
    Header = 1,
    Write  = 2,
    Free   = 3,
    Intent = 4,
};

enum class State : unsigned char {
    Initial = 0,
    Active  = 1,
    Closed  = 2,
};

[[nodiscard]] inline Status validate_block_type(BlockType t) noexcept {
    switch (t) {
    case BlockType::Header:
    case BlockType::Write:
    case BlockType::Free:
    case BlockType::Intent:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::zfs::zil
""",
    "zfs/pbsd.zfs.l2arc.cppm": """module;
#include <cstdint>

export module pbsd.zfs.l2arc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/arc_impl.h — L2ARC header constants.
export namespace pbsd::zfs::l2arc {

inline constexpr unsigned kHeaderSize = 4096;
inline constexpr unsigned kDevBlockSize = 4096;

enum class Flag : unsigned int {
    Enabled = 1u << 0,
    Feed    = 1u << 1,
    Write   = 1u << 2,
};

[[nodiscard]] inline Status validate_dev_size(unsigned long long bytes) noexcept {
    if (bytes < kDevBlockSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool flag_has(Flag f, Flag bit) noexcept {
    return (static_cast<unsigned>(f) & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::zfs::l2arc
""",
    "kde/frameworks/kguiaddons.kcolorscheme.cppm": """export module pbsd.kde.frameworks.kguiaddons.kcolorscheme;

import pbsd.core;

/// Wave 9 burst — KColorScheme semantic roles.
/// Upstream: kde/frameworks/kguiaddons/src/colorscheme/kcolorscheme.cpp
export namespace pbsd::kde::frameworks::kguiaddons::kcolorscheme {

enum class ColorSet : unsigned char {
    Window,
    Button,
    View,
    Selection,
    Tooltip,
    Complementary,
};

enum class ShadeRole : unsigned char {
    Background,
    Foreground,
    Decoration,
};

inline constexpr int kMaxShades = 7;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kguiaddons/src/colorscheme/kcolorscheme.cpp";
}

} // namespace pbsd::kde::frameworks::kguiaddons::kcolorscheme
""",
    "kde/frameworks/kirigami.units.cppm": """export module pbsd.kde.frameworks.kirigami.units;

import pbsd.core;

/// Wave 9 burst — Kirigami grid unit constants.
/// Upstream: kde/frameworks/kirigami/src/units.cpp
export namespace pbsd::kde::frameworks::kirigami::units {

inline constexpr int kGridUnit = 8;
inline constexpr int kSmallSpacing = 4;
inline constexpr int kLargeSpacing = 16;
inline constexpr float kDefaultFontSize = 10.0f;

[[nodiscard]] inline int gridUnits(int n) noexcept {
    return n * kGridUnit;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kirigami/src/units.cpp";
}

} // namespace pbsd::kde::frameworks::kirigami::units
""",
    "kde/plasma.volume.cppm": """export module pbsd.kde.plasma.volume;

import pbsd.core;

/// Wave 9 burst — Plasma volume applet constants.
/// Upstream: kde/plasma-workspace/applets/volume/volume.cpp
export namespace pbsd::kde::plasma::volume {

inline constexpr int kDefaultVolume = 50;
inline constexpr int kMaxVolume = 100;
inline constexpr int kSliderWidth = 120;

[[nodiscard]] inline Status validate_volume(int v) noexcept {
    if (v < 0 || v > kMaxVolume) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/applets/volume/volume.cpp";
}

} // namespace pbsd::kde::plasma::volume
""",
    "kde/plasma.mediacontroller.cppm": """export module pbsd.kde.plasma.mediacontroller;

import pbsd.core;

/// Wave 9 burst — Plasma media controller MPRIS roles.
/// Upstream: kde/plasma-workspace/applets/mediacontroller/mediacontroller.cpp
export namespace pbsd::kde::plasma::mediacontroller {

enum class PlaybackStatus : unsigned char {
    Playing,
    Paused,
    Stopped,
};

inline constexpr unsigned kDefaultPollMs = 1000;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/applets/mediacontroller/mediacontroller.cpp";
}

} // namespace pbsd::kde::plasma::mediacontroller
""",
    "kde/kwin/effects/fade.cppm": """export module pbsd.kde.kwin.effects.fade;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 9 burst — KWin fade effect stub.
/// Upstream: kde/kwin/src/plugins/fade/main.cpp
export namespace pbsd::kde::kwin::effects::fade {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "kwin_effect_fade",
    "Fade",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/fade/main.cpp",
};

inline constexpr const char kEffectId[] = "fade";
inline constexpr const char kDisplayName[] = "Fade";
inline constexpr const char kPluginCategory[] = "Appearance";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/fade/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::fade
""",
    "kde/kwin/effects/dialogparent.cppm": """export module pbsd.kde.kwin.effects.dialogparent;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 9 burst — KWin dialog parent effect stub.
/// Upstream: kde/kwin/src/plugins/dialogparent/main.cpp
export namespace pbsd::kde::kwin::effects::dialogparent {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "kwin_effect_dialogparent",
    "Dialog Parent",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/dialogparent/main.cpp",
};

inline constexpr const char kEffectId[] = "dialogparent";
inline constexpr const char kDisplayName[] = "Dialog Parent";
inline constexpr const char kPluginCategory[] = "Accessibility";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/dialogparent/main.cpp";
}

} // namespace pbsd::kde::kwin::effects::dialogparent
""",
    "theme/plasma/aero.panel.cppm": """export module pbsd.theme.plasma.aero.panel;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 9 burst — Aero panel chrome metrics.
export namespace pbsd::theme::plasma::aero::panel {

inline constexpr int kHeight = 40;
inline constexpr int kIconSize = 32;
inline constexpr int kSpacing = 4;
inline constexpr float kOpacity = 0.72f;

[[nodiscard]] inline int height_from_blur(const ::pbsd::kde::plasma::aero::BlurParams& p) noexcept {
    return p.title_bar_height + kSpacing;
}

} // namespace pbsd::theme::plasma::aero::panel
""",
    "theme/plasma/aero.notification.cppm": """export module pbsd.theme.plasma.aero.notification;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 9 burst — Aero notification popup metrics.
export namespace pbsd::theme::plasma::aero::notification {

inline constexpr int kWidth = 360;
inline constexpr int kMinHeight = 80;
inline constexpr int kCornerRadius = 8;
inline constexpr float kOpacity = 0.85f;

[[nodiscard]] inline float opacity_from_blur(const ::pbsd::kde::plasma::aero::BlurParams& p) noexcept {
    return p.dialog_opacity;
}

} // namespace pbsd::theme::plasma::aero::notification
""",
    "theme/plasma/aero.dialog.cppm": """export module pbsd.theme.plasma.aero.dialog;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 9 burst — Aero dialog frame metrics.
export namespace pbsd::theme::plasma::aero::dialog {

inline constexpr int kTitleHeight = 32;
inline constexpr int kButtonSize = 24;
inline constexpr int kPadding = 12;
inline constexpr int kCornerRadius = 8;

[[nodiscard]] inline int corner_from_blur(const ::pbsd::kde::plasma::aero::BlurParams& p) noexcept {
    return p.corner_radius;
}

} // namespace pbsd::theme::plasma::aero::dialog
""",
    "theme/plasma/aero.icons.cppm": """export module pbsd.theme.plasma.aero.icons;

import pbsd.core;

/// Wave 9 burst — Aero icon sizing constants.
export namespace pbsd::theme::plasma::aero::icons {

inline constexpr int kSmall = 16;
inline constexpr int kMedium = 22;
inline constexpr int kLarge = 32;
inline constexpr int kHuge = 48;

enum class Context : unsigned char {
    Panel,
    StartMenu,
    Dialog,
    Notification,
};

[[nodiscard]] inline int size_for(Context ctx) noexcept {
    switch (ctx) {
    case Context::Panel: return kMedium;
    case Context::StartMenu: return kLarge;
    case Context::Dialog: return kMedium;
    case Context::Notification: return kSmall;
    default: return kMedium;
    }
}

} // namespace pbsd::theme::plasma::aero::icons
""",
}


def main() -> None:
    created = 0
    for rel, content in MODULES.items():
        path = ROOT / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        if not path.exists() or path.read_text(encoding="utf-8") != content:
            path.write_text(content, encoding="utf-8", newline="\n")
            created += 1
    print(f"Wrote/updated {created} of {len(MODULES)} module files")


if __name__ == "__main__":
    main()
