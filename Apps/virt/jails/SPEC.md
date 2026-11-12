<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Jails and Containers

By Odin Loch

| | |
|---|---|
| ID | `VM02` |
| Bundle | `org.pbsd.jails` |
| Category | virt |
| Priority | P2 |
| Backing repos | ParanoidBSD (`pbsd/kernel/jail`, `hbsd/src/usr.sbin/jail`, plan items P4-02 and section 7) |
| Status | Planned. Nothing built. |

## 1. Purpose

Jails and Containers runs services and tools in isolated environments that share the host kernel: native jails for BSD userlands and Linux containers from OCI images. It is for developers, home administrators and anyone who wants to run server software without giving it the whole machine. It replaces Docker Desktop, Podman Desktop, and hand-written `jail.conf` files.

Each jail or container is a capability domain. It starts with nothing and receives only the handles the user attached: a root dataset, mounts, a network endpoint, device nodes.

## 2. Scope

### In scope

- Library of jails, Linux containers and multi-service stacks.
- Templates and a creation wizard.
- OCI image store: pull, import, verify signatures, inspect, tag, prune, export, build (later phase).
- Start, stop, restart, pause, autostart, restart policies, health checks.
- Networking with VNET: private networks, published ports, routes.
- Resource limits: CPU, memory, processes, files, I/O, storage quota.
- Mounts and named volumes on ZFS.
- Snapshots, rollback, clones, export and import.
- Logs, process list, statistics, shell hand-off to SY03.
- Compose-style stacks.
- Per-container capability view and security profiles.

### Out of scope

| Item | Owner |
|---|---|
| Full virtual machines, other kernels | VM01 |
| Running one foreign desktop app with launcher integration | VM03 |
| Host firewall rules (VM02 hands its rules over) | SE02 |
| VPN and Tor routes | SE03 |
| Registry credentials, signing keys | SE04 |
| Dataset and pool administration | AD02 |
| Scheduled off-site backup | AD03 |
| The terminal itself | SY03 |
| Host packages and system updates | SY05 |
| Cluster orchestration across machines | not planned |

## 3. Users and top tasks

Personas:

- **Dara, developer.** Runs databases and build environments from OCI images; uses compose files from projects.
- **Sam, home administrator.** Runs a few long-lived services in native jails with autostart and snapshots before upgrades.
- **Ines, analyst.** Runs an untrusted tool once in a container with no network and a read-only input folder.
- **Lee, packager.** Builds and signs images and checks what is inside them.

| # | Task | Target |
|---|---|---|
| T1 | Run a container from a local image | 3 steps: select image, "Run", accept defaults |
| T2 | Pull an image and verify its signature | 3 steps: `Ctrl+L`, type reference, "Pull" (verification is automatic) |
| T3 | Create a native jail from a template | 3 steps: `Ctrl+N`, pick template, "Create and start" |
| T4 | Open a shell inside | 1 step: `Ctrl+Shift+T` (opens SY03) |
| T5 | See the logs of a failing service | 1 step: select, Logs tab (`Ctrl+3`) |
| T6 | Publish a port | 3 steps: Network tab, "Publish port", numbers |
| T7 | Limit memory and CPU | 2 steps: Limits tab, move sliders (applies live) |
| T8 | Mount a folder read-only | 3 steps: Mounts tab, "Add folder", pick |
| T9 | Snapshot before an upgrade, roll back after | 1 step each: `Ctrl+T`, `Ctrl+Shift+Z` on the snapshot row |
| T10 | Bring up a compose stack | 2 steps: drop `compose.yaml`, "Up" |
| T11 | Update a container to a newer image | 3 steps: "Check for newer image", review digest change, "Recreate" |
| T12 | See what a container can reach | 1 step: Capabilities tab |
| T13 | Run something once and leave nothing behind | 2 steps: image, "Run once (disposable)" |

## 4. Features

Status of the backend (Verified): `pbsd/kernel/jail/pbsd.kernel.jail.cppm` (94 lines) ports the constants, flags and validators of `sys/jail.h` (`kJailApiVersion = 2`, `kJailMax = 999999`, `PrisonState {Invalid, Alive, Dying}`). `hbsd/src/usr.sbin/jail` exists; `pbsd/usr.sbin/` has no jail tool. Linux containers depend on the Linux personality (P4-02, acceptance "Docker/Podman containers run") which has no directory in the tree yet (`pbsd/kernel/personality` is absent). Plan section 7 states that namespaces and cgroup v2 live in the Linux personality, jails in the BSD personality, "both mapped to capability domains". All features below are Proposed.

### 4.1 Library

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F001 | Library | All jails, containers and stacks of the user, from `svc.jail`; list with name, kind (Jail, Linux), image or template, state, uptime, CPU, memory, ports, badges | M | 1 |
| VM02-F002 | States | Created, Starting, Running, Paused, Stopping, Exited (with code), Restarting, Unhealthy, Removing | M | 1 |
| VM02-F003 | Sidebar | Containers, Stacks, Images, Volumes, Networks, Registries, Templates, Jobs | M | 1 |
| VM02-F004 | Grouping | Group by stack, by kind, by network, or flat | S | 1 |
| VM02-F005 | Search and filter | By name, image, label, port, state, network; saved filters | M | 1 |
| VM02-F006 | Labels and notes | Key=value labels (OCI labels shown read-only, user labels editable) and a Markdown note | S | 2 |
| VM02-F007 | Rename | `F2`; the jail name and hostname can follow or stay | M | 1 |
| VM02-F008 | Remove | Stops, then moves the root dataset to a 14-day held snapshot (setting); "Remove for ever" destroys it; named volumes are never removed with a container | M | 1 |
| VM02-F009 | Multi-select | Start, stop, restart, remove, snapshot on a selection | S | 2 |
| VM02-F010 | Live columns | CPU, memory, network and block I/O refresh at 1 Hz while visible | M | 1 |
| VM02-F011 | System search | Names contributed to `svc.search` | C | 3 |

### 4.2 Templates and creation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F012 | Create wizard | Three pages: Base, Resources and network, Review. "Create" enabled from page 1 | M | 1 |
| VM02-F013 | Jail bases | Thin jail (ZFS clone of a base release dataset), thick jail (full copy), empty jail (user supplies a root), service jail (one host service confined with its own view of the file system) | M | 1 |
| VM02-F014 | Base releases | Base datasets are installed and updated through SY05; VM02 lists those present with version and signature state | M | 1 |
| VM02-F015 | Linux container from image | Root file system is a clone of the image's top layer dataset; config (command, env, user, working directory, ports, volumes) is read from the image | M | 1 |
| VM02-F016 | Template files | Signed TOML templates in the bundle (web server, database, DNS resolver, build environment, empty); user templates in the app data directory | S | 2 |
| VM02-F017 | Save as template | Save a container's configuration (not its data) as a user template | S | 2 |
| VM02-F018 | Command and entry point | Override entry point and arguments; argument list editor, no shell string parsing | M | 1 |
| VM02-F019 | Environment | Key=value editor; import from an env file; values marked secret come from `svc.keyring` | M | 1 |
| VM02-F020 | User | Run as a named user or numeric uid:gid inside; uid 0 inside maps to an unprivileged host identity | M | 1 |
| VM02-F021 | Hostname and hosts | Hostname, extra hosts entries, DNS servers and search list | S | 1 |
| VM02-F022 | Run once (disposable) | Creates a clone, runs, shows output, destroys the dataset at exit; default network none | M | 2 |
| VM02-F023 | Review as text | The resulting definition shown as TOML with copy button; equivalent `jail.conf` fragment shown for jails | S | 2 |
| VM02-F024 | Import run command | Paste a `docker run …` or `podman run …` line; the app parses the flags it supports and lists the ones it ignores | C | 3 |
| VM02-F025 | Recreate | Rebuild a container from its definition and a (possibly newer) image while keeping volumes; the old root is kept as a snapshot | M | 2 |

