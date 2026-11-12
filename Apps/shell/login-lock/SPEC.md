<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Login, Lock and Session

By Odin Loch

| | |
|---|---|
| ID | `SH03` |
| Bundle | `org.pbsd.login-lock` |
| Category | shell |
| Priority | P0 |
| Backing repos | FUSE (UI toolkit), FORGE (ForgeScript, keystore, IPC), ParanoidBSD `pbsd/compositor/` (Wayland server side) |
| Status | Planned. Nothing built. |

## 1. Purpose

SH03 is every screen that stands between a person and a session: the disk-unlock screen, the login greeter, the lock screen, the authentication prompt for privileged actions, and the power and session dialogs. It also owns user switching, guest and disposable sessions, hardware-token sign-in, and the duress and panic options. It replaces SDDM, the Plasma lock screen (`kscreenlocker`), the polkit agent and the logout dialog of the current plan (`docs/plan/PBSD_PLAN.md`, P5-02).

These are the surfaces an attacker most wants to imitate. Their central rule: they are drawn on a trusted path that no app can draw on, cover or capture.

## 2. Scope

In scope:

- Disk-unlock screen at boot and the hand-off of that unlock to the first login.
- Greeter: user list, authentication, session type choice, accessibility at login.
- Lock screen: locking rules, unlock, what is shown while locked.
- Authentication prompts for privileged actions (the "elevation prompt"), and the trusted-path rules they follow.
- Power and session dialogs: log out, switch user, lock, suspend, hibernate, restart, shut down.
- Fast user switching; guest sessions; disposable sessions.
- Hardware tokens (FIDO2, smart card) for login, unlock and elevation.
- Duress credentials and panic actions.

Out of scope:

