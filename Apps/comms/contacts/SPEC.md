<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Contacts

By Odin Loch

| | |
|---|---|
| ID | `CO03` (from CATALOGUE.md) |
| Bundle | `org.pbsd.contacts` |
| Category | comms |
| Priority | P1 |
| Backing repos | FUSE (widgets), FORGE (ForgeScript, Unicode services in `forge::text`). No contacts code exists in any repo read. |
| Status | Planned. Nothing built. |

## 1. Purpose

Contacts is the address book for the whole desktop. It stores people and organisations locally, syncs with CardDAV servers, keeps each person's keys and verification state, and gives other apps a picker so they never read the whole address book. It replaces KAddressBook from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Local address books and CardDAV address books (RFC 6352).
- vCard 3.0 (RFC 2426) and 4.0 (RFC 6350) with loss-free round trip; vCard 2.1 read.
- Groups, tags, favourites, photos, custom fields, related people.
- Duplicate detection and merge, with undo.
- Per-contact keys: OpenPGP, S/MIME, age, SSH; messenger identities with their verification state.
- Import and export: vCard, CSV, LDIF; QR sharing.
- Services for other apps: contact picker, address completion, key lookup, dates (birthdays), trust state, interaction history.
- Optional, local-only history of interactions.

Out of scope:

| Item | Owner |
|---|---|
| Private keys, key generation, certificate chains, key servers | SE04 Passwords and Keys (Contacts holds public keys and references) |
| Sending mail, chat, calls | CO01 Mail, CO05 Messenger (Contacts emits intents) |
| Showing birthdays on a calendar, reminders | CO02 Calendar (Contacts supplies the dates) |
| Device verification ceremonies (emoji, QR for Matrix; OMEMO fingerprints) | CO05 Messenger (Contacts shows and stores the result) |
| Scanning a QR code with the camera | ME04 Camera (returns the payload by `scan` intent) |
| Maps of addresses | IN04 Maps |
| LDAP directory browsing, Exchange GAL, vendor contact APIs | Not planned (LDIF files only) |
| Social-network lookups, enrichment services, Gravatar | Not planned; no network enrichment of any kind |

## 3. Users and top tasks

Personas:

- **Rae, daily user.** 600 contacts on a CardDAV server shared with a phone.
- **Sol, journalist.** A local encrypted "sources" address book; verifies keys in person.
- **Tam, migrating user.** Arrives with CSV exports and 4 000 contacts full of duplicates.
- **Any other app.** Needs one address, with consent, without seeing the rest.

| # | Task | Target steps |
|---|---|---|
| T1 | Find a person | 1 (type in the search field, which has focus at start) |
| T2 | Start a mail, chat or call from a contact | 1 click or one key (`M`, `C`) |
| T3 | Add a contact | 3 (`Ctrl+N`, type, `Ctrl+S`) |
| T4 | Add a contact from a mail or chat | 1 in the other app, 1 confirm here |
| T5 | Edit one field | 2 (`E`, type; saves on leave) |
| T6 | Merge two duplicates | 2 (select both, Merge; review only on conflict) |
| T7 | Clean all duplicates | 3 (Tools > Find duplicates, review, Merge all safe) |
| T8 | Import a vCard or CSV file | 3 |
| T9 | Share a contact by QR | 2 |
| T10 | Mark a person's key as verified | 3 (open keys, compare fingerprint, Verify) |
| T11 | Pick a recipient inside Mail | type 2–3 letters, Enter (picker service) |
| T12 | Add a CardDAV account | 3 |
| T13 | Put ten people in a group | 2 (select, drag onto the group) |
| T14 | Undo a wrong merge or delete | 1 (`Ctrl+Z`), or restore from history later |

## 4. Features

### 4.1 Browse and view

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F001 | Three-pane layout | Sidebar (books, groups, tags), contact list, contact card. | M | 1 |
| CO03-F002 | Contact list | Virtualised; avatar, display name, secondary line (organisation or first e-mail), trust and favourite icons; section headers by initial; index strip for jumping. | M | 1 |
| CO03-F003 | Sort and display order | "Given Family" or "Family, Given"; sort by given, family, organisation, recently used, recently changed. Locale collation; `SORT-AS` honoured. | M | 1 |
| CO03-F004 | Contact card | Read view of all fields grouped: communication, addresses, dates, organisation, keys and identities, notes, groups, history. Each value has actions. | M | 1 |
| CO03-F005 | Value actions | E-mail: compose (CO01). Phone: call or message (CO05 by `tel:`/`sip:`/`xmpp:` intent), copy. Address: show on map (IN04), copy formatted. URL: open (asks per host). Date: show in calendar (CO02). | M | 1 |
| CO03-F006 | All contacts and per-book views | "All contacts" merges linked entries across books into one row. | M | 1 |
| CO03-F007 | Favourites | Star; a Favourites section; favourites rank first in pickers. | S | 1 |
| CO03-F008 | Recently added, recently used | Smart sections. "Recently used" is fed only by the optional history. | S | 2 |
| CO03-F009 | Kinds | Individual, organisation, group, location (vCard `KIND`); icon per kind. | M | 1 |
| CO03-F010 | Avatars | Photo, or initials on a colour derived from the UID; never fetched from the network. | M | 1 |
| CO03-F011 | Grid view | Large avatar tiles as an alternative to the list. | C | 3 |
| CO03-F012 | Linked contacts view | A person present in several books shows as one card with a "Sources" strip; edit targets are per source. | S | 2 |
| CO03-F013 | Card density | Hide empty groups; compact and normal density. | S | 1 |
| CO03-F014 | Multiple selection summary | With several selected, the card area shows count and bulk actions. | M | 1 |
| CO03-F015 | Print | Address list, cards, envelope and label sheets (common label stock sizes as presets) to `svc.portal.print`. | C | 3 |