### 4.3 Images and registries

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F026 | Image store | Content-addressed store; each layer is a ZFS dataset snapshot; images share layers; list shows repository, tag, digest (short), size, shared size, created, signature state, in-use count | M | 1 |
| VM02-F027 | Pull | By reference `registry/repo:tag` or `@sha256:…`; OCI Distribution API over HTTPS; resumable; per-layer progress | M | 1 |
| VM02-F028 | Platform choice | Selects `linux/amd64` (or host arch) or `freebsd/amd64` from an image index; other platforms listed but not runnable without translation (P4-06) | M | 1 |
| VM02-F029 | Digest pinning | Every container records the manifest digest it was created from; tags are display only | M | 1 |
| VM02-F030 | Signature verification | Verify detached signatures stored beside the image (cosign-style keyed signatures and Notary v2 envelopes) against public keys in the trust store; result: verified (key name), unsigned, bad signature | M | 2 |
| VM02-F031 | Trust policy | Per registry and repository pattern: require signature by key set, allow unsigned with warning, or block; default for new registries is "warn"; policy file is human-readable | M | 2 |
| VM02-F032 | No keyless online trust | Signatures that need a third-party online transparency or identity service are shown as "not verifiable offline"; no vendor account is used | M | 2 |
| VM02-F033 | Registries | List of registries the user added: URL, TLS certificate pin (optional), credentials reference in SE04, route (clear, VPN, Tor), mirror order | M | 1 |
| VM02-F034 | First-contact log | Each new registry host contacted is logged to `svc.audit` and shown once as a toast | M | 1 |
| VM02-F035 | Offline import | Load `docker save` archives and OCI image layout directories or tars from the file portal; verify digests | M | 1 |
| VM02-F036 | Export image | Save as OCI layout tar; optional signature | S | 2 |
| VM02-F037 | Inspect | Layers with sizes and creating instruction, config (env, entry point, user, ports, volumes), labels, history, total and unique size | M | 1 |
| VM02-F038 | Browse image files | Read-only file tree of any layer or the merged root, with search; open a file through SY02 in a disposable viewer | S | 2 |
| VM02-F039 | Contents list | Package list from the image's package databases (apk, dpkg, rpm, pkg) and an SBOM if one is attached (SPDX, CycloneDX); export as SPDX JSON | S | 3 |
| VM02-F040 | Known-issue check | Match the contents list against the local advisory database kept by SY05; offline; shows age of the database | C | 3 |
| VM02-F041 | Tag and untag | Local tags; removing the last tag leaves the image dangling | S | 1 |
| VM02-F042 | Prune | Remove dangling and unused images; shows space to be freed first; undo window through held snapshots | M | 2 |
| VM02-F043 | Newer image check | Manual or scheduled check of a tag's digest; shows old and new digest, signature state and layer diff size; never updates by itself | S | 2 |
| VM02-F044 | Push | Push a local image to a registry the user has credentials for | C | 3 |
| VM02-F045 | Build | Build from a build file (Containerfile subset: FROM, RUN, COPY, ADD local only, ENV, ARG, WORKDIR, USER, EXPOSE, VOLUME, ENTRYPOINT, CMD, LABEL); each step is a ZFS snapshot; build network off by default | S | 3 |
| VM02-F046 | Sign | Sign a local image with a key from SE04 | S | 3 |
| VM02-F047 | Image hostile-input rule | Layer tars are unpacked by `layer-helper` in capability mode into a fresh dataset; paths that escape, device nodes, set-id bits on host-meaningful ids, and oversize expansions (ratio limit) are refused or neutralised and reported | M | 1 |

### 4.4 Lifecycle

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F048 | Start, stop, restart | Stop sends the configured signal (default TERM), waits (default 10 s), then kills | M | 1 |
| VM02-F049 | Kill with signal | Choose a signal from a list | S | 1 |
| VM02-F050 | Pause and resume | Freeze all processes in the domain | S | 2 |
| VM02-F051 | Restart policy | No, on failure (max retries, back-off 1–60 s), always, unless stopped | M | 2 |
| VM02-F052 | Autostart | With the system or at login; order by dependency and priority; done by `svc.jail` | M | 2 |
| VM02-F053 | Health check | Command inside, TCP connect, or HTTP GET on the container's own network; interval, timeout, retries, start period; state Unhealthy raises a notification | S | 2 |
| VM02-F054 | Exit details | Exit code, signal, out-of-memory flag, last 20 log lines, in the inspector | M | 1 |
| VM02-F055 | Host shutdown | Ordered stop with per-container timeout | M | 2 |
| VM02-F056 | Run a command | Run a one-off command inside a running container and show its output; history per container | S | 2 |
| VM02-F057 | Init process | Optional minimal init as pid 1 to reap children and forward signals | S | 2 |
| VM02-F058 | Scheduled runs | Run a disposable container on a schedule (timer in `svc.jail`); output kept as a log entry | C | 3 |
| VM02-F059 | Service survival | Containers keep running when the app closes or crashes and when `svc.jail` restarts | M | 1 |

### 4.5 Networking

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F060 | Modes | None (default for disposable), private network (default), host network (needs administrator approval), shared with another container | M | 1 |
| VM02-F061 | VNET | Each container has its own network stack with an interface pair into a bridge owned by `svc.jail`; no access to host interfaces | M | 1 |
| VM02-F062 | Private networks | Create named networks: subnet (IPv4 and IPv6 ULA), gateway, internal-only flag, route (clear, VPN, Tor, fail closed) | M | 1 |
| VM02-F063 | Name resolution | Built-in resolver per network: containers resolve each other by name and alias | M | 2 |
| VM02-F064 | Publish ports | Host address (loopback default), host port, container port, protocol; conflict check; listed in SE02 | M | 1 |
| VM02-F065 | LAN exposure consent | Publishing on a non-loopback address shows the interfaces affected and asks once per container | M | 1 |
| VM02-F066 | Outbound rules | Per container: allow all, allow list of hosts and ports, deny all; rules handed to SE02 | M | 2 |
| VM02-F067 | Static addresses | Fixed IPv4/IPv6 and MAC per container per network | S | 2 |
| VM02-F068 | Multiple networks | Attach and detach networks while running | S | 2 |
| VM02-F069 | Bandwidth limit | Up and down limit per container | C | 3 |
| VM02-F070 | Connections view | Live table of sockets and peers of a container; "Open in Firewall" (SE02) | S | 3 |
| VM02-F071 | Capture | Hand a capture handle for the container's interface to DV05 | C | 3 |
| VM02-F072 | Network badge | `[net]` badge when any route exists; "offline" otherwise | M | 1 |

