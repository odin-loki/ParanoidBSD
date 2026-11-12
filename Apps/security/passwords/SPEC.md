<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Passwords and Keys

By Odin Loch

| | |
|---|---|
| ID | `SE04` |
| Bundle | `org.pbsd.passwords` |
| Category | security |
| Priority | P0 |
| Backing repos | FORGE (keystore, `forge keygen/sign/verify`, ForgeScript), FUSE (UI), ParanoidBSD (`libfido2` and OpenSSH in the `hbsd/` reference tree) |
| Status | Planned. Nothing built. |

## 1. Purpose

Passwords and Keys is where a person keeps every secret: passwords, passkeys, one-time codes, SSH keys, OpenPGP keys, age keys, code-signing keys, certificates and hardware tokens. It fills logins through a trusted path so apps never see the vault, and it is the front end of `svc.keyring`. It replaces a password manager, a key manager, an SSH agent front end and a certificate viewer.

Ground truth (Verified, 12 Nov 2026):

| Fact | Source |
|---|---|
| FORGE has a keystore API `keystore::put/get/remove` and `backend()`. | `FORGE:docs/os.md` (table row `keystore.hpp`), `FORGE:docs/security.md` ("Secrets at rest", `os/include/forge/os/keystore.hpp`) |
| Its file back end keeps one mode-0600 file per secret, sealed with XChaCha20-Poly1305 under a store key. The store key can be wrapped by an Argon2id passphrase or by a TPM (opt-in). `keystore::rewrap(old, new)` changes the passphrase. | `FORGE:docs/os.md`, `FORGE:docs/security.md` |
| Stated weakness: unless a passphrase or the TPM wraps the store key, code running as the same user can read it. | `FORGE:docs/security.md` |
| Signing in FORGE is Ed25519 over a BLAKE3 digest, with `forge keygen`, `forge sign`, `forge verify`; signed bundles use a `FORGE-SIGNATURE.json` manifest and allow multiple signatures. | `FORGE:docs/security.md` (signing table) |
| A build-plan package covers the trust store, key hand-off to this app and revocation. | `Apps/00-foundation/build-plans/FORGE_BUILD_PLAN.md` FORGE-WP-33; app cryptography in FORGE-WP-22 |
| The reference tree carries a FIDO2 library and OpenSSH. | `hbsd/src/contrib/libfido2`, `hbsd/src/crypto/openssh` |
| `svc.keyring` is planned: "Secrets, keys, passkeys; unlocks with the session; apps see only their own items". | PLATFORM.md section 8 |

Everything else in this document is Proposed.

## 2. Scope

In scope:

- Password vaults: entries, folders, tags, generator, history, health audit, offline breach check.
- Autofill and autosave through a trusted path; passkeys (WebAuthn) as a platform authenticator; TOTP and HOTP.
- Keys: SSH keys and agent, OpenPGP, age, code-signing keys for `forge sign`, X.509 certificates and the trust store view.
- Hardware tokens: FIDO2, PIV, OpenPGP cards.
- Secret sharing, emergency access, sharing items without a server.
- Import and export: KeePass KDBX, CSV, JSON, others.
- Management view of `svc.keyring`: every app's stored secrets, by app, with revoke.
- Post-quantum options.

Out of scope:

| Item | Owner |
|---|---|
| File and folder encryption, containers, secure erase | SE05 (uses keys from here) |
| Mail encryption and signing workflow | CO01 (uses OpenPGP and S/MIME keys from here) |
| Git commit signing workflow | DV03 (uses keys from here) |
| Disk encryption passphrases and recovery keys set-up | AD02 (may store them here) |
| Login password, lock screen, session unlock UI | SH03 (unlock of `svc.keyring` happens there) |
| The prompt surfaces themselves (autofill picker, key-use confirmation) | SH04 draws them from data SE04 supplies |
| App capability grants | AD05 |
| Web browser's own form detection | IN01 (calls the autofill interface) |
| TLS trust decisions at run time | network service (FORGE-WP-21.5); SE04 shows and edits the store |

## 3. Users and top tasks

Personas:

- **Everyday user**: wants logins filled and saved; never wants to think about formats.
- **Developer**: SSH keys, an agent that asks before each use, commit and code-signing keys, tokens.
- **Administrator**: certificates, requests, trust store, hardware-token provisioning.
- **High-risk user**: hardware-bound keys, no cloud, split recovery, post-quantum options.
- **Migrating user**: arrives with a KDBX file or a CSV export.

| # | Task | Target |
|---|---|---|
| T1 | Fill a login in an app or the browser | `Ctrl+Shift+L` in the field, `Enter` (2 steps) |
| T2 | Save a new login when signing up | 1 click on the shell's "Save" prompt |
| T3 | Find an entry and copy its password | `Ctrl+K`, type, `Ctrl+Shift+C` (3 steps) |
| T4 | Generate a password | `Ctrl+G`, `Enter` (2 steps) |
| T5 | Copy a current TOTP code | Select entry, `Ctrl+T` |
| T6 | Sign in with a passkey | Confirm on the shell prompt (1 step) |
| T7 | Create an SSH key and copy the public half | 3 steps |
| T8 | Approve an SSH key use | 1 click or touch on the prompt |
| T9 | Import a KDBX file | Drop file, enter its password, review (3 steps) |
| T10 | See weak, reused and breached passwords | 1 click: Health |
| T11 | Create a code-signing key for `forge sign` | Wizard, 3 steps |
| T12 | Inspect a certificate file | Open it (0 further steps) |
| T13 | Change a hardware token's PIN | 3 steps |
| T14 | Split a recovery key among 5 trustees, any 3 to recover | Wizard, 4 steps |
| T15 | Lock everything now | `Ctrl+Shift+Delete` or `Super+L` |

## 4. Features

### 4.1 Vaults

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F001 | Several vaults | Personal by default; more can be added (work, shared, travel); each has its own key and unlock policy | M | 1 |
| SE04-F002 | Vault file | One file `.pbsdvault` per vault, documented open format (section 6.2); can live anywhere, opened by portal | M | 1 |
| SE04-F003 | Unlock factors | Passphrase (Argon2id); plus any of: key file, FIDO2 token (hmac-secret), TPM seal, session unlock through `svc.keyring` | M | 1 |
| SE04-F004 | Factor policy | Per vault: which factors are required together (e.g. passphrase AND token) and which are alternatives | S | 2 |
| SE04-F005 | Auto-lock | On idle (default 10 min), on session lock, on suspend, on lid close, on token removal | M | 1 |
| SE04-F006 | Quick unlock | After a full unlock, a short PIN or token touch for the session; limited tries, then full unlock | S | 2 |
| SE04-F007 | Change passphrase and KDF settings | Re-wraps the vault key; shows estimated unlock time on this machine | M | 1 |
| SE04-F008 | Key rotation | New vault key, re-encrypts all items in the background | S | 2 |
| SE04-F009 | Read-only open | Open a vault without write access (backup copies, shared media) | S | 1 |
| SE04-F010 | Travel mode | Mark vaults to be removed from the machine until switched back; restored from backup after | C | 3 |
| SE04-F011 | Merge and sync by file | Two copies of a vault merge by item and field with a conflict view; works with any file sync the user runs; no server | S | 2 |
| SE04-F012 | Vault history | Per-vault change log: who (which app or user action) changed which item when | S | 2 |
| SE04-F013 | Memory hygiene | Decrypted material only in the vault service; locked pages, no core dumps, zeroed on lock | M | 1 |