### 4.2 Create and edit

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F016 | Inline editor | The card becomes editable in place; fields save on leave; `Esc` reverts the field; whole-edit undo. | M | 1 |
| CO03-F017 | Name fields | Prefix, given, additional, family, suffix, nickname, phonetic given and family, formatted name (auto or manual). | M | 1 |
| CO03-F018 | Communication fields | E-mail, phone, messaging (`IMPP`: matrix, xmpp, irc, sip), URL; each with type labels (home, work, mobile, custom) and a preferred mark (`PREF`). | M | 1 |
| CO03-F019 | Phone handling | Stored as typed; a normalised E.164 form is computed for matching using a bundled offline numbering table and the user's default region. | S | 2 |
| CO03-F020 | Postal addresses | Structured fields laid out per country from bundled address-format tables; free-form label kept; `GEO` optional. | M | 1 |
| CO03-F021 | Dates | Birthday (year optional, `--MMDD`), anniversary, death date, custom dated events with labels. | M | 1 |
| CO03-F022 | Organisation | Organisation, units, title, role, manager and assistant as related contacts. | M | 1 |
| CO03-F023 | Related people | `RELATED` with type (spouse, child, parent, friend, colleague, custom), linking to another contact or free text. | S | 2 |
| CO03-F024 | Notes | Plain text or Markdown shown by `markdownView`; never executed, links confirmed. | M | 1 |
| CO03-F025 | Custom fields | Any label with a typed value (text, number, date, URL, boolean); stored as grouped `X-ABLabel` items and as `X-PBSD-FIELD` for typed values. | M | 2 |
| CO03-F026 | Photo | From a file (portal), paste, drag, or camera (hand-off to ME04 by `pick` intent; Contacts has no camera capability). Crop to square, scale to at most 512×512, strip metadata, store as JPEG or PNG. | M | 1 |
| CO03-F027 | Pronouns, languages, time zone, gender | vCard 4 `LANG`, `TZ`, `GENDER`, and RFC 9554 `PRONOUNS`. Time zone is offered to CO02 scheduling. | S | 2 |
| CO03-F028 | Free/busy and calendar URLs | `FBURL`, `CALURI`, `CALADRURI`, used by CO02 on request. | S | 2 |
| CO03-F029 | New contact from data | From an address chip in CO01, a chat member in CO05, a pasted block of text (signature parser: deterministic patterns for e-mail, phone, URL; optional `svc.model` pass), a `.vcf`, or a QR payload. Always shown for review before saving. | M | 2 |
| CO03-F030 | Templates per kind | Field sets shown by default for person, organisation, minimal. | S | 2 |
| CO03-F031 | Field validation | E-mail syntax, URL scheme allow-list, date sanity; warnings only, never blocks saving odd but legal data. | M | 1 |
| CO03-F032 | Delete | To trash with Undo; trash kept 30 days; delete for ever with confirmation. | M | 1 |
| CO03-F033 | Change history | Every edit is a versioned row (who: this device or sync); card shows history with restore per version. | S | 2 |
| CO03-F034 | Read-only fields | Fields from a read-only book show a lock; "Copy to my book" makes an editable linked copy. | S | 2 |
| CO03-F035 | Me card | One contact marked as the user; supplies identity defaults to CO01, CO02 and the share sheet. | S | 2 |

### 4.3 Organise

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F036 | Groups | Named groups; a contact can be in many. Stored as vCard 4 `KIND:group` with `MEMBER`, or as `CATEGORIES`, or Apple-style `X-ADDRESSBOOKSERVER-*`, chosen per server by probing. | M | 1 |
| CO03-F037 | Nested groups | Group as member of a group; expansion with cycle guard. | C | 3 |
| CO03-F038 | Tags | Free tags (`CATEGORIES`) separate from groups when the server keeps both; colour per tag. | S | 2 |
| CO03-F039 | Smart groups | Saved queries: "no e-mail", "birthday this month", "has verified key", "changed in 30 days", "organisation is X". | S | 2 |
| CO03-F040 | Bulk edit | Add or remove group, tag, organisation; move or copy to another book; delete; export; for many selected. | M | 2 |
| CO03-F041 | Move and copy between books | Move re-homes the UID; copy makes a linked entry. | M | 2 |
| CO03-F042 | Group actions | Mail the group (To or Bcc choice), start a room in CO05, export, print. | S | 2 |
| CO03-F043 | Distribution list export | Group as a list of `Name <address>` or as a vCard group. | S | 2 |
| CO03-F044 | Block list | A "Blocked" flag used by CO01 rules and CO05; local only. | C | 3 |

### 4.4 Search

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F045 | Instant search | Prefix and substring over names, nicknames, phonetic names, organisation, e-mail, phone digits, notes, tags; diacritic- and case-insensitive; under 16 ms per key for 100 000 contacts. | M | 1 |
| CO03-F046 | Phone number search | Digits match regardless of spacing, prefix or country code. | M | 1 |
| CO03-F047 | Field operators | `email:`, `tel:`, `org:`, `tag:`, `group:`, `book:`, `has:key`, `has:photo`, `is:verified`, `is:favourite`, `bday:11`. | S | 2 |
| CO03-F048 | Transliteration match | Latin input matches names in other scripts through `forge::text` transliteration where available (Unverified coverage). | C | 3 |
| CO03-F049 | System search | Names and organisations to `svc.search` (default on for unencrypted books); opening a result opens the card. | S | 2 |
| CO03-F050 | Type-ahead in list | Typing in the list jumps to the name. | M | 1 |

### 4.5 Duplicates and merge

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F051 | Duplicate detection | Candidate pairs by blocking keys (normalised e-mail, E.164 phone, name key), scored by: same e-mail (0.6), same phone (0.5), name similarity by Jaro-Winkler on folded names (up to 0.4), same organisation (0.1), conflicting birthdays (−0.5). Thresholds: ≥0.9 "safe", 0.6–0.9 "review". | M | 2 |
| CO03-F052 | Duplicate review | Pairs or clusters side by side; per field choose left, right or both; "Merge all safe" for exact matches. | M | 2 |
| CO03-F053 | Merge rules | Multi-value fields are unioned and de-duplicated after normalisation; single-value conflicts ask; photo keeps the larger; notes are concatenated with a separator; keys and verification state are never dropped; the lower trust state does not overwrite a higher one, and a verified state survives only for the exact identity it was given to. | M | 2 |
| CO03-F054 | Merge undo | Merge is one undo step; the sources are kept in history for 30 days and can be split again. | M | 2 |
| CO03-F055 | Not a duplicate | Mark a pair as distinct; it is not offered again. | S | 2 |
| CO03-F056 | Link instead of merge | Across books, link the entries (shown as one) without changing either server copy. | S | 2 |
| CO03-F057 | Duplicate hint on create | While typing a new contact, a hint offers the existing match. | S | 2 |
| CO03-F058 | Import-time de-duplication | Import preview marks new, identical, and updates (same UID or safe match) with a per-row choice. | M | 2 |
| CO03-F059 | Clean-up tools | Find contacts with no name, no reachable field, invalid e-mail, phone without country code; fix formats in bulk with preview. | C | 3 |

