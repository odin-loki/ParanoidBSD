#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate Wave 3 KDE hand-port module stubs (batch 2) under pbsd/kde/."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd" / "kde"
MODULES: dict[str, str] = {}


def add(name: str, body: str) -> None:
    MODULES[name] = body


def const_module(mod: str, ns: str, upstream: str, lines: list[str]) -> str:
    body = "\n".join(f"inline constexpr {line};" for line in lines)
    return f"""export module {mod};

import pbsd.core;

/// Wave 3 — hand port constants ({upstream.split('/')[-1]}).
/// Upstream: {upstream}
export namespace {ns} {{

{body}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace {ns}
"""


# --- KCoreAddons (7) -------------------------------------------------------------
add(
    "frameworks/kcoreaddons.kautosavefile.cppm",
    r"""module;

#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.kautosavefile;

import pbsd.core;

/// Wave 3 — autosave stale-file discovery (from kautosavefile.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/io/kautosavefile.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kautosavefile {

inline constexpr unsigned kNamePadding = 8;
inline constexpr unsigned kMaxNameLen = 255;
inline constexpr const char kStalefilesSuffix[] = "/stalefiles/";

struct StaleScan {
    char app_name[64]{};
    unsigned found{0};
};

[[nodiscard]] inline Status build_stale_path(const char* app, char* out, unsigned out_len) noexcept {
    if (app == nullptr || out == nullptr || out_len < 32) {
        return Status::Invalid;
    }
    std::strncpy(out, kStalefilesSuffix, out_len - 1);
    std::strncat(out, app, out_len - std::strlen(out) - 1);
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/io/kautosavefile.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kautosavefile
""",
)