### 4.6 Resource limits

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F073 | CPU | Share weight, hard cap in percent of one CPU, CPU set | M | 1 |
| VM02-F074 | Memory | Hard limit, soft limit, swap limit; out-of-memory action (kill container, kill largest process) | M | 1 |
| VM02-F075 | Processes and threads | Maximum count (default 1,024) to stop fork bombs | M | 1 |
| VM02-F076 | Open files and sockets | Maximum descriptors, maximum listening sockets | S | 2 |
| VM02-F077 | Block I/O | Read and write bytes per second and IOPS limits | S | 2 |
| VM02-F078 | Storage quota | ZFS quota and reservation on the root dataset and on each volume | M | 1 |
| VM02-F079 | Live change | Limits apply without restart where the kernel allows; the UI marks the ones that need a restart | M | 2 |
| VM02-F080 | Pressure view | Shows CPU, memory and I/O pressure (stall percentages) per container, from the kernel's pressure telemetry (plan P1-08) | S | 3 |
| VM02-F081 | Presets | Small (1 CPU, 512 MiB), Medium (2, 2 GiB), Large (4, 8 GiB), Unlimited (with warning) | S | 1 |
| VM02-F082 | Limit hits | Counters and a notification when a limit was reached (memory kills, throttled time, denied forks) | S | 2 |

### 4.7 Storage, mounts and volumes

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F083 | Root file system | A ZFS dataset per container; writable or read-only root (with tmpfs for `/tmp`, `/run`) | M | 1 |
| VM02-F084 | Folder mounts | Host folder picked in the file portal, mounted at a path inside; read-only by default; `nosuid`, `noexec` options; each mount is a derived handle | M | 1 |
| VM02-F085 | Named volumes | ZFS datasets managed by the app: create, rename, quota, snapshot, attach to several containers, remove with typed confirmation | M | 1 |
| VM02-F086 | Volume browser | Open a volume in SY02 through a read-only handle | S | 2 |
| VM02-F087 | Temporary file systems | tmpfs mounts with size limit and mode | S | 1 |
| VM02-F088 | Device nodes | devfs rule sets: minimal (null, zero, random, pts) by default; extra nodes need a device grant (`dev.usb:<class>`, `dev.gpu.compute`) and are listed in the capability view | M | 2 |
| VM02-F089 | Secrets | Secret values from `svc.keyring` appear as files on an in-memory mount (mode 0400) or as environment variables; never written to the definition or logs | M | 2 |
| VM02-F090 | Config files | Small text files kept with the definition and mounted read-only; edited in a `codeEdit` view | S | 2 |
| VM02-F091 | Copy in and out | Copy files between host (portal) and a container path; copies out go to a quarantine folder with a provenance tag | S | 2 |
| VM02-F092 | Changes since image | List of files added, changed and deleted relative to the image or base (ZFS diff) | S | 2 |
| VM02-F093 | Encryption | Root and volume datasets can use ZFS native encryption with keys in `svc.keyring` | S | 3 |
| VM02-F094 | Space view | Per container and per volume: used, referenced, snapshots, shared with image | M | 2 |

### 4.8 Snapshots, clones, export

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F095 | Snapshot | Recursive ZFS snapshot of root and, optionally, attached volumes; name and note | M | 1 |
| VM02-F096 | Roll back | Stops the container, saves the present state as "before rollback", rolls back, starts again if it was running | M | 1 |
| VM02-F097 | Automatic snapshots | Before recreate, before image update, on a schedule; retention count | S | 2 |
| VM02-F098 | Clone | New container from a snapshot (ZFS clone), with new name, addresses and identity | M | 2 |
| VM02-F099 | Commit to image | Make a new local image from a container's present state | S | 3 |
| VM02-F100 | Export | Container as a ZFS send stream plus definition, or as a root tar; optional signature | S | 2 |
| VM02-F101 | Import | From an export of this app, a root tar, or an existing jail directory (`jail.conf` parsed for the supported parameters, others listed) | S | 2 |
| VM02-F102 | Back up | "Back up" opens AD03 with the datasets preselected | S | 2 |

### 4.9 Logs, shell and processes

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F103 | Log capture | Standard output and error of pid 1 captured by `svc.jail` with timestamps; rotation by size (10 MiB × 5 default) | M | 1 |
| VM02-F104 | Log view | Follow mode, pause, wrap, filter by text or regular expression, level colouring for common formats (JSON lines, logfmt, syslog), jump to time, copy, save | M | 1 |
| VM02-F105 | Stack log | Merged log of all services in a stack with a coloured service column | S | 2 |
| VM02-F106 | Shell hand-off | "Open shell" emits an intent to SY03 with a handle to a new pty whose far end runs the chosen shell inside the container; SY03 shows the container name and badge in the tab | M | 1 |
| VM02-F107 | Shell choice | Shell path and user per container (default: the image user and `/bin/sh`) | S | 1 |
| VM02-F108 | Attach | Attach to the main process's terminal (read-only or interactive) in SY03 | S | 2 |
| VM02-F109 | Process list | Processes inside: pid, user, CPU, memory, command; send a signal; "Open in System Monitor" (SY04) at the domain | M | 2 |
| VM02-F110 | Statistics | Graphs of CPU, memory, network, block I/O; 1 min, 1 h, 24 h | S | 2 |
| VM02-F111 | Events | Timeline of lifecycle events (created, started, health change, killed for memory, limit hit, mount added) | S | 2 |
| VM02-F112 | Open in Logs | Jump to AD04 filtered to the container | S | 2 |
| VM02-F113 | Local log explanation | With `ai.local`: summarise why a container exited from its last log lines; local only, off by default | C | 3 |

