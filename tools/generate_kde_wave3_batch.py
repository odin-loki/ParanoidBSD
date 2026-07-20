#!/usr/bin/env python3
"""Generate Wave 3 KDE hand-port module stubs under pbsd/kde/."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd" / "kde"

# Module bodies use NUL escapes as \\0 in source; replaced at write time.
MODULES: dict[str, str] = {}


def add(name: str, body: str) -> None:
    MODULES[name] = body


add(
    "frameworks/kconfig.kdesktopfileaction.cppm",
    r"""module;

#include <cstring>

export module pbsd.kde.frameworks.kconfig.kdesktopfileaction;

import pbsd.core;

/// Wave 3 — desktop file action separator + field keys.
/// Upstream: kde/frameworks/kconfig/src/core/kdesktopfileaction.cpp
export namespace pbsd::kde::frameworks::kconfig::kdesktopfileaction {

inline constexpr const char kSeparatorKey[] = "_SEPARATOR_";
inline constexpr unsigned kMaxFieldLen = 512;

struct Action {
    char internal_key[kMaxFieldLen]{};
    char name[kMaxFieldLen]{};
    char icon[kMaxFieldLen]{};
    char exec[kMaxFieldLen]{};
    char desktop_path[kMaxFieldLen]{};
};

[[nodiscard]] inline bool is_separator(const char* key) noexcept {
    if (key == nullptr) {
        return false;
    }
    return std::strcmp(key, kSeparatorKey) == 0;
}

[[nodiscard]] inline Status init_action(Action& a, const char* key, const char* name,
                                        const char* icon, const char* exec,
                                        const char* path) noexcept {
    if (key == nullptr || name == nullptr) {
        return Status::Invalid;
    }
    std::strncpy(a.internal_key, key, kMaxFieldLen - 1);
    std::strncpy(a.name, name, kMaxFieldLen - 1);
    if (icon != nullptr) {
        std::strncpy(a.icon, icon, kMaxFieldLen - 1);
    }
    if (exec != nullptr) {
        std::strncpy(a.exec, exec, kMaxFieldLen - 1);
    }
    if (path != nullptr) {
        std::strncpy(a.desktop_path, path, kMaxFieldLen - 1);
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/core/kdesktopfileaction.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kdesktopfileaction
""",
)

add(
    "frameworks/kconfig.kconfigskeleton.cppm",
    r"""export module pbsd.kde.frameworks.kconfig.kconfigskeleton;

import pbsd.core;

/// Wave 3 — KConfigSkeleton item type tags (from kconfigskeleton.cpp).
/// Upstream: kde/frameworks/kconfig/src/gui/kconfigskeleton.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfigskeleton {

enum class ItemKind : unsigned char { Color, Font, Generic, Unknown };

inline constexpr unsigned kMaxItemNameLen = 128;
inline constexpr unsigned kMaxGroupLen = 128;
inline constexpr unsigned kMaxKeyLen = 128;

struct ItemRef {
    ItemKind kind{ItemKind::Unknown};
    char group[kMaxGroupLen]{};
    char key[kMaxKeyLen]{};
    char name[kMaxItemNameLen]{};
};

[[nodiscard]] inline ItemKind kind_for_name(const char* name) noexcept {
    if (name == nullptr) {
        return ItemKind::Unknown;
    }
    if (name[0] == 'C' && name[1] == 'o' && name[2] == 'l' && name[3] == 'o' && name[4] == 'r') {
        return ItemKind::Color;
    }
    if (name[0] == 'F' && name[1] == 'o' && name[2] == 'n' && name[3] == 't') {
        return ItemKind::Font;
    }
    return ItemKind::Generic;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kconfigskeleton.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfigskeleton
""",
)

add(
    "frameworks/kconfig.kconfigwatcher.cppm",
    r"""export module pbsd.kde.frameworks.kconfig.kconfigwatcher;

import pbsd.core;

/// Wave 3 — KConfigWatcher group/key notification constants.
/// Upstream: kde/frameworks/kconfig/src/core/kconfigwatcher.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfigwatcher {

inline constexpr const char kShortcutsGroup[] = "Shortcuts";
inline constexpr const char kGeneralGroup[] = "General";
inline constexpr unsigned kMaxKeys = 64;
inline constexpr unsigned kMaxKeyLen = 128;

struct ChangeSet {
    char group[128]{};
    char keys[kMaxKeys][kMaxKeyLen]{};
    unsigned key_count{0};
};

[[nodiscard]] inline bool is_shortcuts_group(const char* group) noexcept {
    if (group == nullptr) {
        return false;
    }
    return group[0] == 'S' && group[1] == 'h' && group[2] == 'o' && group[3] == 'r'
        && group[4] == 't' && group[5] == 'c' && group[6] == 'u' && group[7] == 't'
        && group[8] == 's' && group[9] == '\0';
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/core/kconfigwatcher.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfigwatcher
""",
)

add(
    "frameworks/kconfig.kstandardshortcutwatcher.cppm",
    r"""export module pbsd.kde.frameworks.kconfig.kstandardshortcutwatcher;

import pbsd.core;
import pbsd.kde.frameworks.kconfig.kconfigwatcher;

/// Wave 3 — standard shortcut watcher config group filter.
/// Upstream: kde/frameworks/kconfig/src/gui/kstandardshortcutwatcher.cpp
export namespace pbsd::kde::frameworks::kconfig::kstandardshortcutwatcher {

inline constexpr const char kConfigFile[] = "kdeglobals";

[[nodiscard]] inline bool should_handle_group(const char* group) noexcept {
    return kconfigwatcher::is_shortcuts_group(group);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kstandardshortcutwatcher.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kstandardshortcutwatcher
""",
)

add(
    "frameworks/kconfig.kwindowstatesaver.cppm",
    r"""export module pbsd.kde.frameworks.kconfig.kwindowstatesaver;

import pbsd.core;

/// Wave 3 — window geometry/state persistence keys.
/// Upstream: kde/frameworks/kconfig/src/gui/kwindowstatesaver.cpp
export namespace pbsd::kde::frameworks::kconfig::kwindowstatesaver {

inline constexpr const char kWidthKey[] = "Width";
inline constexpr const char kHeightKey[] = "Height";
inline constexpr const char kXKey[] = "X";
inline constexpr const char kYKey[] = "Y";
inline constexpr const char kMaximizedKey[] = "Maximized";
inline constexpr const char kFullscreenKey[] = "Fullscreen";

struct Geometry {
    int x{0};
    int y{0};
    int width{0};
    int height{0};
    bool maximized{false};
    bool fullscreen{false};
};

[[nodiscard]] inline bool geometry_valid(const Geometry& g) noexcept {
    return g.width > 0 && g.height > 0;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kwindowstatesaver.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kwindowstatesaver
""",
)

add(
    "frameworks/kconfig.kwindowstatesaverquick.cppm",
    r"""export module pbsd.kde.frameworks.kconfig.kwindowstatesaverquick;

import pbsd.core;
import pbsd.kde.frameworks.kconfig.kwindowstatesaver;

/// Wave 3 — QML window state saver bridge constants.
/// Upstream: kde/frameworks/kconfig/src/qml/kwindowstatesaverquick.cpp
export namespace pbsd::kde::frameworks::kconfig::kwindowstatesaverquick {

inline constexpr const char kQmlModule[] = "org.kde.kconfig";
inline constexpr const char kTypeName[] = "KWindowStateSaver";

[[nodiscard]] inline const char* width_key() noexcept { return kwindowstatesaver::kWidthKey; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/qml/kwindowstatesaverquick.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kwindowstatesaverquick
""",
)

add(
    "frameworks/kconfig.kconfigpropertymap.cppm",
    r"""export module pbsd.kde.frameworks.kconfig.kconfigpropertymap;

import pbsd.core;

/// Wave 3 — KConfigPropertyMap QML bridge keys.
/// Upstream: kde/frameworks/kconfig/src/qml/kconfigpropertymap.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfigpropertymap {

inline constexpr const char kQmlModule[] = "org.kde.kconfig";
inline constexpr const char kTypeName[] = "KConfigPropertyMap";
inline constexpr unsigned kMaxProperties = 128;
inline constexpr unsigned kMaxKeyLen = 128;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/qml/kconfigpropertymap.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfigpropertymap
""",
)

add(
    "frameworks/kconfig.kconfiggui.cppm",
    r"""export module pbsd.kde.frameworks.kconfig.kconfiggui;

import pbsd.core;

/// Wave 3 — KConfigGui module identity (from kconfiggui.cpp).
/// Upstream: kde/frameworks/kconfig/src/gui/kconfiggui.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfiggui {

inline constexpr const char kModuleName[] = "KConfigGui";
inline constexpr const char kVersion[] = "6.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kconfiggui.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfiggui
""",
)

add(
    "frameworks/kconfig.kemailsettings.cppm",
    r"""export module pbsd.kde.frameworks.kconfig.kemailsettings;

import pbsd.core;

/// Wave 3 — KEMailSettings profile keys (from kemailsettings.cpp).
/// Upstream: kde/frameworks/kconfig/src/core/kemailsettings.cpp
export namespace pbsd::kde::frameworks::kconfig::kemailsettings {

enum class Setting : unsigned char {
    ClientProgram,
    ClientTerminal,
    RealName,
    EmailAddress,
    ReplyToAddress,
    Organization,
};

inline constexpr const char kProfilePrefix[] = "PROFILE_";
inline constexpr const char kEmailClientKey[] = "EmailClient";
inline constexpr const char kTerminalClientKey[] = "TerminalClient";
inline constexpr const char kFullNameKey[] = "FullName";
inline constexpr const char kEmailAddressKey[] = "EmailAddress";
inline constexpr const char kReplyAddrKey[] = "ReplyAddr";
inline constexpr const char kOrganizationKey[] = "Organization";

[[nodiscard]] inline const char* key_for(Setting s) noexcept {
    switch (s) {
    case Setting::ClientProgram: return kEmailClientKey;
    case Setting::ClientTerminal: return kTerminalClientKey;
    case Setting::RealName: return kFullNameKey;
    case Setting::EmailAddress: return kEmailAddressKey;
    case Setting::ReplyToAddress: return kReplyAddrKey;
    case Setting::Organization: return kOrganizationKey;
    }
    return nullptr;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/core/kemailsettings.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kemailsettings
""",
)

add(
    "frameworks/kconfig.kconfparameters.cppm",
    r"""module;

#include <cstring>

export module pbsd.kde.frameworks.kconfig.kconfparameters;

import pbsd.core;

/// Wave 3 — KConfig compiler .kcfgc parameter keys.
/// Upstream: kde/frameworks/kconfig/src/kconfig_compiler/KConfigParameters.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfparameters {

inline constexpr const char kCodegenSuffix[] = ".kcfgc";
inline constexpr const char kDefaultInherits[] = "KConfigSkeleton";
inline constexpr unsigned kSuffixLen = 6;

[[nodiscard]] inline bool has_kcfgc_suffix(const char* path) noexcept {
    if (path == nullptr) {
        return false;
    }
    const unsigned len = static_cast<unsigned>(std::strlen(path));
    if (len < kSuffixLen) {
        return false;
    }
    return std::strcmp(path + len - kSuffixLen, kCodegenSuffix) == 0;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/kconfig_compiler/KConfigParameters.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfparameters
""",
)

add(
    "frameworks/kconfig.kauthorized.cppm",
    r"""export module pbsd.kde.frameworks.kconfig.kauthorized;

import pbsd.core;

/// Wave 3 — KAuthorized action identifiers (hosted constants).
/// Upstream: kde/frameworks/kconfig/src/core/kauthorized.cpp
export namespace pbsd::kde::frameworks::kconfig::kauthorized {

inline constexpr const char kShellAccess[] = "shell_access";
inline constexpr const char kRunCommand[] = "run_command";
inline constexpr const char kRunCommandOnDesktop[] = "run_command_on_desktop";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/core/kauthorized.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kauthorized
""",
)

add(
    "frameworks/kcoreaddons.kstaticpluginhelpers.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kstaticpluginhelpers;

import pbsd.core;

/// Wave 3 — static plugin registry helpers (from kstaticpluginhelpers.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/plugin/kstaticpluginhelpers.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kstaticpluginhelpers {

inline constexpr unsigned kMaxPluginIdLen = 256;
inline constexpr unsigned kMaxPluginsPerDir = 64;

struct PluginSlot {
    char id[kMaxPluginIdLen]{};
    bool occupied{false};
};

struct DirectoryMap {
    PluginSlot slots[kMaxPluginsPerDir]{};
    unsigned count{0};
};

[[nodiscard]] inline Status register_plugin(DirectoryMap& map, const char* id) noexcept {
    if (id == nullptr || map.count >= kMaxPluginsPerDir) {
        return map.count >= kMaxPluginsPerDir ? Status::NoMemory : Status::Invalid;
    }
    auto& slot = map.slots[map.count++];
    for (unsigned i = 0; i < kMaxPluginIdLen - 1 && id[i]; ++i) {
        slot.id[i] = id[i];
    }
    slot.occupied = true;
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/plugin/kstaticpluginhelpers.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kstaticpluginhelpers
""",
)

add(
    "frameworks/kcoreaddons.kformat.cppm",
    r"""module;

#include <cmath>

export module pbsd.kde.frameworks.kcoreaddons.kformat;

import pbsd.core;

/// Wave 3 — byte size dialect + IEC formatting (from KFormat::formatByteSize).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kformat.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kformat {

enum class BinaryUnitDialect : unsigned char { IEC, JEDEC, MetricBinary };

struct ByteSizeResult {
    double value{0.0};
    char unit[8]{};
};

[[nodiscard]] inline ByteSizeResult format_byte_size(double bytes, int precision,
                                                     BinaryUnitDialect dialect) noexcept {
    ByteSizeResult r{};
    if (bytes < 0.0) {
        bytes = -bytes;
    }
    double base = dialect == BinaryUnitDialect::MetricBinary ? 1000.0 : 1024.0;
    double v = bytes;
    unsigned idx = 0;
    while (v >= base && idx < 4) {
        v /= base;
        ++idx;
    }
    const double scale = std::pow(10.0, -precision);
    r.value = std::floor(v / scale + 0.5) * scale;
    const char suffixes[] = {'B', 'K', 'M', 'G', 'T'};
    r.unit[0] = suffixes[idx];
    r.unit[1] = (dialect == BinaryUnitDialect::IEC && idx > 0) ? 'i' : '\0';
    r.unit[2] = 'B';
    r.unit[3] = '\0';
    return r;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kformat.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kformat
""",
)

add(
    "frameworks/kcoreaddons.kcompositejob.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kcompositejob;

import pbsd.core;

/// Wave 3 — composite job subjob registry (from kcompositejob.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/jobs/kcompositejob.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kcompositejob {

inline constexpr unsigned kMaxSubjobs = 32;

struct SubjobRegistry {
    unsigned ids[kMaxSubjobs]{};
    unsigned count{0};
    int first_error{0};
};

[[nodiscard]] inline Status add_subjob(SubjobRegistry& reg, unsigned job_id) noexcept {
    if (reg.count >= kMaxSubjobs) {
        return Status::NoMemory;
    }
    reg.ids[reg.count++] = job_id;
    return Status::Ok;
}

[[nodiscard]] inline Status propagate_error(SubjobRegistry& reg, int error) noexcept {
    if (error != 0 && reg.first_error == 0) {
        reg.first_error = error;
    }
    return reg.first_error != 0 ? Status::Failed : Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/jobs/kcompositejob.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kcompositejob
""",
)

add(
    "frameworks/kcoreaddons.kjobtrackerinterface.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kjobtrackerinterface;

import pbsd.core;

/// Wave 3 — KJobTrackerInterface notification slots.
/// Upstream: kde/frameworks/kcoreaddons/src/lib/jobs/kjobtrackerinterface.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kjobtrackerinterface {

inline constexpr const char kInterfaceIid[] = "org.kde.kjobtrackerinterface/1.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/jobs/kjobtrackerinterface.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kjobtrackerinterface
""",
)

add(
    "frameworks/kcoreaddons.kjobuidelegate.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kjobuidelegate;

import pbsd.core;

/// Wave 3 — KJobUiDelegate capability flags.
/// Upstream: kde/frameworks/kcoreaddons/src/lib/jobs/kjobuidelegate.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kjobuidelegate {

enum class Capability : unsigned char { None, ShowErrorMessage, ShowInfoMessage, All };

[[nodiscard]] inline bool shows_errors(Capability c) noexcept {
    return c == Capability::ShowErrorMessage || c == Capability::All;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/jobs/kjobuidelegate.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kjobuidelegate
""",
)

add(
    "frameworks/kcoreaddons.kpluginfactory.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kpluginfactory;

import pbsd.core;

/// Wave 3 — KPluginFactory metadata keys.
/// Upstream: kde/frameworks/kcoreaddons/src/lib/plugin/kpluginfactory.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kpluginfactory {

inline constexpr const char kPluginIdKey[] = "KPlugin";
inline constexpr const char kIdKey[] = "Id";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/plugin/kpluginfactory.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kpluginfactory
""",
)

add(
    "frameworks/kcoreaddons.formats.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.formats;

import pbsd.core;

/// Wave 3 — KCoreAddons QML format helpers.
/// Upstream: kde/frameworks/kcoreaddons/src/qml/formats.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::formats {

inline constexpr const char kQmlModule[] = "org.kde.kcoreaddons";
inline constexpr const char kFormatsUri[] = "org.kde.kcoreaddons.formats";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/qml/formats.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::formats
""",
)

add(
    "frameworks/kcoreaddons.kcoreaddonsplugin.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kcoreaddonsplugin;

import pbsd.core;

/// Wave 3 — KCoreAddons QML plugin registration.
/// Upstream: kde/frameworks/kcoreaddons/src/qml/kcoreaddonsplugin.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kcoreaddonsplugin {

inline constexpr const char kPluginId[] = "kcoreaddonsplugin";
inline constexpr const char kQmlModule[] = "org.kde.kcoreaddons";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/qml/kcoreaddonsplugin.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kcoreaddonsplugin
""",
)

add(
    "frameworks/kcoreaddons.kuserproxy.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kuserproxy;

import pbsd.core;

/// Wave 3 — KUser QML proxy property keys.
/// Upstream: kde/frameworks/kcoreaddons/src/qml/kuserproxy.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kuserproxy {

inline constexpr const char kUidKey[] = "uid";
inline constexpr const char kLoginNameKey[] = "loginName";
inline constexpr const char kHomeDirKey[] = "homeDir";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/qml/kuserproxy.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kuserproxy
""",
)

add(
    "frameworks/kcoreaddons.kstringhandler.cppm",
    r"""module;

#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.kstringhandler;

import pbsd.core;

/// Wave 3 — squeeze helpers (from KStringHandler::{l,c,r}squeeze).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/text/kstringhandler.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kstringhandler {

inline constexpr const char kEllipsis[] = "...";
inline constexpr unsigned kMaxOutLen = 512;

[[nodiscard]] inline Status lsqueeze(const char* str, int maxlen, char* out,
                                   unsigned out_len) noexcept {
    if (str == nullptr || out == nullptr || out_len == 0) {
        return Status::Invalid;
    }
    const int len = static_cast<int>(std::strlen(str));
    if (len <= maxlen) {
        std::strncpy(out, str, out_len - 1);
        return Status::Ok;
    }
    const int part = maxlen - 3;
    if (part <= 0) {
        return Status::Invalid;
    }
    std::strncpy(out, kEllipsis, out_len - 1);
    std::strncat(out, str + len - part, out_len - std::strlen(out) - 1);
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/text/kstringhandler.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kstringhandler
""",
)

add(
    "frameworks/kcoreaddons.kemoticonsparser.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kemoticonsparser;

import pbsd.core;

/// Wave 3 — emoticon token delimiters (from kemoticonsparser.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/text/kemoticonsparser.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kemoticonsparser {

inline constexpr char kOpenParen = '(';
inline constexpr char kCloseParen = ')';
inline constexpr char kColon = ':';

[[nodiscard]] inline bool is_delimiter(char c) noexcept {
    return c == kOpenParen || c == kCloseParen || c == kColon;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/text/kemoticonsparser.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kemoticonsparser
""",
)

add(
    "frameworks/kcoreaddons.kjob.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kjob;

import pbsd.core;

/// Wave 3 — KJob error propagation constants.
/// Upstream: kde/frameworks/kcoreaddons/src/lib/jobs/kjob.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kjob {

inline constexpr int kNoError = 0;
inline constexpr int kUserCanceled = 1;

struct JobState {
    int error{kNoError};
    unsigned percent{0};
    bool finished{false};
};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/jobs/kjob.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kjob
""",
)

add(
    "frameworks/kio.workerfactory.cppm",
    r"""export module pbsd.kde.frameworks.kio.workerfactory;

import pbsd.core;

/// Wave 3 — KIO WorkerFactory stub (from workerfactory.cpp).
/// Upstream: kde/frameworks/kio/src/core/workerfactory.cpp
export namespace pbsd::kde::frameworks::kio::workerfactory {

inline constexpr const char kInterfaceIid[] = "org.kde.kio.workerfactory/1.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/workerfactory.cpp";
}

} // namespace pbsd::kde::frameworks::kio::workerfactory
""",
)

add(
    "frameworks/kio.kpasswdserver.cppm",
    r"""export module pbsd.kde.frameworks.kio.kpasswdserver;

import pbsd.core;

/// Wave 3 — kpasswdserver KDED module identity.
/// Upstream: kde/frameworks/kio/src/kpasswdserver/kiod_kpasswdserver.cpp
export namespace pbsd::kde::frameworks::kio::kpasswdserver {

inline constexpr const char kModuleName[] = "kpasswdserver";
inline constexpr const char kDbusService[] = "org.kde.kpasswdserver";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/kpasswdserver/kiod_kpasswdserver.cpp";
}

} // namespace pbsd::kde::frameworks::kio::kpasswdserver
""",
)

add(
    "plasma.packagestructure.cppm",
    r"""export module pbsd.kde.plasma.packagestructure;

import pbsd.core;

/// Wave 3 — Plasma generic package structure plugin.
/// Upstream: kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_generic_packagestructure.cpp
export namespace pbsd::kde::plasma::packagestructure {

inline constexpr const char kPluginId[] = "Plasma/Generic";
inline constexpr const char kJsonFile[] = "plasma_generic_packagestructure.json";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_generic_packagestructure.cpp";
}

} // namespace pbsd::kde::plasma::packagestructure
""",
)

add(
    "plasma.startmenu.cppm",
    r"""export module pbsd.kde.plasma.startmenu;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 — Aero Start menu layout constants (panel/start-menu.md).
export namespace pbsd::kde::plasma::startmenu {

inline constexpr const char kNotesPath[] = "plasma/panel/start-menu.md";
inline constexpr const char kBackgroundSvg[] = "plasma/panel/start-menu.svg";
inline constexpr const char kApplicationsSvg[] = "plasma/panel/applications.svg";
inline constexpr int default_width{480};
inline constexpr int default_height{560};
inline constexpr int search_height{36};
inline constexpr int item_height{32};

struct Layout {
    int width{default_width};
    int height{default_height};
    float blur_radius{aero::default_blur().blur_radius};
};

[[nodiscard]] inline Layout default_layout() noexcept { return Layout{}; }

} // namespace pbsd::kde::plasma::startmenu
""",
)

add(
    "plasma.touchpad.logging.cppm",
    r"""export module pbsd.kde.plasma.touchpad.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 — touchpad KCM logging category.
/// Upstream: kde/plasma-desktop/kcms/touchpad/logging.cpp
export namespace pbsd::kde::plasma::touchpad::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KCM_TOUCHPAD",
    "kcm_touchpad",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/plasma-desktop/kcms/touchpad/logging.cpp",
};

} // namespace pbsd::kde::plasma::touchpad::logging
""",
)

add(
    "plasma.mouse.logging.cppm",
    r"""export module pbsd.kde.plasma.mouse.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 — mouse KCM logging stub.
/// Upstream: kde/plasma-desktop/kcms/mouse/inputdevice.cpp
export namespace pbsd::kde::plasma::mouse::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KCM_MOUSE",
    "kcm_mouse",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/plasma-desktop/kcms/mouse/inputdevice.cpp",
};

} // namespace pbsd::kde::plasma::mouse::logging
""",
)

add(
    "kwin/blur.cppm",
    r"""export module pbsd.kde.kwin.blur;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.logging.common;

/// Wave 3 — upstream BlurEffect factory constants.
/// Upstream: kde/kwin/src/plugins/blur/main.cpp
export namespace pbsd::kde::kwin::blur {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_BLUR",
    "kwin_blur",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/plugins/blur/main.cpp",
};

inline constexpr const char kMetadataFile[] = "metadata.json.stripped";
inline constexpr const char kEffectId[] = "blur";

[[nodiscard]] inline plasma::aero::BlurParams default_params() noexcept {
    return plasma::aero::default_blur();
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/blur/main.cpp";
}

} // namespace pbsd::kde::kwin::blur
""",
)

add(
    "kwin/effects.registry.cppm",
    r"""export module pbsd.kde.kwin.effects.registry;

import pbsd.core;

/// Wave 3 — built-in KWin effect plugin IDs (from */plugins/*/main.cpp).
export namespace pbsd::kde::kwin::effects::registry {

inline constexpr const char kBlur[] = "blur";
inline constexpr const char kSlide[] = "slide";
inline constexpr const char kMagicLamp[] = "magiclamp";
inline constexpr const char kOverview[] = "overview";
inline constexpr const char kPbsdAeroBlur[] = "pbsd_aero_blur";

[[nodiscard]] inline bool is_pbsd_aero(const char* id) noexcept {
    return id != nullptr && id[0] == 'p' && id[1] == 'b' && id[2] == 's' && id[3] == 'd';
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/";
}

} // namespace pbsd::kde::kwin::effects::registry
""",
)

add(
    "kwin/compositing.cppm",
    r"""export module pbsd.kde.kwin.compositing;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 — compositor glass/blur bridge constants.
export namespace pbsd::kde::kwin::compositing {

inline constexpr const char kCompositorBackend[] = "wayland";
inline constexpr bool kBlurBehindPanels{true};

[[nodiscard]] inline plasma::aero::BlurParams panel_blur() noexcept {
    auto p = plasma::aero::default_blur();
    p.panel_opacity = 0.72f;
    return p;
}

[[nodiscard]] inline plasma::aero::BlurParams dialog_blur() noexcept {
    auto p = plasma::aero::default_blur();
    p.dialog_opacity = 0.78f;
    return p;
}

} // namespace pbsd::kde::kwin::compositing
""",
)

add(
    "kwin/input_event.cppm",
    r"""export module pbsd.kde.kwin.input_event;

import pbsd.core;

/// Wave 3 — input event type tags.
/// Upstream: kde/kwin/src/input_event.cpp
export namespace pbsd::kde::kwin::input_event {

enum class Type : unsigned char { Keyboard, Pointer, Touch, Tablet, Switch, Unknown };

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/input_event.cpp";
}

} // namespace pbsd::kde::kwin::input_event
""",
)


def main() -> int:
    for rel, content in MODULES.items():
        path = ROOT / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        print(f"wrote {rel}")
    print(f"total {len(MODULES)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