### 4.2 Entries

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F014 | Item types | Login, passkey, secure note, card, identity, Wi-Fi, server, database, API token, recovery codes, licence, key reference, custom | M | 1 |
| SE04-F015 | Fields | Typed fields: text, secret, URL, e-mail, date, TOTP, multi-line, file; custom fields with labels | M | 1 |
| SE04-F016 | Several URLs and app bindings | An entry matches web origins and app bundle IDs (with signer) | M | 1 |
| SE04-F017 | Match rules | Exact origin, registrable domain, host, starts-with, never; per entry | M | 2 |
| SE04-F018 | Attachments | Files stored encrypted inside the vault, up to 50 MiB each | S | 2 |
| SE04-F019 | Folders and tags | Nested folders, many tags, favourites | M | 1 |
| SE04-F020 | Password history | Every previous value with date; restore | M | 1 |
| SE04-F021 | Item history | Full version history per item; diff; restore | S | 2 |
| SE04-F022 | Expiry and reminders | Optional expiry date; reminders through `svc.notify` | S | 2 |
| SE04-F023 | Secret fields | Hidden by default, reveal on hold, large-type view, spell-out view (phonetic), copy with auto-clear | M | 1 |
| SE04-F024 | Clipboard safety | Copies carry the sensitivity flag for `svc.clipboard`; cleared after 30 s; never kept in clipboard history | M | 1 |
| SE04-F025 | Type into window | Auto-type through the trusted input path for apps with no fill support | S | 2 |
| SE04-F026 | Templates | Field sets for new items; user-defined templates | S | 2 |
| SE04-F027 | Duplicate, move, copy between vaults | With history kept or dropped by choice | M | 1 |
| SE04-F028 | Trash | Deleted items stay 30 days, then are purged | M | 1 |
| SE04-F029 | Search | Instant, over titles, user names, URLs, tags, notes; secrets are searched only on request | M | 1 |
| SE04-F030 | Filters and smart lists | By type, tag, vault, age, strength, has TOTP, has passkey, expiring | S | 2 |
| SE04-F031 | Notes | Markdown notes per item | S | 1 |
| SE04-F032 | QR display | Show Wi-Fi credentials or any field as a QR code | C | 2 |
| SE04-F033 | Bulk edit | Tag, move, delete many | S | 2 |

### 4.3 Generator

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F034 | Random passwords | Length 4–128; classes; exclude look-alikes; required counts per class; custom set | M | 1 |
| SE04-F035 | Passphrases | Word lists per language (bundled, signed); 3–12 words; separator; capitalisation; optional number | M | 1 |
| SE04-F036 | Pronounceable and PIN modes | Syllable-based; numeric PIN 4–12 | S | 2 |
| SE04-F037 | Entropy display | Bits of entropy and an estimated guess time under stated assumptions | M | 1 |
| SE04-F038 | Site rules | Per-entry constraints (max length, forbidden characters) remembered | S | 2 |
| SE04-F039 | Generator history | Last 20 generated values, encrypted, 24 h, so a value is not lost if a form fails | S | 1 |
| SE04-F040 | Randomness source | Kernel CSPRNG only; a self-test on start | M | 1 |
| SE04-F041 | User names and e-mail aliases | Random user names; plus-address aliases from a user's own address pattern | C | 3 |

### 4.4 Health and breach check

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F042 | Weak passwords | Strength estimate by pattern analysis (dictionary words, sequences, dates, keyboard walks) | M | 2 |
| SE04-F043 | Reused passwords | Groups of entries sharing a password, compared inside the vault service | M | 2 |
| SE04-F044 | Old passwords | Not changed in N months (default 24) | S | 2 |
| SE04-F045 | Offline breach check | Test each password against a local set of known-breached password hashes, shipped as an optional signed package; no network request is ever made | M | 2 |
| SE04-F046 | Breach set format | A compact filter with a stated false-positive rate (target 1 in 10 million), plus an exact check file when space allows | S | 2 |
| SE04-F047 | Missing second factor | Entries for sites known to support TOTP or passkeys but with none stored (from an offline list) | C | 3 |
| SE04-F048 | Insecure URLs | Entries with `http:` login URLs | S | 2 |
| SE04-F049 | Expiring items | Cards, certificates, keys, licences near expiry | S | 2 |
| SE04-F050 | Health score and list | One page; each finding has a "Fix" that opens the entry with the generator ready | M | 2 |
| SE04-F051 | Ignore with reason | Mark a finding accepted; listed separately | S | 2 |
| SE04-F052 | Feed to SE01 | Counts only (never the secrets) reach Security Centre | S | 2 |

### 4.5 Autofill and autosave

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F053 | Trusted fill path | An app asks the fill interface; the shell draws the picker; the chosen secret goes straight into the focused field through the input path. The app never sees the candidate list | M | 1 |
| SE04-F054 | Requester identity | Matching uses the app's bundle ID and signer; for the browser, also the web origin that the browser attests | M | 1 |
| SE04-F055 | Picker content | Matching entries first; search box; user name and vault shown, never the password | M | 1 |
| SE04-F056 | Fill confirmation | First fill of an entry into a new app or origin asks; later fills follow the entry's policy (always ask, ask once, never ask) | M | 1 |
| SE04-F057 | Phishing guard | No match for the origin means no suggestion; filling an entry into a non-matching origin needs a typed confirmation | M | 1 |
| SE04-F058 | Autosave | After a sign-up or changed password the shell offers Save or Update; generator values are pre-linked | M | 2 |
| SE04-F059 | TOTP fill | After a password fill, the code is offered to the next one-time-code field or copied | S | 2 |
| SE04-F060 | Cards and identities | Fill payment and address fields on request only, never suggested unprompted | C | 3 |
| SE04-F061 | Global shortcut | `Ctrl+Shift+L` opens the picker for the focused field in any app | M | 1 |
| SE04-F062 | Non-cooperating apps | Auto-type with a per-entry key sequence (`{USER}{TAB}{PASS}{ENTER}`) | S | 2 |
| SE04-F063 | Terminal and sudo fill | Fill into a terminal password prompt only when the terminal reports echo is off | C | 3 |
| SE04-F064 | Fill audit | Each fill is recorded: entry, app, origin, time | M | 1 |

### 4.6 Passkeys and one-time codes

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F065 | Platform authenticator | The vault service acts as a WebAuthn authenticator for the browser and for apps: create and assert discoverable credentials | M | 2 |
| SE04-F066 | Algorithms | ES256 and EdDSA; others as the standard requires | M | 2 |
| SE04-F067 | User verification | Each assertion needs presence (click or key) and, by policy, verification (session re-authentication or token touch) | M | 2 |
| SE04-F068 | Passkey list | Relying party, user handle, created, last used, use count, where stored (vault or hardware token) | M | 2 |
| SE04-F069 | Hardware passkeys | Create and use credentials on a FIDO2 token instead of the vault; chosen at creation | M | 2 |
| SE04-F070 | Passkey export and import | Encrypted transfer file to move passkeys between vaults or machines; follows the published credential-exchange format where it is stable | S | 3 |
| SE04-F071 | TOTP | RFC 6238; SHA-1, SHA-256, SHA-512; 6–8 digits; 30 or 60 s; add by `otpauth:` URI, QR image, camera or manual secret | M | 1 |
| SE04-F072 | HOTP | RFC 4226 with a counter | S | 2 |
| SE04-F073 | Code display | Large digits, countdown ring, next code preview, copy | M | 1 |
| SE04-F074 | Clock check | Warns when the system clock is unsynchronised | S | 2 |
| SE04-F075 | Separation option | Keep TOTP secrets in a different vault from passwords, so one vault file alone is not both factors | S | 2 |
| SE04-F076 | Recovery codes | Typed item with used and unused marking | S | 2 |

### 4.7 SSH keys and agent

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F077 | Key list | Type, bits, fingerprint (SHA-256), comment, created, last used, where stored | M | 1 |
| SE04-F078 | Generate | Ed25519 (default), ECDSA P-256/384, RSA 3072/4096; token-backed `sk` keys | M | 1 |
| SE04-F079 | Import and export | OpenSSH private and public formats, PEM, PPK; private export needs authentication | M | 1 |
| SE04-F080 | Agent | The key service speaks the SSH agent protocol on a per-app socket; an app gets a socket only with the right capability | M | 1 |
| SE04-F081 | Per-use confirmation | Each signature request shows app, key, host (when the client supplies it); policy per key: always ask, ask once per session, ask once per host, never | M | 1 |
| SE04-F082 | Key constraints | Lifetime, destination constraints, confirm-on-use, per-app allow list | S | 2 |
| SE04-F083 | Certificates | Hold and present SSH user certificates; show validity and principals; simple signing with a local CA key | C | 3 |
| SE04-F084 | Known hosts | View, verify, remove host keys; show changes as alerts; hashed names supported | S | 2 |
| SE04-F085 | Authorised keys helper | Copy a public key in the right one-line form; show as QR | S | 1 |
| SE04-F086 | Agent forwarding | Off by default; when on, every forwarded use asks | S | 2 |
| SE04-F087 | Git and commit signing | Offer SSH or OpenPGP key to DV03 by reference | S | 2 |