### 4.10 Stacks

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F114 | Stack model | A stack is a named set of services, networks, volumes and secrets in one file; native format is TOML | M | 2 |
| VM02-F115 | Compose import | Read `compose.yaml` (Compose Specification subset: services with image, build, command, entrypoint, environment, env_file, ports, volumes, networks, depends_on, healthcheck, restart, user, working_dir, read_only, tmpfs, cap_drop, labels, deploy.resources.limits; top-level networks, volumes, secrets, configs) | M | 2 |
| VM02-F116 | Unsupported keys report | Keys outside the subset are listed with file line numbers; privileged keys (`privileged`, `cap_add`, `pid: host`, `network_mode: host`, host device paths) are refused or mapped to a grant request | M | 2 |
| VM02-F117 | Up and down | Start in dependency order waiting for health where declared; down stops in reverse order; volumes are kept | M | 2 |
| VM02-F118 | Plan before apply | Shows a diff: services to create, recreate, remove; images to pull; ports to publish; grants needed | M | 2 |
| VM02-F119 | Scale | Run N copies of a service (names suffixed); ports must not collide | C | 3 |
| VM02-F120 | Variables | `${VAR}` substitution from a `.env` file beside the stack file or from the editor; missing variables are errors | S | 2 |
| VM02-F121 | Stack editor | `codeEdit` with schema validation and a side form for common fields; graph view of services and dependencies with `nodeGraph` (read-only) | S | 3 |
| VM02-F122 | Stack folder handle | Relative paths in a stack file resolve only inside the folder the user granted | M | 2 |
| VM02-F123 | Stack snapshot | One atomic snapshot across all service roots and volumes | S | 3 |
| VM02-F124 | Export stack | Write native TOML or compose YAML | S | 3 |

### 4.11 Security and capability mapping

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F125 | Rootless by design | No container process holds host root or any ambient authority (SI-2); `svc.jail` creates the domain and passes handles; identity 0 inside has rights only over the container's own objects | M | 1 |
| VM02-F126 | Capability view | Tree of the domain's handles: root dataset, volumes, folder mounts, network endpoint, published ports, device nodes, secrets; rights and source for each | M | 2 |
| VM02-F127 | Revoke | Revoke one handle (the mount or device disappears at once, SI-6) or the whole domain (emergency stop) | M | 2 |
| VM02-F128 | Reach summary | Plain-language list of what the container can and cannot reach, shown before first start and in the inspector | M | 1 |
| VM02-F129 | Profiles | Strict (read-only root, no network, no new privileges, minimal devfs), Standard (default), Custom | M | 2 |
| VM02-F130 | Jail parameters | For jails: `allow.*` parameters shown as switches with plain explanations, all off by default (raw sockets, mount, sysvipc, chflags, mlock); `securelevel`, `enforce_statfs`, `children.max` | M | 2 |
| VM02-F131 | Linux syscall filter | For Linux containers: seccomp profile chooser (default, strict, custom JSON); Linux capability set shown and dropped to a small default; requests to add capabilities are mapped to PBSD grants or refused | M | 2 |
| VM02-F132 | Hardening flags | Per container: W^X enforcement cannot be turned off (SI-3); ASLR and other HardenedBSD process hardening are on; an exemption needs a named binary and is logged | M | 2 |
| VM02-F133 | Privileged requests | "Privileged" containers do not exist. A definition asking for it gets a list of the specific grants it would need and the user chooses each or none | M | 2 |
| VM02-F134 | Image trust badge | Each container shows the trust state of its image (verified, unverified, tampered) next to its name | M | 2 |
| VM02-F135 | Audit | Create, start, stop, mount change, port publish, device grant, registry first contact, signature failures: to `svc.audit` | M | 2 |
| VM02-F136 | Policy lock | Administrator policy can forbid host networking, LAN publishing, unsigned images or device grants | S | 3 |
| VM02-F137 | Open in Permissions | Jump to AD05 at the domain's lineage node | S | 2 |

### 4.12 Automation, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| VM02-F138 | Command-line tool | `ctr` CLI over the `svc.jail` interface: ps, run, stop, logs, exec, pull, images, stack up/down | S | 2 |
| VM02-F139 | Intents | `open` for compose files, stack TOML, OCI archives; `ctr:` URLs to a container's page | M | 2 |
| VM02-F140 | Command palette | Every action and every container by name in `Ctrl+K` | M | 1 |
| VM02-F141 | Definition history | Last 20 versions of each definition with diff and restore | S | 2 |
| VM02-F142 | Repair | A definition that fails validation opens in a repair view | S | 2 |
| VM02-F143 | App crash safety | The app holds no container state; nothing is lost on crash | M | 1 |
| VM02-F144 | Interrupted jobs | Pulls resume by layer; a failed unpack destroys its partial dataset | M | 1 |
| VM02-F145 | Help | Help pages per view; a "coming from Docker" page mapping commands and terms | S | 2 |

## 5. GUI

### 5.1 Window map

```
Jails and Containers
├─ Main window
│  ├─ Sidebar: Containers · Stacks · Images · Volumes · Networks · Registries · Templates · Jobs
│  ├─ Content list for the chosen section
│  ├─ Detail page (replaces the list; Alt+Left returns)
│  │  └─ Tabs: Overview · Logs · Processes · Network · Mounts · Limits · Snapshots · Capabilities · Definition
│  ├─ Inspector (F4): summary of the selection, reach summary, quick actions
│  ├─ Create wizard (sheet, 3 pages)
│  ├─ Pull image dialog (reference, platform, verification result)
│  ├─ Image inspect page (layers, config, files, contents)
│  ├─ Stack page (services, plan, merged log, graph, editor)
│  ├─ Network editor dialog · Publish port popover
│  ├─ Volume dialog · Add mount popover · Secret picker
│  ├─ Registry dialog (URL, pin, credentials, route, trust policy)
│  ├─ Trust policy editor
│  ├─ Prune dialog · Remove confirmation · Roll back confirmation
│  └─ Preferences (Ctrl+,)
├─ Shell-drawn prompts (SH04): folder picker, device grant, administrator approval
└─ Hand-offs: SY03 (shell, attach), SY02 (browse), SE02 (rules), AD05 (lineage), AD04 (logs)
```

### 5.2 Main window

Shown at 1180×720, Containers section, one row selected.

```
+----------------------------------------------------------------------------------------+
| [=] Jails and Containers             [ Search  Ctrl+F ]          [net]  [▁] [▢] [✕]   |
+------------+---------------------------------------------------+-----------------------+
|> Containers| [+ New] [Pull image] | [▶] [■] [↻] [Shell] [Logs] (1)| postgres-dev      (4) |
|  Stacks  2 |                         [group: stack v] [filter v] | ● Running · 2 d 4 h   |
|  Images 14 +---------------------------------------------------+ Linux · ✓ verified    |
|  Volumes 6 | Name          Kind  Image/base      State  CPU  Mem (2)| postgres:17@9c1e…   |
|  Networks 3| ▾ shop (stack)                                      | ---------------------  |
|  Registries|   > postgres-dev Linux postgres:17  ● Run  2%  310M | Can reach:        (5) |
|  Templates |     web          Linux nginx:1.27   ● Run  0%   24M |  volume pgdata (rw)   |
|  Jobs    1 |     worker       Linux shop/worker  ✕ Exit 1  –   – |  network shop-net     |
|            | ▾ (no stack)                                        |  port 127.0.0.1:5432  |
|            |     dns-jail     Jail  15.0-base    ● Run  0%   38M | Cannot reach:         |
|            |     build-env    Jail  15.0-base    ○ Stopped       |  your files, devices, |
|            |     scratch-7f   Linux alpine:3.22  ○ Created  disp.|  the internet         |
|            |                                                     | [Open shell] [Logs]   |
|            |                                                     | [Snapshot] [Revoke…]  |
+------------+---------------------------------------------------+-----------------------+
| 4 running · 1 exited · images 6.2 GiB (3.1 shared) (3) · job: pulling redis:8 62%       |
+----------------------------------------------------------------------------------------+
```

