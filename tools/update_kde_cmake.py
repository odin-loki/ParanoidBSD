#!/usr/bin/env python3
"""Regenerate PBSD_KDE module lists in pbsd/kde/CMakeLists.txt from *.cppm files."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
KDE = ROOT / "pbsd" / "kde"
CMAKE = KDE / "CMakeLists.txt"

EFFECTS = sorted(p.relative_to(KDE).as_posix() for p in (KDE / "kwin" / "effects").glob("*.cppm"))
FRAMEWORKS = sorted(p.relative_to(KDE).as_posix() for p in (KDE / "frameworks").glob("*.cppm"))
PLASMA = sorted(p.relative_to(KDE).as_posix() for p in KDE.glob("plasma*.cppm"))

KWIN_TOP = [
    "kwin.pbsd_aero.cppm",
    "kwin.decorations.cppm",
]
KWIN_EXCLUDE = {"effects", "logging.common.cppm"}

def kwin_core_modules() -> list[str]:
    paths: list[str] = []
    for p in sorted(KDE.rglob("kwin/**/*.cppm")):
        rel = p.relative_to(KDE).as_posix()
        if rel.startswith("kwin/effects/"):
            continue
        if rel.endswith(".logging.cppm"):
            continue
        if p.name == "logging.common.cppm":
            continue
        paths.append(rel)
    return paths

KWIN_LOGGING = sorted(p.relative_to(KDE).as_posix() for p in (KDE / "kwin").rglob("*.logging.cppm"))
if (KDE / "kwin" / "logging.common.cppm").exists():
    KWIN_LOGGING = ["kwin/logging.common.cppm"] + [
        x for x in KWIN_LOGGING if x != "kwin/logging.common.cppm"
    ]

PBSD_KDE_KWIN_MODULES = KWIN_TOP + kwin_core_modules()


def cmake_list(name: str, items: list[str]) -> str:
    lines = [f"set({name}"]
    for item in items:
        lines.append(f"    {item}")
    lines.append(")")
    return "\n".join(lines)


content = f"""# Wave 3 — KDE C++23 module stubs (theme constants + KWin bridge + KF conversions)
set(PBSD_KDE_CXX -fno-exceptions -fno-rtti)

{cmake_list("PBSD_KDE_KWIN_LOGGING_MODULES", KWIN_LOGGING)}

{cmake_list("PBSD_KDE_KWIN_EFFECT_MODULES", EFFECTS)}

{cmake_list("PBSD_KDE_FRAMEWORKS_MODULES", FRAMEWORKS)}

{cmake_list("PBSD_KDE_PLASMA_MODULES", PLASMA)}

{cmake_list("PBSD_KDE_KWIN_MODULES", PBSD_KDE_KWIN_MODULES)}

add_library(pbsd_kde_kwin_logging)
target_sources(pbsd_kde_kwin_logging PUBLIC FILE_SET CXX_MODULES FILES ${{PBSD_KDE_KWIN_LOGGING_MODULES}})
target_link_libraries(pbsd_kde_kwin_logging PUBLIC pbsd_core)
target_compile_features(pbsd_kde_kwin_logging PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_kwin_logging PRIVATE ${{PBSD_KDE_CXX}})

add_library(pbsd_kde_kwin_effects)
target_sources(pbsd_kde_kwin_effects PUBLIC FILE_SET CXX_MODULES FILES ${{PBSD_KDE_KWIN_EFFECT_MODULES}})
target_link_libraries(pbsd_kde_kwin_effects PUBLIC pbsd_kde_kwin_logging)
target_compile_features(pbsd_kde_kwin_effects PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_kwin_effects PRIVATE ${{PBSD_KDE_CXX}})

add_library(pbsd_kde_plasma_aero)
target_sources(pbsd_kde_plasma_aero PUBLIC FILE_SET CXX_MODULES FILES ${{PBSD_KDE_PLASMA_MODULES}})
target_link_libraries(pbsd_kde_plasma_aero PUBLIC pbsd_core pbsd_kde_kwin_logging)
target_compile_features(pbsd_kde_plasma_aero PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_plasma_aero PRIVATE ${{PBSD_KDE_CXX}})

add_library(pbsd_kde_kwin_aero)
target_sources(pbsd_kde_kwin_aero PUBLIC FILE_SET CXX_MODULES FILES ${{PBSD_KDE_KWIN_MODULES}})
target_link_libraries(pbsd_kde_kwin_aero PUBLIC pbsd_kde_plasma_aero pbsd_kde_kwin_effects pbsd_handles pbsd_kde_kwin_logging)
target_compile_features(pbsd_kde_kwin_aero PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_kwin_aero PRIVATE ${{PBSD_KDE_CXX}})

add_library(pbsd_kde_frameworks)
target_sources(pbsd_kde_frameworks PUBLIC FILE_SET CXX_MODULES FILES ${{PBSD_KDE_FRAMEWORKS_MODULES}})
target_link_libraries(pbsd_kde_frameworks PUBLIC pbsd_core)
target_compile_features(pbsd_kde_frameworks PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_frameworks PRIVATE ${{PBSD_KDE_CXX}})

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_link_libraries(pbsd_kde_frameworks PRIVATE procstat)
endif()

add_library(pbsd_kde INTERFACE)
target_link_libraries(pbsd_kde INTERFACE
    pbsd_kde_plasma_aero
    pbsd_kde_kwin_aero
    pbsd_kde_kwin_effects
    pbsd_kde_kwin_logging
    pbsd_kde_frameworks)

set(PBSD_KDE_MODULE_FILES
    ${{PBSD_KDE_PLASMA_MODULES}}
    ${{PBSD_KDE_KWIN_MODULES}}
    ${{PBSD_KDE_KWIN_EFFECT_MODULES}}
    ${{PBSD_KDE_KWIN_LOGGING_MODULES}}
    ${{PBSD_KDE_FRAMEWORKS_MODULES}})

install(FILES ${{PBSD_KDE_MODULE_FILES}}
    DESTINATION share/pbsd/kde/modules
    OPTIONAL)
"""

CMAKE.write_text(content, encoding="utf-8")
print(
    f"CMake updated: {len(EFFECTS)} effects, {len(FRAMEWORKS)} frameworks, "
    f"{len(PLASMA)} plasma, {len(PBSD_KDE_KWIN_MODULES)} kwin, {len(KWIN_LOGGING)} logging"
)