### 4.8 OpenPGP, age and signing keys

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F088 | OpenPGP key ring | Own and others' certificates; user IDs, subkeys, capabilities, expiry, algorithms | M | 2 |
| SE04-F089 | Generate OpenPGP key | Ed25519 and X25519 by default; RSA for compatibility; subkeys per use; expiry default 2 years | M | 2 |
| SE04-F090 | Key maintenance | Extend expiry, add and revoke subkeys and user IDs, change passphrase, make a revocation certificate at creation | M | 2 |
| SE04-F091 | Import and export | Armoured and binary; public only by default; secret export needs authentication | M | 2 |
| SE04-F092 | Verification of others' keys | Compare fingerprints (words or QR); local "verified" mark with date and method; link to contacts in CO03 | S | 2 |
| SE04-F093 | Key discovery | Off by default. Optional lookup by e-mail through the published web key directory method, per request, over the user's chosen route | C | 3 |
| SE04-F094 | Crypto operations service | Sign, verify, encrypt, decrypt for CO01, SE05, DV03 through the key service; private keys never leave it | M | 2 |
| SE04-F095 | age identities and recipients | Generate X25519 identities; passphrase-wrapped identities; recipient list with names; export recipient string and QR | M | 2 |
| SE04-F096 | age with SSH keys | Use SSH Ed25519 and RSA keys as age recipients and identities | S | 3 |
| SE04-F097 | Code-signing keys | Ed25519 keys for `forge sign`: create, name the publisher, export the public key, sign through the key service without exposing the seed | M | 2 |
| SE04-F098 | Publisher identity | A signed statement binding a publisher name to a public key; shown as a card; exportable for others' trust stores | S | 2 |
| SE04-F099 | Signing policy | Per key: ask on every signature, show what is being signed (file name, BLAKE3), allowed apps (DV01, SY05 build tools) | M | 2 |
| SE04-F100 | Rotation and revocation | Successor key signed by the old one; revocation record; export for the trust store | S | 3 |
| SE04-F101 | Multiple signatures | Add a second signature to a bundle (FORGE bundles allow several) | C | 3 |
| SE04-F102 | Key backup | Paper backup (words or QR pages) and encrypted file backup for any private key | S | 2 |

### 4.9 Certificates and trust store

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F103 | Certificate viewer | Subject, issuer, validity, serial, key, extensions, fingerprints, chain; opens `.crt`, `.pem`, `.der`, `.p7b`, `.p12` | M | 1 |
| SE04-F104 | Chain validation | Build and check a chain against the system store at a chosen time; explain each failure | S | 2 |
| SE04-F105 | System trust store view | Roots shipped on the verified dataset, read-only; each with fingerprint and source | M | 1 |
| SE04-F106 | User trust changes | Add a private root, distrust a shipped root; needs authentication; every change is a finding in SE01 | M | 2 |
| SE04-F107 | Per-app pins | View the pins apps declare (network service data); read-only | C | 3 |
| SE04-F108 | Client certificates | Hold client certificates with private keys; present through the network service with a confirmation prompt | S | 2 |
| SE04-F109 | Signing requests | Generate a key and a certificate signing request; import the issued certificate and match it | S | 2 |
| SE04-F110 | Self-signed and local CA | Create a self-signed certificate; small local CA for development (issue, revoke, list) | C | 3 |
| SE04-F111 | S/MIME certificates | Store and present to CO01 | S | 2 |
| SE04-F112 | Expiry watch | Reminders for own certificates | S | 2 |
| SE04-F113 | Import PKCS #12 | In the sandboxed parser; legacy ciphers accepted for import only, flagged | M | 2 |
| SE04-F114 | Revocation data | Show CRL and OCSP pointers; check only on request and over the user's route | C | 3 |

### 4.10 Hardware tokens

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F115 | Token list | Connected tokens with model, firmware, serial where readable, supported applications | M | 2 |
| SE04-F116 | FIDO2 management | Set and change PIN, list and delete resident credentials, enforce user verification, reset (with a clear warning) | M | 2 |
| SE04-F117 | FIDO2 as unlock factor | Enrol a token for vault unlock (hmac-secret); enrol at least two or record a recovery path | M | 2 |
| SE04-F118 | PIV | Slots, certificates, generate key on token, import certificate, change PIN and PUK, management key, touch and PIN policy | S | 3 |
| SE04-F119 | OpenPGP card | Key slots, generate on card or move a key to it, PINs and retry counters, touch policy, cardholder data | S | 3 |
| SE04-F120 | Token-backed SSH | `sk` keys and PIV keys offered by the agent | S | 2 |
| SE04-F121 | Attestation view | Show the token's attestation certificate chain where offered | C | 3 |
| SE04-F122 | Device access | Only through the USB portal with class limits (HID for FIDO, CCID for smart cards); indicator while in use | M | 2 |
| SE04-F123 | NFC and Bluetooth tokens | Deferred until the platform has those transports | C | 3 |

### 4.11 Sharing, recovery, emergency access

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F124 | Share an item as a file | Encrypt selected items to a recipient's public key (age or OpenPGP) or to a one-time passphrase; the result is a file the user sends any way they like | S | 2 |
| SE04-F125 | Shared vaults | A vault whose key is wrapped for several people's public keys; members listed; removing a member rotates the key | S | 3 |
| SE04-F126 | Secret splitting | Split a vault key or any secret into N shares with threshold K (Shamir over GF(256)); shares as words, QR or files | M | 2 |
| SE04-F127 | Share verification | Each share carries a check value; a recovery rehearsal confirms K shares work without revealing the secret to the screen | S | 2 |
| SE04-F128 | Emergency kit | Printable sheet: vault location, format version, recovery instructions, optional share or key; laid out for paper | M | 2 |
| SE04-F129 | Emergency access, offline | A sealed recovery package for a trusted person: encrypted to their key, plus shares held by others. There is no server and so no timed release; stated plainly | S | 3 |
| SE04-F130 | Recovery from shares | Guided entry of shares; checks each; rebuilds the key inside the vault service | M | 2 |
| SE04-F131 | Duress vault (discussion only) | Not planned for phase 1–3; hidden-volume limits are discussed in SE05 | C | 3 |

### 4.12 Import and export

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F132 | KDBX import | KDBX 3.1 and 4.x: groups, entries, history, attachments, custom fields, TOTP attributes, key files | M | 1 |
| SE04-F133 | KDBX export | KDBX 4 with Argon2id and ChaCha20 | S | 2 |
| SE04-F134 | CSV import | Column mapping screen with preview; presets for common exporters | M | 1 |
| SE04-F135 | JSON imports | Documented exports of other managers (each a separate mapping file, signed, updatable) | S | 2 |
| SE04-F136 | Browser CSV | Imports the CSV shape browsers export | S | 2 |
| SE04-F137 | `otpauth:` and migration QR | Import one or many TOTP secrets | S | 2 |
| SE04-F138 | Encrypted export | Whole vault or selection to `.pbsdvault`, KDBX or an age-encrypted JSON | M | 1 |
| SE04-F139 | Plain export | CSV or JSON only after authentication and a typed confirmation; the file is flagged sensitive for SY02 | S | 2 |
| SE04-F140 | Duplicate detection | On import, show probable duplicates and let the user merge | S | 2 |
| SE04-F141 | Import in a sandbox | Every foreign file is parsed by a helper with no access but the input handle | M | 1 |
| SE04-F142 | Import report | Counts per type, skipped rows with reasons | M | 1 |

### 4.13 Keyring management, post-quantum, basics

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| SE04-F143 | App secrets view | Every item apps stored in `svc.keyring`, grouped by app: label, kind, created, last read; values hidden | M | 1 |
| SE04-F144 | Revoke and delete app secrets | Delete an app's items; block an app from the keyring | M | 1 |
| SE04-F145 | Access log | Which app read which of its items when; denials | M | 2 |
| SE04-F146 | Keyring unlock policy | With the session, with an extra passphrase, with a token; auto-lock timing | M | 1 |
| SE04-F147 | Store key protection | Shows whether the keyring's store key is wrapped by passphrase, TPM or neither; "neither" is a finding | M | 1 |
| SE04-F148 | Hybrid sharing | Sharing and shared-vault wraps use X25519 together with ML-KEM-768 | S | 3 |
| SE04-F149 | Post-quantum signatures | Optional ML-DSA-65 signing keys and dual signatures (Ed25519 plus ML-DSA) for code signing, when FORGE's verifier supports them | C | 3 |
| SE04-F150 | Algorithm inventory | A page listing every key by algorithm with a "quantum-vulnerable" mark and migration advice | S | 3 |
| SE04-F151 | Symmetric margins | Vault encryption uses 256-bit keys; stated as adequate against known quantum attacks | M | 1 |
| SE04-F152 | Lock now | One shortcut locks all vaults and flushes agent keys | M | 1 |
| SE04-F153 | Command-line twin | `pwctl get|generate|totp|lock`, each use confirmed on the trusted path | S | 2 |
| SE04-F154 | Print | Emergency kit, paper key backups, entry lists (after authentication) | S | 2 |
| SE04-F155 | Crash safety | Vault writes are atomic (write new, sync, rename); the previous file is kept as `.bak` | M | 1 |
| SE04-F156 | Local assistant: none | No `ai.local` use; secrets are never sent to a model | M | 1 |