1. Toolbar: New is primary. Run controls apply to the selection.
2. Container table (`treeView` with columns when grouped, `tableView` when flat). A failed row shows its exit code in `--danger` with an icon.
3. Status bar: counts, image store size, running job.
4. Inspector header: state, kind, trust badge, pinned digest.
5. Reach summary and quick actions. "Revoke…" is the outlined destructive button.

States: empty ("Nothing is running here yet", `[+ New]`, link "Pull an image"), loading, service error banner, unsupported (Linux personality missing: Linux containers section shows "Linux containers are not available on this system" and jails still work), read-only (policy), narrow (sidebar overlay, inspector sheet, columns reduce to name and state).

### 5.3 Other views and dialogs

#### Create wizard, page 1 (sheet, 720×500)

```
+----------------------------------------------------------------------+
| New container                               1 Base   2 Resources  3   |
+----------------------------------------------------------------------+
| (•) Linux container from an image   ( ) Native jail   ( ) Template    |
| Image [ postgres:17  ✓ verified (key "postgres-release")   v] [Pull…] |
| Name  [ postgres-dev            ]                                     |
| Command      (from image) docker-entrypoint.sh postgres    [Change…]  |
| Environment  POSTGRES_PASSWORD = ●●●●●●  (secret)  [+ Add]      (1)   |
| Volumes      /var/lib/postgresql/data → new volume "pgdata"     (2)   |
| Ports        5432/tcp → [127.0.0.1]:[5432]   [x] publish              |
| [ ] Run once and remove (disposable)                                  |
+----------------------------------------------------------------------+
| [Cancel]                        [Next →]        [Create and start]    |
+----------------------------------------------------------------------+
```

(1) Secret values go to `svc.keyring`. (2) Volumes declared by the image are proposed as named volumes. States: image not present (Pull first), unverified image (warning line with policy result), blocked by policy.

#### Pull image (dialog, 640×420)

```
+--------------------------------------------------------------+
| Pull an image                                                 |
| Reference [ registry.example.org/library/redis:8          ]   |
| Registry  registry.example.org · pinned certificate ✓ · clear |
| Platform  [ linux/amd64 v]                                    |
| Layers    [■■■■■■■■□□] 5 of 7 · 48.2 / 77.9 MiB               |
| Signature ✓ verified with key "redis-maintainers" (Ed25519)   |
| Digest    sha256:4b0e…a71c   [Copy]                           |
| Policy    This registry requires a valid signature.           |
| [Cancel]                                [Pull]  [Pull and run]|
+--------------------------------------------------------------+
```

States: resolving, downloading, verifying, unsigned (`--warn`, button reads "Pull unsigned image"), bad signature (`--danger`, no override in the dialog; the policy must be changed first), offline (reference resolved from the local store only), first contact (note "This is the first connection to this registry").

#### Detail page, Logs tab (1180×720)

```
+----------------------------------------------------------------------------------------+
| ← postgres-dev   ● Running   [■] [↻] [Shell]                       [Snapshot] [⋯]      |
| [Overview][Logs][Processes][Network][Mounts][Limits][Snapshots][Capabilities][Definition]|
+----------------------------------------------------------------------------------------+
| [ filter text or /regex/ ] [stdout x] [stderr x] [since: start v] [Follow ●] [Wrap] [⤓] |
| 11:02:14.120 out  database system is ready to accept connections                        |
| 11:02:19.553 err  WARNING: could not open statistics file "pg_stat/global.stat"         |
| 11:05:40.001 out  checkpoint starting: time                                             |
| …                                                                                       |
+----------------------------------------------------------------------------------------+
| 1,204 lines · 3 warnings · following                                                    |
+----------------------------------------------------------------------------------------+
```

Log lines are plain text in a monospace `modelListView`; terminal escape sequences are stripped except SGR colours.

#### Limits tab

```
+----------------------------------------------------------------------+
| Preset [Medium v]                                                     |
| CPU cap      [====|------]  200 % (2 CPUs)     now 2 %                |
| CPU weight   [ 100 ]        CPU set [ all v]                          |
| Memory hard  [=====|-----]  2.0 GiB            now 310 MiB            |
| Memory soft  [===|-------]  1.0 GiB   On limit [kill container v]     |
| Processes    [ 1024 ]       now 14                                    |
| Storage      root [ 10 ] GiB (used 1.2)   pgdata [ 20 ] GiB (used 3.4)|
| I/O          read [ none v]  write [ none v]                          |
| Limit hits   memory kills 0 · CPU throttled 0 s · forks denied 0      |
| ⓘ Changes apply now. Storage reduction below current use is refused. |
+----------------------------------------------------------------------+
```

`slider` + `spinBox` pairs; live "now" values beside each.

#### Capabilities tab

```
+----------------------------------------------------------------------+
| Domain postgres-dev   profile: Standard   image ✓ verified            |
| ├─ root      dataset …/ctr/postgres-dev        R W     svc.jail       |
| ├─ volume    pgdata → /var/lib/postgresql/data R W               [✕]  |
| ├─ network   shop-net (10.88.2.0/24, internal) R W               [✕]  |
| ├─ port      127.0.0.1:5432 → 5432/tcp         listen            [✕]  |
| ├─ secret    POSTGRES_PASSWORD (keyring)       R                 [✕]  |
| └─ devices   null zero random pts              R W                    |
| Linux capabilities kept: CHOWN SETUID SETGID DAC_OVERRIDE (inside only)|
| Syscall filter: default   W^X: enforced   New privileges: blocked     |
| [Revoke selected] [Change profile…] [Open in Permissions] [Revoke all]|
+----------------------------------------------------------------------+
```

#### Stack page, plan view

```
+----------------------------------------------------------------------+
| ← shop (stack)   source: ~/Projects/shop/compose.yaml  [Edit] [Up]    |
| Plan                                                                  |
|  + create   cache      redis:8        pull needed · ✓ signed          |
|  ~ recreate web        nginx:1.27 → 1.28   digest changes             |
|  = keep     postgres-dev                                              |
|  ! ignored  services.web.privileged (line 14): not supported   (1)    |
|  ? grant    services.worker.volumes "./data" → folder access needed   |
| Ports to publish: 127.0.0.1:8080 → web:80                             |
| [Cancel]                                   [Apply plan]               |
+----------------------------------------------------------------------+
```

(1) Unsupported and refused keys are listed with line numbers, never dropped silently.

#### Registry dialog

```
+--------------------------------------------------------------+
| Registry                                                      |
| URL          [ https://registry.example.org              ]    |
| Certificate  ( ) system trust  (•) pin this certificate ✓     |
| Credentials  [ none v]  (items come from Passwords and Keys)  |
| Route        [ clear v]   Mirror of [ none v]                 |
| Trust policy [ require signature v]  Keys [redis-maintainers] |
| [Test connection]                       [Cancel]  [Save]      |
+--------------------------------------------------------------+
```