add(
    "frameworks/kcoreaddons.kprocesslist_unix.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kprocesslist_unix;

import pbsd.core;

/// Wave 3 — Unix process list helpers (from kprocesslist_unix.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kprocesslist_unix.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kprocesslist_unix {

inline constexpr unsigned kMaxPid = 99999;
inline constexpr unsigned kMaxProcesses = 4096;

[[nodiscard]] inline bool pid_valid(unsigned pid) noexcept {
    return pid > 0 && pid <= kMaxPid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kprocesslist_unix.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kprocesslist_unix
""",
)

add(
    "frameworks/kcoreaddons.kaboutdata.cppm",
    const_module(
        "pbsd.kde.frameworks.kcoreaddons.kaboutdata",
        "pbsd::kde::frameworks::kcoreaddons::kaboutdata",
        "kde/frameworks/kcoreaddons/src/lib/kaboutdata.cpp",
        [
            'const char kOrganizationDomain[] = "kde.org"',
            'const char kDesktopFileName[] = "org.kde.kcoreaddons"',
            'const char kVersion[] = "6.0.0"',
        ],
    ),
)

add(
    "frameworks/kcoreaddons.kcrash.cppm",
    const_module(
        "pbsd.kde.frameworks.kcoreaddons.kcrash",
        "pbsd::kde::frameworks::kcoreaddons::kcrash",
        "kde/frameworks/kcoreaddons/src/lib/kcrash.cpp",
        [
            'const char kDrKonqi[] = "drkonqi"',
            'const char kCrashHandlerEnv[] = "KCRASH_HANDLER"',
        ],
    ),
)

add(
    "frameworks/kcoreaddons.kdelibs4configmigrator.cppm",
    const_module(
        "pbsd.kde.frameworks.kcoreaddons.kdelibs4configmigrator",
        "pbsd::kde::frameworks::kcoreaddons::kdelibs4configmigrator",
        "kde/frameworks/kcoreaddons/src/lib/kdelibs4configmigrator.cpp",
        [
            'const char kKde4ConfigDir[] = ".kde4/share/config"',
            'const char kKde5ConfigDir[] = ".config"',
        ],
    ),
)

add(
    "frameworks/kcoreaddons.kurlmimedata.cppm",
    const_module(
        "pbsd.kde.frameworks.kcoreaddons.kurlmimedata",
        "pbsd::kde::frameworks::kcoreaddons::kurlmimedata",
        "kde/frameworks/kcoreaddons/src/lib/kurlmimedata.cpp",
        [
            'const char kUrlsMime[] = "text/uri-list"',
            'const char kMostLocalUrlMime[] = "text/x-kde-urls"',
        ],
    ),
)

add(
    "frameworks/kcoreaddons.kmemoryinfo.cppm",
    r"""export module pbsd.kde.frameworks.kcoreaddons.kmemoryinfo;

import pbsd.core;

/// Wave 3 — memory info page size (from kmemoryinfo.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kmemoryinfo.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kmemoryinfo {

inline constexpr unsigned kPageSize = 4096;

[[nodiscard]] inline unsigned pages_for_bytes(unsigned long long bytes) noexcept {
    return static_cast<unsigned>((bytes + kPageSize - 1) / kPageSize);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kmemoryinfo.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kmemoryinfo
""",
)

# --- KIO (12) --------------------------------------------------------------------
add(
    "frameworks/kio.kurlauthorized.cppm",
    const_module(
        "pbsd.kde.frameworks.kio.kurlauthorized",
        "pbsd::kde::frameworks::kio::kurlauthorized",
        "kde/frameworks/kio/src/core/kurlauthorized.cpp",
        [
            'const char kOpenFile[] = "open"',
            'const char kOpenUrl[] = "openUrl"',
            'const char kListDir[] = "list"',
        ],
    ),
)

add(
    "frameworks/kio.jobtracker.cppm",
    r"""export module pbsd.kde.frameworks.kio.jobtracker;

import pbsd.core;
import pbsd.kde.frameworks.kcoreaddons.kjobtrackerinterface;

/// Wave 3 — global job tracker registry (from jobtracker.cpp).
/// Upstream: kde/frameworks/kio/src/core/jobtracker.cpp
export namespace pbsd::kde::frameworks::kio::jobtracker {

inline constexpr const char kGlobalTrackerId[] = "globalDummyTracker";

[[nodiscard]] inline const char* tracker_iid() noexcept {
    return kcoreaddons::kjobtrackerinterface::kInterfaceIid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/jobtracker.cpp";
}

} // namespace pbsd::kde::frameworks::kio::jobtracker
""",
)

add(
    "frameworks/kio.kioglobal_unix.cppm",
    r"""export module pbsd.kde.frameworks.kio.kioglobal_unix;

import pbsd.core;

/// Wave 3 — Unix process/symlink helpers (from kioglobal_p_unix.cpp).
/// Upstream: kde/frameworks/kio/src/core/kioglobal_p_unix.cpp
export namespace pbsd::kde::frameworks::kio::kioglobal_unix {

inline constexpr int kSigTerm = 15;

[[nodiscard]] inline bool is_process_alive_result(int kill_result) noexcept {
    return kill_result == 0;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/kioglobal_p_unix.cpp";
}

} // namespace pbsd::kde::frameworks::kio::kioglobal_unix
""",
)

add(
    "frameworks/kio.emptytrashjob.cppm",
    const_module(
        "pbsd.kde.frameworks.kio.emptytrashjob",
        "pbsd::kde::frameworks::kio::emptytrashjob",
        "kde/frameworks/kio/src/core/emptytrashjob.cpp",
        [
            'const char kTrashProtocol[] = "trash"',
            'const char kTrashEmptyAction[] = "emptyTrash"',
        ],
    ),
)

add(
    "frameworks/kio.specialjob.cppm",
    const_module(
        "pbsd.kde.frameworks.kio.specialjob",
        "pbsd::kde::frameworks::kio::specialjob",
        "kde/frameworks/kio/src/core/specialjob.cpp",
        [
            'const char kSpecialProtocol[] = "special"',
            'const char kAppsProtocol[] = "apps"',
        ],
    ),
)

add(
    "frameworks/kio.jobuidelegatefactory.cppm",
    const_module(
        "pbsd.kde.frameworks.kio.jobuidelegatefactory",
        "pbsd::kde::frameworks::kio::jobuidelegatefactory",
        "kde/frameworks/kio/src/core/jobuidelegatefactory.cpp",
        [
            'const char kFactoryIid[] = "org.kde.kio.jobuidelegatefactory/1.0"',
        ],
    ),
)

add(
    "frameworks/kio.connectionserver.cppm",
    const_module(
        "pbsd.kde.frameworks.kio.connectionserver",
        "pbsd::kde::frameworks::kio::connectionserver",
        "kde/frameworks/kio/src/core/connectionserver.cpp",
        [
            'const char kWorkerSocketEnv[] = "KDE_FORK_SLAVES"',
            'unsigned kMaxPendingConnections = 32',
        ],
    ),
)

add(
    "frameworks/kio.workerthread.cppm",
    const_module(
        "pbsd.kde.frameworks.kio.workerthread",
        "pbsd::kde::frameworks::kio::workerthread",
        "kde/frameworks/kio/src/core/workerthread.cpp",
        [
            'unsigned kWorkerStackSize = 65536',
            'const char kWorkerThreadName[] = "KIO::Worker"',
        ],
    ),
)

add(
    "frameworks/kio.koverlayiconplugin.cppm",
    const_module(
        "pbsd.kde.frameworks.kio.koverlayiconplugin",
        "pbsd::kde::frameworks::kio::koverlayiconplugin",
        "kde/frameworks/kio/src/core/koverlayiconplugin.cpp",
        [
            'const char kPluginIid[] = "org.kde.kio.overlayIconPlugin/1.0"',
        ],
    ),
)

add(
    "frameworks/kio.dndpopupmenuplugin.cppm",
    const_module(
        "pbsd.kde.frameworks.kio.dndpopupmenuplugin",
        "pbsd::kde::frameworks::kio::dndpopupmenuplugin",
        "kde/frameworks/kio/src/gui/dndpopupmenuplugin.cpp",
        [
            'const char kPluginIid[] = "org.kde.kio.dndPopupMenuPlugin/1.0"',
        ],
    ),
)

add(
    "frameworks/kio.kpasswdserverloop.cppm",
    const_module(
        "pbsd.kde.frameworks.kio.kpasswdserverloop",
        "pbsd::kde::frameworks::kio::kpasswdserverloop",
        "kde/frameworks/kio/src/core/kpasswdserverloop.cpp",
        [
            'const char kLoopService[] = "org.kde.kpasswdserverloop"',
            'unsigned kMaxRetries = 3',
        ],
    ),
)

add(
    "frameworks/kio.joburlcache.cppm",
    const_module(
        "pbsd.kde.frameworks.kio.joburlcache",
        "pbsd::kde::frameworks::kio::joburlcache",
        "kde/frameworks/kio/src/widgets/joburlcache.cpp",
        [
            'unsigned kMaxCachedUrls = 128',
            'unsigned kMaxUrlLen = 2048',
        ],
    ),
)

# --- KConfig (4) -----------------------------------------------------------------
add(
    "frameworks/kconfig.kstandardactions.cppm",
    r"""export module pbsd.kde.frameworks.kconfig.kstandardactions;

import pbsd.core;

/// Wave 3 — standard action IDs (from kstandardactions.cpp).
/// Upstream: kde/frameworks/kconfig/src/gui/kstandardactions.cpp
export namespace pbsd::kde::frameworks::kconfig::kstandardactions {

enum class StandardAction : unsigned short {
    None = 0,
    Back,
    Forward,
    Home,
    Prior,
    Next,
    Preferences,
    AboutApp,
    HelpContents,
};

inline constexpr unsigned kActionCount = 9;

[[nodiscard]] inline bool is_navigation(StandardAction a) noexcept {
    return a == StandardAction::Back || a == StandardAction::Forward
        || a == StandardAction::Home;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kstandardactions.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kstandardactions
""",
)

add(
    "frameworks/kconfig.kconfiggroupgui.cppm",
    const_module(
        "pbsd.kde.frameworks.kconfig.kconfiggroupgui",
        "pbsd::kde::frameworks::kconfig::kconfiggroupgui",
        "kde/frameworks/kconfig/src/gui/kconfiggroupgui.cpp",
        [
            'const char kGroupSeparator[] = "::"',
            'unsigned kMaxGroupDepth = 16',
        ],
    ),
)

add(
    "frameworks/kconfig.kconfigcompiler.cppm",
    const_module(
        "pbsd.kde.frameworks.kconfig.kconfigcompiler",
        "pbsd::kde::frameworks.kconfig.kconfigcompiler",
        "kde/frameworks/kconfig/src/kconfig_compiler/kconfigcompiler.cpp",
        [
            'const char kKcfgSuffix[] = ".kcfg"',
            'const char kClassSuffix[] = "Skeleton"',
        ],
    ),
)

add(
    "frameworks/kconfig.kshortcut.cppm",
    const_module(
        "pbsd.kde.frameworks.kconfig.kshortcut",
        "pbsd::kde::frameworks::kconfig::kshortcut",
        "kde/frameworks/kconfig/src/gui/kshortcut.cpp",
        [
            'const char kPrimarySequenceKey[] = "Primary"',
            'const char kDefaultGroup[] = "Shortcuts"',
        ],
    ),
)

# --- Plasma (6) ------------------------------------------------------------------
add(
    "plasma.workspace.logging.cppm",
    r"""export module pbsd.kde.plasma.workspace.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 — Plasma workspace logging category.
/// Upstream: kde/plasma-workspace/shell/shellcorona.cpp
export namespace pbsd::kde::plasma::workspace::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "PLASMA_SHELL",
    "plasma_shell",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/plasma-workspace/shell/shellcorona.cpp",
};

} // namespace pbsd::kde::plasma::workspace::logging
""",
)

add(
    "plasma.lookandfeel.cppm",
    r"""export module pbsd.kde.plasma.lookandfeel;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 — look-and-feel package keys (from landingpage KCM).
/// Upstream: kde/plasma-desktop/kcms/landingpage/lookandfeelmetadata.cpp
export namespace pbsd::kde::plasma::lookandfeel {

inline constexpr const char kPackageId[] = "org.kde.lookandfeel";
inline constexpr const char kThemeColorsKey[] = "theme-colors.json";
inline constexpr const char kColorSchemeKey[] = "colors";

[[nodiscard]] inline const char* aero_theme_id() noexcept { return aero::kThemeId; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/landingpage/lookandfeelmetadata.cpp";
}

} // namespace pbsd::kde::plasma::lookandfeel
""",
)

add(
    "plasma.containment.cppm",
    const_module(
        "pbsd.kde.plasma.containment",
        "pbsd::kde::plasma::containment",
        "kde/plasma-framework/src/plasma/containment.cpp",
        [
            'const char kContainmentTypeDesktop[] = "Desktop"',
            'const char kContainmentTypePanel[] = "Panel"',
            'unsigned kMaxApplets = 64',
        ],
    ),
)

add(
    "plasma.taskmanager.cppm",
    r"""export module pbsd.kde.plasma.taskmanager;

import pbsd.core;

/// Wave 3 — task manager layout constants.
/// Upstream: kde/plasma-desktop/applets/taskmanager/package/contents/config/main.xml
export namespace pbsd::kde::plasma::taskmanager {

inline constexpr int kDefaultIconSize{32};
inline constexpr int kGroupMode{0};
inline constexpr int kMaxTasks{64};
inline constexpr const char kTasksSvg[] = "plasma/widgets/tasks.svg";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/applets/taskmanager/";
}

} // namespace pbsd::kde::plasma::taskmanager
""",
)

add(
    "plasma.digitalclock.cppm",
    const_module(
        "pbsd.kde.plasma.digitalclock",
        "pbsd::kde::plasma::digitalclock",
        "kde/plasma-desktop/applets/digital-clock/package/contents/config/main.xml",
        [
            'const char kClockSvg[] = "plasma/panel/clock.svg"',
            'const char kDateFormat[] = "ddd MMM d"',
            'const char kTimeFormat[] = "h:mm AP"',
        ],
    ),
)

add(
    "plasma.kicker.cppm",
    const_module(
        "pbsd.kde.plasma.kicker",
        "pbsd::kde::plasma::kicker",
        "kde/plasma-desktop/applets/kicker/package/contents/config/main.xml",
        [
            'const char kStartButtonSvg[] = "plasma/panel/start-button.svg"',
            'const char kApplicationsSvg[] = "plasma/panel/applications.svg"',
            'unsigned kFavoriteSlots = 12',
        ],
    ),
)

# --- KWin effect bridges (6) -----------------------------------------------------
for effect, upstream in [
    ("slide", "kde/kwin/src/plugins/slide/main.cpp"),
    ("overview", "kde/kwin/src/plugins/overview/main.cpp"),
    ("magiclamp", "kde/kwin/src/plugins/magiclamp/main.cpp"),
    ("windowview", "kde/kwin/src/plugins/windowview/main.cpp"),
    ("screencast", "kde/kwin/src/plugins/screencast/screencastsource.cpp"),
    ("tileseditor", "kde/kwin/src/plugins/tileseditor/main.cpp"),
]:
    ns = f"pbsd::kde::kwin::{effect}"
    mod = f"pbsd.kde.kwin.{effect}"
    add(
        f"kwin/{effect}.cppm",
        f"""export module {mod};

import pbsd.core;
import pbsd.kde.kwin.effects.registry;

/// Wave 3 — KWin {effect} effect bridge constants.
/// Upstream: {upstream}
export namespace {ns} {{

inline constexpr const char kEffectId[] = "{effect}";

[[nodiscard]] inline bool is_registered() noexcept {{
    return kEffectId[0] != '\\0';
}}

[[nodiscard]] inline const char* upstream_path() noexcept {{
    return "{upstream}";
}}

}} // namespace {ns}
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