## 5. GUI

Layout pattern: three-pane master-detail.

### 5.1 Window map

```
Main window
├─ Sidebar
│   ├─ Vaults: All items · Favourites · each vault with its folders · Tags · Trash
│   ├─ Types: Logins · Passkeys · One-time codes · Notes · Cards · …
│   ├─ Keys: SSH · OpenPGP · age · Signing · Certificates · Trust store
│   ├─ Tokens
│   ├─ Health
│   └─ App secrets (keyring)
├─ Item list (middle)
├─ Detail / editor (right)
├─ Dialogs: Unlock · New vault · Generator · Import (wizard) · Export · New SSH key · New OpenPGP key ·
│           New signing key · Certificate request · Split secret (wizard) · Recover from shares ·
│           Token PIN · Token reset · Share item · Merge conflicts · Change passphrase
├─ Sheets: Field history · Item history · Match rules · Key use policy
├─ Popovers: Generator (inline) · TOTP large view · QR · Fingerprint compare
└─ Mini window: Quick access (search + copy), opened by a global shortcut
Shell surfaces (SH04/SH03): fill picker, save/update prompt, passkey prompt, key-use confirmation,
                            vault unlock prompt, token touch prompt
```

### 5.2 Main window

Logins at 1280×800, unlocked.

```
+------------------------------------------------------------------------------------------+
| [=] Passwords and Keys — Personal    [ Search items…  Ctrl+K ]         🔓 (1) [▁][▢][✕]  |
+---------------+----------------------------+---------------------------------------------+
| VAULTS        | (3) [+ New ▾] [Sort: name▾]| (4) example.org                    ★  [Edit]|
|  All items 212|----------------------------|   Login · Personal / Web · changed 3 Mar    |
|  Favourites 9 | ▸ bank.example             |---------------------------------------------|
| > Personal    |   anna@…                   | User name   anna@example.org         [Copy] |
|    Web    140 | > example.org              | Password    ••••••••••••••  [👁][Copy][⟳] (5)|
|    Work    31 |   anna@example.org   ⏱ 🔑  |             strong · 86 bits · not breached |
|  Shared    12 |   forum.example            | One-time    492 031   ◔ 14 s         [Copy] |
|  Tags        ▸|   anna_k                   | Passkey     ES256 · created 2 Jan · 41 uses |
|  Trash      3 |   mail.example             | Website     https://example.org/login [Open]|
| TYPES         |   anna@…             ⏱     | Matches     example.org (domain) · app      |
|  Logins   171 |   shop.example             |             org.pbsd.browser                |
|  Passkeys  14 |   anna@…          ! reused | Fill policy ask once per app                |
|  Codes     38 |   wiki.example             | Tags        [personal] [2fa]                |
| KEYS (2)      |   anna                     | Notes       Recovery codes in "example.org  |
|  SSH        4 |                            |             codes".                         |
|  OpenPGP    2 |                            |---------------------------------------------|
|  age        1 |                            | (6) History: password changed 3 Mar 2026 ·  |
|  Signing    1 |                            |     5 earlier versions            [Show…]   |
|  Certificates |                            |                                             |
| TOKENS      1 |                            |                                             |
| HEALTH    ! 6 |                            |                                             |
| APP SECRETS 23|                            |                                             |
+---------------+----------------------------+---------------------------------------------+
| (7) 212 items · Personal unlocked, locks in 9 min · Agent: 2 keys · Clipboard clears 22 s|
+------------------------------------------------------------------------------------------+
```

Callouts:

1. Lock state button in the header: click locks; shows which vaults are open.
2. Sidebar: `treeView` with counts and badges; sections are collapsible.
3. Item list: `modelListView`, two-line rows, icons for TOTP `⏱`, passkey `🔑`, findings `!`; type-ahead.
4. Detail header: title, type, location, change date; favourite `toolButton`; `button` Edit.
5. Secret field (design-system pattern): reveal on hold, copy, regenerate (opens the generator popover). Strength and breach line under it.
6. History summary with a `link` to the history sheet.
7. Status bar: counts, lock countdown, agent state, clipboard countdown.

States: locked (the whole content is replaced by an unlock panel; titles are not shown), empty vault ("No items yet" with "New login" and "Import…"), loading (unlock derivation progress), read-only vault (banner, edit disabled), narrow (list → detail navigation).

### 5.3 Other views and dialogs

**Locked**

```
+---------------------------------------------------------------------------------------+
|                               🔒  Personal is locked                                  |
|                 Passphrase [______________________________] [👁]                      |
|                 [ ] Also unlock: Work, Shared                                         |
|                 Token: touch your security key when it blinks                         |
|                                  [Unlock]        [Recover from shares…]               |
|                 Vault file: ~/Vaults/personal.pbsdvault · last opened today 08:02     |
+---------------------------------------------------------------------------------------+
```

The passphrase field is drawn by this app only for vaults opened by file. The session keyring and quick-access unlock are drawn by SH03 on the trusted path.

**Generator**

```
+---------------------------------------------------------------------+
| Generator                                                           |
| (•) Password  ( ) Passphrase  ( ) PIN  ( ) Pronounceable            |
|  hR7#vq2!Lm9xT$w4Zc8p                              [⟳] [Copy]       |
|  strong · 131 bits · centuries at 10¹² guesses/s                    |
| Length [ 20 ] ──────●──────────                                     |
| [x] a–z  [x] A–Z  [x] 0–9  [x] symbols [ !#$%&*… ]                  |
| [x] Avoid look-alikes (O0 Il1)   Minimum digits [1] symbols [1]     |
| Site rule for example.org: max 32, no spaces                        |
|                                   [Cancel] [Use this password]      |
+---------------------------------------------------------------------+
```

**Health**

```
+---------------------------------------------------------------------------------------+
| Health   Score 78   Breach set: 2026-10 package, checked locally   [Check again]      |
+---------------------------------------------------------------------------------------+
| ! Breached passwords (1)                                                              |
|     forum.example       anna_k        found in the offline breach set        [Fix]    |
| ! Reused (2 groups, 5 items)                                                          |
|     shop.example · wiki.example · old-shop.example            same password  [Review] |
| ! Weak (2)                                                                            |
|     router              admin         9 bits (dictionary word + digit)       [Fix]    |
| · Old (11)  · No second factor where available (4)  · Expiring (1 certificate)        |
| Accepted risks (1)                                                           [Show]   |
+---------------------------------------------------------------------------------------+
```

**Keys › SSH**

```
+----------------------------+----------------------------------------------------------+
| SSH keys   [+ New…] [Import]| laptop-2026 (Ed25519)                          [Export…]|
|----------------------------| SHA256:Qm3…k9E                           [Copy] [QR]     |
| > laptop-2026   Ed25519    | Public key  ssh-ed25519 AAAAC3… anna@laptop     [Copy]   |
|   deploy        Ed25519-sk | Stored in   Personal vault                               |
|   legacy        RSA 4096   | Created 4 Jan 2026 · last used today 13:20 by Terminal   |
|   work-piv      ECDSA (PIV)| Agent       [x] Offer through the agent                  |
|                            | Confirm     ( ) Never (•) Once per host ( ) Every use    |
|                            | Lifetime    [ until lock v]   Apps  [Terminal] [IDE] [+] |
|                            | Constraints [ + destination constraint ]                 |
|                            |----------------------------------------------------------|
|                            | Recent uses                                              |
|                            | 13:20 Terminal → git.example.org   approved (host rule)  |
|                            | 09:02 IDE      → build.example.org approved by you       |
+----------------------------+----------------------------------------------------------+
```

**Certificates › viewer**