### 5.4 Menus

```
File       New container… Ctrl+N · New stack… Ctrl+Shift+N · Pull image… Ctrl+L ·
           Import… Ctrl+I · Export… Ctrl+E · Open stack file… Ctrl+O ·
           Preferences Ctrl+, · Close Ctrl+W · Quit Ctrl+Q
Edit       Undo Ctrl+Z · Redo Ctrl+Shift+Z · Rename F2 · Clone Ctrl+D · Select all Ctrl+A ·
           Find Ctrl+F · Remove Delete · Remove for ever Shift+Delete
View       Containers Alt+1 · Stacks Alt+2 · Images Alt+3 · Volumes Alt+4 · Networks Alt+5 ·
           Tab: Overview Ctrl+1 … Definition Ctrl+9 · Group by ▸ · Sidebar F9 ·
           Inspector F4 · Refresh F5 · Follow log Ctrl+Shift+F
Container  Start Ctrl+R · Stop Ctrl+. · Restart Ctrl+Shift+R · Pause · Kill with signal ▸ ·
           Open shell Ctrl+Shift+T · Attach · Run command… Ctrl+Shift+E · Snapshot Ctrl+T ·
           Roll back… · Recreate… · Check for newer image · Autostart · Revoke all…
Image      Inspect Enter · Run… · Tag… · Verify again · Export… · Sign… · Build… · Prune…
Stack      Up Ctrl+U · Down Ctrl+Shift+U · Plan · Restart service ▸ · Merged log · Export…
Help       Help F1 · Coming from Docker · About
```

### 5.5 Keyboard and pointer

App shortcuts are those in 5.4. In the log view: `End` follow, `Space` pause, `/` filter, `n`/`N` next and previous match, `Ctrl+C` copy selection.

Context menus: container (Start/Stop, Shell, Logs, Snapshot, Clone, Recreate, Move to stack, Copy name, Copy digest, Open in Permissions, Remove); image (Run, Inspect, Tag, Export, Remove); volume (Browse, Snapshot, Attach to…, Remove); network (Edit, Members, Remove); log line (Copy, Copy as JSON, Filter by this value).

Drag and drop: compose file or stack TOML on the window opens a plan; image archive on Images imports it; folder on a container's Mounts tab starts "Add folder" (still goes through the portal grant); container row on a network attaches it; volume row on a container attaches it at a path asked in a popover.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | One line and one button per section (Containers, Images, Volumes, Networks, Registries) |
| Loading | Skeleton rows; job progress in the status bar |
| Error | Banner with cause and Details; a failed pull keeps completed layers |
| Offline | Pull and newer-image checks are disabled with the reason; everything local works |
| Locked | Encrypted dataset without key: lock icon and Unlock |
| Read-only | Policy lock or snapshot browse: controls disabled with the policy named |
| Narrow | Sidebar overlay; detail tabs become a `comboBox`; inspector a sheet |
| Service restarting | Banner; containers keep running |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar, grouped container list, handle tree, layer file tree | `treeView` |
| Flat lists, images, volumes, processes, ports | `tableView` |
| Logs | `modelListView` (virtualised, monospace) |
| Detail tabs | `tabWidget` |
| Limits | `slider`, `spinBox`, `comboBox`, `progressBar` |
| Statistics | `chart` |
| Definition, stack file, config files | `codeEdit` |
| Service graph | `nodeGraph` (read-only) |
| Notes, help | `markdownView` |
| Dialogs and wizard | `dialog`, `lineEdit`, `radioButton`, `checkBox`, `toggleSwitch` |

Missing:

- `VM02-GAP-1`: `svc.jail`, a container service (not in PLATFORM.md section 8).
- `VM02-GAP-2`: a pty hand-off contract with SY03 (a handle to a pty in another domain passed by intent).
- `VM02-GAP-3`: administrator-approval prompt (same need as VM01-GAP-4).
- `VM02-GAP-4`: `codeEdit` grammars for TOML and YAML (FUSE grammar files).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, wizard, stack planning, definition editing |
| UI | FUSE UI | app host | Windows in 5.1 |
| `stack-helper` | C++23 | helper, capability mode | Parse compose YAML and `docker run` lines; no handles but the input file |
| Container service `svc.jail` | C++23 | system service (Proposed) | Registry of containers; creates jails and Linux domains; datasets; networks; limits; logs; autostart; jobs |
| `registry-client` | C++23 | sandbox per job, started by `svc.jail` | OCI Distribution client; holds a socket to one registry and a write handle to a staging dataset |
| `layer-helper` | C++23 | sandbox per layer | Decompress and unpack one layer tar into one dataset; no network |
| `verify-helper` | C++23 | sandbox | Parse signature envelopes, check signatures with FORGE crypto |
| Per-network resolver | C++23 | sandbox per network | Name resolution and DHCP-less address plan |
| Kernel | C++23 | kernel | Jails (`pbsd/kernel/jail`), Linux personality (P4-02), resource control (P1-08) |
| `ctr` CLI | ForgeScript | own process | Same bus interface |

```
 app host ──bus──▶ svc.jail ──▶ kernel: jail / Linux domain (capability domain)
   │                  ├─ registry-client ──TLS──▶ registry (only in pull jobs)
   └─ stack-helper    ├─ layer-helper (per layer) ──▶ ZFS dataset
                      ├─ verify-helper
                      └─ resolver (per network)
 SY03 ◀── pty handle (intent) ── svc.jail
```

The app has no network capability of its own for pulls: the registry client runs under `svc.jail` with an endpoint list derived from the user's registry entries. The app requests `net.user` only so that the shell can show and log those endpoints against this app (PLATFORM.md section 6, rule 5).

### 6.2 Data model

Container definition `container.toml` (schema version 1), held by `svc.jail`:

| Field | Type | Notes |
|---|---|---|
| `id` | uuid | Stable identity |
| `name`, `hostname`, `labels{}`, `notes` | string, string, map, string | |
| `kind` | enum `jail`, `linux` | |
| `base` | `{image_digest, image_ref}` or `{release, mode: thin\|thick\|empty}` | Digest pinned |
| `command[]`, `entrypoint[]`, `workdir`, `user` | list, list, path, string | Argument vectors, never shell strings |
| `env[]` | `{name, value}` or `{name, secret_ref}` | |
| `mounts[]` | `{kind: folder\|volume\|tmpfs\|config\|secret, source, target, readonly, nosuid, noexec, size}` | |
| `networks[]` | `{network, ipv4, ipv6, mac, aliases[]}` | |
| `ports[]` | `{host_addr, host_port, port, proto}` | |
| `outbound` | `{mode: all\|list\|none, allow[]}` | |
| `limits` | `{cpu_cap, cpu_weight, cpuset, mem_hard, mem_soft, swap, pids, files, io_rbps, io_wbps, quota}` | |
| `lifecycle` | `{restart, max_retries, autostart, order, stop_signal, stop_timeout_s, init, disposable}` | |
| `health` | `{kind, target, interval_s, timeout_s, retries, start_period_s}` | |
| `security` | `{profile, readonly_root, jail_allow{}, seccomp, linux_caps[], devfs_ruleset, device_grants[]}` | |
| `stack` | stack id or null | |

