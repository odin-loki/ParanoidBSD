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