```
+---------------------------------------------------------------------------------------+
| www.example.org                                    ✔ valid · chain trusted (system)   |
+--------------------------+------------------------------------------------------------+
| ▾ Example Root CA        | Subject     CN=www.example.org                             |
|   ▾ Example Issuing CA 2 | Issuer      CN=Example Issuing CA 2                        |
|     > www.example.org    | Valid       1 Oct 2026 – 30 Dec 2026 (48 days left)        |
|                          | Key         ECDSA P-256                                    |
|                          | Names       www.example.org, example.org                   |
|                          | Usage       digital signature · server authentication      |
|                          | SHA-256     7A:1F:…:C3                             [Copy]  |
|                          | [Details ▸]  [Export…]  [Check at another date…]           |
+--------------------------+------------------------------------------------------------+
```

**Tokens**

```
+---------------------------------------------------------------------------------------+
| Tokens                                                                                |
| ● Security key (USB) · firmware 5.4 · FIDO2, PIV, OpenPGP                             |
| [FIDO2] [PIV] [OpenPGP card]                                                          |
| PIN: set · 8 tries left      [Change PIN…]   User verification: required              |
| Passkeys on this token (3 of 25)                                                      |
|   example.org        anna@example.org      created 2 Jan      [Delete]                |
|   git.example.org    anna                  created 9 Feb      [Delete]                |
| Used for: vault unlock (Personal), SSH key "deploy"                                   |
|                                              [Reset token…] (deletes everything on it)|
+---------------------------------------------------------------------------------------+
```

**Import wizard (step 2 of 3, CSV mapping)**

```
+---------------------------------------------------------------------+
| Import — match the columns                              step 2 of 3 |
| File export.csv · 214 rows · parsed in a sandbox                    |
| Column in file     → Field            Sample                        |
| name               → [Title      v]   example.org                   |
| login_uri          → [URL        v]   https://example.org/login     |
| login_username     → [User name  v]   anna@example.org              |
| login_password     → [Password   v]   ••••••••                      |
| login_totp         → [TOTP       v]   otpauth://…                   |
| notes              → [Notes      v]                                 |
| Into vault [Personal v]  folder [Imported 12 Nov v]                 |
| 9 probable duplicates  [Review…]            [Back] [Cancel] [Next]  |
+---------------------------------------------------------------------+
```

**Split secret wizard (step 2 of 4)**

```
+---------------------------------------------------------------------+
| Split the recovery key for "Personal"                   step 2 of 4 |
| Shares to make [ 5 ]      Shares needed to recover [ 3 ]            |
| Any 3 of the 5 people can recover the vault together.               |
| Any 2 learn nothing.                                                |
| Give each share as (•) printed words  ( ) QR page  ( ) file         |
| Label each share:  [Mum] [Sam] [Lawyer] [Safe] [Bank box]           |
|                                         [Back] [Cancel] [Make shares]|
+---------------------------------------------------------------------+
```

**Shell-drawn fill picker** (data from SE04, drawn by SH04)

```
+----------------------------------------------------+
| Fill for example.org in Browser                    |
| [ search…                    ]                     |
| > anna@example.org        Personal                 |
|   anna.work@example.org   Work                     |
| [Enter] fill · [Ctrl+Enter] fill and submit · Esc  |
+----------------------------------------------------+
```

### 5.4 Menus

```
File      New Item ▸ Ctrl+N · New Vault… · Open Vault… Ctrl+O · Import… Ctrl+I · Export… Ctrl+E ·
          Print ▸ (Emergency Kit, Key Backup, List) Ctrl+P · Lock All Ctrl+Shift+Delete · Close Ctrl+W · Quit Ctrl+Q
Edit      Undo Ctrl+Z · Redo Ctrl+Shift+Z · Edit Item Ctrl+Return · Duplicate Ctrl+D · Move to ▸ ·
          Delete Delete · Delete for Ever Shift+Delete · Find Ctrl+F · Preferences Ctrl+,
Item      Copy User Name Ctrl+B · Copy Password Ctrl+Shift+C · Copy One-Time Code Ctrl+T ·
          Copy URL Ctrl+U · Open URL Ctrl+Shift+O · Reveal Ctrl+R · Large Type Ctrl+Shift+R ·
          Auto-Type Ctrl+Shift+V · Favourite Ctrl+* · Show History Ctrl+H · Share… · Show QR
Tools     Generator Ctrl+G · Health Check Ctrl+Shift+H · Merge Vault… · Change Passphrase… ·
          Rotate Vault Key… · Split Secret… · Recover from Shares… · Emergency Kit…
Keys      New SSH Key… · New OpenPGP Key… · New age Identity… · New Signing Key… ·
          New Certificate Request… · Import Key… · Agent ▸ (List Keys, Remove All, Lock Agent)
Tokens    Refresh F5 · Change PIN… · Manage Passkeys… · Enrol for Vault Unlock… · Reset…
View      Sidebar F9 · Detail Pane F4 · Sort ▸ · Group ▸ · Show Secrets in List (off) · Quick Access Ctrl+Shift+Space
Help      Passwords and Keys Help F1 · Vault Format · Keyboard Shortcuts · About
```

### 5.5 Keyboard and pointer

- App shortcuts beyond the reserved set: `Ctrl+B` copy user name, `Ctrl+Shift+C` copy password, `Ctrl+T` copy code, `Ctrl+G` generator, `Ctrl+R` reveal, `Ctrl+Shift+Delete` lock all, `Ctrl+Shift+V` auto-type. Global: `Ctrl+Shift+L` fill picker, `Ctrl+Shift+Space` quick access (both registered with the shell, rebindable).
- Context menu, item: Copy ▸ (each field), Open URL, Auto-type, Edit, Duplicate, Move to ▸, Add tag ▸, Favourite, Share…, Show history, Delete.
- Context menu, secret field: Copy, Reveal, Large type, Spell out, Show QR, Generate new, Show history.
- Context menu, key: Copy public key, Copy fingerprint, Show QR, Export…, Use policy…, Back up…, Delete.
- Context menu, app secret: Show details, Delete, Block this app from the keyring, Open app in Permissions.
- Drag and drop: drag an item to a folder, tag or vault; drop a `.kdbx`, `.csv`, key or certificate file on the window to import; dragging a secret out of the window is refused (copy with auto-clear is the only way); drag a public key out as a file.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | "No items yet": New login, Import…; Keys pages: "No keys" with the matching New button |
| Loading | Unlock shows a determinate bar for key derivation; large imports show row progress |
| Error | Wrong passphrase: inline message, exponential delay after 5 tries. Damaged vault: offer the `.bak` copy and the last snapshot |
| Offline | No effect; nothing here needs the network |
| Locked | Content replaced by the unlock panel; item titles and counts hidden; agent refuses requests |
| Read-only | Banner "Opened read-only"; edit, delete and autosave disabled |
| Conflict | After a file-level merge with conflicts: a badge on affected items and a merge dialog per item |
| Token missing | Unlock panel says which token is expected; alternatives listed if policy allows |
| Narrow | Sidebar overlay, list then detail; the mini quick-access window is the 480×320 form |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `treeView` |
| Item list | `modelListView` (virtualised, two-line delegate) |
| Detail | `scrollArea` with form rows; secret field pattern from the design system |
| Editor | `lineEdit`, `comboBox`, `dateTimeEdit`, `checkBox`, `codeEdit` (notes source), `markdownView` (notes view) |
| Generator | `dialog` or popover with `slider`, `spinBox`, `checkBox`, `radioButton` |
| Health | `listView` with group headers; `progressBar` for the score |
| Key and certificate details | `propertyGrid`, `treeView` for chains |
| Import mapping | `tableView` with `comboBox` cells |
| TOTP countdown | `progressBar` (ring style by CSS) |
| QR | `image` |
| Wizards | `dialog` with page stack |

Missing:

- **SE04-GAP-1**: secret text field with locked memory, no selection-to-primary, no drag-out, reveal-on-hold, and no exposure of its value through the a11y tree unless revealed (FUSE-WP-18 covers selection; the secure variant is new).
- **SE04-GAP-2**: QR encode and decode helper (shared with SE03-GAP-1).
- **SE04-GAP-3**: trusted fill path between the shell picker and a text field in another app (input protocol, with AX04 and SH04).
- **SE04-GAP-4**: ring-style countdown indicator (CSS conic gradient or a small custom widget).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, editing, generator options, import mapping UI. Holds decrypted fields only for the item being shown |
| UI | FUSE UI | app host | Windows, dialogs |
| Vault service | C++23 | per-user service inside `svc.keyring`; holds secrets (PLATFORM.md section 2 rule c) | Opens vault files, derives keys, encrypts and decrypts items, search index in memory, health checks, autofill matching, passkey authenticator, TOTP |
| Key service | C++23 | same service, separate process from the vault service | Private-key operations: SSH agent, OpenPGP, age, Ed25519 code signing, X.509; per-use policy; never exports a key without authentication |
| Keyring core | C++23 on FORGE `keystore` | `svc.keyring` | Per-app secret store; the store key wrapped by passphrase or TPM |
| Token helper | C++23 over `libfido2` (BSD-2-Clause, in the reference tree) and an own CCID layer | helper per connected token, capability mode, one USB handle from the portal | CTAP2, PIV and OpenPGP card commands |
| Import parser | C++23 | helper, capability mode, input handle only | KDBX, CSV, JSON, PKCS #12, key and certificate files → a neutral record stream |
| Breach set reader | C++23 | library in the vault service, read-only handle to the package file | Filter lookups |
| Generator | ForgeScript calling the service's CSPRNG | app host and service | Passwords and passphrases |
| Prompt bridge | ForgeScript schema | shell (SH04) | Fill picker, save prompt, passkey and key-use confirmations |
| `pwctl` | ForgeScript | command line | Requests through the same interfaces, confirmed on the trusted path |

```
 apps / browser ──fill or passkey request──▶ svc.keyring front door ──▶ vault service ──┐
      ▲                                              │                                  │ shell prompt
      └── secret typed into the field ◀── SH04 ◀─────┘ (picker, confirm)  ◀─────────────┘
 ssh, git, forge sign ──agent / sign request──▶ key service ──▶ token helper ──▶ USB token (portal)
                                                   │
 Passwords and Keys app ── org.pbsd.vault.v1 / org.pbsd.keys.v1 ──▶ vault service, key service
 (ForgeScript + FUSE UI)                             │
                    import parser (sandbox) ─────────┘      keyring core (FORGE keystore) ◀── all apps (own items)
```

### 6.2 Data model

**Vault file `.pbsdvault`** (open, documented, version 1):

| Part | Content |
|---|---|
| Header (plain, authenticated) | magic `PBSDVLT1`, `format_version` u16, `vault_id` UUID, `kdf` {`id`: argon2id, `mem_kib` u32 (default 262 144), `iters` u32 (default 3), `lanes` u8, `salt` 32 bytes}, `key_slots[]`, `cipher`: xchacha20-poly1305, `header_mac` |
| `key_slot` | `kind` enum(passphrase, keyfile, fido2_hmac, tpm, recipient_x25519, recipient_hybrid_mlkem768, share_set), `params`, `wrapped_vault_key` 48 bytes, `policy_group` u8 (slots in one group are all required; groups are alternatives) |
| Body | Append-friendly pages; each item is one record sealed separately with a key derived from the vault key and the `item_id` |
| Footer | Record index (encrypted), BLAKE3 over the body, counter for roll-back detection |

Items:

| Entity | Fields |
|---|---|
| `item` | `id` UUID, `type` enum, `title`, `folder_id`, `tags[]`, `favourite` bool, `created_ts`, `modified_ts`, `expires_ts` null, `fields[]`, `matches[]`, `fill_policy` enum(always_ask, ask_once, never_ask), `notes_md`, `attachments[]`, `version` u32, `tombstone` bool |
| `field` | `id`, `label`, `kind` enum(text, secret, url, email, date, totp, multiline, file_ref), `value`, `history[]` {`value`, `ts`} |
| `match` | `kind` enum(origin, domain, host, prefix, app), `value`, `signer_fpr` null |
| `totp` | `secret` bytes, `algo` enum(sha1, sha256, sha512), `digits` u8, `period` u16, `counter` u64 (HOTP), `issuer`, `account` |
| `passkey` | `credential_id` bytes, `rp_id`, `rp_name`, `user_handle` bytes, `user_name`, `alg` i32 (COSE), `private_key` bytes, `sign_count` u32, `created_ts`, `last_used_ts`, `discoverable` bool |
| `folder` | `id`, `parent_id`, `name` |
| `attachment` | `id`, `name`, `size`, `blake3`, `data` (sealed) |