Other stores:

| Store | Location | Content |
|---|---|---|
| Image store | dataset `…/ctr/images/` | `blobs/sha256/<digest>` datasets per layer, `index.db` (SQLite: `image(digest, config_json)`, `tag(ref, digest)`, `layer(digest, parent, size)`, `signature(image, key_id, state, checked_at)`) |
| Containers | dataset `…/ctr/c/<id>/` | Root (clone), `container.toml`, `history/`, `log/` |
| Volumes | dataset `…/ctr/v/<name>/` | Data |
| Stacks | `…/ctr/stacks/<id>/stack.toml` | `{name, services{}, networks{}, volumes{}, secrets{}, source_path}` |
| Networks, registries, trust policy | `svc.jail` store: `networks.toml`, `registries.toml`, `trust.toml` | Trust entries `{pattern, mode: require\|warn\|block, key_ids[]}` |
| App state | `~/.local/share/org.pbsd.jails/ui.db` | View state, saved filters, user templates, command history |

Every store has `schema_version`; migration is forward only after a snapshot.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Container, stack, policy TOML | yes | yes | Own code (FORGE-WP-23) | app host; re-validated in `svc.jail` |
| Compose YAML (subset) | yes | yes | Own YAML subset parser (no anchors expansion bombs: alias depth and size limits) | `stack-helper` |
| `jail.conf` (import) | yes | fragment only | Own code | `stack-helper` |
| OCI image manifest, index, config (JSON) | yes | yes | Own code | `registry-client`, re-checked in `svc.jail` |
| OCI Distribution API (HTTPS) | yes | push only in phase 3 | FORGE network stack (FORGE-WP-21) | `registry-client` |
| Layer tar (ustar, pax), gzip, zstd | yes | yes (export, build) | Own tar reader; zlib and zstd from base (zlib and BSD licences) | `layer-helper` |
| `docker save` archive, OCI layout | yes | OCI layout | Own code | `layer-helper` |
| Signature envelopes (cosign simple signing payload, Notary v2 JWS/COSE) | yes | cosign-style | Own code + FORGE crypto (FORGE-WP-22) | `verify-helper` |
| SPDX JSON, CycloneDX JSON | yes | SPDX | Own code | `verify-helper` |
| Package databases inside images (apk, dpkg status, rpm sqlite, pkg) | yes | no | Own readers; rpm database through SQLite (public domain) | `layer-helper` |
| seccomp profile JSON | yes | yes | Own code | app host; enforced by the kernel personality |
| Containerfile subset | yes | no | Own code | `stack-helper` |
| ZFS send stream | kernel | kernel | OpenZFS in base | kernel |

### 6.4 Integration

- Intents handled: `open` for `application/x-compose+yaml`, `application/x-pbsd-stack+toml`, `application/vnd.oci.image.layout.v1+tar`, `application/x-docker-archive`, `ctr:` URLs. Emitted: `open` to SY03 (pty handle), SY02, SE02, AD03, AD04, AD05, SY04, DV05; `share`.
- Services: `svc.jail` (gap), `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.usb`, `svc.intent`, `svc.search`, `svc.keyring`, `svc.audit`, `svc.backup`, `svc.update` (base releases, advisory database), `svc.model` (optional).
- IPC offered by `svc.jail`: `org.pbsd.jail.Manager/1`; used by VM03 (Linux and Android app sandboxes), DV01 (build containers), SY05 (service jails).
- Backing repo: facts cited at the top of section 4 are Verified; the service, image store and helpers are Proposed.

## 7. Security and privacy

### Capabilities requested

| Capability | Reason |
|---|---|
| `fs.app` | UI state, templates |
| `fs.portal` | Folder mounts, stack files, image archives, exports |
| `net.user` | Registries the user added; each first contact logged. Sockets are opened by the registry client, not by the app's script |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Per-registry route choice |
| `proc.helper:stack-helper` | Parse compose and import formats |
| `svc.jail` | All container operations (VM02-GAP-1) |
| `svc.settings`, `svc.notify`, `svc.intent`, `svc.search`, `svc.audit`, `svc.backup`, `svc.update` | As in 6.4 |
| `svc.keyring` | Secrets, registry credentials, verification keys, dataset keys |
| `dev.usb:*`, `dev.gpu.compute` | Only when the user grants a device to a container |
| `ai.local` | Optional log explanation |

Not requested: `proc.exec` (shells run in SY03), `fs.path`, camera, microphone, screen.

### Threat model

Assets: host kernel and other domains, user files, secrets, registry credentials, the integrity of images.

| Adversary | Attack | Mitigation |
|---|---|---|
| Hostile image | Tar path traversal, device nodes, hard-link tricks, decompression bomb | Unpack in `layer-helper` with one dataset handle; refuse escapes; ratio and size limits; fuzzing |
| Hostile image | Replaced tag on the registry | Digest pinning; signature policy; newer-image check never applies by itself |
| Hostile registry or network | Tampered blobs, downgrade | Every blob checked against its digest; TLS with optional pin; signatures verified offline against local keys |
| Hostile process inside | Escape the domain | No ambient authority, capability domain per container, minimal devfs, syscall filter, limits, W^X, no "privileged" mode |
| Hostile process inside | Exhaust the host | Default process, memory and storage limits; pressure view |
| Hostile compose file | Mount host paths, request host network or privilege | Paths resolve inside the granted folder; privileged keys refused or turned into explicit grant requests |
| Hostile log content | Terminal escape injection, UI spoofing | Logs rendered as plain text; only SGR colours kept |
| Other local app | Control containers | `svc.jail` checks peer and per-container grants |
| Secrets leakage | Secrets in definitions, logs, exports | Secrets referenced by keyring id; exports strip them; env secrets masked in the UI |

Stored: definitions, images, logs on the user's container datasets; UI state in app data; credentials and keys in `svc.keyring`. Leaves the machine: registry requests (reference names and pulls) to registries the user added; nothing else.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `defaults.kind` | enum `linux`, `jail` | `linux` | Wizard default |
| `defaults.network` | string | `private` | Network for new containers |
| `defaults.publish_addr` | string | `127.0.0.1` | Host address for published ports |
| `defaults.limits_preset` | enum | `medium` | |
| `defaults.pids` | u32 | `1024` | |
| `defaults.profile` | enum `strict`, `standard` | `standard` | |
| `stop.timeout_s` | u32 | `10` | Before kill |
| `remove.keep_days` | u32 | `14` | Root retention after remove |
| `logs.max_size_mib`, `logs.files` | u32, u32 | `10`, `5` | Rotation |
| `logs.follow` | bool | `true` | Follow on open |
| `images.trust_default` | enum `require`, `warn`, `block` | `warn` | For registries without a rule |
| `images.check_newer` | enum `never`, `daily`, `weekly` | `never` | Scheduled digest check |
| `images.unpack_ratio_limit` | u32 | `200` | Maximum expansion ratio per layer |
| `snapshots.before_recreate` | bool | `true` | |
| `shell.default` | path | `/bin/sh` | |
| `stats.interval_s` | u32 | `1` | |
| `ai.explain_logs` | bool | `false` | Needs `ai.local` |