### 4.6 Keys, identities and trust

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F060 | Keys per contact | Public OpenPGP keys, S/MIME certificates, age recipients, SSH public keys; stored as `KEY` properties (inline or as a fingerprint reference to SE04's public keyring). | M | 2 |
| CO03-F061 | Key card | Per key: type, fingerprint (grouped monospace), user IDs, created, expires, algorithm, state (valid, expired, revoked, weak). Parsed by the SE04 engine, not here. | M | 2 |
| CO03-F062 | Trust states | One shared scale: `unknown`, `seen` (trust on first use), `introduced` (signed by a contact the user verified), `verified` (compared by the user in person or by QR), `changed` (a new key replaced a verified one: alert), `revoked`. Icon plus word everywhere. | M | 2 |
| CO03-F063 | Verify by fingerprint | Side-by-side fingerprint with a spoken-word form (word list) and a QR code; "Verify" records who, when and how. | M | 2 |
| CO03-F064 | Messenger identities | Matrix user ID, XMPP JID, IRC nick and network, SIP URI, each with a verification state written by CO05 through the trust interface (cross-signing verified, OMEMO fingerprints trusted, none). | M | 2 |
| CO03-F065 | Trust interface | Service `org.pbsd.contacts.trust`: CO01, CO05 and SE04 read and write trust records for an identity. Each write is attributed to the calling app and logged to `svc.audit`. Only apps holding `svc.contacts.trust` may write. | M | 2 |
| CO03-F066 | Key change alert | When an app reports a different key for a verified identity, the contact shows a `--danger` banner until the user resolves it; the old key is kept. | M | 2 |
| CO03-F067 | Key import and export | Import a key file onto a contact (parsed in SE04); export public keys; include or omit keys in vCard export. | S | 2 |
| CO03-F068 | Encryption preferences | Per contact: mail encryption policy (always, when possible, never), preferred key, preferred protocol; read by CO01. | S | 2 |
| CO03-F069 | Key discovery hand-off | "Look for a key" opens SE04's lookup (WKD, key server) with the address; Contacts makes no network request itself. | S | 3 |
| CO03-F070 | Identity proof list | A plain list of the contact's claimed identities with which ones are verified, for the user's review; no automatic checking of remote proofs. | C | 3 |

### 4.7 Accounts and sync

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F071 | Local address books | Any number; name, colour. | M | 1 |
| CO03-F072 | CardDAV accounts | Discovery by RFC 6764 (`.well-known/carddav`, DNS SRV), `addressbook-home-set`. One `svc.accounts` record shared with CO02 and CO04. Auth as in CO02. | M | 1 |
| CO03-F073 | Efficient sync | `sync-collection`, else CTag and ETag diff with `addressbook-multiget`; `If-Match` on write. | M | 1 |
| CO03-F074 | Conflict resolution | Three-way merge by property against the last synced copy; a sheet only when the same property changed on both sides. | M | 2 |
| CO03-F075 | vCard version negotiation | Write the version the server accepts (`supported-address-data`); keep vCard 4-only data in `X-` properties on vCard 3 servers so nothing is lost. | M | 1 |
| CO03-F076 | Offline edits | Journal and replay. | M | 1 |
| CO03-F077 | Per-account route | `clear`, `vpn`, `tor`; fail-closed. | S | 2 |
| CO03-F078 | Encrypted address books | Local book with a store key in `svc.keyring`; locked books vanish from the list, the pickers and search. | S | 2 |
| CO03-F079 | Read-only and shared books | ACL state shown; write actions disabled with a reason. | S | 2 |
| CO03-F080 | Server-side search | For very large shared books kept un-synced: `addressbook-query` on demand, results marked "from server". | C | 3 |
| CO03-F081 | Photo size policy | Per server limit on photo bytes; scale down before upload. | S | 2 |
| CO03-F082 | Sync log and status | Per book last sync, pending count, errors. | S | 2 |

### 4.8 Import, export, sharing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F083 | Import vCard | 2.1, 3.0, 4.0; multi-card files; charset and quoted-printable handling; lenient with a repair list. | M | 1 |
| CO03-F084 | Import CSV | Column mapping with auto-detect for common exports (Google, Outlook, Thunderbird), delimiter and encoding detection, preview, saved mappings. | M | 2 |
| CO03-F085 | Import LDIF | `inetOrgPerson` and Mozilla address-book schema. | S | 2 |
| CO03-F086 | Import from other sources | jCard (RFC 7095), xCard (RFC 6351), contacts handed over by CO01's PST import. | C | 3 |
| CO03-F087 | Export | vCard 3.0 or 4.0 (one file or one per contact), CSV (chosen columns), LDIF, jCard. Options: without photos, without notes, without keys, without private fields. | M | 1 |
| CO03-F088 | Share a contact | Share sheet (`svc.intent`): `.vcf` attachment to CO01 or CO05, file, QR. A field picker chooses what to include; default is name, e-mail, phone only. | M | 2 |
| CO03-F089 | QR sharing | Show a QR of a minimal vCard (or MeCard). Byte count and QR version shown; photos never included. Full-screen high-contrast mode for scanning. | S | 2 |
| CO03-F090 | QR import | Accept a payload from ME04's `scan` intent or from an image file decoded in the QR helper. | S | 2 |
| CO03-F091 | Drag out | Drag a contact to Files, Mail or Messenger as `text/vcard` plus plain text. | M | 1 |
| CO03-F092 | Backup | Store in `svc.backup`; one-click full export on a schedule to a chosen folder (portal grant remembered). | S | 3 |

### 4.9 Services for other apps

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F093 | Contact picker | Service `org.pbsd.contacts.picker`. The calling app asks for N contacts and named fields (for example "one e-mail"). A picker window drawn by Contacts appears; the app receives only the chosen values. Capability `svc.contacts.pick`; no prompt needed because the user chooses. | M | 1 |
| CO03-F094 | Address completion | `org.pbsd.contacts.complete`: an app sends a typed prefix (minimum 2 characters) and a field kind and gets at most 8 matches (name, value, trust icon). Capability `svc.contacts.complete`, granted by prompt per app; rate-limited to stop harvesting (60 distinct prefixes per minute, no empty prefix, audit entry when the limit is hit). | M | 1 |
| CO03-F095 | Reverse lookup | `org.pbsd.contacts.lookup`: an app sends an address or identity it already has and gets display name, avatar and trust state. Used by CO01, CO05 and notifications. | M | 2 |
| CO03-F096 | Dates feed | `org.pbsd.contacts.dates`: birthdays and anniversaries for a range, for CO02; names only, no other fields. | S | 2 |
| CO03-F097 | Key lookup | `org.pbsd.contacts.keys`: public keys and encryption preferences for an address, for CO01 and SE05. | M | 2 |
| CO03-F098 | Add-contact request | `new` intent with fields; always shows the review form; never saves silently. | M | 1 |
| CO03-F099 | Per-app visibility | In settings and in AD05: which books each app's completion and lookup may see. Encrypted books are excluded unless listed. | S | 2 |

### 4.10 Interaction history

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F100 | Local history | Off by default. When on, CO01, CO05 and CO02 may record events: kind (mail in/out, chat, call, meeting), time, a link (`mid:`, room link, event link). No content, no subject unless `history.subjects` is on. | S | 3 |
| CO03-F101 | History on the card | Timeline of the last interactions with open-in-app links; "last contacted" line. | S | 3 |
| CO03-F102 | Retention and erase | Keep N days (default 365); erase per contact or all; history is excluded from sync and from vCard export always. | S | 3 |
| CO03-F103 | Stay-in-touch list | Optional smart group "not contacted in N months" for chosen contacts. | C | 3 |

### 4.11 Settings, accessibility, recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| CO03-F104 | Command palette | All actions, every contact by name, groups, "mail <name>", "copy phone of <name>". | M | 1 |
| CO03-F105 | Session restore | Selected book, contact, scroll and unsaved edits return. | M | 1 |
| CO03-F106 | Store recovery | Integrity check, index rebuild, re-sync keeping local changes. | M | 2 |
| CO03-F107 | Audit entries | Picker use is not logged (user action); completion grants, limit hits, trust writes, first contact with servers are. | M | 2 |
| CO03-F108 | Name pronunciation | Phonetic fields are passed to the screen reader as the spoken name. | S | 2 |
| CO03-F109 | Snapshot restore | "Restore address book as of…" from ZFS snapshots through `svc.backup`, as a diff the user applies. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Contacts
├─ Main window
│  ├─ Header bar: menu, title, search field, [net] badge
│  ├─ Sidebar: All contacts, Favourites, smart groups, books by account, groups, tags
│  ├─ Contact list with index strip
│  ├─ Contact card (read view / inline edit)
│  │  └─ Sections: header, communication, addresses, dates, organisation,
│  │     keys and identities, notes, groups, sources, history
│  └─ Status bar: count, sync state
├─ Picker window (drawn for other apps; single or multiple choice)
├─ Dialogs: duplicate review · merge conflict · import wizard (vCard, CSV mapping, LDIF) ·
│  export · account wizard · book properties · key details and verification ·
│  share field picker · QR display · photo crop · print setup · preferences
├─ Popovers: value actions · type label chooser · group and tag chooser · avatar menu
├─ Sheets: sync conflict · key change alert · add-contact review (from intent)
└─ Shell-drawn: file portal, print dialog, capability prompts (completion grant)
```

### 5.2 Main window

Shown at 1180×760.

```
+------------------------------------------------------------------------------------------+
| [=] Contacts                     [/ Search 612 contacts…      ] (1)        [net] _ ▢ ✕   |
+------------------------------------------------------------------------------------------+
| [+ New] (2)        | Sort: Family v      (4) |                              [Edit] [⋯](6)|
+--------------------+-------------------------+-------------------------------------------+
| ★ Favourites    12 | M                     A |  (SM)  Sol Marsh              ★           |
| All contacts   612 | > (SM) Sol Marsh    ✔ B |        Journalist · Example Press         |
| Recently added     |   (TM) Tam Mbeki      C |        [✉ Mail] [💬 Chat] [📞 Call] (7)    |
| SMART              |   (JM) Jo Müller      D |-------------------------------------------|
|  Birthday this mo. | N                     … | COMMUNICATION                             |
|  No e-mail         |   (AN) Ana Novak   ⚠  M |  work   sol@example.org        [✉][⧉]     |
| LOCAL              |   (RN) Raj Nair      >N |  mobile +61 4 1234 5678        [📞][⧉]    |
|  ● Personal    140 | O                     O |  matrix @sol:example.org   ✔ verified (8) |
|  ● Sources 🔒   18 |   (RO) Rae Okafor  ★  P |-------------------------------------------|
| dav.example.org(3) |   (EO) Example Org 🏢  … | KEYS                                      |
|  ● Work        454 |                       Z |  OpenPGP 4F2A 91C0 … 7B3E 9C1D            |
| GROUPS             |                     (5) |  ✔ Verified by you, 3 Mar 2026, in person |
|  Family          9 |                         |  [Details] [Verify again]             (9) |
|  Team           14 |                         |-------------------------------------------|
| TAGS               |                         | DATES     Birthday 21 November (turns 34) |
|  ● press           |                         | ADDRESS   12 Kent St, Sydney NSW 2000     |
|                    |                         | GROUPS    [Team] [press] [+]              |
|                    |                         | NOTES     Met at the 2024 conference.     |
|                    |                         | SOURCES   Work (dav.example.org) · Personal|
|                    |                         | HISTORY   Last mail 3 days ago  [Show] (10)|
+--------------------+-------------------------+-------------------------------------------+
| 612 contacts · 1 selected · Work synced 09:15                                    (11)    |
+------------------------------------------------------------------------------------------+
```

1. Search has focus at start; typing anywhere in the list also searches.
2. Primary action. A split menu offers New contact, New organisation, New group.
3. Books grouped by account; lock for encrypted books; warning icon on sync error.
4. List toolbar: sort and display order.
5. Index strip; letters with no entries are dimmed; click or drag to jump.
6. Card actions: Edit (`E`), and a menu with Share, Show QR, Export, Merge with…, Delete, History.
7. Quick actions use the preferred value; a long press or the arrow lists all values.
8. Messenger identity with the state written by CO05.
9. Key block with trust state, who verified and how.
10. History section appears only when the feature is on.
11. Status bar.

Avatar tokens like `(SM)` are initials on a colour. The trust icons in the list are a tick (verified), a warning triangle (key changed), none otherwise.

### 5.3 Other views and dialogs

#### 5.3.1 Inline edit (card area)

```
| [Photo ▾]  Given [Sol_______]  Family [Marsh______]  [More name fields v]  |
|            Organisation [Example Press____]  Title [Journalist____]        |
|---------------------------------------------------------------------------|
| E-mail  [work v]   [sol@example.org___________] (•) preferred  [–]         |
|         [home v]   [__________________________] ( )            [–]         |
| Phone   [mobile v] [+61 4 1234 5678___________] (•)            [–]         |
| [+ E-mail] [+ Phone] [+ Messaging] [+ URL] [+ Address] [+ Date] [+ Field v]|
|---------------------------------------------------------------------------|
| Birthday [21] [November v] [1992_] (year optional)                         |
| Notes    [Met at the 2024 conference.________________________________]     |
|---------------------------------------------------------------------------|
| ℹ Looks like "S. Marsh" in Personal. [Show] [Link them]              (1)   |
|                                           [Cancel]  [Done]  (saved 09:21)  |
```

1. Duplicate hint. Fields save on leave; "Done" leaves edit mode. States: read-only source (locks on fields, "Copy to my book"), offline ("will sync later"), validation warning under a field.

#### 5.3.2 Picker window (560×520, drawn for another app)

```
+----------------------------------------------------------+
| Mail wants: e-mail addresses (choose any number)    (1)  |
+----------------------------------------------------------+
| [/ sol_______________________________]                   |
+----------------------------------------------------------+
| [x] (SM) Sol Marsh        sol@example.org        ✔ 🔒    |
|          └ other: sol.marsh@example.net  [ ]             |
| [ ] (SO) Solveig Olsen    solveig@example.com            |
| [ ] 👥   Team (14)        expands to 14 addresses   (2)  |
+----------------------------------------------------------+
| Mail will receive only the selected addresses.      (3)  |
|                               [Cancel]  [Choose 1]       |
+----------------------------------------------------------+
```

1. Names the calling app and the fields requested. 2. Groups can be picked. 3. States exactly what crosses. Icons: tick for verified, lock when a usable key exists. States: empty ("No contacts have an e-mail address"), locked books hidden with a line "1 locked address book not shown · Unlock".

#### 5.3.3 Duplicate review (860×600)

```
+--------------------------------------------------------------------------------+
| Possible duplicates: 37 pairs (22 safe, 15 to review)   [Merge all 22 safe]    |
+--------------------+-----------------------------------------------------------+
| > Sol Marsh   0.95 |            Personal              Work                     |
|   Jo Müller   0.91 | Name       (•) Sol Marsh         ( ) S. Marsh             |
|   A. Novak    0.72 | E-mail     [x] sol@example.org   [x] sol@example.org (same)|
|   R Nair      0.64 | Phone      [x] +61 4 1234 5678   [ ] —                    |
|   …                | Org        ( ) —                 (•) Example Press        |
|                    | Birthday   (•) 21 Nov 1992       ( ) 21 Nov               |
|                    | Key        [x] OpenPGP ✔ verified [ ] —               (1) |
|                    | Photo      (•) 512×512           ( ) 96×96                |
|                    +-----------------------------------------------------------+
|                    | Why: same e-mail, similar name                        (2) |
|                    | [Not duplicates] [Link only] [Skip]       [Merge]         |
+--------------------+-----------------------------------------------------------+
```

1. Keys and trust are always kept. 2. The score is explained in words.

#### 5.3.4 Key details and verification (600×520)

```
+--------------------------------------------------------------+
| Verify Sol Marsh's OpenPGP key                               |
+--------------------------------------------------------------+
| Fingerprint                                                  |
|   4F2A 91C0 55D1 0E7A 23B9  6A44 08CE 7B3E 9C1D 2210         |
| As words: lotus anchor pixel … (20 words)        [Show QR]   |
| User IDs:  Sol Marsh <sol@example.org>                       |
| Created 2 Feb 2024 · Expires 2 Feb 2027 · Ed25519/X25519     |
| State:  seen in 14 mails since 9 Jan 2025 (not verified)     |
+--------------------------------------------------------------+
| Compare this with what Sol shows or reads to you.       (1)  |
| How did you check?  (•) In person ( ) Voice call ( ) QR scan |
|                     ( ) Trusted introducer [contact v]       |
|                      [Not matching…]  [Cancel]  [Mark verified]|
+--------------------------------------------------------------+
```

1. One plain instruction. "Not matching…" records a failed check and sets the state to `changed`. Key parsing and fingerprint computation happen in SE04.

#### 5.3.5 CSV import mapping (760×560)

```
+---------------------------------------------------------------------------+
| Import contacts.csv — 4 112 rows · UTF-8 · delimiter ","  [Change…]       |
+---------------------------------------------------------------------------+
| Column in file      Sample                 Import as                      |
| First Name          Sol                    [Given name v]                 |
| Last Name           Marsh                  [Family name v]                |
| E-mail Address      sol@example.org        [E-mail (work) v]              |
| Mobile Phone        0412 345 678           [Phone (mobile) v]             |
| Birthday            21/11/1992             [Birthday v] format [D/M/Y v]  |
| Custom 1            press                  [Tag v]                        |
| Notes               Met at…                [Notes v]                      |
| Internal ID         8841                   [Do not import v]              |
+---------------------------------------------------------------------------+
| Into [Personal v]   Preview: 3 890 new · 180 identical · 42 updates   (1) |
| Default phone region [Australia v]   [Save mapping as…]                   |
|                                              [Cancel]  [Review]  [Import] |
+---------------------------------------------------------------------------+
```

1. De-duplication result before anything is written; "Review" lists the 42 updates.

#### 5.3.6 Share and QR (480×560)

```
+------------------------------------------------+
| Share Sol Marsh                                |
+------------------------------------------------+
| Include: [x] Name  [x] E-mail (work)           |
|          [x] Phone (mobile)  [ ] Address       |
|          [ ] Birthday  [ ] Notes  [x] Key id   |
+------------------------------------------------+
|              ▄▄▄▄▄▄▄ ▄  ▄ ▄▄▄▄▄▄▄              |
|              █ ▄▄▄ █ ▀█▄  █ ▄▄▄ █              |
|              █ ███ █ ▄▀ ▄ █ ███ █         (1)  |
|              ▀▀▀▀▀▀▀ ▀ ▀ ▀ ▀▀▀▀▀▀▀             |
| 214 bytes · QR version 9 · vCard 4             |
+------------------------------------------------+
| [Full screen]  [Save .vcf…]  [Send to…] [Close]|
+------------------------------------------------+
```

1. The code is redrawn as fields are ticked. Notes and photos are off and photos cannot be ticked.

### 5.4 Menus

Items separated by ` · `; shortcut in brackets.

```
File     New contact [Ctrl+N] · New organisation · New group [Ctrl+Shift+N] ·
         New address book… · Add account… · Import… [Ctrl+O] · Export… [Ctrl+Shift+S] ·
         Share… [Ctrl+Shift+E] · Show QR [Ctrl+R] · Print… [Ctrl+P] · Close [Ctrl+W] · Quit [Ctrl+Q]
Edit     Undo [Ctrl+Z] · Redo [Ctrl+Shift+Z] · Cut · Copy · Paste [Ctrl+X/C/V] ·
         Copy as text [Ctrl+Shift+C] · Select all [Ctrl+A] · Edit contact [E or F2] ·
         Delete [Delete] · Find [Ctrl+F, /] · Preferences [Ctrl+,]
View     Sort by > Given | Family | Organisation | Recently used | Recently changed ·
         Name order > Given Family | Family, Given · List | Grid · Density > ·
         Show empty sections · Sidebar [F9] · History section · Zoom [Ctrl++ / - / 0]
Contact  Mail [M] · Chat [C] · Call [Shift+C] · Favourite [S] · Add to group… [G] ·
         Tag… [T] · Move to book… · Copy to book… · Merge selected [Ctrl+M] ·
         Link selected · Split linked · Set as my card · Block · Show history [Ctrl+H]
Keys     Key details [Ctrl+I] · Verify… · Import key from file… · Export public keys… ·
         Encryption preferences… · Look for a key (opens Passwords and Keys)
Tools    Find duplicates… [Ctrl+D] · Clean up… · Sync now [F5] · Sync all [Shift+F5] ·
         Lock encrypted books [Ctrl+Shift+L] · App access to contacts… · Sync log · Check store
Help     Contacts help [F1] · Keyboard shortcuts [Ctrl+/] · About Contacts
```

### 5.5 Keyboard and pointer

List focus: arrows move; letters type-ahead; `Enter` moves focus to the card; `E` edits; `M`, `C` act on the preferred value; `Space` toggles selection; `Ctrl+Up/Down` jumps by initial. Card focus: `Tab` walks values; `Enter` runs the default action of the value; `Ctrl+C` copies it; `Shift+F10` opens its actions. Edit mode: `Ctrl+Enter` done, `Esc` reverts the field (twice: leaves edit mode), `Alt+Up/Down` reorders a value among its kind.

Context menus:

- **Contact row**: Open, Edit, Mail, Chat, Call, Favourite, Add to group, Tag, Share, Show QR, Export, Merge with…, Move to book, Copy to book, Delete.
- **Value on the card**: default action, Copy, Copy formatted, Set preferred, Change label, Remove, Show QR of this value.
- **Book**: Rename, Colour, Properties, Sync now, Export, Lock, Remove.
- **Group or tag**: Rename, Mail all, Start room, Export, Delete (members are kept).
- **Avatar**: Change photo, Paste photo, Remove photo.

Drag and drop:

| Drag | Onto | Result |
|---|---|---|
| Contacts | Group or tag | Add to it |
| Contacts | Book | Move (Ctrl: copy) |
| Contact | Another contact | Offer Merge or Link |
| Contacts | Outside (Files, Mail, Messenger) | `.vcf` and plain text |
| `.vcf`, `.csv`, `.ldif` | Window or book | Import preview |
| Image file | Avatar | Photo crop |
| Text selection (from any app) | Window | Add-contact review with parsed fields |
| Key file | Card | Key import review (through SE04) |

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | "No contacts yet." [New contact] [Import a file] [Add account]. |
| No results | "No match for 'solx'." [Clear search]; offers "New contact named solx". |
| Loading | Cached list at once; photos fade in at 80 ms; sync progress in the status bar. |
| Error | Warning icon on the book; status bar message with Details. |
| Offline | "Offline · 3 pending"; all edits allowed. |
| Locked | Encrypted books show name and lock only; contacts absent from list, pickers, completion and search; [Unlock]. |
| Read-only | Edit shows locks; "Copy to my book". |
| Key changed | `--danger` banner on the card until resolved. |
| Narrow (<640) | One pane at a time: list, then card; sidebar is an overlay; index strip stays. |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Sidebar | `treeView` | counts as badges |
| Contact list | `modelListView` over `SqliteModel` | needs section headers and an index strip (`CO03-GAP-1`; section headers in FUSE-WP-28) |
| Grid view | `gridView` | |
| Card read view | `scrollArea` with `panel`, `label`, `link`, `toolButton`, `image` | |
| Card edit | `lineEdit`, `comboBox`, `radioButton`, `dateTimeEdit`, `toolButton` | |
| Notes | `markdownView` (read), `codeEdit` or G-10 (edit) | |
| Groups and tags on card | chips | missing: tag input (same gap as CO01-GAP-1) |
| Duplicate review | `listView` + `tableView` with radio and check cells | |
| CSV mapping | `tableView` with `comboBox` cells | |
| QR code | `image` fed by the QR encoder | |
| Photo crop | `image` with a crop overlay | simple own overlay; full canvas (G-12) not needed |
| History | `listView` | |
| Picker window | `lineEdit`, `modelListView`, `checkBox`, `button` | |
| Popovers, sheets, toasts | missing in V3; FUSE-WP-30 | |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | View models, editing, undo, merge logic, duplicate scoring, journal |
| UI | FUSE UI | app host | Main window, dialogs |
| Contacts service | ForgeScript (headless entry of the same bundle) | background app host, started on demand by the bus | Picker, completion, lookup, dates, keys and trust interfaces; rate limits; per-app visibility |
| vCard helper | C++23 | pooled, `proc.helper:vcard` | Parse and serialise vCard 2.1/3.0/4.0, jCard, xCard, LDIF, CSV sniffing. No capabilities. |
| DAV sync helper | C++23 | per account, `proc.helper:dav` | CardDAV; same source as CO02's helper |
| Image helper | C++23 | pooled, `proc.helper:photo` | Decode, crop, scale, strip metadata, encode photos; decode QR from an image file. No capabilities. |
| QR encoder | ForgeScript | app host | Encode QR (ISO/IEC 18004) from own data; no hostile input |
| Key engine | C++23 (SE04) | SE04 process | Parse keys and certificates, compute fingerprints |

```
 CO01, CO05, CO02, SE05, shell notifications
        │ picker · complete · lookup · dates · keys · trust   (capability-gated bus interfaces)
        v
 +---------------------------+        +-----------------------------+
 | contacts service (headless)|<------>| main app (UI, edit, merge)  |
 +-------------+-------------+  same   +----+-----------+------------+
               |                store       |           |
               v                            v           v
        contacts.sqlite              vcard helper   photo helper     SE04 key engine
               ^                      (no caps)      (no caps)
               |
        dav sync helper ──sockets──> network service (rules, route, TLS)
```

### 6.2 Data model

Store: `~/.local/share/org.pbsd.contacts/contacts.sqlite`, `cards/` raw vCards for round trip, `photos/<blake3>` (deduplicated), `enc/<book-id>.sqlite` for encrypted books, `history.sqlite` (separate so it can be erased and is never synced).

| Entity | Fields |
|---|---|
| `book` | `id: uuid`, `account_ref: text?`, `kind: enum(local, carddav)`, `name`, `colour`, `url`, `ctag`, `sync_token`, `vcard_version: enum(3, 4)`, `group_style: enum(kind_group, categories, apple)`, `read_only: bool`, `encrypted: bool`, `max_photo_bytes: u32` |
| `card` | `id: i64`, `book_id`, `uid: text`, `href`, `etag`, `raw_ref`, `base_ref`, `kind: enum(individual, org, group, location)`, `dirty: bool`, `deleted_at: i64?`, `rev: i64` |
| `person` | `id: i64` (the linked identity shown as one row), `display_name`, `sort_key: blob`, `favourite: bool`, `is_me: bool`, `blocked: bool`, `photo_ref: text?` |
| `person_card` | `person_id`, `card_id` (links across books) |
| `name` | `card_id`, `prefix`, `given`, `additional`, `family`, `suffix`, `nickname`, `phonetic_given`, `phonetic_family`, `fn`, `sort_as` |
| `field` | `id`, `card_id`, `kind: enum(email, tel, impp, url, adr, date, org, title, role, related, note, lang, tz, geo, fburl, custom)`, `label: text`, `types: json`, `pref: u8`, `value: text`, `value_norm: text` (folded e-mail, E.164), `structured: json` (address parts, date parts) |
| `membership` | `group_card_id`, `member_uid` |
| `tag` | `card_id`, `name` |
| `pubkey` | `id`, `person_id`, `kind: enum(openpgp, smime, age, ssh)`, `fingerprint: text`, `keyring_ref: text?`, `inline: blob?`, `expires: i64?`, `state: enum` |
| `identity_trust` | `id`, `person_id`, `scheme: enum(openpgp, smime, matrix, xmpp, irc, sip)`, `identity: text` (address, MXID, JID), `key_fpr: text?`, `state: enum(unknown, seen, introduced, verified, changed, revoked)`, `method: enum(in_person, voice, qr, introducer, app)`, `set_by: text` (bundle id), `set_at: i64`, `first_seen: i64` |
| `enc_pref` | `person_id`, `mail_policy: enum`, `preferred_key_id`, `preferred_protocol` |
| `version` | `card_id`, `rev`, `at: i64`, `origin: enum(local, sync, merge, import)`, `raw_ref` |
| `not_duplicate` | `uid_a`, `uid_b` |
| `journal_op` | `seq`, `book_id`, `kind`, `card_id`, `state`, `attempts`, `error` |
| `app_access` | `bundle_id`, `interface`, `books: json`, `granted_at` |
| `interaction` (in `history.sqlite`) | `person_id`, `kind: enum(mail_in, mail_out, chat, call, meeting)`, `at: i64`, `link: text`, `subject: text?`, `app: text` |

Writes patch the raw vCard so unknown properties, parameters and group prefixes survive. Migration is forward only with a copy of the previous database.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| vCard 4.0 (RFC 6350), 3.0 (RFC 2426), extensions RFC 6474, 6715, 8605, 9554 | yes | yes | own | vCard helper |
| vCard 2.1 | yes | no | own | vCard helper |
| jCard (RFC 7095), xCard (RFC 6351) | yes | jCard yes, xCard no | own | vCard helper |
| CSV | yes | yes | own | vCard helper (read), app (write) |
| LDIF (RFC 2849) | yes | yes | own | vCard helper |
| CardDAV (RFC 6352), WebDAV, sync-collection | yes | yes | own; shared with CO02 | DAV sync helper |
| JPEG, PNG, WebP, GIF, HEIF photos | yes | JPEG, PNG | system decoders shared with `svc.thumbnail` | photo helper |
| QR code | yes (image file) | yes | own encoder; decoder in helper | encoder in app host; decoder in photo helper |
| MeCard | yes | yes | own | vCard helper |
| OpenPGP keys, X.509 certificates, age and SSH public keys | yes | export only | SE04 engine | SE04 engine |
| Numbering plan and address-format tables | yes | — | bundled data derived from public tables (source and licence to be chosen) | app host (trusted bundle data) |

No third-party code library is proposed. Bundled data tables need a licence check.

### 6.4 Integration

- **Intents handled**: `open`/`view` (`text/vcard`), `new` (contact from fields or text), `pick` (contact fields; routed to the picker), `share` (receive `.vcf`). **Emitted**: `compose` (CO01, CO05), `open` (URL, map), `share`, `print`, `scan` and `pick` to ME04 (QR, photo).
- **MIME**: view/edit `text/vcard`; import `text/vcard`, `text/x-vcard`, `text/csv`, `text/x-ldif`, `application/vcard+json`, `application/vcard+xml`; export `text/vcard`, `text/csv`, `text/x-ldif`, `application/vcard+json`.
- **Services used**: `svc.accounts`, `svc.keyring`, `svc.settings`, `svc.intent`, `svc.portal.file`, `svc.portal.print`, `svc.search`, `svc.notify` (key change alert), `svc.backup`, `svc.audit`, `svc.clipboard`, `svc.undo-trash`, `svc.a11y`, `svc.model` (optional).
- **Services offered** (Proposed): `org.pbsd.contacts.picker`, `.complete`, `.lookup`, `.dates`, `.keys`, `.trust`, `.history` (write-only for CO01, CO05, CO02). Each maps to a capability `svc.contacts.<name>` that other apps declare.
- **Other apps**: CO01 (completion, sender card, keys, policy), CO05 (identities and trust, reverse lookup), CO02 (birthdays, attendees, `FBURL`, shared DAV account), CO04 (assignees through the picker), SE04 (key parsing, lookup), SE05 (recipients for file encryption), ME04 (QR scan, photo), IN04 (addresses), SH04 (share sheet shows favourite contacts through `lookup`).
- **Backing repos**: Verified: FUSE has `modelListView`, `gridView`, `image`, `markdownView`, `SqliteModel` (PLATFORM.md V3, V8, from `FUSE:docs/ui/README.md`). Verified: FORGE Unicode conformance is reported in `FORGE:docs/STATUS.md` as cited by FORGE-WP-23 in the build plan; collation and transliteration coverage were not read: Unverified. Everything else is Proposed.

## 7. Security and privacy

### 7.1 Capabilities requested

| Capability | Reason |
|---|---|
| `fs.app` | Store, raw cards, photos |
| `fs.portal` | Import, export, photo files, scheduled export folder |
| `net.user` | CardDAV servers the user typed |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Per-account route |
| `proc.helper:vcard`, `proc.helper:dav`, `proc.helper:photo` | Helpers in 6.1 |
| `svc.accounts`, `svc.keyring` | Endpoints; secrets and book keys |
| `svc.intent`, `svc.portal.file`, `svc.portal.print` | Hand-offs, files, printing |
| `svc.search`, `svc.notify`, `svc.settings`, `svc.backup`, `svc.audit`, `svc.clipboard`, `svc.undo-trash`, `svc.a11y` | Standard services |
| `svc.model`, `ai.local` | Optional signature parsing; off by default |
| `bg.run` | The headless contacts service answers other apps with no window open |

Not requested: `dev.camera` (photos and QR scans come from ME04), `proc.exec`, `bg.autostart` (the bus starts the service on demand).

### 7.2 Threat model

Assets: the social graph (who the user knows), private notes, sensitive address books (sources), key trust decisions.

| Adversary | Attack | Mitigation |
|---|---|---|
| Other local app | Harvest the address book | No bulk read interface exists. Picker returns only chosen values. Completion needs a grant, a prefix of 2+ characters, returns 8 rows, is rate-limited and audited. Per-app book visibility. |
| Other local app | Forge trust ("mark attacker key verified") | Trust writes need `svc.contacts.trust`; every write is attributed, audited and shown on the card; `verified` by method `in_person`, `voice` or `qr` can be set only from Contacts' own dialog or by CO05's ceremony result, never by a plain call from other apps. |
| Sender of a vCard or CSV | Parser exploit; oversize photo; billion-field card | Zero-capability helper; limits (card 2 MiB, photo 8 MiB, 2 000 properties); T2 target; fuzzing |
| Sender of a vCard | Overwrite an existing contact by reusing its UID | Import preview shows updates explicitly; never applied silently |
| Sender of a vCard | Hostile URLs, `tel:` to premium numbers, scripts in notes | URL scheme allow-list; confirmation on open; notes are inert Markdown |
| CardDAV server | Reads all contacts; serves altered keys | Sensitive books stay local and encrypted; a key change on a verified identity raises the alert instead of replacing silently |
| Network attacker | Interception | TLS required |
| Merge mistakes | Loss of data or wrongful trust transfer | Merge keeps all values; trust binds to the exact identity and key; undo and 30-day split |
| QR observer | Reads shared data from the screen | Field picker with minimal defaults; photos and notes excluded |
| Local model | Leak of pasted text | `svc.model` is on-device; off by default |

Stored: section 6.2. Leaves the machine: CardDAV traffic to the user's servers only. No lookups, no enrichment, no avatars from the network.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `view.name_order` | enum given_family/family_given | locale | Display order |
| `view.sort` | enum given/family/org/recent | family | List sort |
| `view.show_empty_sections` | bool | false | Show empty card sections |
| `view.density` | enum compact/normal | normal | List density |
| `edit.default_book` | book id | first local | Target for new contacts |
| `edit.default_region` | region code | from locale | Phone parsing region |
| `edit.photo_max_px` | int | 512 | Stored photo size |
| `dup.safe_threshold` | float | 0.9 | "Safe" merge score |
| `dup.review_threshold` | float | 0.6 | Lowest score offered |
| `dup.hint_on_create` | bool | true | Hint while typing |
| `share.default_fields` | set | name, email, tel | Fields ticked in Share |
| `share.vcard_version` | enum 3/4 | 4 | Export version |
| `keys.include_in_export` | bool | false | Keys in vCard export |
| `keys.alert_on_change` | bool | true | Notify on key change |
| `service.complete_limit_per_min` | int | 60 | Distinct prefixes per app per minute |
| `service.complete_min_chars` | int ≥2 | 2 | Minimum prefix |
| `service.visible_books.<bundle>` | list | all unencrypted | Books an app may see |
| `history.enabled` | bool | false | Interaction history |
| `history.subjects` | bool | false | Store subjects |
| `history.keep_days` | int | 365 | Retention |
| `sync.interval_min` | int | 30 | Background sync |
| `search.system_index` | bool | true | Names to `svc.search` |
| `trash.keep_days` | int | 30 | Trash and merge-split retention |
| `ai.parse_text` | bool | false | Model help for pasted text |

## 9. Accessibility and languages

- The a11y name of a row is the display name, then organisation, then states ("favourite", "key verified", "key changed").
- Phonetic name fields are used for speech and for sorting in languages that need them (Japanese kana, Chinese pinyin or zhuyin).
- Fingerprints are exposed as grouped text and as the word form so they can be read aloud and compared by ear.
- QR display has a text alternative (the vCard text) and a "Copy text" action.
- Name handling makes no assumption of given and family: single-name contacts, family-first orders, patronymics and long names all store in `fn` with optional parts.
- Collation follows the locale (for example `ä` after `a` or after `z`); the index strip is built from the locale's index characters, with `#` for the rest.
- Address forms and phone formats follow the contact's country, not the UI locale.
- Right-to-left: the card mirrors; phone numbers and e-mail addresses stay left-to-right.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | under 150 ms (small app bar in PLATFORM.md section 11) |
| Search keystroke to updated list, 100 000 contacts | under 16 ms |
| Completion service reply | under 10 ms at the 95th percentile |
| Open a card with a photo | under 30 ms |
| Duplicate scan of 50 000 contacts | under 5 s, in the background, cancellable |
| Import 50 000 vCards | under 30 s |
| Memory | under 120 MiB with 100 000 contacts; headless service under 40 MiB |
| Idle | zero frames, zero wake-ups; the service exits after 60 s without requests |
| Largest supported | 500 000 contacts, 200 books |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: merge rules (union, conflict, trust binding), duplicate scoring against a labelled set, phone normalisation table tests, address formatting per country, sort keys.
- **Property tests**: parse then serialise then parse is stable; merge is associative for multi-value fields; merge then split returns the originals; trust state never rises through a merge.
- **vCard corpus**: files from common servers, phones and clients, with odd encodings, folded lines, grouped properties; unknown properties must survive a sync round trip.
- **DAV**: the fake server from CO02 with CardDAV fixtures; differential runs against Radicale and Baïkal offline; vCard 3-only server path.
- **Service abuse tests**: a test app tries to enumerate through completion; the limit and audit entry must trigger; locked books must never appear in any interface.
- **Fuzz**: vCard, jCard, xCard, LDIF, CSV sniffing, QR decoder, MeCard.
- **Golden renders**: list, card, edit, picker, duplicate review, QR, in all themes, 200%, RTL.
- **Scripted UI**: the 14 top tasks, keyboard only.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| vCard helper | No runtime errors; bounded line, property and nesting counts | T2 | PRISM bounded checks, fuzzing | — |
| QR decoder, photo pipeline | No runtime errors | T2 for own code; T4 for system decoders | PRISM, fuzzing | Third-party decoders |
| Completion rate limiter | No app gets more than the limit of distinct prefixes per window; empty prefix never answered | T1 | Small pure function, proved | Needs a C++23 or provable form |
| Trust state machine | `verified` reachable only through an allowed method and caller; merge never raises a state | T1 | State model proved; table tests | Model to be written |
| Merge engine | No value lost (every source value appears in the result or in history) | T3, T1 as a stretch | Property tests | ForgeScript not readable by PRISM (Unverified) |
| QR encoder | Output decodes to the input (round trip) | T3 | Round-trip tests against the decoder and fixtures | — |
| DAV helper | As in CO02 | T2 | Shared | — |
| SQLite | Vendored C | T4 | — | Third-party C |

## 12. Build plan

### Phase 1 — usable core

| ID | Work item | Depends on |
|---|---|---|
| CO03-W01 | Store, raw cards, photo store, journal, migrations | G-07, G-09, FORGE-WP-23.1 |
| CO03-W02 | vCard helper (2.1, 3.0, 4.0) | FORGE-WP-27, FORGE-WP-28 |
| CO03-W03 | Main window: sidebar, list with sections and index strip, card | G-01, G-02, FUSE-WP-28 |
| CO03-W04 | Inline editor, validation, undo, trash | CO03-W03 |
| CO03-W05 | Search index and collation | FORGE-WP-23.4 |
| CO03-W06 | Local books; vCard import and export; drag out | CO03-W02 |
| CO03-W07 | CardDAV sync with the shared DAV helper | CO02-W09, FORGE-WP-21 |
| CO03-W08 | Headless service: picker and completion with limits | G-08, FORGE-WP-25 |
| CO03-W09 | Photo helper, crop | `svc.portal.file` |
| CO03-W10 | Groups, favourites, intents (`new`, `open`), session restore | CO03-W04 |
| CO03-W11 | Corpus, fuzzers, goldens, service abuse tests | FORGE-WP-35 |

### Phase 2 — full feature

| ID | Work item | Depends on |
|---|---|---|
| CO03-W12 | Duplicate detection, review, merge, link, split | CO03-W05 |
| CO03-W13 | Keys per contact, key card, verification dialog | SE04 engine (FORGE-WP-22.4) |
| CO03-W14 | Trust interface, messenger identities, key change alert | CO05, CO01, `svc.audit` |
| CO03-W15 | Lookup, dates and keys services; per-app visibility | CO03-W08, AD05 |
| CO03-W16 | CSV and LDIF import; phone normalisation; address formats | CO03-W02 |
| CO03-W17 | Share sheet, QR encode and decode | SH04, ME04 |
| CO03-W18 | Conflict merge, version history, linked contacts view | CO03-W07 |
| CO03-W19 | Encrypted books, per-account route | `svc.keyring`, SE03 |
| CO03-W20 | Smart groups, tags, bulk edit, custom fields, me card | CO03-W04 |
| CO03-W21 | New contact from pasted text and from other apps | CO01, CO05 |

### Phase 3 — advanced

| ID | Work item | Depends on |
|---|---|---|
| CO03-W22 | Interaction history and its write interface | CO01, CO05, CO02 |
| CO03-W23 | Printing: lists, labels, envelopes | G-16 |
| CO03-W24 | jCard, xCard, server-side search, nested groups, grid view | CO03-W02 |
| CO03-W25 | Clean-up tools, block list, stay-in-touch | CO03-W12 |
| CO03-W26 | Snapshot restore, scheduled export | `svc.backup` |
| CO03-W27 | Model-assisted text parsing; transliteration search | G-15 |

## 13. Open questions and risks

1. **App-provided services and capabilities.** PLATFORM.md section 6 lists `svc.<name>` for system services only. Contacts needs other apps to declare `svc.contacts.pick`, `.complete`, `.lookup`, `.keys`, `.trust`. The platform must say how an app-provided service is named, granted and shown in AD05.
2. **On-demand start.** The headless service needs the bus to start a bundle entry on first call. Not stated in PLATFORM.md section 7.
3. **Completion privacy against a patient app.** Rate limits slow harvesting but cannot stop it over weeks. The grant prompt must say so; sensitive books should stay excluded.
4. **Group representation** differs by server (vCard 4 groups, categories, Apple extension). Probing may guess wrong; a per-book override exists.
5. **Phone and address tables** need an offline data source with a compatible licence; none chosen. Unverified.
6. **Collation and transliteration** in `forge::text`: coverage Unverified.
7. **Trust scale shared by three apps** must be agreed with SE04 and CO05 before any of them ships; changing it later means a migration in all three.
8. **Linked contacts across books** add complexity to every edit; if it proves confusing, fall back to plain merge only.
9. **Where public keys live.** This spec stores public keys with the contact and lets SE04 hold a reference. SE04 may prefer the reverse. To settle with the SE04 writer.