Keys (in the key service's store, same file format, `type` = key):

| Entity | Fields |
|---|---|
| `ssh_key` | `id`, `algo`, `public_blob`, `private_blob` or `token_ref`, `comment`, `confirm` enum(never, per_host, per_session, always), `lifetime_s`, `allowed_apps[]`, `dest_constraints[]`, `offer_in_agent` bool |
| `pgp_cert` | `fingerprint`, `user_ids[]`, `subkeys[]` {`fpr`, `caps`, `algo`, `expires`, `secret` or `card_ref`}, `own` bool, `verified` {`ts`, `method`} null |
| `age_identity` | `id`, `recipient` text, `secret` sealed, `name` |
| `signing_key` | `id`, `algo` enum(ed25519, mldsa65), `public`, `seed` sealed, `publisher` text, `confirm` enum, `allowed_apps[]`, `successor_of` null, `revoked_ts` null |
| `x509` | `id`, `der`, `private_key` sealed null, `role` enum(client, smime, ca_local, other), `chain[]` |
| `trust_override` | `fingerprint`, `action` enum(add_root, distrust), `ts`, `by_uid`, `reason` |
| `known_host` | `host_hash`, `key_type`, `key_blob`, `first_seen_ts`, `verified` bool |
| `token` | `serial` or `aaguid`, `label`, `apps` set, `enrolled_for[]` |
| `use_log` | `ts`, `key_id` or `item_id`, `app_id`, `target` (host, origin, file hash), `outcome` enum(approved_user, approved_policy, denied, failed) |

Keyring items (per app): `app_id`, `label`, `kind`, `created_ts`, `last_read_ts`, `value` sealed. Stored by FORGE `keystore` as one sealed file per secret (Verified design, `FORGE:docs/os.md`).

Versioning: `format_version` in the header; a reader refuses a higher version; migration writes a new file and keeps the old as `.bak`.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `.pbsdvault` | yes | yes | Own; primitives from FORGE-WP-22 (XChaCha20-Poly1305, Argon2id, BLAKE3) | vault service |
| KDBX 3.1 / 4.x | yes | 4.x | Own parser (header, inner stream, XML body, gzip) | import parser |
| CSV, other managers' JSON | yes | CSV, JSON | Own | import parser |
| `otpauth:` URIs, migration QR payload | yes | yes | Own | import parser; vault service |
| TOTP / HOTP (RFC 6238 / 4226) | — | — | Own | vault service |
| WebAuthn / CTAP2 (platform authenticator side) | yes | yes | Own CBOR and COSE code | vault service |
| CTAP2 to hardware tokens | yes | yes | `libfido2` (BSD-2-Clause, `hbsd/src/contrib/libfido2`) | token helper |
| CCID, ISO 7816 APDUs, PIV, OpenPGP card | yes | yes | Own, small | token helper |
| SSH agent protocol | yes | yes | Own (message set is small); key formats as in OpenSSH (`hbsd/src/crypto/openssh` as reference) | key service |
| OpenSSH key files, PEM, PPK | yes | yes | Own | import parser (read); key service (write) |
| OpenPGP (RFC 9580 and RFC 4880 for old keys) | yes | yes | Proposed: own C++23 implementation of the needed profile. Fall-back if that slips: rPGP (Rust; licence MIT or Apache-2.0, Unverified from memory) as a Rust helper | key service; parsing of others' data in a separate parse helper |
| age v1 | yes | yes | Own (X25519, scrypt, ChaCha20-Poly1305, HKDF); small and fully specified | key service |
| FORGE signatures (Ed25519 over BLAKE3, `FORGE-SIGNATURE.json`) | yes | yes | FORGE signing code (Verified to exist, `FORGE:docs/security.md`) | key service |
| X.509, PKCS #10, PKCS #7, PKCS #12, PEM, DER | yes | yes (#10, #12, PEM, DER) | Own ASN.1 DER reader and writer; signature checks through the platform crypto | import parser (read); key service (write) |
| ML-KEM-768, ML-DSA-65 | — | — | From FORGE-WP-22 when present; otherwise deferred | key service |
| Shamir shares (GF(256), with check value) | yes | yes | Own | vault service |
| QR codes | yes | yes | Own (SE04-GAP-2) | import parser (decode); app host (encode) |
| Breach filter package | yes | no | Own filter reader | vault service |

### 6.4 Integration

- Intents: handles `open` and `view` for `.pbsdvault`, `.kdbx`, certificate and key files; `otpauth:` and `ssh-key:` hand-offs; `pick` (choose a key or certificate for another app, returns a reference, never the key). Emits `open` (URLs to IN01), `print`, `share` (encrypted share files only).
- MIME: `application/vnd.pbsd.vault`, `application/x-keepass2`, `application/x-x509-ca-cert`, `application/x-pem-file`, `application/x-pkcs12`, `application/pkcs10`, `application/pgp-keys`, `text/csv`.
- Services used: `svc.keyring` (it is this app's back end), `svc.clipboard` (sensitivity flag), `svc.notify`, `svc.settings`, `svc.search` (titles only, opt-in, off by default), `svc.audit`, `svc.backup`, `svc.portal.file`, `svc.portal.usb`, `svc.portal.camera`, `svc.portal.print`.
- Interfaces offered: `org.pbsd.vault.v1` (app-facing management), `org.pbsd.autofill.v1` (`RequestFill(field context)`, `OfferSave(context)`), `org.pbsd.webauthn.v1` (`MakeCredential`, `GetAssertion` for IN01 and apps), `org.pbsd.keys.v1` (`Sign`, `Verify`, `Encrypt`, `Decrypt`, `ListPublic`, `PickKey`), the SSH agent socket per app.
- Other apps: SH04 draws every prompt; SH03 unlocks the session keyring; IN01 attests origins and calls autofill and WebAuthn; SY03 and DV01 get agent sockets; DV03 signs commits by key reference; CO01 uses OpenPGP and S/MIME; CO03 links verified keys to contacts; SE05 takes keys by reference for file encryption and signing; SY05 and DV01 call code signing; SE01 reads health counts and trust-store overrides; AD05 shows which apps may use the keyring and agent; AX04 carries the trusted input path.

## 7. Security and privacy

Capabilities requested by the app:

| Capability | Reason |
|---|---|
| `fs.app` | Window state, non-secret preferences |
| `fs.portal` | Open vault files, import, export |
| `net.none` | No network at all. Breach checks are local |
| `svc.keyring` | The app's back end; management scope is granted only to this bundle by signer |
| `svc.clipboard` | Copy with sensitivity flag and auto-clear |
| `svc.notify`, `svc.settings`, `svc.audit`, `svc.backup` | Reminders, preferences, use log, backup rules |
| `svc.search` | Optional index of item titles (off by default) |
| `svc.portal.file`, `svc.portal.print`, `svc.portal.usb`, `svc.portal.camera` | Files, paper backups, tokens, QR scanning |
| `dev.usb:hid`, `dev.usb:ccid` | FIDO2 tokens and smart cards, through the helper |
| `dev.camera` | QR codes; prompt on first use |
| `proc.helper:import-parser`, `proc.helper:token-helper` | Hostile-input parsing; token access |
| `bg.run` | Not requested: the services run in the background, the app does not |

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Vault contents at rest | Thief with the disk or the vault file | Offline guessing | Argon2id with high memory cost; optional second factors; per-item sealing; no plaintext metadata beyond the header |
| Vault contents in memory | Malware with user rights | Read process memory, core dumps, swap | Secrets live in the vault service only; locked pages; dumps disabled; other apps cannot attach (capability mode, no `proc.exec`); auto-lock |
| A password during fill | A malicious or look-alike app or site | Ask for another site's login | Match by signer and attested origin; no match, no suggestion; picker on the trusted path; typed confirmation for a mismatch |
| The clipboard | Any app that may read it | Scrape copied secrets | Sensitivity flag, 30 s clear, excluded from history; fill is preferred over copy |
| Private keys | Any app with an agent socket | Silent signing | Per-use confirmation on the trusted path with app and target shown; per-app key lists; constraints |
| Code-signing key | Compromised build tool | Sign a malicious bundle | The prompt shows file name and BLAKE3; policy "ask every time"; hardware-backed option; use log |
| The vault service | Hostile import file or certificate | Parser exploit | All foreign formats parsed in the import parser, which has only an input handle and returns a neutral record stream |
| Tokens | Malicious app | Talk to the token directly | USB only through the portal and the token helper; class-limited |
| Trust store | Malware or a coerced user | Add a rogue root | Needs authentication; shown in SE01; shipped roots are on the verified dataset |
| Passkeys | Phishing | Use a credential on the wrong origin | Relying-party ID bound to the attested origin; user presence each time |
| Recovery | Loss of passphrase or token | Lock-out | Shares, emergency kit, second token; rehearsal flow |
| Roll-back | Attacker replacing the vault with an old copy | Undo a password change or a revocation | Monotonic counter in the footer, last-seen counter kept in the keyring; a lower counter warns |
| Keyring store key | Same-user code | Read an unwrapped store key (stated FORGE weakness) | Policy: the store key must be wrapped by passphrase or TPM; "neither" is a finding |

Stored: vault files where the user puts them; key store and keyring under the user's data directory; use logs local. Leaves the machine: nothing. Optional key discovery and revocation checks are per request, off by default, and need a `net.host` grant that is not in the default manifest.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `lock.idle_minutes` | int | 10 | Auto-lock after idle |
| `lock.on_session_lock` | bool | true | Lock vaults when the session locks |
| `lock.on_suspend` | bool | true | Lock on suspend |
| `lock.on_token_removed` | bool | true | Lock when the unlock token is removed |
| `unlock.quick` | enum | off | off, pin, token_touch |
| `unlock.failed_delay` | bool | true | Growing delay after failed tries |
| `clipboard.clear_seconds` | int | 30 | Clear copied secrets |
| `reveal.hold_only` | bool | true | Reveal only while held |
| `kdf.mem_kib` | int | 262144 | Argon2id memory for new vaults |
| `kdf.iterations` | int | 3 | Argon2id passes |
| `generator.mode` | enum | password | password, passphrase, pin, pronounceable |
| `generator.length` | int | 20 | Default length |
| `generator.words` | int | 6 | Default passphrase words |
| `generator.wordlist` | text | system language | Word list |
| `fill.shortcut` | keys | `Ctrl+Shift+L` | Global fill shortcut |
| `fill.default_policy` | enum | ask_once | Default fill policy for new entries |
| `fill.offer_save` | bool | true | Offer to save new logins |
| `fill.match_default` | enum | domain | Default URL match rule |
| `totp.copy_after_fill` | bool | true | Copy the code after a password fill |
| `health.old_months` | int | 24 | Age that counts as old |
| `health.breach_check` | bool | true | Use the offline breach set when installed |
| `trash.retention_days` | int | 30 | Trash retention |
| `agent.enabled` | bool | true | SSH agent |
| `agent.default_confirm` | enum | per_host | Default confirmation for new SSH keys |
| `agent.forwarding` | bool | false | Allow agent forwarding |
| `signing.confirm` | enum | always | Code-signing confirmation |
| `pgp.default_expiry_months` | int | 24 | Expiry for new keys |
| `pq.hybrid_sharing` | bool | true | Use the hybrid KEM for new shares when available |
| `search.index_titles` | bool | false | Let system search see item titles |
| `history.use_log_days` | int | 365 | Use-log retention |
| `backup.keep_bak` | bool | true | Keep the previous vault file |

## 9. Accessibility and languages

- Secret fields expose "hidden, 20 characters" to assistive tools until revealed; revealing is an explicit action and is announced. A spell-out mode reads characters with phonetic words and marks case and symbols.
- TOTP codes are read in groups of three with the seconds left; the countdown does not rely on the ring alone.
- Fingerprints are offered as word lists for comparison by voice.
- The shell picker is fully keyboard-driven and announces the requesting app and origin first.
- Passphrase word lists exist per language; generated passphrases respect the keyboard layout hint (a warning if the passphrase holds characters the login screen layout cannot type).
- Paper kits and share sheets use a large, monospace, unambiguous face and include a check value per line.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| First frame | under 300 ms cold (locked view) |
| Unlock | set by the KDF: about 1 s on the reference machine at the default cost; progress shown |
| Search | under 20 ms over 20 000 items |
| Fill picker | on screen within 100 ms of the shortcut when unlocked |
| Agent signature (software key, no prompt) | under 5 ms |
| Save | under 50 ms for one item in a 20 000-item vault (append and index update) |
| Import | 10 000 CSV rows in under 3 s; KDBX with 5 000 entries in under 5 s after its own KDF |
| Health check | 10 000 items in under 2 s, breach filter lookups included |
| Memory | app under 120 MiB; vault service under 80 MiB plus the KDF's working memory during unlock |
| Largest vault | 100 000 items, 2 GiB of attachments |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: every primitive against published vectors (Argon2id, XChaCha20-Poly1305, BLAKE3, Ed25519, X25519, TOTP and HOTP vectors from the RFCs, Shamir round trips, ML-KEM and ML-DSA vectors when present).
- **Format round trip**: `.pbsdvault` write, read, merge; KDBX files made by other tools read and compared field by field; KDBX written here opened by a reference tool on a test machine.
- **Differential**: SSH agent against the OpenSSH client and server in the reference tree; OpenPGP messages and keys against an independent implementation's test suite; age against its published test vectors; X.509 chains against a public path-validation suite.
- **Golden**: `.fuseui` renders for all pages, locked and unlocked.
- **Scripted UI**: the 15 top tasks through the a11y tree, including the shell prompts.
- **Fuzz**: KDBX, CSV, JSON, PKCS #12, DER, PEM, OpenPGP packets, OpenSSH key files, CBOR, `otpauth:` URIs, QR decode, vault file reader, agent messages.
- **Security tests**: a test app tries to read the fill candidates, to attach to the vault service, to use an agent socket it was not given, to fill across origins; all must fail and be audited. Memory scan after lock finds no secret patterns.
- **Crash tests**: power cut at each step of a save leaves the old or the new vault, never a mix.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Fill matcher | A secret is offered only when the requester's signer and origin match the entry's rules | T1 | Pure function; Lean model and ESBMC | Origin attestation by the browser is trusted |
| Key-slot policy | The vault key is released only when one full policy group is satisfied | T1 | Small state machine; PRISM | none known |
| Shamir split and combine | Any K shares recover; fewer reveal nothing (information-theoretic, by construction); round trip | T1 for round trip and bounds | Lean proof over GF(256) arithmetic; ESBMC on the code | Side channels not covered |
| Vault file reader and writer | No runtime errors; atomic replace; counter never decreases | T2 | Contracts, bounded proof, crash-point tests | File system semantics assumed |
| TOTP, HOTP | Match the RFC algorithm | T2 | Contracts plus RFC vectors | none known |
| Agent policy | No signature without the key's policy being met | T1 | State-machine model; ESBMC on the dispatch | Prompt path trusted (SH04) |
| Import parser | No runtime errors on any input; output only through the record stream | T2 | Safe subset, contracts, fuzzing | Format breadth |
| Cryptographic primitives | Functional correctness against vectors; constant-time behaviour | T3 (T2 for memory safety) | Vectors, timing tests | Full functional proofs of primitives are out of reach here |
| OpenPGP implementation | Interoperability; memory safety | T3 | Differential tests, fuzzing | Size and legacy formats |
| Token helper | Memory safety at the USB boundary | T3 | Sanitizers, fuzzing with a device simulator | `libfido2` is C, outside the safe subset |
| App logic | Safe subset | T3 | ForgeScript checker, unit tests | Interpreter not verified |

## 12. Build plan

**Phase 1 — usable core**

| ID | Work item | Depends on |
|---|---|---|
| SE04-W01 | `svc.keyring` core on FORGE `keystore`, with the store key always wrapped; per-app scoping; management scope for this bundle | G-09, FORGE-WP-22, FORGE-WP-24, FORGE-WP-33 |
| SE04-W02 | `.pbsdvault` format specification and reference reader and writer | FORGE-WP-22 |
| SE04-W03 | Vault service: unlock factors (passphrase, key file), items, history, trash, search, auto-lock | SE04-W02, FORGE-WP-27, FORGE-WP-28 |
| SE04-W04 | App: three-pane window, editor, generator, locked state, quick access | G-01, G-02, G-07, FUSE-WP-12, FUSE-WP-13, SE04-GAP-1 |
| SE04-W05 | Clipboard path with sensitivity flag | `svc.clipboard` (G-09), AX05 |
| SE04-W06 | Autofill interface and shell picker; fill audit | SH04, AX04, SE04-GAP-3 |
| SE04-W07 | TOTP | SE04-W03 |
| SE04-W08 | Import parser: KDBX and CSV; import wizard; encrypted export | SE04-W03 |
| SE04-W09 | Key service: SSH keys, agent sockets, per-use confirmation | SE04-W01, SH04, SY03 |
| SE04-W10 | Certificate viewer and system trust store view | FORGE-WP-21.5 |
| SE04-W11 | App secrets page: list, delete, block | SE04-W01, AD05 |

**Phase 2 — full feature**

| ID | Work item | Depends on |
|---|---|---|
| SE04-W12 | Health: strength, reuse, age, offline breach filter package and reader | SE04-W03, SY05 (package) |
| SE04-W13 | Autosave, TOTP fill, auto-type | SE04-W06 |
| SE04-W14 | Passkey platform authenticator and `org.pbsd.webauthn.v1` | SE04-W03, IN01 |
| SE04-W15 | Token helper: FIDO2 management, token unlock factor, hardware passkeys, `sk` SSH keys | `svc.portal.usb`, VM05, `libfido2` build on PBSD |
| SE04-W16 | OpenPGP: key ring, generation, maintenance, crypto operations service | SE04-W09, decision on own code or helper |
| SE04-W17 | age identities and operations | SE04-W09 |
| SE04-W18 | Code-signing keys, publisher identity, signing policy, hand-off with `forge sign` | FORGE-WP-33, FORGE-WP-09 |
| SE04-W19 | Certificates: client certificates, requests, PKCS #12 import, S/MIME, trust overrides | SE04-W10 |
| SE04-W20 | Secret splitting, recovery, emergency kit, paper backups | SE04-W03, G-16 |
| SE04-W21 | File-level merge and conflict view; vault history; key rotation | SE04-W03 |
| SE04-W22 | Share item as file; other managers' JSON imports; KDBX export; `pwctl` | SE04-W16, SE04-W17 |
| SE04-W23 | Known hosts view, key constraints, use log page | SE04-W09 |

**Phase 3 — advanced**

| ID | Work item | Depends on |
|---|---|---|
| SE04-W24 | PIV and OpenPGP card management | SE04-W15 |
| SE04-W25 | Shared vaults with member management | SE04-W22 |
| SE04-W26 | Hybrid KEM sharing; ML-DSA signing keys and dual signatures; algorithm inventory | FORGE-WP-22 (post-quantum primitives), FORGE verifier support |
| SE04-W27 | Passkey transfer format; SSH certificates and local CA; local X.509 CA | SE04-W14, SE04-W19 |
| SE04-W28 | Key discovery and revocation checks on request | SE03 routes, a `net.host` grant |
| SE04-W29 | Travel mode; terminal fill; cards and identities fill | SE04-W13, SY03 |
| SE04-W30 | Rotation and revocation records for signing keys; multiple signatures | SE04-W18 |

## 13. Open questions and risks

1. **Trusted fill path (SE04-GAP-3)** needs a protocol between the shell, the input method layer and the target text field. Without it, fill falls back to clipboard or auto-type, which are weaker. Shared work with SH04 and AX04; likely a new platform gap.
2. **Secure text field (SE04-GAP-1)** does not exist in FUSE. Until it does, secrets shown in the app pass through ordinary widget memory.
3. OpenPGP is large. Own implementation is the cleanest fit for the safe subset but is months of work; a Rust helper is faster but adds a toolchain (same concern as G-04). Decision needed before SE04-W16.
4. `libfido2` is C and depends on a CBOR library and a crypto library; its build inside the PBSD tree and the HID access path on PBSD are Unverified beyond the directory being present.
5. Breach set size: a full set is several gibibytes; a filter with a 1-in-10-million false-positive rate is on the order of 3 GiB for 850 million entries at about 28 bits each. It must be an optional package. Who builds and signs it is undecided.
6. Offline emergency access cannot do a timed release. The design uses shares and sealed packages; users coming from server-based managers may expect more.
7. Passkey portability: the credential-exchange format is still moving; export is Phase 3 and may change.
8. Post-quantum primitives depend on FORGE-WP-22 delivering ML-KEM and ML-DSA; FORGE's signature verifier is Ed25519 only today (Verified), so dual signatures need a FORGE change.
9. TPM-sealed slots lose the key if the TPM is cleared (FORGE states the same for its keystore). The UI must force a second slot or a recovery path at enrolment.
10. Origin attestation relies on IN01 being honest about the origin. A compromised browser can lie; the picker shows the claimed origin so the user can still catch it.
11. The management scope of `svc.keyring` (seeing every app's item labels) is powerful. It is bound to this bundle's signer; the binding mechanism needs definition in the manifest schema.