| Item | Owner |
|---|---|
| Capability grant prompts, file choosers and other portals (they reuse the trusted path defined here) | SH04 |
| Creating, editing and deleting users; password policy pages; enrolling tokens (UI pages) | SY01 (using SH03's enrolment service) |
| Password manager, passkeys for web sites, token management | SE04 |
| Disk encryption set-up, GELI and ZFS key management | AD01, AD02 |
| Boot environments list at restart | SY05 (SH03 offers "restart into…") |
| Notification rows on the lock screen (content) | SH02 (redacted feed) |
| Idle detection and window stacking | compositor, SH05 |
| Verified-boot state and integrity checks | SE01 (SH03 displays the state) |
| Remote login | VM04 |

## 3. Users and top tasks

Personas:

- **Single-user laptop owner**: types one passphrase at boot and wants to be at the desktop.
- **Shared-machine user**: picks their name, logs in, switches without closing the other person's work.
- **Administrator**: approves privileged actions; wants to know exactly what is being approved.
- **At-risk user** (journalist, activist): needs a fast panic lock, a duress credential and no information on the lock screen.
- **Token user**: logs in with a FIDO2 key and PIN.
- **Assistive-technology user**: needs speech, high contrast and an on-screen keyboard before login.

| # | Task | Target |
|---|---|---|
| T1 | Boot to desktop on a single-user encrypted machine | type one passphrase, `Enter` (2 steps) |
| T2 | Log in from the greeter | pick user (pre-selected), type password, `Enter` |
| T3 | Lock the screen | `Super+L` (1 step) |
| T4 | Unlock | type password, `Enter`; or touch token |
| T5 | Approve a privileged action | read, type password, `Enter` (3 steps) |
| T6 | Refuse a privileged action | `Esc` (1 step) |
| T7 | Shut down | `Ctrl+Alt+Delete`, `Shut down`, `Enter`; or power key, `Enter` |
| T8 | Switch user | `Ctrl+Alt+Delete`, "Switch user", pick, authenticate |
| T9 | Start a guest session | greeter, "Guest", `Enter` (2 steps) |
| T10 | Start a disposable session | greeter, session type "Disposable", log in |
| T11 | Panic lock | hold `Super+Shift+Esc` 1 s (1 step) |
| T12 | Log in with a token | insert, type PIN, touch |
| T13 | Turn on the screen reader at the greeter | `Super+Alt+S` (1 step) |
| T14 | Restart into another boot environment | session dialog, "Restart into…", pick, confirm |

## 4. Features

### 4.1 Disk unlock and hand-off
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH03-F001 | Unlock screen | Graphical passphrase screen shown in early boot, before any user data is mounted; drawn by the FUSE CPU rasteriser on the boot framebuffer, no compositor needed (SH03-GAP-7) | M | 1 |
| SH03-F002 | Text fallback | If the graphical screen cannot start within 5 s, the plain text prompt is used; same key derivation | M | 1 |
| SH03-F003 | What is unlocked | The screen names the pool or provider being unlocked and the machine name; no user names | M | 1 |
| SH03-F004 | Keyboard layout | Layout chosen at install is used; a layout switcher shows the active layout; a "show typed text" hold button | M | 1 |
| SH03-F005 | Attempts and delay | Delay doubles after each wrong passphrase from 1 s to 60 s; the count is shown; no lockout that could be used to deny access | M | 1 |
| SH03-F006 | Token unlock | Unlock with a FIDO2 token (`hmac-secret`) plus PIN, or passphrase plus token (two factor), when AD01 or AD02 enrolled one | S | 2 |
| SH03-F007 | Recovery key | "Use recovery key" switches to a grouped input for the long recovery key with checksum per group | S | 2 |
| SH03-F008 | Verified-boot state | A line shows the boot chain state from the loader: verified, unverified, changed since last boot; changed is `--danger` with a word and blocks auto hand-off | M | 2 |
| SH03-F009 | Hand-off to login | On a single-user machine with `handoff.enabled`, a one-time token derived at unlock logs that user in without a second prompt; the token lives in kernel memory, is read once by the authentication service and erased | M | 1 |
| SH03-F010 | Hand-off limits | Only for the first login after boot, only for the user bound to the disk key, never after resume, never if F008 reports "changed", never for administrators if policy says so | M | 1 |
| SH03-F011 | Keyring unlock | A successful password or hand-off login unlocks `svc.keyring` for that user; token-only login unlocks it through a token-wrapped key | M | 1 |
| SH03-F012 | Per-user encrypted homes | If the home dataset has its own key, the greeter login unlocks it; logout unloads the key when no session of that user remains | S | 2 |
| SH03-F013 | Duress at unlock | A duress passphrase enrolled for the disk triggers the configured duress action (4.9) | C | 3 |
| SH03-F014 | Emergency shell | Not offered on the unlock screen; available only from the loader menu with the loader password | M | 1 |
| SH03-F015 | Power from unlock screen | Shut down and restart buttons; no other action | M | 1 |

### 4.2 Greeter
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH03-F016 | Greeter session | The greeter runs as a dedicated unprivileged user in its own compositor instance on its own virtual terminal, with only `svc.auth` and settings read access | M | 1 |
| SH03-F017 | User list | Avatars and display names; last user pre-selected; hidden entirely when `greeter.show_users` is false (then a name field is shown) | M | 1 |
| SH03-F018 | Manual name entry | "Other…" lets a name be typed; unknown names get the same delay and the same error text as wrong passwords | M | 1 |
| SH03-F019 | Password field | Hidden by default; reveal while held; Caps Lock and Num Lock warnings; layout indicator and switcher | M | 1 |
| SH03-F020 | Session type | Choice per login: Desktop, Disposable, Safe mode (no third-party applets, no autostart), Recovery (administrators only) | M | 1 |
| SH03-F021 | Guest entry | "Guest" tile when guest sessions are enabled; no password | S | 2 |
| SH03-F022 | Already logged in | Users with a running session show "Signed in"; choosing one unlocks that session instead of starting a second | M | 2 |
| SH03-F023 | Failure handling | Same message for every failure ("That did not work. Try again."); delay 1, 2, 4 … 60 s per user and per seat; count kept by the authentication service | M | 1 |
| SH03-F024 | Last login notice | After login, a notification: last successful login time and number of failed attempts since | M | 1 |
| SH03-F025 | Password expiry and change | Forced change flow inside the greeter: old, new, repeat, strength meter, policy text | S | 2 |
| SH03-F026 | Token login | "Use security key" on a user: PIN field then touch; works alone or as a second factor per user policy | S | 2 |
| SH03-F027 | Smart card login | Certificate login (PIV) when a reader and card are present | C | 3 |
| SH03-F028 | Auto login | Off by default; only possible when the disk is encrypted and hand-off (F009) is on; never for administrators | S | 2 |
| SH03-F029 | Login banner | Optional administrator text shown before login; must be acknowledged if policy says so | C | 3 |
| SH03-F030 | Status area | Clock, battery, network state (read-only), keyboard layout, accessibility menu, power menu | M | 1 |
| SH03-F031 | Network at the greeter | Wi-Fi selection only when policy allows (needed for directory logins); off by default | C | 3 |
| SH03-F032 | Accessibility menu | Screen reader (AX01), magnifier (AX02), on-screen keyboard (AX03), high contrast, large text; shortcuts work before login | M | 1 |
| SH03-F033 | Multi-monitor | The greeter form is on the primary output; other outputs show the wallpaper and clock; the form follows the pointer's output on click | M | 1 |
| SH03-F034 | Idle at the greeter | Screens blank after 2 min; the machine suspends after 20 min on battery (policy) | S | 2 |
| SH03-F035 | Integrity line | A small line shows verified-boot state and the system version; a tampered state is `--danger` and links to a read-only detail sheet | S | 2 |
| SH03-F036 | First boot | If no user exists the greeter starts AD01's first-run instead | M | 1 |
| SH03-F037 | Greeter theme | System theme and the system greeter wallpaper only; a user's wallpaper is never shown before that user authenticates | M | 1 |
| SH03-F038 | Session start | After authentication the session manager starts the user's compositor, session bus, SH05, SH01, SH02 in order; a progress line appears if this takes over 2 s | M | 1 |
| SH03-F039 | Session start failure | If the session dies within 10 s, return to the greeter with "The session could not start. [Try safe mode] [Details]" | M | 1 |

### 4.3 Lock screen
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH03-F040 | Lock protocol | Locking uses `ext_session_lock_v1`: the compositor shows nothing but lock surfaces from the moment it accepts the lock; one lock surface per output (SH03-GAP-1) | M | 1 |
| SH03-F041 | Fail closed | If the lock client crashes, the compositor keeps every output blank and locked, and the session supervisor restarts the client; unlocking is possible only through a new authenticated lock client | M | 1 |
| SH03-F042 | Lock triggers | `Super+L`, session dialog, idle timeout, lid close, suspend, token removal (optional), panic, remote request from the same user on another seat | M | 1 |
| SH03-F043 | Lock before sleep | The lock is confirmed by the compositor before the machine is allowed to suspend | M | 1 |
| SH03-F044 | Grace period | Optional 0–30 s after an idle lock during which any input unlocks without a password; default 5 s; never applies to manual, lid or panic locks | S | 2 |
| SH03-F045 | Two-stage screen | First a clock face with date and status; any key or click shows the unlock form; typing starts the password at once | M | 1 |
| SH03-F046 | Unlock form | Avatar, name, password field with reveal-on-hold, Caps Lock warning, layout switcher, "Use security key", "Switch user" | M | 1 |
| SH03-F047 | Unlock failure | Field shakes once (not with reduced motion), message "Wrong password", delay 1, 2, 4 … 60 s shown as a countdown | M | 1 |
| SH03-F048 | Token unlock | Touch a known token (with PIN at most once per `token.pin_interval`, default 8 h) | S | 2 |
| SH03-F049 | Notifications on lock | Rows from SH02's redacted feed: count only, app names only, or full, per SH02 rules; default "app and count" | M | 1 |
| SH03-F050 | Media controls | Title, play or pause, next, previous for the current `svc.media` session; can be turned off | S | 2 |
| SH03-F051 | Quick controls | Volume, brightness, DND, kill switches (off only), from SH02's lock subset | S | 2 |
| SH03-F052 | Status | Clock, date, battery, network state icon, keyboard layout; no network names when `lock.hide_details` is on | M | 1 |
| SH03-F053 | Power from lock | Suspend always; shut down and restart only if `lock.allow_power` (default off on laptops, so a thief cannot easily cold-boot into a tool) | M | 1 |
| SH03-F054 | Owner message | Optional text ("If found, call …"); plain text, 200 characters | C | 2 |
| SH03-F055 | Other users | "Switch user" leaves this session locked and shows the greeter | M | 2 |
| SH03-F056 | Screens off | Outputs blank 30 s after the lock form was last touched; any key wakes | M | 1 |
| SH03-F057 | Inhibitors | Video playback and presentations may inhibit the idle lock (`…wayland.idle_inhibit`); they cannot inhibit a manual lock; the lock form lists which app delayed the last idle lock | M | 2 |
| SH03-F058 | Evict secrets on lock | On lock: `svc.keyring` re-locks items marked "lock with screen", clipboard secrets are cleared, SE05 vaults marked so are closed | M | 2 |
| SH03-F059 | Long-lock hardening | After `lock.deep_after` (default 30 min locked) the session keyring is fully locked and token PIN is required again; optional suspend-to-disk with keys dropped | S | 3 |
| SH03-F060 | Wallpaper | The user's lock wallpaper, blurred and dimmed, or a solid colour; decoded before lock time, never at unlock time | S | 1 |
| SH03-F061 | No app surfaces | No app window, banner, OSD or popup is composed while locked; the OSDs of volume and brightness keys are drawn by the lock client itself | M | 1 |
| SH03-F062 | No capture | Lock and greeter surfaces are never included in screen capture or remote sessions unless VM04 is serving this very session's login | M | 1 |
| SH03-F063 | Emergency info | Optional button showing medical and contact information the user entered; read-only | C | 3 |
| SH03-F064 | Unlock notice | After unlock: toast "Locked for 2 h 10 min. 3 failed attempts." when any failed attempt happened | M | 1 |
| SH03-F065 | Accessibility on lock | Same accessibility menu as the greeter; the screen reader speaks only lock-screen content | M | 1 |
| SH03-F066 | Input methods | Password fields accept only direct keyboard input; input-method composition and prediction are off | M | 1 |

### 4.4 Authentication prompts for privileged actions
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH03-F067 | Elevation service | Apps and services ask `svc.auth` to authorise a named action (`org.pbsd.auth.Authorise1`); only SH03 draws the prompt | M | 1 |
| SH03-F068 | Action registry | Each privileged action has an id, a fixed description, a risk level and a default policy in a signed registry file shipped by the component that owns the action; the requesting app cannot supply the text | M | 1 |
| SH03-F069 | Prompt content | Who asks (real app name, icon, trust badge, security label from SH05), what (registry text), on what (object details as typed fields: path, device, package), and what approval allows | M | 1 |
| SH03-F070 | Untrusted details marked | Fields supplied by the requester are shown in a boxed "Details from the app" area, monospace, 400 characters, control characters shown as symbols, never styled | M | 1 |
| SH03-F071 | Who approves | The current user if an administrator; otherwise a list of administrators to pick from; each uses their own credential | M | 1 |
| SH03-F072 | Credential kinds | Password, token touch, token PIN plus touch, per action risk: `low` (touch or password), `high` (password plus token if enrolled), `critical` (always both, never cached) | M | 2 |
| SH03-F073 | Duration | Once (default); "for 5 minutes for this app and action" only where the registry allows; never "always" | M | 1 |
| SH03-F074 | Cached approvals | Shown as a status item with remaining time and a "Forget" button; dropped on lock, on user switch and when the app exits | M | 2 |
| SH03-F075 | Input delay | Approve is disabled for 800 ms after the prompt appears and after any content change, with a visible fill; stops approval by a stray `Enter` | M | 1 |
| SH03-F076 | Default is deny | `Esc`, closing, timeout (120 s) and focus loss all deny; `Enter` in the password field approves only after the delay | M | 1 |
| SH03-F077 | One at a time | Requests queue; the prompt shows "1 of 3"; identical repeated requests from one app within 10 s are merged; more than 5 denied in a minute mutes that app's requests for 5 minutes and logs it | M | 1 |
| SH03-F078 | Origin window | The prompt is attached above the requesting window when there is one; other windows are dimmed; if there is no window the prompt says "No window: requested in the background" | M | 1 |
| SH03-F079 | Details expander | "Show details" lists action id, registry file and signer, requesting process and domain id, command line if any, and the audit id | S | 1 |
| SH03-F080 | Audit | Every request, approval, denial and timeout goes to `svc.audit` with action, requester, approver, credential kind | M | 1 |
| SH03-F081 | Failure handling | Wrong credential: inline error and back-off shared with the lock screen counters; three failures deny the request | M | 1 |
| SH03-F082 | Non-interactive policy | Registry policy may be `allow` (no prompt), `auth` (prompt), `deny`; an administrator policy file can only tighten the default | S | 2 |
| SH03-F083 | Terminal elevation | Elevation commands run in SY03 raise this same prompt; no password is ever typed into a terminal | S | 2 |
| SH03-F084 | Remote and headless | Over VM04 or SSH the prompt is refused unless that session is the active seat; headless elevation uses the token or a console prompt | S | 3 |
| SH03-F085 | Explain in plain words | Optional "What does this mean?" opens the UT05 page for the action; no model-generated text in the prompt | S | 2 |
| SH03-F086 | Change preview | For actions whose owner supplies a typed change list (packages to install, rules to add), the prompt shows it in a scrollable list | S | 3 |

### 4.5 Trusted path rules
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH03-F087 | Trusted layer | The compositor has a layer above `Overlay` that only clients with `shell.trusted-path` may use; greeter, lock, elevation prompt and SH04 prompts are drawn there (SH03-GAP-2) | M | 1 |
| SH03-F088 | Nothing on top | No other surface, cursor theme image, drag icon, input-method popup or OSD is composed above a trusted surface | M | 1 |
| SH03-F089 | Input isolation | While a trusted prompt is up, keyboard and pointer go only to it; synthetic input, virtual keyboards from apps and automation through `svc.a11y` are refused for it (the system AX apps are allowed) | M | 1 |
| SH03-F090 | Trust mark | Every trusted prompt shows the user's trust mark: a picture and two words chosen at account creation, stored where only trusted-path clients can read; an app cannot know it, so a fake prompt lacks it | M | 1 |
| SH03-F091 | Trust frame | The compositor itself draws a 3 px frame in a reserved colour and pattern around trusted surfaces and dims everything else to 40%; apps cannot dim the screen or draw outside their window | M | 1 |
| SH03-F092 | Secure attention key | `Ctrl+Alt+Delete` is consumed by the compositor and never delivered to any client; it opens the session dialog on the trusted layer. A prompt that survives this key is real; a fake one is covered | M | 1 |
| SH03-F093 | No capture, no mirror | Trusted surfaces are excluded from screen capture, window thumbnails and remote sessions; captured frames show a grey box with "Protected" | M | 1 |
| SH03-F094 | No fullscreen spoofing | A fullscreen app cannot hide the panel's indicators without the compositor's own indicator dot (SH02-F132), and gets a "press and hold Esc to leave fullscreen" hint on entry | M | 2 |
| SH03-F095 | Greeter never in-session | Inside a session, nothing ever asks for the login password except the lock screen and the elevation prompt; help text says so in AD01's tour | M | 1 |
| SH03-F096 | Teaching | The first three elevation prompts carry a one-line explanation of the trust mark and the secure attention key | S | 2 |
| SH03-F097 | Consistent placement | Trusted prompts are always centred on the output with keyboard focus at 520 px width; they never move after appearing | M | 1 |

### 4.6 Power and session dialogs
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH03-F098 | Session dialog | Opened by `Ctrl+Alt+Delete`, the launcher power row (SH01-F039), quick settings (SH02-F075) or the power key: Lock, Switch user, Log out, Suspend, Hibernate, Restart, Shut down | M | 1 |
| SH03-F099 | Confirmation | Choosing an action shows a 30 s countdown confirm (setting 0–60 s; 0 skips) with the action as the default button | M | 1 |
| SH03-F100 | Unsaved work | Lists apps that report unsaved documents (through the session's save-query interface) with "Save all", "Review", "Discard and continue"; apps get 10 s to answer | M | 2 |
| SH03-F101 | Inhibitors | Lists what blocks shutdown or sleep (backup running, update installing, transfer, VM) with each app's reason; "Do it anyway" needs a second click, and elevation if another user's or the system's job is blocked | M | 2 |
| SH03-F102 | Other users | "2 other users are signed in" with names; shutting down needs administrator approval | M | 2 |
| SH03-F103 | Restart options | Restart into: default, another boot environment (list from SY05), firmware set-up, the installer or recovery image; each is a privileged action | S | 2 |
| SH03-F104 | Update on shutdown | If SY05 has staged updates: "[x] Install updates first (about 4 min)" | S | 2 |
| SH03-F105 | Scheduled power | Shut down or restart at a time or after a delay; a status item shows the countdown with Cancel | C | 3 |
| SH03-F106 | Power key | Short press opens the dialog (or suspends, per setting); long press is hardware | M | 1 |
| SH03-F107 | Lid and idle actions | Lid close and idle actions per power source come from SY01 power settings; SH03 runs lock then sleep in that order | M | 1 |
| SH03-F108 | Hibernate safety | Hibernate is offered only when swap is encrypted with a key that is not kept in the image in clear | M | 2 |
| SH03-F109 | Log out | Ends the session: apps are asked to close (10 s), then stopped; home key unloaded (F012); returns to the greeter | M | 1 |
| SH03-F110 | Session restore choice | "[x] Reopen my windows next time" on log out and shut down | S | 2 |
| SH03-F111 | Low battery action | At the critical level SH03 runs the configured action (hibernate, shut down) after a 60 s warning dialog | M | 1 |
| SH03-F112 | Thermal and power-loss | The same warning dialog for UPS on battery and critical temperature, with the time left | C | 3 |

### 4.7 Users, guest and disposable sessions
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH03-F113 | Fast user switching | Several users signed in at once, each with a compositor on its own virtual terminal; switching locks the current one first | M | 2 |
| SH03-F114 | Switcher | List of signed-in users with avatar, time signed in, and "locked" state; `Ctrl+Alt+F-keys` are not exposed to users | M | 2 |
| SH03-F115 | Isolation between sessions | A background session gets no input, no outputs, no camera, microphone or screen portals; audio is muted unless the user allowed "keep playing" | M | 2 |
| SH03-F116 | Resource view | The switcher shows memory used by each background session; an administrator may end one after an elevation prompt | S | 3 |
| SH03-F117 | Guest session | Home is a fresh dataset cloned from a template and destroyed at logout; no keyring; no access to other users' data; network allowed per policy; session mark on the desktop (SH01-F085) | S | 2 |
| SH03-F118 | Guest limits | No elevation, no software install, no token enrolment; removable media only if policy allows | S | 2 |
| SH03-F119 | Disposable session | A real user logs in to a throw-away copy: home is a snapshot clone discarded at logout; chosen folders can be mounted read-only; the keyring is not unlocked unless asked | S | 2 |
| SH03-F120 | Disposable end | Logout dialog says "Everything in this session will be erased" and offers "Keep these files…" (SH04 save chooser into the real home, one file or folder at a time) | S | 2 |
| SH03-F121 | Erase on exit | Destroying the clone is followed by a check that the dataset is gone; failure raises a `critical` notification for the administrator | S | 2 |
| SH03-F122 | Safe mode session | Starts with no autostart apps, no third-party applets, default settings overlay; nothing is deleted | S | 2 |
| SH03-F123 | Kiosk session | Single-app session defined by policy; leaving needs elevation (SH01-F149) | C | 3 |
| SH03-F124 | Recovery session | Administrators only: a minimal session with SY03, AD02, AD03, SY05 and AD04 | C | 3 |
| SH03-F125 | Multi-seat | Two seats (two sets of screen, keyboard, pointer) on one machine | C | 3 |

### 4.8 Hardware tokens
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH03-F126 | FIDO2 for login and unlock | CTAP2 over USB HID through the token helper shared with SE04; resident or server-side credentials bound to the machine | S | 2 |
| SH03-F127 | Token prompt | States: insert, enter PIN, touch now (with a pulsing-free steady icon and countdown), wrong PIN (retries left from the token), blocked | S | 2 |
| SH03-F128 | Factor policy | Per user: password only, token only, password and token, either; per action risk for elevation (F072) | S | 2 |
| SH03-F129 | Enrolment service | `org.pbsd.auth.Enrol1` lets SY01 add and remove tokens after an elevation prompt; at least one recovery method must remain | S | 2 |
| SH03-F130 | Lock on removal | Optional: removing the token that unlocked the session locks it | C | 3 |
| SH03-F131 | Token over NFC and Bluetooth | Not supported before login; USB only on the greeter and lock screen | S | 2 |
| SH03-F132 | Smart card (PIV) | Certificate chain checked against a local trust list; PIN entry on the trusted path | C | 3 |
| SH03-F133 | Biometric readers | Not planned in phases 1–3; if added, only as a convenience factor for unlock, never for disk unlock or `critical` actions | C | 3 |
| SH03-F134 | USB policy while locked | New USB devices other than HID keyboards, pointers and known tokens are refused while locked or at the greeter (with VM05) | M | 2 |
| SH03-F135 | Unknown keyboard notice | A keyboard first seen while locked can type the password but raises a notice after unlock ("A new keyboard was attached while locked") | S | 3 |

### 4.9 Duress and panic
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH03-F136 | Panic lock | Hold `Super+Shift+Esc` for 1 s: lock at once, lock the keyring fully, close SE05 vaults, clear the clipboard, cut camera and microphone (SH02 kill switches) | M | 2 |
| SH03-F137 | Panic power-off | Optional second stage (keep holding 3 s, or `panic.action = poweroff`): unload dataset keys and power off without asking apps to save | S | 2 |
| SH03-F138 | Panic from the lock screen | The same shortcut works while locked and at the greeter | S | 2 |
| SH03-F139 | Duress password | A second password per user. Entering it at the greeter, lock screen or elevation prompt looks like success or like a normal failure (per setting) and runs the duress action | S | 3 |
| SH03-F140 | Duress actions | One of: open a decoy home (separate dataset with plausible content the user prepared); destroy the wrapped keys of chosen datasets and vaults; send nothing over the network by default | S | 3 |
| SH03-F141 | Duress safety | Enrolment explains that key destruction cannot be undone, requires typing a confirmation phrase, and forces a backup-key decision; the duress password must differ from the real one by more than one character | S | 3 |
| SH03-F142 | No visible trace | The lock screen and greeter show no sign that a duress password exists; timing of success and failure paths is equalised to within 50 ms | S | 3 |
| SH03-F143 | Dead-man lock | Optional: if the session has been locked or suspended for more than n days, the next unlock needs the disk passphrase as well | C | 3 |
| SH03-F144 | Failed-attempt action | Optional: after n wrong passwords in a row (default off, minimum 10) power off and require the disk passphrase | C | 3 |
| SH03-F145 | Travel mode | One switch for crossing borders: lock-screen details hidden, tokens required, chosen vaults and datasets unloaded until turned off with the disk passphrase | C | 3 |

### 4.10 Settings, accessibility, recovery
| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| SH03-F146 | Settings pages | "Lock screen", "Sign-in options" and "Panic and duress" pages in SY01 use the schema in section 8; changing them needs an elevation prompt | M | 1 |
| SH03-F147 | Trust mark set-up | Chosen in AD01 first run from 48 built-in pictures plus two words from a word list; can be changed in SY01 after authentication | M | 1 |
| SH03-F148 | Screen reader | All trusted surfaces expose an a11y tree to the system AX apps only; password fields announce "edit, password" and character counts only if the user opts in | M | 1 |
| SH03-F149 | On-screen keyboard | The system OSK (AX03) is allowed above trusted surfaces as a trusted client; its key order can be shuffled for password entry (setting) | S | 2 |
| SH03-F150 | High contrast and scaling | Trusted surfaces follow the last user's accessibility settings on lock, and the system defaults on the greeter | M | 1 |
| SH03-F151 | Languages | Greeter language is the system language; a language menu changes it for the greeter and for the session being started | S | 2 |
| SH03-F152 | Lock client crash | See F041; the restart budget is 1 s; after three crashes a minimal built-in form (password only, no wallpaper, no feed) is used | M | 1 |
| SH03-F153 | Greeter crash | The session manager restarts it; after three crashes it falls back to a text login on the console | M | 1 |
| SH03-F154 | Authentication service down | Greeter and lock show "Sign-in is temporarily unavailable"; nothing unlocks; the service is restarted by the supervisor | M | 1 |
| SH03-F155 | Clock skew guard | Back-off timers use a monotonic clock; changing the wall clock does not shorten delays | M | 1 |
| SH03-F156 | Help | `F1` on any trusted surface shows built-in static help text (no app is launched from the greeter or lock screen) | S | 2 |

## 5. GUI

Every SH03 surface is full-output and drawn on the trusted layer (F087), except the disk-unlock screen, which runs before any compositor. There is no ordinary window.

### 5.1 Window map
```
Login, Lock and Session
├─ Disk-unlock screen               boot framebuffer, no compositor
│  ├─ Passphrase form / recovery-key form / token form
│  └─ Power menu
├─ Greeter                          greeter compositor instance, one surface per output
│  ├─ User list and sign-in form
│  ├─ Session type menu
│  ├─ Password change form
│  ├─ Token prompt
│  ├─ Accessibility menu · Language menu · Power menu
│  ├─ Integrity detail sheet
│  └─ Login banner
├─ Lock screen                      ext_session_lock surfaces, one per output
│  ├─ Clock face (stage 1)
│  ├─ Unlock form (stage 2): password, token, switch user
│  ├─ Notification feed · media controls · quick controls (from SH02)
│  ├─ Built-in OSD (volume, brightness)
│  └─ Emergency info sheet
├─ Elevation prompt                 trusted layer, attached above the requesting window
│  ├─ Approver chooser
│  ├─ Details expander / change list
│  └─ Token prompt
├─ Session dialog                   trusted layer
│  ├─ Confirm with countdown
│  ├─ Unsaved work list · Inhibitors list · Other users notice
│  ├─ Restart options
│  └─ Disposable-session end ("Keep these files…")
├─ User switcher                    trusted layer
├─ Panic progress strip             trusted layer, top edge
├─ Battery / thermal warning dialog
└─ Status items: cached approval, scheduled power (hosted by SH01)
Settings: pages in SY01. No window.
```

### 5.2 Main window
The lock screen, stage 2 (unlock form), whole primary output at 1440×900. The 3 px trust frame (1) is drawn by the compositor, not by SH03.

```
+========================================================================================+
‖ (1)                                                            [EN v] [♿ v] [⏻ v] (2) ‖
‖                                                                                        ‖
‖                                    14:32                                        (3)    ‖
‖                             Thursday 12 November                                       ‖
‖                                                                                        ‖
‖                                     (●)                                                ‖
‖                                 Odin Loch                                       (4)    ‖
‖                      +----------------------------------+                              ‖
‖                      | [••••••••••                 ] [👁] | (5)                         ‖
‖                      +----------------------------------+                              ‖
‖                        ⚠ Caps Lock is on                (6)                            ‖
‖                      [Unlock]   [Use security key]   [Switch user]  (7)                ‖
‖                                                                                        ‖
‖     +-------------------------------+        +--------------------------------------+  ‖
‖     | [🦉 picture]  amber · lantern | (8)    | Messenger            3 new       (9) |  ‖
‖     | Your trust mark               |        | Mail                 2 new           |  ‖
‖     +-------------------------------+        | Security Centre      1 alert  ⚠      |  ‖
‖                                              +--------------------------------------+  ‖
‖     +-------------------------------+        +--------------------------------------+  ‖
‖     | ♪ Track title — Artist        | (10)   | 🔈 [=======|-----]  ☼ [=========|--]  |  ‖
‖     |        [⏮] [⏯] [⏭]            |        | [☾ DND] [◉ Cam OFF] [🎙 Mic OFF] (11)|  ‖
‖     +-------------------------------+        +--------------------------------------+  ‖
‖  🔋 78%   ≋ connected          If found, call 0400 000 000                     (12)   ‖
+========================================================================================+
```

1. **Trust frame.** Compositor-drawn, reserved colour and dash pattern (F091). Present on every trusted surface.
2. **Top-right menus.** Keyboard layout `comboBox`, accessibility `toolButton` with `popupMenu`, power `toolButton` with `popupMenu` (Suspend; Restart and Shut down only if `lock.allow_power`).
3. **Clock and date.** `label`s, display size 24 scaled ×3.
4. **User.** `image` avatar and `label`.
5. **Password field.** `lineEdit` in password mode; `[👁]` reveals while held. Focused on show.
6. **Warnings.** `label` with icon: Caps Lock, Num Lock, back-off countdown ("Try again in 8 s"), service errors.
7. **Actions.** Primary `button` Unlock; `button`s for token and switch user.
8. **Trust mark.** `image` and `label`. Shown on the unlock form, the greeter after the user is chosen, and every in-session prompt.
9. **Notification feed.** `listView`, read-only rows from SH02 (F049). No actions.
10. **Media controls.** `label` and three `toolButton`s (F050).
11. **Quick controls.** `slider` × 2 and `toggleSwitch` × 3 (kill switches can only be turned off here).
12. **Status line.** Battery, network state, owner message.

Other outputs show the clock, date and blurred wallpaper only; clicking one moves the form there.

States: stage 1 clock face (no form, no feed bodies); typing; verifying (field disabled, spinner, at most 2 s); wrong password; back-off countdown; token waiting for touch; deep lock (form says "Password and security key needed"); details hidden (`lock.hide_details`: no feed, no media, no network state, no owner message); service unavailable; minimal fallback form (F152); screens off.

### 5.3 Other views and dialogs
#### 5.3.1 Greeter (whole primary output, 1440×900)
```
+========================================================================================+
‖  pbsd-laptop                                              [EN v] [English v] [♿ v] [⏻ v]‖
‖                                                                                  (1)   ‖
‖                                    14:32                                               ‖
‖                             Thursday 12 November                                       ‖
‖                                                                                        ‖
‖              (●)              (●)               (●)              (○)            (2)    ‖
‖           Odin Loch          Ada B.            Admin            Guest                  ‖
‖           ▔▔▔▔▔▔▔▔▔         Signed in                                                  ‖
‖                                                                          [Other…] (3)  ‖
‖                      +----------------------------------+                              ‖
‖                      | [Password                   ] [👁] | (4)                         ‖
‖                      +----------------------------------+                              ‖
‖                      Session [Desktop            v] (5)                                ‖
‖                      [Sign in]        [Use security key]      (6)                      ‖
‖                                                                                        ‖
‖                      +----------------------------------+                              ‖
‖                      | [🦉 picture]  amber · lantern     | (7) shown once the name is  ‖
‖                      +----------------------------------+     chosen, only if policy   ‖
‖                                                               `greeter.show_trust_mark`‖
‖                                                                                        ‖
‖  ✓ Verified boot · ParanoidBSD 0.1 · 🔋 78% · ≋ not connected                    (8)   ‖
+========================================================================================+
```
1. Machine name `label`; layout, language, accessibility and power menus.
2. User tiles, `gridView` of `image` + `label`; selected tile underlined; "Signed in" under users with a session. Hidden when `greeter.show_users` is off.
3. "Other…" `link` shows a name `lineEdit`.
4. Password `lineEdit`.
5. Session type `comboBox`: Desktop, Disposable, Safe mode, Recovery (administrators).
6. `button`s. Sign in is primary.
7. Trust mark of the chosen user. Off by default at the greeter, because showing it before authentication reveals it to anyone at the keyboard; on by default on the lock screen, where the session owner is already known.
8. Status line: verified-boot state (links to the integrity sheet), version, battery, network.

States: no users (first run starts), users hidden, failure with back-off, password expired (change form replaces 4–6), token prompt, starting session (progress line), session start failed, guest selected (no password field, "Start guest session"), service unavailable.

#### 5.3.2 Elevation prompt (520×auto, centred over the requesting window; whole screen shown)
```
+----------------------------------------------------------------------------------------+
|░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ everything else dimmed to 40% ░░░░░░░░░░░░░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░+==========================================================+░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ Allow Disks to erase a disk?                         (1) ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖                                                          ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ [D] Disks ✓  · system app · native                   (2) ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ wants to: Erase all data on a storage device         (3) ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ Risk: critical — this cannot be undone                   ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ +------------------------------------------------------+ ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ | Details from the app                              (4) | ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ | device  da1  (SanDisk Ultra, 64 GB, USB)              | ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ +------------------------------------------------------+ ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ Approve as  [Odin Loch (administrator)        v]     (5) ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ Password    [••••••••                     ] [👁]      (6) ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ and touch your security key                  ◌ waiting   ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ Allow: (•) once  ( ) for 5 minutes (not offered: critical)(7)‖░░░░░░░░░░░░░|
|░░░░░░░░░░░‖ [🦉] amber · lantern (8)   [Show details v]   1 of 2  (9) ‖░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░‖                              [Deny  Esc]  [▓▓▒ Approve] (10)‖░░░░░░░░░░░░░░|
|░░░░░░░░░░░+==========================================================+░░░░░░░░░░░░░░░░|
|░░░░░░░░░░░░░░░░░░░░░░ Press Ctrl+Alt+Delete if you are unsure this is real ░░░░░░ (11)░|
+----------------------------------------------------------------------------------------+
```
1. Title is the question, built from the registry text and the app's real name.
2. Requester line: icon, name, trust badge, kind, security label word (same vocabulary as SH05).
3. Registry description and risk word; `critical` in `--danger` with an icon.
4. Untrusted details box, monospace `label`s inside a bordered `panel` (F070).
5. Approver `comboBox` (only administrators).
6. Credential inputs per F072: `lineEdit`, token state `label`.
7. Duration `radioButton`s; options not allowed by the registry are shown disabled with the reason.
8. Trust mark.
9. Details expander and queue position.
10. `button` Deny (always enabled, `Esc`) and primary Approve with the 800 ms fill (F075).
11. Compositor-drawn hint line under the dim layer.

States: waiting for input; input delay; verifying; wrong credential (inline, back-off); token wrong PIN; requester exited (prompt closes with a toast "The app that asked has closed"); timeout (closes, denied); non-administrator with no administrators present ("Ask an administrator to sign in"); muted requester (no prompt, notification instead).

#### 5.3.3 Session dialog and confirm (trusted layer, 640×auto)
```
+==============================================================================+
‖ What do you want to do?                                           Odin Loch  ‖
‖  [🔒 Lock]  [⇄ Switch user]  [⇥ Log out]  [☾ Suspend]  [❄ Hibernate]        ‖
‖  [↻ Restart v]  [⏻ Shut down]                                          (1)   ‖
‖------------------------------------------------------------------------------‖
‖ Shut down in 27 s                                                      (2)   ‖
‖ Unsaved work                                                           (3)   ‖
‖   Writer        report-draft.odt                [Save] [Review]              ‖
‖   Text Editor   2 files                         [Save all] [Review]          ‖
‖ Still running                                                          (4)   ‖
‖   Backup        Snapshot upload, 4 min left     will be paused               ‖
‖   Software      Installing 3 updates            cannot be interrupted  ⚠     ‖
‖ 1 other user is signed in: Ada B.               needs approval         (5)   ‖
‖ [x] Install updates first (about 4 min)   [x] Reopen my windows next time    ‖
‖ [🦉] amber · lantern                 [Cancel  Esc]   [Wait]   [Shut down now] ‖
+==============================================================================+
```
1. Action tiles, `toolButton`s; arrow keys move, first letter jumps. Restart has a `popupMenu`: Restart, Restart into… (boot environments), Firmware set-up, Recovery.
2. Countdown `label` with `progressBar`; stops when any list below has a blocking row.
3. Unsaved work `modelListView` (F100).
4. Inhibitors `modelListView` (F101).
5. Other users notice (F102).

States: plain (no lists, countdown only); blocked (default button becomes "Wait"); disposable end ("Everything in this session will be erased. [Keep these files…]"); guest; policy forbids an action (tile disabled with the reason).

#### 5.3.4 Disk-unlock screen (boot framebuffer, 1024×768 minimum)
```
+----------------------------------------------------------------------------------------+
|  ParanoidBSD                                                              [EN] [⏻ v]   |
|                                                                                        |
|                          Unlock this computer                                          |
|                    pbsd-laptop · pool "zroot" · 1 of 1                                 |
|                                                                                        |
|                 +--------------------------------------------+                         |
|                 | [Passphrase                           ] [👁] |                         |
|                 +--------------------------------------------+                         |
|                   Wrong passphrase. 2 attempts. Try again in 4 s.                      |
|                 [Unlock]   [Use security key]   [Use recovery key]                     |
|                                                                                        |
|   ✓ Boot chain verified (loader, kernel, modules)                                      |
|   Last successful unlock: 11 Nov 2026 08:02                                            |
+----------------------------------------------------------------------------------------+
```
Widgets: `label`, `lineEdit`, `button`, `popupMenu` drawn by the CPU rasteriser. One static layout, no animation, no images beyond the logo. States: waiting, deriving key ("Checking…", up to several seconds by design of the key derivation), wrong, back-off, token, recovery key (8 groups of 6 characters, each `lineEdit` turns `--ok` when its checksum matches), boot chain changed (`--danger` panel: "The boot files changed since the last boot. If you did not update the system, do not type your passphrase." with [Continue anyway] needing a 5 s hold), text fallback.

#### 5.3.5 User switcher (trusted layer, 480×auto)
```
+====================================================+
‖ Switch user                                        ‖
‖ > (●) Odin Loch      this session · since 08:02    ‖
‖   (●) Ada B.         locked · since 09:40 · 2.1 GB ‖
‖   (●) Admin          not signed in                 ‖
‖   (○) Guest          start a guest session         ‖
‖ Your session will be locked.                       ‖
‖ [🦉] amber · lantern           [Cancel]  [Switch]  ‖
+====================================================+
```
`modelListView`; `Enter` switches. The target user authenticates on their own lock screen or the greeter. States: fast switching disabled by policy (only "Log out" offered), memory low ("Switching may be slow: 1.2 GB free").

#### 5.3.6 Token prompt (inside greeter, lock or elevation prompt, 360×auto)
```
+--------------------------------------+   +--------------------------------------+
| Security key                         |   | Security key                         |
| [🔑] Insert your key                  |   | [🔑] Touch your key now       18 s   |
|                                      |   |      "YubiKey 5 (…4821)"             |
| PIN [••••••          ]  5 tries left |   |                                      |
| [Cancel]              [Continue]     |   | [Cancel]                             |
+--------------------------------------+   +--------------------------------------+
```
States: insert, PIN, touch, wrong PIN (tries left from the token), PIN blocked ("This key is blocked. Use your password or recovery method."), unknown key ("This key is not registered for Odin Loch"), helper error.

#### 5.3.7 Panic strip and duress set-up (SY01 page content defined here)
```
+==============================================================================+
‖ ▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒  Keep holding to lock and cut devices… release to cancel ‖
+==============================================================================+

Panic and duress (page in SY01, after an elevation prompt)
+------------------------------------------------------------------------------+
| Panic shortcut      [Super+Shift+Esc]  hold [1.0 s v]                        |
| When triggered      [x] Lock  [x] Lock keyring  [x] Close vaults             |
|                     [x] Cut camera and microphone  [ ] Cut network           |
| Keep holding 3 s    [Power off and drop disk keys            v]              |
|------------------------------------------------------------------------------|
| Duress password     [Not set]                         [Set up…]              |
| When entered        ( ) Open decoy home  ( ) Destroy keys of: [Choose…]      |
| Looks like          (•) a normal sign-in  ( ) a wrong password               |
| ⚠ Destroying keys cannot be undone. Type DESTROY to enable: [________]       |
|------------------------------------------------------------------------------|
| Travel mode         [OFF ○]     Dead-man lock after [off v] days             |
+------------------------------------------------------------------------------+
```
The strip is a `progressBar` with a `label` on the trusted layer, top edge, shown while the shortcut is held. The page uses `comboBox`, `checkBox`, `radioButton`, `lineEdit`, `toggleSwitch`.

### 5.4 Menus
No menu bar. Menus (`popupMenu`):

| Menu | Items (shortcut) |
|---|---|
| Power (greeter, unlock screen) | Suspend · Restart · Shut down |
| Power (lock screen) | Suspend · Restart, Shut down (only if `lock.allow_power`) |
| Accessibility | Screen reader (`Super+Alt+S`) · Magnifier (`Super+Alt+M`) · On-screen keyboard (`Super+Alt+K`) · High contrast (`Super+Alt+H`) · Large text |
| Keyboard layout | Installed layouts (`Super+Space` cycles) |
| Language (greeter) | Installed languages |
| Session type | Desktop · Disposable · Safe mode · Recovery |
| Restart ▸ (session dialog) | Restart · Restart into… ▸ boot environments · Firmware set-up · Recovery image |
| User tile (greeter, context) | Sign in · Use security key · Session type ▸ |
| Cached approval status item | "Disks: erase — 4 min left" · Forget this approval · Forget all |
| Scheduled power status item | Cancel · Change time… |

### 5.5 Keyboard and pointer
| Keys | Action |
|---|---|
| `Ctrl+Alt+Delete` | Secure attention key: session dialog (compositor-handled, never delivered to apps) |
| `Super+L` | Lock |
| `Super+Shift+Esc` (hold) | Panic lock; keep holding for the second stage |
| Power key | Session dialog or suspend (setting) |
| Any key on the clock face | Show the unlock form and type into it |
| `Enter` / `Esc` | Submit / clear the field, then return to the clock face; in prompts `Esc` denies |
| `Tab` | Field, buttons, menus in reading order |
| `Up` / `Down` on greeter | Previous or next user |
| `Alt+S` | Session type menu (greeter) |
| `Alt+K` | Use security key |
| `Alt+U` | Switch user (lock screen) |
| `Alt+D` | Show details (elevation prompt) |
| `Super+Space` | Next keyboard layout |
| Accessibility shortcuts | As in the accessibility menu; work on every trusted surface |
| Media, volume and brightness keys | Work on the lock screen |

Pointer: click a user tile to select; click another output to move the form. No drag and drop anywhere on trusted surfaces. No context menus except the user tile. Touch: swipe up on the clock face shows the form. Paste is disabled in password fields on the greeter and unlock screen (there is no clipboard) and allowed in the elevation prompt only from the keyboard.

### 5.6 States
| State | Behaviour |
|---|---|
| Empty | No users: first run. No administrators present for elevation: explanatory message, deny only |
| Loading | "Checking…" on submit; session start progress line after 2 s |
| Error | One generic failure text for credentials; specific text for service faults (never for credential faults) |
| Offline | No effect; directory logins (phase 3) fall back to cached credentials if policy allows |
| Locked | The lock screen itself; deep-lock variant |
| Read-only / policy | Disabled actions say "Turned off by your administrator" |
| Narrow (< 640 px wide or < 480 px high) | Single column: clock, form, then feed below in a `scrollArea`; menus collapse into one `[=]` button |
| Rotated or mixed-scale outputs | Each lock surface lays out for its own output size and scale |
| Fallback | Minimal built-in form (lock), text login (greeter), text prompt (disk unlock) |

### 5.7 Widgets
| Region | FUSE widget |
|---|---|
| Forms | `panel`, `label`, `lineEdit` (password mode), `button`, `link` |
| User tiles | `gridView`, `image` |
| Menus | `toolButton`, `popupMenu`, `comboBox` |
| Feed, lists in dialogs, switcher | `listView`, `modelListView` |
| Sliders and switches from SH02 | `slider`, `toggleSwitch` |
| Countdown, input delay fill, panic strip | `progressBar` |
| Options | `radioButton`, `checkBox` |
| Scrolling in narrow layouts | `scrollArea` |
| Elevation prompt, session dialog | `dialog` on a layer surface |

Missing:

| Gap | What is missing | Where it lands |
|---|---|---|
| G-01, FUSE-WP-10.5 | Session lock client in FUSE | FUSE |
| SH03-GAP-1 | `ext_session_lock_v1` in the compositor. No lock module is in `PBSD:pbsd/compositor/` (also noted in FUSE_BUILD_PLAN.md, FUSE-WP-10) | compositor |
| SH03-GAP-2 | Trusted layer above `Overlay`, compositor-drawn trust frame and dim, secure attention key, exclusion from capture, refusal of synthetic input | compositor |
| SH03-GAP-3 | Authentication service `svc.auth`, action registry format and policy; not in PLATFORM.md section 8 | platform, G-09 |
| SH03-GAP-4 | Idle notification from the compositor (`ext_idle_notify_v1` or own); not found in `pbsd/compositor/` | compositor |
| SH03-GAP-5 | Session manager: seats, virtual terminals, one compositor per session, start order, save-query and inhibitor interfaces | platform |
| SH03-GAP-6 | Hand-off token from early boot to the authentication service (kernel-held, read once) | kernel, loader |
| SH03-GAP-7 | FUSE on a bare framebuffer for the disk-unlock screen (CPU rasteriser with a KMS or EFI framebuffer presenter and raw keyboard input) | FUSE-WP-25 plus a new presenter |
| SH03-GAP-8 | Password-mode `lineEdit` guarantees: no selection copy, no input method, buffer zeroed on clear | FUSE-WP-18 |
| SH03-GAP-9 | Trust-mark store readable only by trusted-path clients | SH03-GAP-3 |

## 6. Architecture

### 6.1 Components
| Component | Language | Process | Responsibility |
|---|---|---|---|
| Authentication service (`svc.auth`) | C++23 | `auth-service`, system service, capability mode after start | Verifies passwords and tokens, keeps failure counters and back-off, evaluates the action registry and policy, mints session and approval records, reads the hand-off token |
| Password back end | C++23 wrapper | inside `auth-service` | Calls the base system's OpenPAM stack (BSD-3-Clause, part of the base system) for `auth`, `account` and `password` phases |
| Token helper | C++23 | `token-helper`, one per operation, capability mode, only the HID descriptor it was passed | CTAP2 framing and CBOR parsing; shared design with SE04 |
| Session manager | C++23 (platform, SH03-GAP-5) | system service | Seats, virtual terminals, starts and stops compositors and sessions, inhibitors, save queries, power actions |
| Greeter | ForgeScript + FUSE UI | app host as user `greeter`, in the greeter compositor | 5.3.1 |
| Lock client | ForgeScript + FUSE UI on the FUSE lock client (FUSE-WP-10.5) | app host in the user session | 5.2 |
| Minimal lock form | C++23 | inside the lock client's host binary | Fallback form with no script and no assets (F152) |
| Prompt agent | ForgeScript + FUSE UI | app host in the user session, `shell.trusted-path` | Elevation prompt, session dialog, user switcher, warnings |
| Disk-unlock screen | C++23 + FUSE UI (static `.fuseui`, no script engine) | early-boot process in the boot image | 5.3.4; hands the passphrase to the key-loading tool over a pipe |
| Duress and panic controller | C++23 | inside `auth-service` | Runs configured actions; equalises timing |

Native code is used where a secret is held or hostile bytes are parsed (PLATFORM.md section 2, reasons a and c). The prompt UIs never see a verified secret: they pass typed characters to `auth-service` over a private channel and receive yes or no.

```
 early boot:  [disk-unlock screen] ──passphrase──▶ key loader ──one-time token──▶ kernel store
                                                                                      │ read once
 +------------------+   svc.auth    +------------------- auth-service ---------------v-----+
 | greeter (own     |◀────────────▶| passwords (OpenPAM) · tokens · counters · registry   |
 | compositor, VT n)|               | policy · approvals · duress/panic controller         |
 +------------------+               +---^-----------^-------------------^-----------------+
 +------------------+   svc.auth        |           | spawn             | audit
 | lock client      |◀──────────────────+     +-----v------+      +-----v-----+
 | (session lock)   |                         | token-helper|      | svc.audit |
 +------------------+                         +------------+      +-----------+
 +------------------+  Authorise1 from apps ─▶ auth-service ─▶ prompt agent draws, user answers
 | prompt agent     |◀──────────────────────────────────────────┘
 | (trusted layer)  |──power/session requests──▶ session manager ──▶ compositors, power
 +------------------+
 compositor: trusted layer, trust frame, secure attention key, lock protocol, idle events
```

### 6.2 Data model
System stores are under `/var/db/pbsd/auth/` (root-owned, readable only by `auth-service`). Per-user settings are in `svc.settings`.

| Entity | Fields |
|---|---|
| `AuthUser` | `uid: u32`, `factor_policy: enum{password,token,both,either}`, `trust_mark: {picture_id: u8, words: [u16;2]}`, `duress: DuressRecord?`, `handoff_bound: bool`, `last_success: timestamp`, `failed_since: u32` |
| `TokenCredential` | `uid`, `credential_id: bytes`, `public_key: bytes` (COSE), `aaguid: bytes[16]`, `label: string`, `sign_count: u32`, `uses: flags{login,unlock,elevate,disk}`, `added_at` |
| `BackoffState` | `subject: enum{uid,seat,disk}`, `id`, `failures: u32`, `next_allowed: monotonic_ns` |
| `ActionDef` (registry file, signed) | `id: string` (e.g. `org.pbsd.disks.erase`), `owner_bundle`, `description_key: string` (string table key), `risk: enum{low,high,critical}`, `default_policy: enum{allow,auth,deny}`, `cache_allowed: bool`, `detail_fields: [{name, type: enum{path,device,package,text,number}}]` |
| `PolicyOverride` (administrator file) | `action_id`, `policy` (may only tighten), `groups: [gid]` |
| `Approval` | `id: uuid`, `action_id`, `requester_bundle`, `requester_domain`, `approver_uid`, `credential_kinds: flags`, `granted_at`, `expires_at?`, `audit_id` |
| `AuthRequest` (memory) | `id`, `action_id`, `details: map<string, value>`, `window_token?`, `queued_at`, `state: enum{queued,shown,approved,denied,timeout}` |
| `SessionRecord` (session manager) | `session_id`, `uid`, `seat`, `vt: u8`, `type: enum{desktop,disposable,guest,safe,recovery,kiosk}`, `state: enum{active,locked,background}`, `home_dataset`, `started_at`, `locked_at?` |
| `DuressRecord` | `verifier: bytes` (same KDF as the real password), `action: enum{decoy,destroy_keys}`, `targets: [dataset or vault id]`, `appearance: enum{success,failure}` |
| `PanicConfig` (settings) | `shortcut`, `hold_ms: u16`, `stage1: flags`, `stage2: enum{none,poweroff,hibernate_drop_keys}` |
| `Inhibitor` (session manager) | `owner_bundle`, `what: flags{sleep,shutdown,idle,lock_idle}`, `reason: string`, `since` |

Registry and policy files carry `schema: u32`. Token and user records are versioned and migrate forward with the old file kept as `.bak.<version>`.

### 6.3 File formats and protocols
| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| CTAP2 over USB HID (CBOR) | yes | yes | own CBOR subset parser (definite lengths only) and CTAP framing; alternative under review: libfido2 (BSD-2-Clause) with libcbor (MIT), both build offline | `token-helper` (capability mode) |
| PIV smart card (ISO 7816 APDUs, X.509) | yes | yes | own APDU code; certificate parsing by the platform crypto library (FORGE-WP-22) | `token-helper` |
| Action registry and policy files (JSON, detached Ed25519 signature) | yes | no | own JSON reader with schema; signature check as FORGE V14 | `auth-service` |
| OpenPAM conversation | yes | yes | base system library | `auth-service` |
| `svc.auth` bus interfaces (`Authenticate1`, `Authorise1`, `Enrol1`) | yes | yes | schema compiler (G-08) | both ends; requests from apps are bounds-checked in `auth-service` |
| `ext_session_lock_v1` | client | client | FUSE lock client | lock client host |
| Lock feed from SH02 | yes | — | typed bus records, plain text only | lock client host |
| Lock wallpaper | pixels only | — | decoded by `svc.thumbnail` before locking, cached as raw pixels | thumbnail sandbox |
| Trust-mark pictures | yes | — | 48 pre-rasterised images in the signed bundle | none (no parsing at run time) |

### 6.4 Integration
- Intents: none handled or emitted from trusted surfaces. After unlock or login, links in notices open through `svc.intent` as usual.
- Services provided: `svc.auth` (`org.pbsd.auth.Authenticate1` for greeter and lock only; `org.pbsd.auth.Authorise1` for any app holding `svc.auth`; `org.pbsd.auth.Enrol1` for SY01 and SE04), `org.pbsd.session.Session1` (`lock()`, `request(action)`, `inhibit(what, reason) -> handle`, `query_save()` callbacks).
- Services used: `svc.settings`, `svc.keyring` (unlock and lock), `svc.audit`, `svc.power`, `svc.media` (lock controls), `svc.thumbnail`, `svc.a11y` (system AX apps only), SH02's `Indicators1`, `KillSwitch1` and lock feed.
- Other apps: SH01 (power row, session mark, kiosk), SH02 (feed, quick controls, critical fallback SH02-F166), SH04 (shares the trusted layer and trust mark; "Keep these files…" uses its chooser), SH05 (security label words, dimming of the requester window), SY01 (settings pages, users), SY05 (boot environments, staged updates), SE01 (verified-boot state), SE04 (token helper, enrolment), SE05 (vault close on lock and panic), AD01 (first run, trust mark set-up), AD02 (disk keys), AD03 (inhibitors), VM04 (remote rules), VM05 (USB policy while locked), AX01–AX03.
- Compositor (module files Verified present in `PBSD:pbsd/compositor/`): `…wayland.idle_inhibit.cppm` (F057), `…wayland.layer.cppm` with `…layer.keyboard.cppm` (exclusive keyboard for the prompt agent until the trusted layer exists), `…wayland.security_context.cppm` (identify sandboxed clients so they are refused the trusted layer), `…wayland.output.cppm` (one surface per output), `…wayland.keyboard_shortcuts_inhibit.cppm` (must not be able to swallow the secure attention key). Only headers were read. Not found: session lock, idle notify, any trusted-layer or secure-attention mechanism (SH03-GAP-1, -2, -4).
- FORGE: a keystore and OS sandbox layer exist (PLATFORM.md V18); how they would back `svc.keyring` unlock is Proposed (FORGE-WP-33).

## 7. Security and privacy

Capabilities requested (session-side bundle; `auth-service` and the session manager are system services outside the app capability model):

| Capability | Why |
|---|---|
| `fs.app` | Cached lock wallpaper pixels, UI state |
| `net.none` | No network use |
| `proc.helper:token-helper` | Token operations requested through `auth-service` |
| `svc.settings`, `svc.audit`, `svc.power`, `svc.media`, `svc.thumbnail`, `svc.a11y`, `svc.keyring` | As in 6.4 |
| `svc.auth` (Proposed service) | Authenticate and draw authorisation prompts |
| `dev.usb:hid-fido` | Token access, passed to the helper only |
| `bg.run`, `bg.autostart` | Lock client and prompt agent are session components |
| `shell.lock` (Proposed) | Bind the session lock protocol |
| `shell.trusted-path` (Proposed) | Draw on the trusted layer; read the trust mark |
| `shell.session` (Proposed) | Ask the session manager for power and session actions |

Not requested: `fs.portal`, any `net.*`, `proc.exec`, `dev.camera`, `dev.mic`, `dev.screen`, `ai.local`.

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Login password | Hostile app in the session | Fake lock screen or fake elevation prompt | Trusted layer, trust frame, trust mark, secure attention key (4.5); apps cannot go above `Overlay`, dim the screen or see the trust mark |
| Login password | Hostile app | Keylogging the real prompt | Input goes only to the trusted surface (F089); no client can observe another's keys under Wayland; synthetic input refused |
| Approval | Hostile app | Tricks the user into approving something else | Text comes from the signed registry (F068); app details are boxed and marked (F070); real requester identity from credentials |
| Approval | Hostile app | Prompt flood until the user gives in; pop-under timed to a keypress | Queue, merge, mute (F077); input delay (F075); default deny |
| Locked session | Person with physical access | Crash the lock client to reach the desktop | Compositor-side lock state, fail closed (F041); minimal fallback |
| Locked session | Person with physical access | Hostile USB device (keyboard emulator, storage) | USB policy while locked (F134); back-off on guesses |
| Locked session | Person with physical access | Reads notifications, network names | Redacted feed, `lock.hide_details`, travel mode |
| Disk key | Thief of a powered-off machine | Guessing | Key derivation cost and back-off at the unlock screen; token factor |
| Disk key | Evil-maid | Replaces the boot files to capture the passphrase | Verified-boot state shown before typing (F008); hand-off refused when changed |
| Hand-off token | Local process | Reads the token to log in | Kernel-held, readable once by `auth-service`, erased after; never after resume |
| Credentials at rest | Root-level reader of `/var/db` | Offline attack on verifiers | Memory-hard KDF verifiers; token public keys only; duress verifier indistinguishable from an unused slot |
| User under coercion | Coercer | Forces unlock | Duress password, panic, travel mode; no visible trace (F142) |
| Other users' sessions | Signed-in user | Reads a background session | Session isolation (F115); switching always locks |
| `auth-service` | Any app | Malformed `Authorise1` request | Schema and bounds checks; details limited to typed fields; T2 target |
| Token helper | Hostile token | Malformed CBOR or APDU | Helper in capability mode with one descriptor; T2 target on the parser |

Stored: verifiers and token public keys (system store), trust mark, counters, approvals (memory only), audit records through `svc.audit`. Leaves the machine: nothing. Duress actions never use the network by default.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `lock.idle_minutes` | u16 (0 = never) | 5 | Idle time before lock |
| `lock.grace_seconds` | u8 (0–30) | 5 | F044 |
| `lock.on_lid_close` | bool | true | Lock when the lid closes |
| `lock.on_suspend` | bool | true | Cannot be false on encrypted laptops under default policy |
| `lock.screen_off_seconds` | u16 | 30 | F056 |
| `lock.notifications` | enum{none,count,app_and_count,full} | app_and_count | Upper bound; SH02 per-app rules can only show less |
| `lock.media_controls` | bool | true | F050 |
| `lock.quick_controls` | bool | true | F051 |
| `lock.hide_details` | bool | false | F052 |
| `lock.allow_power` | bool | false on laptops, true on desktops | F053 |
| `lock.owner_message` | string(200) | "" | F054 |
| `lock.deep_after_minutes` | u16 (0 = off) | 30 | F059 |
| `lock.wallpaper` | enum{blurred_desktop,image,colour} | blurred_desktop | F060 |
| `lock.show_trust_mark` | bool | true | Trust mark on the lock screen |
| `greeter.show_users` | bool | true | F017 |
| `greeter.show_trust_mark` | bool | false | 5.3.1 callout 7 |
| `greeter.guest` | bool | false | F021 |
| `greeter.banner` | string | "" | F029 |
| `greeter.network` | bool | false | F031 |
| `handoff.enabled` | bool | true on single-user installs | F009 |
| `autologin.user` | string | "" | F028; needs hand-off |
| `auth.factor_policy.<uid>` | enum | password | F128 |
| `auth.cache_minutes` | u8 (0–15) | 5 | Upper bound for F073 |
| `auth.input_delay_ms` | u16 (≥ 500) | 800 | F075 |
| `auth.prompt_timeout_s` | u16 | 120 | F076 |
| `auth.backoff_max_s` | u16 | 60 | Ceiling of the delay |
| `token.pin_interval_hours` | u8 | 8 | F048 |
| `token.lock_on_removal` | bool | false | F130 |
| `session.confirm_seconds` | u8 (0–60) | 30 | F099 |
| `session.power_key` | enum{dialog,suspend,nothing} | dialog | F106 |
| `session.restore_windows` | bool | true | F110 |
| `session.fast_switching` | bool | true | F113 |
| `session.background_audio` | bool | false | F115 |
| `panic.config` | json | lock, keyring, vaults, camera, mic; stage 2 none | `PanicConfig` |
| `duress.enabled` | bool | false | F139 |
| `travel_mode` | bool | false | F145 |
| `deadman.days` | u8 (0 = off) | 0 | F143 |
| `failed_attempts.poweroff_after` | u8 (0 = off, min 10) | 0 | F144 |

All keys except `lock.wallpaper`, `lock.owner_message` and `session.restore_windows` need an elevation prompt to change.

## 9. Accessibility and languages

- The accessibility menu and its shortcuts work on the disk-unlock screen (high contrast and large text only there; speech needs audio, which is not up yet, so the screen also beeps once when ready for input and twice on a wrong passphrase).
- The system screen reader is a trusted client; third-party assistive tools get no access to trusted surfaces.
- Password fields: optional audible key feedback; never speak characters; announce Caps Lock.
- Time limits: the elevation prompt's 120 s timeout is announced at 30 s remaining and can be extended once with `F6`; back-off countdowns are announced at start and end only.
- The trust mark has a text form (the two words) so it does not depend on seeing the picture; the trust frame has a dash pattern so it does not depend on colour.
- Greeter and lock screen mirror for right-to-left languages; the clock follows the locale; user names are shown as entered with correct bidirectional isolation.
- All strings, including action descriptions in the registry, come from string tables keyed by `description_key`; a missing translation falls back to English, never to app-supplied text.

## 10. Performance budgets

| Item | Budget |
|---|---|
| `Super+L` to locked (compositor confirms) | ≤ 100 ms; first lock frame ≤ 50 ms after (wallpaper pre-decoded) |
| Key press on the clock face to form ready | ≤ 50 ms; no typed character is lost |
| Password check | ≤ 500 ms plus the KDF's set cost |
| Elevation request to prompt on screen | ≤ 150 ms |
| Secure attention key to session dialog | ≤ 100 ms |
| Greeter first frame after the compositor is up | ≤ 400 ms |
| Disk-unlock screen first frame after the framebuffer is up | ≤ 500 ms |
| Lock client memory | ≤ 40 MB resident; minimal form ≤ 4 MB |
| Idle while locked | 1 frame per minute (clock); 0 with screens off |
| Panic stage 1 complete | ≤ 300 ms from the hold threshold |
| Users on the greeter | up to 200 in the list (virtualised); above that the name field is forced |

## 11. Testing and proof

Test plan (targets; nothing has run):

- Unit: back-off arithmetic (monotonic, per user and per seat), registry parsing and policy tightening, approval cache expiry rules, factor policy evaluation, duress timing equaliser, inhibitor and save-query aggregation.
- Golden renders: every view in 5.2–5.3 in all themes, densities, RTL, 100–300% scale, and the narrow layout.
- Scripted UI through the a11y tree (system AX path): tasks T1–T14 on the headless compositor with a fake `auth-service`.
- Protocol and fault tests: kill the lock client at each state and check the outputs stay blank and locked; kill `auth-service` during a check; unplug an output during lock; suspend during the lock handshake.
- Security tests: an ordinary client tries to bind the lock protocol and the trusted layer (refused); tries to capture a trusted surface (gets "Protected"); injects input during a prompt (dropped); sends 100 `Authorise1` requests (queue, merge, mute); sends oversize and malformed details.
- Fuzz: `Authorise1` decoder, registry and policy readers, CBOR and CTAP parser, APDU parser, lock feed records.
- Differential: token helper against libfido2 test vectors and at least three physical token models; password path against the system's own `login` behaviour for the same PAM stack.
- Hardware matrix: lid, power key, suspend and resume, hibernate, multi-monitor hot-plug while locked.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Lock client state machine (FUSE-WP-10.5) | `unlock_and_destroy` is unreachable unless the host's authenticated flag was set by `auth-service` | T1 | PRISM model check (the job named in FUSE-WP-10) | Compositor protocol must exist (SH03-GAP-1) |
| Compositor lock state | From lock accept to authorised unlock, no non-lock surface is composed, including after client death | T1 | Model of the compositor's lock module | Module not written |
| `auth-service` decision function | Approve only if policy allows and required credential kinds verified; approvals never outlive expiry, lock or requester exit | T1 | Functional model checked against the C++ | Registry format must be frozen |
| Back-off counters | Delay is non-decreasing under failures and independent of wall-clock changes | T2 | Proof over integer arithmetic; unit tests | — |
| `Authorise1` request decoder | No runtime error on any input; outputs within limits | T2 | PRISM; fuzz | — |
| CBOR and CTAP parser | No runtime error on any input; bounded allocation | T2 | PRISM; fuzz; differential | Scope of CTAP messages supported |
| Registry signature check | Unsigned or altered registry entries are never used | T2 | Reuses the verified signature code (FORGE-WP-09) | Depends on that work |
| Duress path | Observable timing within 50 ms of the normal path | T3 | Measurement tests | Timing is not a property PRISM states |
| OpenPAM wrapper | Memory safety of our wrapper only | T3 | Sanitizers, review | Third-party C library and modules |
| Prompt and greeter UI (ForgeScript) | No runtime error in tests; never holds a verified secret | T3/T4 | Scripted UI; code review of the secret path | Interpreted code |
| Disk-unlock screen | No allocation after start; passphrase buffer zeroed on every exit path | T2 | PRISM on the C++ | Framebuffer presenter not written |

## 12. Build plan

Phase 1 — usable core:

| Item | Work | Depends on |
|---|---|---|
| SH03-W01 | `auth-service`: password check through OpenPAM, counters, back-off, audit | SH03-GAP-3, G-08 |
| SH03-W02 | Session manager minimum: one seat, one session, start order, log out, power actions | SH03-GAP-5 |
| SH03-W03 | Greeter: user list, password, session type (Desktop, Safe mode), status, accessibility menu, fallbacks | G-01, G-07, SH03-W01, SH03-W02 |
| SH03-W04 | Compositor session lock and fail-closed behaviour | SH03-GAP-1 |
| SH03-W05 | Lock client: two-stage screen, unlock, feed, status, screens off, minimal fallback form | FUSE-WP-10.5, SH03-W04, SH02-W16 |
| SH03-W06 | Lock triggers: shortcut, idle, lid, suspend ordering | SH03-GAP-4, `svc.power` |
| SH03-W07 | Trusted layer, trust frame, dim, secure attention key, capture exclusion in the compositor | SH03-GAP-2 |
| SH03-W08 | Trust mark store and set-up interface | SH03-GAP-9, AD01 |
| SH03-W09 | Action registry format, signature check, policy evaluation | FORGE-WP-09, SH03-W01 |
| SH03-W10 | Elevation prompt: content, approver, password, input delay, queue, audit | SH03-W07, SH03-W08, SH03-W09 |
| SH03-W11 | Session dialog with countdown; power key | SH03-W02, SH03-W07 |
| SH03-W12 | Disk-unlock screen: passphrase, back-off, text fallback | SH03-GAP-7, AD01 |
| SH03-W13 | Hand-off to first login and keyring unlock | SH03-GAP-6, `svc.keyring` |
| SH03-W14 | Settings schema and SY01 pages; last-login and unlock notices | SY01, SH02 |
| SH03-W15 | A11y on trusted surfaces for the system AX apps | G-04, AX01–AX03 |

Phase 2 — full feature:

| Item | Work | Depends on |
|---|---|---|
| SH03-W16 | Token helper (CTAP2), enrolment interface, token login, unlock and elevation factors | SE04, VM05 |
| SH03-W17 | Risk-based credential kinds, cached approvals with status item, terminal elevation | SH03-W10, SY03 |
| SH03-W18 | Unsaved work, inhibitors, other users, restart options, update on shutdown, restore choice | SH03-W11, SY05, AD03 |
| SH03-W19 | Fast user switching and the switcher; session isolation | SH03-GAP-5 |
| SH03-W20 | Guest and disposable sessions; erase-on-exit check; "Keep these files…" | ZFS clone support, SH04 |
| SH03-W21 | Media and quick controls on the lock screen; evict secrets on lock; inhibitor list | SH02-W28, SE05 |
| SH03-W22 | Panic lock and panic power-off | SH02-W26, SE05 |
| SH03-W23 | Verified-boot line, token and recovery-key disk unlock, per-user home keys | SE01, AD02 |
| SH03-W24 | USB policy while locked; password change flow; languages menu; shuffled OSK | VM05, AX03 |
| SH03-W25 | Hibernate safety, auto login, grace period, teaching hints, fullscreen hint | SH05 |

Phase 3 — advanced:

| Item | Work | Depends on |
|---|---|---|
| SH03-W26 | Duress password and actions, timing equaliser, set-up flow | SH03-W16, SE05, AD02 |
| SH03-W27 | Travel mode, dead-man lock, failed-attempt action, deep lock | SH03-W22 |
| SH03-W28 | Smart card login; lock on removal; unknown keyboard notice | SH03-W16 |
| SH03-W29 | Kiosk, recovery session, multi-seat | SH01-W32, SH03-W19 |
| SH03-W30 | Scheduled power, thermal and UPS dialogs, change preview in prompts | `svc.power` |
| SH03-W31 | Remote and headless elevation rules; greeter network; login banner | VM04, SH02-GAP-1 |
| SH03-W32 | Emergency info; duress at disk unlock | SH03-W26 |

## 13. Open questions and risks

1. **The trusted path is compositor work that does not exist** (SH03-GAP-1, -2, -4). Until it lands, prompts can only use layer `Overlay` with exclusive keyboard, which is weaker: there is no compositor-drawn frame and no secure attention key.
2. **Trust mark at the greeter.** Showing it before authentication reveals it to anyone who can reach the keyboard; not showing it means the greeter relies on the secure attention key alone. Default is off at the greeter. Needs a decision.
3. **OpenPAM** keeps third-party C modules in `auth-service`'s address space. A later replacement with an own verifier store would remove that, at the cost of compatibility with directory logins.
4. **One compositor per session** for fast switching costs memory and needs virtual terminal and DRM master hand-over to work on HardenedBSD; this is Unverified.
5. **Hand-off token** needs kernel and loader support (SH03-GAP-6). Without it, single-user machines ask for two passwords at boot.
6. **Framebuffer UI for disk unlock** (SH03-GAP-7) adds FUSE to the boot image. Size and the input stack there are unknown. The text fallback must always work.
7. **Duress features can destroy data** and can be dangerous to the user if discovered. They are phase 3, off by default, and need outside review before shipping.
8. **Hibernate** is only safe with encrypted swap and keys dropped; whether the platform supports this is Unverified.
9. **Timing equalisation** of the duress path is hard to guarantee on real hardware (disk activity from key destruction).
10. **Token support breadth.** An own CTAP2 implementation is small but must be tested against many devices; libfido2 is the alternative and brings a C dependency into the helper only.
11. **Action registry coverage.** Every privileged service must ship registry entries; an action with no entry is denied, which will surface as bugs during bring-up.