## 9. Accessibility and languages

- Log view exposes each line as a list item; follow mode announces only lines matching a user filter (to avoid a flood) and a count of new lines.
- State changes (exited, unhealthy) are live-region announcements.
- The service graph has an equivalent table of services and dependencies.
- Digests are read in groups of four characters and can be copied in one action.
- Terms are translated; image names, labels and log text are never translated.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 250 ms with 300 containers |
| Table refresh | under 4 ms per 1 Hz tick for 100 running containers |
| Log view | 1,000,000 lines virtualised; follow at 5,000 lines/s without dropped frames; filter over 100,000 lines under 200 ms |
| Container create from a local image | under 500 ms (clone) |
| App memory | under 100 MiB with a 1,000,000-line log open |
| Pull | limited by network and disk; `layer-helper` under 128 MiB |

## 11. Testing and proof

Nothing has been run. All entries are targets.

- Unit: definition validation, stack planning (diff), port conflict check, trust policy matching, limit conversion.
- Golden renders of every view in 5.2 and 5.3.
- Scripted UI: T1–T13 against a fake `svc.jail`.
- Fuzz: tar reader, gzip/zstd wrappers, OCI JSON, signature envelopes, YAML subset, `jail.conf`, Containerfile, `docker run` line parser.
- Differential: unpacked trees compared with a reference OCI unpacker on Linux; compose plans compared with `docker compose config` output for the supported subset (test-time only).
- Conformance: OCI Distribution and Image specification test vectors.
- Integration: escape test suite inside containers (mount, device, raw socket, ptrace, set-id); limit enforcement; restart policies; rollback.
- Fault injection: kill each helper and the service mid-job; network loss mid-pull; pool full.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Domain creation in `svc.jail` | A container holds only handles derived from its definition (SI-1, SI-2) | T1 | Model in Lean 4 or TLA+; contracts by ESBMC through PRISM | – |
| Revocation | Revoking the domain invalidates every derived handle (SI-6) | T1 | Lineage model | – |
| `layer-helper` tar unpack | No write outside the target dataset; bounded expansion | T2 | Contracts, bounded model checking, fuzzing | Tar and pax variety |
| Digest and signature check | An image is marked verified only if digest and signature both hold | T2 | Contracts; test vectors | Crypto primitives are FORGE's tier |
| YAML subset parser | Total, bounded memory | T2 | Contracts, fuzzing | – |
| Registry client | Memory safety | T3 | Safe subset, sanitizers, fuzzing | Network protocol surface |
| Kernel jail and Linux personality | Isolation | per plan P4-01/P4-02 tiers | Outside this app | – |
| App logic | No capability widening | T3 | FSS policy, script tests | Interpreter not verified |
| UI | – | T4 | Golden and scripted tests | – |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| VM02-W01 | `org.pbsd.jail.Manager/1` schema and a fake service | G-08, VM02-GAP-1 |
| VM02-W02 | Definition schema and validator; bundled templates | G-07 |
| VM02-W03 | Main window, sidebar sections, inspector, search | G-01, G-02, VM02-W01 |
| VM02-W04 | `svc.jail` core for native jails: datasets, thin jails, start and stop, log capture | `pbsd/kernel/jail`, G-06, G-09 |
| VM02-W05 | Image store, `layer-helper`, offline import | FORGE-WP-27, FORGE-WP-28 |
| VM02-W06 | Registry client and pull dialog with digest checks | FORGE-WP-21, VM02-W05 |
| VM02-W07 | Linux containers on the Linux personality | P4-02, VM02-W05 |
| VM02-W08 | Create wizard, detail page (Overview, Logs, Mounts, Limits basic) | VM02-W03 |
| VM02-W09 | VNET private networks, published ports | SE02 interface, VM02-W04 |
| VM02-W10 | Shell hand-off to SY03 | VM02-GAP-2, SY03 |
| VM02-W11 | Snapshots and rollback | VM02-W04 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| VM02-W12 | Signature verification and trust policy | FORGE-WP-22, FORGE-WP-33, SE04 |
| VM02-W13 | Stacks: native format, compose import, plan, up and down | `stack-helper`, VM02-W08 |
| VM02-W14 | Capability view, profiles, jail parameters, syscall filter | FORGE-WP-06, AD05, P4-02 |
| VM02-W15 | Restart policies, autostart, health checks, events | VM02-W04 |
| VM02-W16 | Full limits with live change and limit-hit counters | P1-08 |
| VM02-W17 | Secrets, config files, copy in and out, changes since image | `svc.keyring` (G-09) |
| VM02-W18 | Disposable runs, recreate, newer-image check, prune | VM02-W06 |
| VM02-W19 | Processes, statistics, merged stack log | VM02-W08 |
| VM02-W20 | Outbound rules, name resolution, routes | SE02, SE03 |
| VM02-W21 | `ctr` CLI; export and import | VM02-W01 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| VM02-W22 | Build, commit, sign, push | VM02-W12 |
| VM02-W23 | Contents list, SBOM, known-issue check | SY05 advisory database |
| VM02-W24 | Pressure view, bandwidth limits, connections view, capture hand-off | P1-08, DV05 |
| VM02-W25 | Stack editor with graph, scale, stack snapshots | VM02-GAP-4 |
| VM02-W26 | Policy lock, encryption of datasets, scheduled runs, assistant help | G-15 |

## 13. Open questions and risks

- Conflict in the repo documents: `docs/migration/LINUXULATOR.md` says "Linux tasks never receive PBSD handles" and keeps the linuxulator outside the capability surface, while plan section 7 maps namespaces and jails to capability domains and P4-02 targets full Linux container support. This spec follows the plan. The owner must settle which holds.
- "Rootless" on PBSD means no ambient authority rather than user namespaces. Images that assume real root (for example to bind low ports or change ownership) need a defined mapping; the details depend on P4-02.
- Overlay semantics: OCI layers assume overlay whiteouts. The store converts each layer into a ZFS clone chain; deep chains (over 100 layers) need a flattening rule.
- FreeBSD OCI images (`freebsd/amd64`) are new upstream; coverage is Unverified.
- Resource control interface (rctl versus cgroup v2 semantics under one model) depends on P1-08, not read in detail here.
- Signature formats in the wild lean on online keyless services; offline verification will mark many public images "not verifiable offline".
- YAML is a large format; the subset must be documented precisely to avoid surprise.
- Whether `svc.jail` and VM01's `svc.vm` share one network layer (bridges, resolver, routes) is undecided; sharing would cut code.
