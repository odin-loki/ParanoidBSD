#!/usr/bin/env bash
# =============================================================================
# PBSD — Source Download & Build Tree Setup
# Author: Odin Loch
# Run this on a FreeBSD, HardenedBSD, or Linux host with git and curl installed.
# On Windows: run inside WSL2 (Ubuntu) or a FreeBSD VM.
# =============================================================================

set -euo pipefail

# --- Configuration -----------------------------------------------------------
ROOT="${PBSD_ROOT:-$(cd "$(dirname "$0")" && pwd)}"
JOBS="${JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

# Source tags/branches — update these as upstream moves
HBSD_BRANCH="hardened/15-stable/main"
KDE_PLASMA_TAG="v6.6.5"
KDE_FRAMEWORKS_TAG="v6.6.0"
KWIN_TAG="v6.6.5"

# Colour helpers
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; CYAN='\033[0;36m'; NC='\033[0m'
info()    { echo -e "${CYAN}[INFO]${NC}  $*"; }
ok()      { echo -e "${GREEN}[OK]${NC}    $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error()   { echo -e "${RED}[ERROR]${NC} $*"; exit 1; }

# --- Preflight ---------------------------------------------------------------
info "PBSD source setup starting"
info "Root: $ROOT"
info "Parallel jobs: $JOBS"

command -v git  >/dev/null 2>&1 || error "git not found — install it first"
command -v curl >/dev/null 2>&1 || error "curl not found — install it first"

# Warn if not on FreeBSD/HardenedBSD (build tools differ on Linux)
if ! uname -s | grep -qi "BSD"; then
    warn "Not running on BSD — you can clone sources here but the FreeBSD build system"
    warn "(make buildworld / buildkernel) requires a BSD host or a FreeBSD VM/jail."
    warn "This script will still set up the full tree and Cursor workspace."
fi

# --- Directory structure -----------------------------------------------------
info "Creating directory tree under $ROOT"

mkdir -p \
    "$ROOT/hbsd/src" \
    "$ROOT/hbsd/ports" \
    "$ROOT/kde/plasma-desktop" \
    "$ROOT/kde/kwin" \
    "$ROOT/kde/frameworks" \
    "$ROOT/kde/apps" \
    "$ROOT/pbsd/kernel" \
    "$ROOT/pbsd/uda" \
    "$ROOT/pbsd/analyser" \
    "$ROOT/pbsd/handles" \
    "$ROOT/pbsd/bifrost" \
    "$ROOT/pbsd/compositor" \
    "$ROOT/pbsd/userland" \
    "$ROOT/pbsd/theme" \
    "$ROOT/docs/specs" \
    "$ROOT/docs/contracts" \
    "$ROOT/docs/provenance" \
    "$ROOT/docs/security" \
    "$ROOT/build" \
    "$ROOT/.cursor"

ok "Directory tree created"

# --- Clone HardenedBSD src ---------------------------------------------------
info "Cloning HardenedBSD source ($HBSD_BRANCH) — this is ~1–2GB, takes a while..."

if [ -d "$ROOT/hbsd/src/.git" ]; then
    warn "hbsd/src already cloned — fetching updates"
    git -C "$ROOT/hbsd/src" fetch origin
    git -C "$ROOT/hbsd/src" checkout "$HBSD_BRANCH" || \
        git -C "$ROOT/hbsd/src" checkout -b "$HBSD_BRANCH" "origin/$HBSD_BRANCH"
    git -C "$ROOT/hbsd/src" pull --ff-only
else
    git clone \
        --branch "$HBSD_BRANCH" \
        --single-branch \
        --depth 50 \
        "https://rad.hardenedbsd.org/z2HLHXgL1xevBNQsf8BmQW7MpJmtm.git" \
        "$ROOT/hbsd/src"
fi
ok "HardenedBSD src ready"

# --- Clone HardenedBSD ports (GitHub mirror) --------------------------------
info "Cloning HardenedBSD ports (GitHub mirror) — this is ~500MB..."

if [ -d "$ROOT/hbsd/ports/.git" ]; then
    warn "hbsd/ports already cloned — fetching updates"
    git -C "$ROOT/hbsd/ports" pull --ff-only
else
    git clone \
        --depth 1 \
        "https://github.com/HardenedBSD/hardenedbsd-ports.git" \
        "$ROOT/hbsd/ports"
fi
ok "HardenedBSD ports ready"

# --- Clone KDE Plasma --------------------------------------------------------
info "Cloning KDE Plasma Desktop ($KDE_PLASMA_TAG)..."

if [ -d "$ROOT/kde/plasma-desktop/.git" ]; then
    warn "kde/plasma-desktop already cloned — fetching updates"
    git -C "$ROOT/kde/plasma-desktop" fetch --tags
else
    git clone \
        --branch "$KDE_PLASMA_TAG" \
        --single-branch \
        --depth 1 \
        "https://invent.kde.org/plasma/plasma-desktop.git" \
        "$ROOT/kde/plasma-desktop"
fi
ok "KDE Plasma Desktop ready"

# --- Clone KWin --------------------------------------------------------------
info "Cloning KWin ($KWIN_TAG) — Wayland compositor..."

if [ -d "$ROOT/kde/kwin/.git" ]; then
    warn "kde/kwin already cloned — fetching updates"
    git -C "$ROOT/kde/kwin" fetch --tags
else
    git clone \
        --branch "$KWIN_TAG" \
        --single-branch \
        --depth 1 \
        "https://invent.kde.org/plasma/kwin.git" \
        "$ROOT/kde/kwin"
fi
ok "KWin ready"

# --- Clone KDE Frameworks (selected — full set is 80+ repos) ----------------
info "Cloning key KDE Frameworks repos ($KDE_FRAMEWORKS_TAG)..."

KDE_FRAMEWORKS=(
    "kcoreaddons"
    "kconfig"
    "kwindowsystem"
    "kservice"
    "kio"
    "kxmlgui"
    "kdecoration"
    "plasma-framework"
    "layer-shell-qt"
    "kwayland"
)

for fw in "${KDE_FRAMEWORKS[@]}"; do
    fw_dir="$ROOT/kde/frameworks/$fw"
    if [ -d "$fw_dir/.git" ] && [ -f "$fw_dir/CMakeLists.txt" ]; then
        warn "kde/frameworks/$fw already cloned — skipping"
    else
        # Remove incomplete/hung clones so retries can succeed
        if [ -d "$fw_dir" ] && [ ! -f "$fw_dir/CMakeLists.txt" ]; then
            warn "  Removing incomplete clone of $fw"
            rm -rf "$fw_dir"
        fi
        info "  Cloning $fw..."
        # invent.kde.org can hang; fall back to the GitHub KDE mirror
        if ! GIT_TERMINAL_PROMPT=0 git clone \
            --branch "$KDE_FRAMEWORKS_TAG" \
            --single-branch \
            --depth 1 \
            "https://invent.kde.org/frameworks/${fw}.git" \
            "$fw_dir" 2>/dev/null; then
            warn "  invent.kde.org failed for $fw — trying GitHub mirror"
            rm -rf "$fw_dir"
            GIT_TERMINAL_PROMPT=0 git clone \
                --branch "$KDE_FRAMEWORKS_TAG" \
                --single-branch \
                --depth 1 \
                "https://github.com/KDE/${fw}.git" \
                "$fw_dir" 2>/dev/null || \
                warn "  Could not clone $fw — check invent.kde.org / github.com/KDE"
        fi
    fi
done
ok "KDE Frameworks ready"

# --- Scaffold PBSD source tree ------------------------------------------
info "Scaffolding PBSD new-code tree..."

# CMakeLists.txt root
cat > "$ROOT/pbsd/CMakeLists.txt" << 'CMAKE'
cmake_minimum_required(VERSION 3.28)
# C is required: LLVMConfig.cmake probes FFI/Terminfo via check_c_source_compiles
project(PBSD LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Kernel code: no exceptions, no RTTI
set(KERNEL_CXX_FLAGS "-fno-exceptions -fno-rtti -ffreestanding -fno-stack-protector")

# Clang static analysis flags
set(ANALYSER_FLAGS
    "-Wthread-safety"
    "-Wthread-safety-analysis"
    "-Wthread-safety-negative"
)

add_compile_options(${ANALYSER_FLAGS})

add_subdirectory(handles)
add_subdirectory(analyser)
add_subdirectory(uda)
add_subdirectory(userland)
CMAKE

# handles/ — the capability type hierarchy
cat > "$ROOT/pbsd/handles/CMakeLists.txt" << 'CMAKE'
add_library(pbsd_handles INTERFACE)
target_include_directories(pbsd_handles INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(pbsd_handles INTERFACE cxx_std_23)
CMAKE

mkdir -p "$ROOT/pbsd/handles/include/ygg"

cat > "$ROOT/pbsd/handles/include/pbsd/handles.hpp" << 'HANDLES'
// PBSD — Kernel Handle & Capability Type Hierarchy
// Author: Odin Loch
// PROVENANCE: Original design; no third-party source derived.
// All ownership-sensitive kernel state MUST route through these types.
// The static analyser reasons about this closed type set only.

#pragma once
#include <cstdint>
#include <utility>

namespace pbsd {

// CapabilityRights — rights bitmask, only narrows on duplication/grant
enum class CapabilityRights : uint32_t {
    None      = 0,
    Read      = 1 << 0,
    Write     = 1 << 1,
    Execute   = 1 << 2,
    Grant     = 1 << 3,
    Duplicate = 1 << 4,
    Destroy   = 1 << 5,
    All       = 0x3F,
};

inline constexpr CapabilityRights operator&(CapabilityRights a, CapabilityRights b) {
    return static_cast<CapabilityRights>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline constexpr bool has_right(CapabilityRights set, CapabilityRights right) {
    return (set & right) != CapabilityRights::None;
}

// UniqueHandle<T> — move-only, single owner.
// [[kernel::owns]] annotation — static analyser tracks ownership state.
template<typename T>
class [[nodiscard]] UniqueHandle {
public:
    UniqueHandle() noexcept = default;

    explicit UniqueHandle(T* ptr, CapabilityRights rights) noexcept
        : ptr_(ptr), rights_(rights) {}

    // Move only
    UniqueHandle(UniqueHandle&& other) noexcept
        : ptr_(other.ptr_), rights_(other.rights_) {
        other.ptr_ = nullptr;
        other.rights_ = CapabilityRights::None;
    }

    UniqueHandle& operator=(UniqueHandle&& other) noexcept {
        if (this != &other) {
            reset();
            ptr_ = other.ptr_;
            rights_ = other.rights_;
            other.ptr_ = nullptr;
            other.rights_ = CapabilityRights::None;
        }
        return *this;
    }

    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;

    ~UniqueHandle() noexcept { reset(); }

    [[nodiscard]] bool valid() const noexcept { return ptr_ != nullptr; }
    [[nodiscard]] bool has_right(CapabilityRights r) const noexcept {
        return pbsd::has_right(rights_, r);
    }

    // Narrow rights only — returns a new handle with reduced rights
    [[nodiscard]] UniqueHandle narrow(CapabilityRights new_rights) && noexcept {
        // Rights can only narrow, not widen
        CapabilityRights narrowed = rights_ & new_rights;
        UniqueHandle h(ptr_, narrowed);
        ptr_ = nullptr;
        rights_ = CapabilityRights::None;
        return h;
    }

    void reset() noexcept {
        if (ptr_) {
            // Kernel object release hook — T must provide static T::release()
            T::release(ptr_);
            ptr_ = nullptr;
            rights_ = CapabilityRights::None;
        }
    }

    T* get() const noexcept { return ptr_; }

private:
    T* ptr_ = nullptr;
    CapabilityRights rights_ = CapabilityRights::None;
};

// BorrowedHandle<T> — non-owning view, [[kernel::no_escape]].
// Must not outlive the UniqueHandle or SharedHandle that produced it.
template<typename T>
class BorrowedHandle {
public:
    BorrowedHandle() noexcept = default;

    explicit BorrowedHandle(T* ptr, CapabilityRights rights) noexcept
        : ptr_(ptr), rights_(rights) {}

    [[nodiscard]] bool valid() const noexcept { return ptr_ != nullptr; }
    [[nodiscard]] bool has_right(CapabilityRights r) const noexcept {
        return pbsd::has_right(rights_, r);
    }
    T* get() const noexcept { return ptr_; }

    // BorrowedHandle is copyable (multiple borrows are fine)
    BorrowedHandle(const BorrowedHandle&) noexcept = default;
    BorrowedHandle& operator=(const BorrowedHandle&) noexcept = default;

private:
    T* ptr_ = nullptr;
    CapabilityRights rights_ = CapabilityRights::None;
};

// Helper: produce a BorrowedHandle from a UniqueHandle
// The borrow MUST NOT outlive the UniqueHandle — enforced by static analyser
template<typename T>
[[nodiscard]] BorrowedHandle<T> borrow(const UniqueHandle<T>& h) noexcept {
    return BorrowedHandle<T>(h.get(), CapabilityRights::Read);
}

} // namespace pbsd
HANDLES

ok "handles/ scaffold complete"

# uda/ — Universal Driver Architecture scaffold
cat > "$ROOT/pbsd/uda/CMakeLists.txt" << 'CMAKE'
add_library(pbsd_uda STATIC
    src/descriptor_db.cpp
    src/block_engine.cpp
    src/net_engine.cpp
)
target_include_directories(pbsd_uda PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(pbsd_uda PRIVATE pbsd_handles)
target_compile_features(pbsd_uda PRIVATE cxx_std_23)
CMAKE

mkdir -p "$ROOT/pbsd/uda/include/pbsd/uda"
mkdir -p "$ROOT/pbsd/uda/src"
mkdir -p "$ROOT/pbsd/uda/descriptors"

cat > "$ROOT/pbsd/uda/include/pbsd/uda/descriptor.hpp" << 'UDA'
// PBSD UDA — Hardware Descriptor Schema
// Author: Odin Loch
// PROVENANCE: Original design. Descriptors populated from public datasheets only.
// Each descriptor MUST have a corresponding PROVENANCE.md entry citing its datasheet.

#pragma once
#include <cstdint>
#include <span>
#include <string_view>

namespace pbsd::uda {

// Register operation types for the bytecode interpreter
enum class RegOp : uint8_t {
    Write8  = 0x01,  // Write 8-bit value to offset
    Write16 = 0x02,
    Write32 = 0x03,
    Read8   = 0x11,  // Read 8-bit value from offset (result in accumulator)
    Read32  = 0x13,
    WaitUs  = 0x20,  // Wait N microseconds
    CheckEq = 0x30,  // Loop until [offset] & mask == expected (poll/wait)
    Done    = 0xFF,  // End of sequence
};

struct RegInsn {
    RegOp    op;
    uint32_t offset;   // MMIO offset from device base
    uint32_t value;    // Write value or expected value for CheckEq
    uint32_t mask;     // Mask for CheckEq operations
    uint32_t timeout;  // Microseconds for CheckEq timeout (0 = no timeout)
};

// Device class
enum class DeviceClass : uint8_t {
    Block   = 0x01,
    Network = 0x02,
    Display = 0x03,
    Input   = 0x04,
    Sensor  = 0x05,
};

// Hardware descriptor — one per chip model
// Populated entirely from public datasheet; never from existing driver source
struct Descriptor {
    std::string_view name;           // e.g. "virtio-blk-1.0"
    std::string_view provenance;     // e.g. "OASIS virtio-v1.2 spec §5.2"
    DeviceClass      device_class;
    uint16_t         vendor_id;      // PCI vendor ID (0 = non-PCI/platform device)
    uint16_t         device_id;      // PCI device ID
    std::span<const RegInsn> init_sequence;   // Bytecode: device initialisation
    std::span<const RegInsn> reset_sequence;  // Bytecode: soft reset
    // Per-class parameters follow in class-specific descriptor subtype
};

} // namespace pbsd::uda
UDA

# Stub source files so CMake doesn't complain
echo "// stub" > "$ROOT/pbsd/uda/src/descriptor_db.cpp"
echo "// stub" > "$ROOT/pbsd/uda/src/block_engine.cpp"
echo "// stub" > "$ROOT/pbsd/uda/src/net_engine.cpp"

ok "uda/ scaffold complete"

# analyser/ — static analyser skeleton
mkdir -p "$ROOT/pbsd/analyser/src"
cat > "$ROOT/pbsd/analyser/CMakeLists.txt" << 'CMAKE'
# Clang-based static analyser — requires LLVM/Clang development packages.
# Prefer a complete install: Ubuntu often ships ClangConfig for older versions
# whose libclangBasic.a is missing; importing those is a hard CMake error.
foreach(_ver IN ITEMS 18 17 16 15)
    set(_llvm_dir "/usr/lib/llvm-${_ver}/lib/cmake/llvm")
    set(_clang_dir "/usr/lib/cmake/clang-${_ver}")
    if(EXISTS "${_llvm_dir}/LLVMConfig.cmake"
       AND EXISTS "${_clang_dir}/ClangConfig.cmake"
       AND EXISTS "/usr/lib/llvm-${_ver}/lib/libclangBasic.a")
        set(LLVM_DIR "${_llvm_dir}")
        set(Clang_DIR "${_clang_dir}")
        break()
    endif()
endforeach()

find_package(LLVM CONFIG QUIET)
find_package(Clang CONFIG QUIET)

if(LLVM_FOUND AND Clang_FOUND)
    message(STATUS "LLVM ${LLVM_PACKAGE_VERSION} found — building analyser")
    add_library(pbsd_analyser MODULE src/ownership_check.cpp)
    target_include_directories(pbsd_analyser PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
    target_link_libraries(pbsd_analyser PRIVATE clangAST clangASTMatchers clangTooling)
    target_compile_features(pbsd_analyser PRIVATE cxx_std_23)
else()
    message(WARNING "LLVM/Clang dev headers not found — analyser not built. Install libclang-*-dev llvm-*-dev.")
endif()
CMAKE

cat > "$ROOT/pbsd/analyser/src/ownership_check.cpp" << 'ANALYSER'
// PBSD Static Analyser — Layer 2 Ownership Dataflow
// Author: Odin Loch
// PROVENANCE: Original implementation using Clang LibTooling public API.
//
// This file is the skeleton for the custom borrow-checker equivalent.
// It uses Clang's AST and CFG infrastructure to track state transitions
// of UniqueHandle / BorrowedHandle / SharedHandle across the control-flow
// graph of each function, flagging:
//   - Use-after-move
//   - Double-release
//   - Exclusive-borrow-while-aliased
//
// Layer 1 (syntactic bans) and Layer 3 (Clang Thread Safety Analysis) are
// enabled via compiler flags in CMakeLists.txt — no source needed here.
//
// TODO: Implement dataflow analysis. Start with a clang-tidy check
// banning raw new/delete as Layer 1 proof-of-concept.

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;

namespace {

class YggOwnershipConsumer : public ASTConsumer {
public:
    explicit YggOwnershipConsumer(ASTContext& /*ctx*/) {}
    // TODO: traverse declarations and build ownership dataflow per function
};

class YggOwnershipAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer>
    CreateASTConsumer(CompilerInstance& CI, llvm::StringRef) override {
        return std::make_unique<YggOwnershipConsumer>(CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance&,
                   const std::vector<std::string>&) override { return true; }
};

} // namespace

static FrontendPluginRegistry::Add<YggOwnershipAction>
    X("pbsd-ownership", "PBSD ownership/lifetime checker");
ANALYSER

ok "analyser/ scaffold complete"

# userland/ placeholder
mkdir -p "$ROOT/pbsd/userland"
cat > "$ROOT/pbsd/userland/CMakeLists.txt" << 'CMAKE'
# C++23 userland utility rewrites — Phase 4+
# Add subdirectory per tool as rewrites are completed
CMAKE

# theme/ — Windows 7 Aero KDE theme scaffold
mkdir -p "$ROOT/pbsd/theme/plasma"
mkdir -p "$ROOT/pbsd/theme/kwin-effects"
mkdir -p "$ROOT/pbsd/theme/widget-style"

cat > "$ROOT/pbsd/theme/README.md" << 'THEME'
# PBSD Theme Layer

Windows 7 Aero aesthetic for KDE Plasma 6 — compositor layer only, zero kernel coupling.

## Components
- `plasma/`        — Plasma theme (colours, backgrounds, panel style)
- `kwin-effects/`  — KWin compositor effects (Aero glass blur, shadow depth, transparency)
- `widget-style/`  — Qt/KDE widget style matching Windows 7 control appearance

## Notes
- Aero glass: KWin blur + transparency effects with a specific colour matrix
- Taskbar: bottom-anchored panel, Windows 7 layout (start button, task buttons, clock, tray)
- Start menu: KRunner or custom Plasmoid matching Win7 start menu layout
- Window chrome: custom KWin decoration (title bar gradient, close/min/max button placement)

## References
- KDE Plasma theming documentation: https://develop.kde.org/docs/plasma/theme/
- KWin effect development: https://develop.kde.org/docs/plasma/kwin/
THEME

ok "theme/ scaffold complete"

# docs/ — document stubs
cat > "$ROOT/docs/PROVENANCE.md" << 'PROV'
# PBSD PROVENANCE LOG
**Author:** Odin Loch  
**Policy:** Every module must have an entry here before implementation begins.  
**Rule:** Spec source must be a public standard, published datasheet, or original design.  
         Never cite an existing implementation's source code.

## Entries

| Module | Spec source | Date | Author |
|---|---|---|---|
| pbsd::UniqueHandle / BorrowedHandle / SharedHandle | Original design — no external spec | $(date +%Y-%m-%d) | Odin Loch |
| pbsd::uda::Descriptor schema | Original design — no external spec | $(date +%Y-%m-%d) | Odin Loch |
| HardenedBSD kernel (inherited, not written) | HardenedBSD Project, BSD-2-Clause | $(date +%Y-%m-%d) | — |
| KDE Plasma 6 (inherited, not written) | KDE Project, GPL-2.0 | $(date +%Y-%m-%d) | — |
PROV

cat > "$ROOT/docs/GLOSSARY.md" << 'GLOSS'
# PBSD Glossary

| Term | Definition |
|---|---|
| **PBSD** | The OS project name. The kernel and overall system. |
| **BIFROST** | The VM/hypervisor subsystem and Windows interop bridge (Phase 6). |
| **UDA** | Universal Driver Architecture — descriptor-engine split for hardware support. |
| **Descriptor** | A declarative data structure describing one chip's register layout and init sequence. Never contains executable logic beyond the bytecode interpreter. |
| **Generic Engine** | The UDA component that interprets a descriptor and drives a device class (block, net, display, input). |
| **UniqueHandle<T>** | Move-only, single-owner kernel resource handle. The default for almost all kernel objects. |
| **BorrowedHandle<T>** | Non-owning view of a handle. Cannot outlive the owning handle. Annotated `[[kernel::no_escape]]`. |
| **SharedHandle<T>** | Refcounted handle. Rare; requires individual audit justification. |
| **CapabilityRights** | Bitmask attached to every handle. Rights only narrow on grant/duplication. |
| **Revocation tree** | Per-process capability lineage. Revoking one handle invalidates all derived handles. |
| **Profile** | A build configuration: embedded, server, or workstation. Same kernel source; different modules compiled in. |
| **PROVENANCE.md** | Living log mapping every module to its spec source (standard/datasheet/original). The legal clean-room trail. |
| **SPEC.md** | Per-module contract document. Must exist before implementation starts. |
| **Phase N** | Project plan phase number. See master-plan.md. |
GLOSS

cat > "$ROOT/docs/security/INVARIANTS.md" << 'INV'
# PBSD Security Invariants

These properties MUST hold across every contract and every module.  
They are stated once here and referenced by each contract's SPEC.md.  
If an implementation would violate any invariant, the implementation is wrong, not the invariant.

## SI-1: Rights only narrow
No operation grants a capability with rights exceeding those held by the caller.
`child_rights ⊆ caller_rights` always.

## SI-2: No ambient authority
A process can only act on resources for which it holds an explicit handle.
There is no "root can do anything" escape hatch in the native capability model.

## SI-3: W^X enforced everywhere
No memory page is simultaneously writable and executable at any point in time.

## SI-4: All kernel memory zero-initialised before use
No kernel allocation may be returned to a caller containing data from a prior use.

## SI-5: No blocking in interrupt context
Functions tagged as ISR entry points must not call any function that can sleep, block, or acquire a sleeping lock. Enforced by the static analyser Layer 4.

## SI-6: Revocation is total and immediate
Revoking a handle invalidates it and all handles derived from it in the same revocation tree, atomically from the holder's perspective.

## SI-7: Descriptor cannot self-escalate
A UDA descriptor executes only within the MMIO/IRQ capability scope granted to its generic engine. A descriptor cannot reference memory or I/O outside that grant.

## SI-8: VM guest is capability-isolated
The BIFROST VM guest (Windows) is a kernel object like any other. Its MMIO access, memory, and I/O are bounded by the VmHandle capability granted to it. The guest cannot access host kernel memory.
INV

ok "docs/ stubs created"

# --- Cursor workspace configuration ------------------------------------------
info "Writing Cursor workspace config..."

cat > "$ROOT/.cursor/settings.json" << 'CURSOR'
{
  "files.exclude": {
    "**/obj/**": true,
    "**/work/**": true,
    "hbsd/ports/**": true,
    "kde/src/qt6/**": true
  },
  "search.exclude": {
    "hbsd/ports/**": true,
    "hbsd/src/contrib/**": true,
    "kde/**": false
  },
  "C_Cpp.default.cppStandard": "c++23",
  "C_Cpp.default.compilerPath": "/usr/bin/clang++",
  "C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json",
  "clangd.arguments": [
    "--compile-commands-dir=${workspaceFolder}/build",
    "--clang-tidy",
    "--header-insertion=never",
    "--background-index"
  ],
  "editor.formatOnSave": true,
  "editor.rulers": [100],
  "[cpp]": {
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
  },
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.configureSettings": {
    "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
    "CMAKE_CXX_COMPILER": "clang++"
  }
}
CURSOR

cat > "$ROOT/.clang-tidy" << 'TIDY'
# PBSD clang-tidy configuration
# Layer 1 of the static analyser — syntactic ownership bans
Checks: >
  -*,
  cppcoreguidelines-no-malloc,
  cppcoreguidelines-owning-memory,
  cppcoreguidelines-pro-bounds-array-to-pointer-decay,
  cppcoreguidelines-pro-type-reinterpret-cast,
  modernize-use-nullptr,
  modernize-use-override,
  modernize-use-using,
  readability-identifier-naming,
  bugprone-use-after-move,
  bugprone-dangling-handle,
  performance-unnecessary-copy-initialization

CheckOptions:
  - key:   readability-identifier-naming.NamespaceCase
    value: lower_case
  - key:   readability-identifier-naming.ClassCase
    value: CamelCase
  - key:   readability-identifier-naming.FunctionCase
    value: lower_case

WarningsAsErrors: '*'
TIDY

cat > "$ROOT/pbsd.code-workspace" << 'WORKSPACE'
{
  "folders": [
    { "name": "pbsd (new code)",  "path": "./pbsd" },
    { "name": "docs",                   "path": "./docs" },
    { "name": "hbsd/src (kernel)",      "path": "./hbsd/src" },
    { "name": "kde/kwin",               "path": "./kde/kwin" },
    { "name": "kde/plasma-desktop",     "path": "./kde/plasma-desktop" }
  ],
  "settings": {
    "C_Cpp.default.cppStandard": "c++23",
    "C_Cpp.default.compilerPath": "/usr/bin/clang++",
    "C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json"
  },
  "extensions": {
    "recommendations": [
      "llvm-vs-code-extensions.vscode-clangd",
      "ms-vscode.cmake-tools",
      "eamodio.gitlens",
      "usernamehw.errorlens",
      "ms-vscode.cpptools"
    ]
  }
}
WORKSPACE

ok "Cursor workspace config written"

# --- Initial CMake configure -------------------------------------------------
info "Running initial CMake configure (generates compile_commands.json for clangd)..."

if command -v cmake >/dev/null 2>&1 && command -v clang++ >/dev/null 2>&1; then
    if cmake \
        -S "$ROOT/pbsd" \
        -B "$ROOT/build" \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON; then
        ok "CMake configure complete — compile_commands.json at $ROOT/build/"
    else
        warn "CMake configure failed — sources/scaffold are still ready"
        warn "Retry: cmake -S $ROOT/pbsd -B $ROOT/build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    fi
else
    warn "cmake or clang++ not found — skipping CMake configure"
    warn "Run: cmake -S $ROOT/pbsd -B $ROOT/build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
fi

# --- Summary -----------------------------------------------------------------
echo ""
echo -e "${GREEN}============================================================${NC}"
echo -e "${GREEN}  PBSD workspace ready${NC}"
echo -e "${GREEN}============================================================${NC}"
echo ""
echo "  Root:              $ROOT"
echo "  Open in Cursor:    cursor $ROOT/pbsd.code-workspace"
echo "  New code:          $ROOT/pbsd/"
echo "  HardenedBSD src:   $ROOT/hbsd/src/  (branch: $HBSD_BRANCH)"
echo "  KDE Plasma:        $ROOT/kde/plasma-desktop/  (tag: $KDE_PLASMA_TAG)"
echo "  KWin:              $ROOT/kde/kwin/  (tag: $KWIN_TAG)"
echo "  Docs:              $ROOT/docs/"
echo "  Build tree:        $ROOT/build/"
echo ""
echo "  Next steps:"
echo "    1. Open pbsd.code-workspace in Cursor"
echo "    2. Install recommended extensions (clangd, CMake Tools, GitLens, ErrorLens)"
echo "    3. Read docs/PROVENANCE.md and docs/security/INVARIANTS.md"
echo "    4. Complete Phase 0 documentation checklist (see master-plan.md)"
echo "    5. Build HardenedBSD from hbsd/src/ in a FreeBSD VM or on real hardware"
echo ""
echo -e "${CYAN}  Code volume you are working against:${NC}"
echo "    HardenedBSD src:    ~10–12M lines  (your deepest eventual target)"
echo "    KDE Plasma + KWin:  ~3–4M lines    (compositor rewrite Phase 7)"
echo "    KDE Frameworks:     ~6–8M lines    (use mostly as-is)"
echo "    Your new code:      ~2–5M lines    (target, across all phases)"
echo "    Total in tree:      ~35–45M lines"
echo ""
echo -e "${YELLOW}  Wayland note: KDE 6.8 (October 2026) will be Wayland-only.${NC}"
echo -e "${YELLOW}  KDE 6.7 (current) is the last version supporting X11.${NC}"
echo -e "${YELLOW}  Build your theming work against Wayland from day one.${NC}"
echo ""
