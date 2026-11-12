<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Input Methods and Emoji

By Odin Loch

| | |
|---|---|
| ID | `AX04` |
| Bundle | `org.pbsd.input-methods` |
| Category | access |
| Priority | P1 |
| Backing repos | FUSE (`Source/FUSE/Core/include/fuse/platform/text_services.hpp`, `events_v2.hpp`), FORGE (`docs/text-locale.md`), this repo (`pbsd/compositor`) |
| Status | Planned. Nothing built. |

## 1. Purpose

Input Methods and Emoji lets people type every language and symbol: compose sequences, dead keys, Chinese, Japanese, Korean, Indic, Vietnamese and phonetic input, an emoji and symbol picker, and text snippets. Each input engine runs in a sandbox with no files and no network, so an engine cannot log keys or send them anywhere. It replaces fcitx/ibus and the Plasma emoji picker of the interim plan (PLATFORM V20).

## 2. Scope

In scope: the input-method framework (broker, engine API, engine sandbox, candidate window, pre-edit, per-window state); the engines; layout data, layout viewer and the input-source switcher; the emoji and symbol picker; text expansion; on-device learning with a visible, erasable dictionary; a prediction service for AX03; per-app input-source memory.

Out of scope:

| Item | Owner |
|---|---|
| The Keyboard page: which layouts are enabled, their order, options, repeat rate, sticky/slow/bounce keys | SY01 (this app supplies the data and a "Manage input sources" hand-off) |
| On-screen keys, swipe decoding, scanning | AX03 |
| Dictation | AX06 |
| Spell check, hyphenation | G-17 service |
| Clipboard history and paste | AX05 |
| Font management, character map by font, glyph inspector | UT03 |
| Text shaping, bidi layout, rendering of pre-edit in apps | FUSE (G-03, FUSE-WP-59) |
| Key code to key symbol mapping at run time | compositor |
| Speaking candidates | AX01 (through the a11y tree of the candidate window) |

## 3. Users and top tasks

Personas: **Wei**, types Chinese with pinyin and switches to English many times an hour. **Yuki**, Japanese kana-kanji with a physical JIS keyboard. **Min-jun**, Korean hangul, occasional hanja. **Priya**, writes Hindi by transliteration on a US keyboard. **Anh**, Vietnamese Telex. **Lukas**, German, uses dead keys and compose for typography. **Sara**, linguist, types IPA. **Omar**, Arabic and English in one document. **Dana**, support worker, lives on snippets. Everyone uses emoji.

| # | Task | Target steps |
|---|---|---|
| 1 | Switch input source | 1 (`Super+Space`) |
| 2 | Type a pinyin word and pick the first candidate | letters + `Space` |
| 3 | Pick another candidate | 1 digit key, or arrows + `Space` |
| 4 | Type é on a US layout | 2 keys (dead key or `Compose ' e`) |
| 5 | Insert an emoji by name | 3 (`Super+.`, type, `Enter`) |
| 6 | Insert a recent emoji | 2 (`Super+.`, `Enter`) |
| 7 | Expand a snippet | type the abbreviation + trigger key, 0 extra |
| 8 | See and delete what the system has learned from typing | 3 (open app, Dictionary, delete) |
| 9 | Erase everything learned in the last hour | 3 |
| 10 | Add a language with its engine | 4 (Add source, language, engine, Add) |
| 11 | See which key gives which character | 1 (`Super+Alt+L` layout viewer) |
| 12 | Type a Unicode code point | `Ctrl+Shift+U`, hex, `Space` |
| 13 | Have each app remember its own input source | 0 (default per window) |
| 14 | Convert typed kana to kanji and fix a segment boundary | `Space`, `Shift+Left/Right`, `Enter` |

## 4. Features

### 4.1 Framework

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX04-F001 | Input broker | One broker process per session sits between the compositor's text-input interface and the engines. It owns input-source state, routes key events of the focused text field to the active engine, and relays pre-edit and commits back. | M | 1 |
| AX04-F002 | Engine API | Versioned record interface. To engine: `activate`, `deactivate`, `reset`, `key(code, sym, mods, press)`, `surrounding(text, cursor, anchor)`, `content(purpose, hints)`, `select(index)`, `page(dir)`, `segment(dir)`, `setting(key, value)`. From engine: `preedit(text, spans, cursor)`, `commit(text)`, `delete_surrounding(before, after)`, `candidates(items, labels, page, pages, cursor, layout)`, `status(label, icon)`, `forward_key`, `learn(reading, word)`. | M | 1 |
| AX04-F003 | Engine sandbox | Each engine is its own process in capability mode. It has one channel to the broker and read-only descriptors for its dictionaries. No file system, no network, no bus, no other device. | M | 1 |
| AX04-F004 | Script engines | Table-driven engines are ForgeScript modules in an engine host with `Caps` = none (PLATFORM V12). | M | 1 |
| AX04-F005 | Native engines | Engines that need speed over large dictionaries (pinyin, kana-kanji) are C++23 helpers behind the same API. | M | 2 |
| AX04-F006 | Third-party engines | Installed as signed bundles of kind "input engine" through SY05. Their manifest may request no capability. The Engines page shows signer and that the engine has no file or network access. | S | 3 |
| AX04-F007 | Pre-edit | Uncommitted text shown inside the app's field with spans: underline, current segment, converted, error. Sent through text-input pre-edit. | M | 1 |
| AX04-F008 | Commit | Final text is delivered as a commit string. Engines cannot send key events except `forward_key` of the key they just received. | M | 1 |
| AX04-F009 | Surrounding text | If the field offers it, engines get up to 256 graphemes either side of the caret, for reconversion and context. Never offered from sensitive fields. | S | 2 |
| AX04-F010 | Content purpose and hints | Engines and the broker see purpose (normal, password, e-mail, URL, number, phone, PIN, terminal, date, time) and hints (no completion, lowercase, sensitive, multiline). | M | 1 |
| AX04-F011 | Password bypass | In password and PIN fields the broker runs no engine at all. Keys go through the layout, dead keys and compose only. The candidate window cannot open. | M | 1 |
| AX04-F012 | Sensitive bypass | Fields with the sensitive hint, and windows the shell marks private (IN01 private windows), get engines but no learning, no surrounding text and no snippets. | M | 1 |
| AX04-F013 | Per-window state | Input source, engine mode (for example Chinese or English, hiragana or katakana) and pending composition are kept per window. | M | 1 |
| AX04-F014 | State scope setting | Input source is remembered per window (default), per app, or globally. | M | 1 |
| AX04-F015 | Focus change | On focus loss the composition is committed or discarded (per engine default; setting). No pre-edit ever follows the user to another window. | M | 1 |
| AX04-F016 | Engine crash | A crashed engine is restarted within 200 ms; the pre-edit text at the time is committed as typed; a toast names the engine. Three crashes in a minute disable it and fall back to the plain layout. | M | 1 |
| AX04-F017 | Engine time limit | A key not answered in 100 ms is passed through unprocessed and the engine is marked slow; 2 s without an answer restarts it. Typing never blocks on an engine. | M | 1 |
| AX04-F018 | No engine, no cost | With only plain layouts enabled, no engine process runs and keys take the compositor's direct path. | M | 1 |
| AX04-F019 | Apps without text-input | For clients that do not speak text-input (compatibility personalities, VM03), dead keys and compose still work in the compositor; engines are unavailable and the switcher says so. | S | 2 |
| AX04-F020 | Panel connection | AX03, the emoji picker, AX06 dictation and AX05 quick paste insert text through the broker as "panels": they send commit strings that the broker forwards only to a field that enabled text input. | M | 1 |
| AX04-F021 | Engine settings | Each engine declares typed settings; the app renders them with `propertyGrid`; values reach the engine by `setting`. | M | 2 |
| AX04-F022 | Engine hot keys | An engine may declare mode keys (for example `Shift` alone toggles Chinese/English); they are active only while that engine is active and are listed and rebindable. | M | 2 |
| AX04-F023 | Reconversion | Select committed text and press the reconvert key: the engine gets it back as pre-edit (Japanese, Chinese). Needs surrounding text. | S | 3 |

### 4.2 Candidate window

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX04-F024 | Placement | A popup placed at the caret rectangle the field reports; below the line, or above when there is no room; never covers the pre-edit. | M | 1 |
| AX04-F025 | Horizontal and vertical lists | Per engine default; setting. Vertical lists show an annotation column. | M | 1 |
| AX04-F026 | Paging | 5–10 per page; `PageUp/PageDown`, `-`/`=`, `[`/`]`, arrows; page indicator "2/14". | M | 1 |
| AX04-F027 | Selection keys | Digits `1`–`9`,`0`, or home-row letters (setting, per engine). | M | 1 |
| AX04-F028 | Pointer and touch | Click or tap selects; scroll pages; hover highlights. | M | 1 |
| AX04-F029 | Annotations | Reading, definition, source dictionary, emoji name, or "learned" mark per candidate. | S | 2 |
| AX04-F030 | Expand grid | A key expands the list to a grid of all candidates. | S | 2 |
| AX04-F031 | Delete learned candidate | `Ctrl+Delete` or `Shift+Delete` on a learned candidate removes it from the dictionary, with a toast and Undo. | M | 2 |
| AX04-F032 | Font and size | Follows the system font; size 80–300%; uses the language's font fallback for CJK variants (the right glyphs for zh-Hans, zh-Hant, ja, ko). Needs FUSE-WP-17. | M | 2 |
| AX04-F033 | Theme | System theme; high contrast; the current candidate uses `--accent` plus an outline, not colour alone. | M | 1 |
| AX04-F034 | Inline mode | For simple engines (Telex, transliteration) no window: only pre-edit. | M | 1 |
| AX04-F035 | A11y | The window is in the a11y tree as a list; each move announces candidate and annotation; AX01 character descriptions for homophones come from the annotation. | M | 2 |
| AX04-F036 | No focus | The window never takes keyboard focus. | M | 1 |
| AX04-F037 | Fallback position | If the app reports no caret rectangle, place at the focused element's bounds from `svc.a11y:track`, else at the bottom centre of the window. | S | 2 |

### 4.3 Engines

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX04-F038 | Dead keys | From the layout: accent then letter. Shown as pre-edit until resolved. Built into the broker (trusted, table-driven). | M | 1 |
| AX04-F039 | Compose sequences | Compose key (setting: Right Alt, Menu, Caps Lock, Right Ctrl, none) then a sequence. System table plus a user table in a text file with the same `<Multi_key> <a> <e> : "æ"` line format as X11 Compose files. | M | 1 |
| AX04-F040 | Compose feedback | The pending sequence shows as underlined pre-edit; an invalid sequence beeps or is typed literally (setting). | M | 1 |
| AX04-F041 | Compose browser | Searchable list of all sequences; search by result character or by keys. | S | 2 |
| AX04-F042 | Unicode hex input | `Ctrl+Shift+U`, hex digits, `Space` or `Enter`. Shows the character and its name while typing. | M | 1 |
| AX04-F043 | Pinyin (Simplified and Traditional) | Full and abbreviated pinyin, fuzzy syllables (z/zh, n/l and others, setting), phrase and sentence conversion over a lexicon with a bigram model, tone input optional, user phrases, Traditional output switch. | M | 2 |
| AX04-F044 | Double pinyin | Schemes selectable (natural code, Microsoft, Xiaohe). | C | 3 |
| AX04-F045 | Zhuyin (Bopomofo) | Standard, Eten, Hsu, IBM key maps; tone keys; phrase conversion. | S | 2 |
| AX04-F046 | Cangjie and Quick | Cangjie 3 and 5, Quick (Simplex); wildcard key; shows codes as annotation. | S | 3 |
| AX04-F047 | Wubi and stroke | Table engines through the generic table format (F060). | C | 3 |
| AX04-F048 | Cantonese | Jyutping table with phrase conversion. | C | 3 |
| AX04-F049 | Japanese kana-kanji | Romaji and direct kana input; hiragana, katakana, half-width katakana, wide Latin modes; segment conversion (lattice with word and connection costs), segment resize `Shift+Left/Right`, candidate per segment, prediction, `F6`–`F10` conversions; key maps: MS-IME style and ATOK style. | M | 2 |
| AX04-F050 | Japanese extras | Era dates, number forms, symbol words ("やじるし" → →), personal-name and place dictionaries as optional packs. | S | 3 |
| AX04-F051 | Korean hangul | Dubeolsik (2-set), Sebeolsik 390 and Final (3-set), romaja; automatic syllable composition by the Unicode algorithm; backspace by jamo; hanja key with candidates; Han/Eng key. | M | 2 |
| AX04-F052 | Indic transliteration | Phonetic Latin to Devanagari, Bengali, Gurmukhi, Gujarati, Odia, Tamil, Telugu, Kannada, Malayalam, Sinhala: ITRANS-style and a simpler natural scheme; word candidates from a lexicon. | M | 2 |
| AX04-F053 | Indic layouts | InScript and common phonetic layouts are plain layouts (no engine). | M | 1 |
| AX04-F054 | Vietnamese | Telex, VNI, VIQR; free tone placement; old and new tone-mark style; undo by repeating the key; spelling check to avoid converting English words. | M | 2 |
| AX04-F055 | Arabic and Hebrew | Layouts; optional engine for vowel marks (harakat, niqqud) by long sequences; Latin-to-Arabic and Latin-to-Hebrew transliteration; direction-mark keys (LRM, RLM) on the layouts. | M | 2 |
| AX04-F056 | Thai | Layouts (Kedmanee, Pattachote) with input-sequence checking in three strictness levels. | S | 2 |
| AX04-F057 | IPA | X-SAMPA and Kirshenbaum to IPA; a palette of IPA symbols grouped by place and manner in the picker. | S | 2 |
| AX04-F058 | Other transliterations | Cyrillic (Russian, Ukrainian, Serbian), Greek and polytonic Greek, Esperanto x-system, Ethiopic (Ge'ez), Georgian, Armenian. | S | 3 |
| AX04-F059 | Maths and technical symbols | Backslash names to symbols (`\alpha` → α, `\le` → ≤, `\to` → →) with a candidate list; own table. | S | 2 |
| AX04-F060 | Generic table engine | One ForgeScript engine driven by a table file: key sequences, outputs, weights, options (auto-commit, wildcard, max length). New table methods need no code. | M | 2 |
| AX04-F061 | Mixed-language typing | Engines pass through words that match the secondary language lexicon (for example English inside pinyin) instead of converting. | S | 3 |
| AX04-F062 | Full-width and half-width | Toggle for letters and punctuation in CJK engines; shown in the status. | M | 2 |
| AX04-F063 | Punctuation maps | Language punctuation (，。「」 and others) per engine with an editable map. | M | 2 |
| AX04-F064 | Emoji in engines | Engines may offer emoji candidates for words (setting). | C | 3 |
| AX04-F065 | Local model re-ranking | Optional: a small text model through `svc.model` re-ranks sentence candidates for pinyin and kana-kanji. The broker makes the call, not the engine. Off by default. | C | 3 |

### 4.4 Layouts and input sources

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX04-F066 | Layout data | The system's layout definitions (names, variants, key-to-symbol tables, dead keys, levels, alternates for AX03). One store used by the compositor, AX03 and this app. | M | 1 |
| AX04-F067 | Input source | A source is a layout plus an optional engine: "English (US)", "Chinese (Pinyin) on US", "Japanese (kana-kanji) on JIS". | M | 1 |
| AX04-F068 | Hand-off to SY01 | "Manage input sources…" opens SY01's Keyboard page; SY01's page calls this app's Add-source dialog and layout viewer. One list, stored in `svc.settings` `system.input.sources`. | M | 1 |
| AX04-F069 | Source switcher | `Super+Space` next, `Super+Shift+Space` previous; holding `Super` shows a popup list; release selects. Most-recently-used order option. | M | 1 |
| AX04-F070 | Direct source keys | `Super+Alt+1…9` jump to a source. | S | 2 |
| AX04-F071 | Panel indicator | SH02 shows the source code ("EN", "拼", "あ") and engine mode; click lists sources and engine actions. | M | 1 |
| AX04-F072 | Layout viewer | Shows the keyboard with all levels (plain, Shift, AltGr, Shift+AltGr), dead keys marked; follows held modifiers live; physical shapes ANSI, ISO, JIS, ABNT. | M | 1 |
| AX04-F073 | Print layout | Print the viewer through `svc.portal.print`. | C | 3 |
| AX04-F074 | Per-app source memory | With scope "per app", a table of bundle id to source, editable; "always use English in Terminal". | M | 2 |
| AX04-F075 | Per-field hints | A field may ask for Latin input (URL, e-mail); the broker switches engine mode for that field only and restores after. | S | 2 |
| AX04-F076 | New-window default | First source in the list, or the source of the previous window (setting). | M | 1 |
| AX04-F077 | Hardware layout mismatch | The viewer can show "what your keys print" for a chosen physical layout so users with unlabelled keys can check. | C | 3 |
| AX04-F078 | Custom layouts | User layout files in `fs.app`, validated, listed with an "unsigned, user" mark. | C | 3 |
| AX04-F079 | Login and lock screen | The source list and switcher work on the greeter and lock screen with plain layouts only; the active layout name is always shown there. Engines never run before login. | M | 1 |

### 4.5 Emoji and symbol picker

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX04-F080 | Open | `Super+.` or `Super+;` opens the picker at the caret; also from SH02's indicator and from AX03. | M | 1 |
| AX04-F081 | Search | Type to search names and keywords in the interface language and English; fuzzy match with `forge::text::fuzzy_filter`; first result is preselected. | M | 1 |
| AX04-F082 | Categories | Smileys, people, animals, food, travel, activities, objects, symbols, flags; tab bar; scroll is continuous across categories. | M | 1 |
| AX04-F083 | Recents and frequent | Last 50 used and a frequency-ranked row; stored locally; clearable; not recorded while incognito. | M | 1 |
| AX04-F084 | Skin tones | Default tone setting; long press or `Alt+Down` for the variants of one emoji; multi-person emoji get a two-tone chooser. | M | 2 |
| AX04-F085 | Gender and other variants | Shown in the same variant popup. | S | 2 |
| AX04-F086 | Emoji version | Data from the Unicode emoji files; emoji the system font cannot draw are hidden. | M | 1 |
| AX04-F087 | Kaomoji | A tab of text faces in groups; user-added entries. | S | 2 |
| AX04-F088 | Special characters | Tabs: punctuation, currency, maths, arrows, Latin extended, Greek, Cyrillic, IPA, box drawing, technical, superscripts and subscripts, invisible characters (shown with labels: NBSP, ZWJ, ZWNJ, LRM, RLM, soft hyphen). | M | 2 |
| AX04-F089 | Character details | Selected item shows name, code point, UTF-8 bytes, block, compose sequence if any, and the key on the current layout if any. | S | 2 |
| AX04-F090 | Insert | `Enter` or click commits into the focused field through the broker and closes; `Shift+Enter` inserts and stays open. | M | 1 |
| AX04-F091 | Clipboard insert | When no text field has focus, or by `Ctrl+C`, the item is copied through `svc.clipboard` and a toast says so. | M | 1 |
| AX04-F092 | Favourites | Pin items to a favourites tab; drag to reorder. | S | 2 |
| AX04-F093 | Keyboard navigation | Arrows in the grid, `Tab` between search, tabs and grid, `Ctrl+Tab` next tab, type anywhere to search. | M | 1 |
| AX04-F094 | Inline `:name:` completion | Typing `:smi` in a field offers emoji candidates (opt-in per app; uses the snippet path). | C | 3 |
| AX04-F095 | Hand-off to UT03 | "Show in Fonts and Characters" for the full character map by font. | S | 2 |
| AX04-F096 | Homoglyph warning | Inserting an invisible or direction-control character into a field with purpose URL or terminal shows a one-line warning in the picker. | S | 2 |

### 4.6 Text expansion and snippets

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX04-F097 | Abbreviations | An abbreviation plus a trigger (space, punctuation, `Tab`, or immediately) is replaced with its expansion. Matching is on the last typed word kept in the broker's memory; the buffer is at most 64 graphemes and is cleared on focus change. | M | 2 |
| AX04-F098 | Snippet picker | `Super+Alt+;` opens a searchable list; `Enter` inserts through the broker. | M | 2 |
| AX04-F099 | Placeholders | `{date}`, `{time}`, `{date:+7d:long}` (formats from `forge::locale`), `{clipboard}` (asks AX05 on use, with the normal paste rule), `{cursor}` (caret position after insert), `{input:Label}` (asks in a small form). | S | 2 |
| AX04-F100 | Multi-line and rich | Plain multi-line text; optional Markdown that rich fields receive as formatted text when the field accepts it. | S | 3 |
| AX04-F101 | Groups and per-app rules | Snippets in folders; a group can be limited to listed apps or excluded from them. | S | 2 |
| AX04-F102 | Case following | `brb` → "be right back", `Brb` → "Be right back", `BRB` → all capitals (per snippet option). | S | 2 |
| AX04-F103 | Undo expansion | `Backspace` or `Ctrl+Z` right after an expansion restores the abbreviation. | M | 2 |
| AX04-F104 | Never in sensitive fields | No expansion in password, PIN or sensitive fields or in private windows. | M | 2 |
| AX04-F105 | Import and export | CSV and JSON through the file portal; import shows a preview and conflicts. | S | 2 |
| AX04-F106 | Autocorrect list | A special group for typo fixes ("teh" → "the") per language, off by default. | C | 3 |
| AX04-F107 | Suggest snippets | Offer to make a snippet from text typed often; computed only from the visible dictionary's phrase counts; off by default. | C | 3 |

### 4.7 Learning, dictionary, prediction

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX04-F108 | Learning store | One local SQLite store per user: learned words, readings, counts, bigram counts, per language. Written only by the broker. | M | 2 |
| AX04-F109 | Learn rule | The broker accepts `learn(reading, word)` only if `word` equals text the engine committed in this composition, the field is not sensitive, and incognito is off. An engine cannot use learning as a hidden log. | M | 2 |
| AX04-F110 | Dictionary view | Every learned entry is listed: word, reading, language, count, first and last used, source engine. Sort, search, filter. | M | 2 |
| AX04-F111 | Delete | Delete one, many, by language, by time range (last hour, day, week), or everything. Deletion is real: rows removed and the store vacuumed. | M | 2 |
| AX04-F112 | Add and edit | Add words and readings by hand; edit; mark "never suggest". | M | 2 |
| AX04-F113 | Learning switch | Global, per language, per app; a quick toggle "incognito" in the panel indicator. | M | 2 |
| AX04-F114 | Never learn list | Apps that never feed learning: SE04, SE05, SY03 by default; editable. | M | 2 |
| AX04-F115 | Export and import | User dictionary as a text file (word, reading, count) through the file portal. | S | 2 |
| AX04-F116 | Decay | Counts halve every 90 days so old habits fade; entries at zero are removed. | S | 3 |
| AX04-F117 | Backup rule | The store is included in backups by default; a setting excludes it (`svc.backup` rules). | S | 2 |
| AX04-F118 | No sync by default | The dictionary never leaves the machine. Optional sync between own devices would use the suite's device-sync engine (owner OF02); off, Phase 3. | C | 3 |
| AX04-F119 | Prediction service | `text.predict(lang, context, stem, touch?) -> suggestions`, `text.learn`, `text.forget` for AX03 and for inline suggestions. Models: per-language word list and bigram counts shipped read-only, plus the user store. | M | 2 |
| AX04-F120 | Inline suggestions for physical keyboards | Optional suggestion strip at the caret while typing; `Tab` accepts. Off by default. | C | 3 |
| AX04-F121 | Language packs | Lexicons and tables are separate signed packages installed through SY05, so a language costs disk only when used. | M | 2 |

### 4.8 Settings, security, recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| AX04-F122 | Main window | Sources, engines, dictionary, snippets, compose, emoji settings, shortcuts, privacy in one window (section 5.2). | M | 1 |
| AX04-F123 | Engine facts panel | For each engine: signer, version, process state, memory, "Files: none. Network: none. Sees keys: only in text fields while active." with a link to AD05. | M | 1 |
| AX04-F124 | Active engine indicator | Whenever an engine is processing keys the SH02 indicator shows its mark. A plain layout shows only the language code. | M | 1 |
| AX04-F125 | Audit | Engine install, enable, disable, crash and learning erase are written to `svc.audit`. Keystrokes are never logged. | M | 1 |
| AX04-F126 | Safe mode | `Super+Alt+Space` held for 2 s turns all engines off and selects the first plain layout; works on the lock screen. | M | 1 |
| AX04-F127 | Reset | Reset shortcuts, engine settings, or everything; learning store untouched unless ticked. | S | 1 |
| AX04-F128 | Debug view | Developer panel: text-input state of the focused field (enabled, purpose, hints, caret rectangle), last 20 broker events without key values. | S | 2 |
| AX04-F129 | Help | Per-engine help pages with the key map, rendered by `markdownView`. | M | 2 |

## 5. GUI

### 5.1 Window map

```
Input Methods and Emoji (broker runs in the background; indicator in SH02)
├─ Main window
│   ├─ Input sources · Engines · Dictionary · Snippets · Compose · Emoji and symbols
│   ├─ Shortcuts · Privacy
│   └─ dialogs: Add input source · Engine settings · Add word · Erase learned data
│               · Snippet editor · Import/Export · Per-app sources
├─ Candidate window (popup at the caret; never focusable)
│   └─ expanded grid · candidate context menu
├─ Source switcher (popup, while Super is held)
├─ Emoji and symbol picker (popup window)
│   └─ variant popup · character details strip
├─ Snippet picker (popup)
├─ Snippet input form (small dialog for {input:…})
├─ Layout viewer (window)
├─ Unicode hex input hint (inline pre-edit + tooltip)
└─ Indicator menu (in SH02)
```

### 5.2 Main window

Shown at 960×620. Sidebar + content; page "Engines".

```
+------------------------------------------------------------------------------------------+
| [=] Input Methods — Engines                  [search Ctrl+K]             [▁] [▢] [✕]     | (1)
+---------------------+--------------------------------------------------------------------+
|   Input sources     |  Engine              Language        State     Signer     Memory   | (3)
| > Engines           | > Pinyin             Chinese (zh)    running   system ✓   38 MB    |
|   Dictionary        |   Kana-kanji         Japanese (ja)   idle      system ✓   —        |
|   Snippets          |   Hangul             Korean (ko)     idle      system ✓   —        |
|   Compose           |   Telex              Vietnamese      idle      system ✓   —        |
|   Emoji and symbols |   Maths symbols      —               off       system ✓   —        |
|   Shortcuts         |  [Add language pack…]  [Disable]  [Restart]                        |
|   Privacy           +--------------------------------------------------------------------+
|                     | Pinyin                                                         (4) |
| (2)                 |  Files: none      Network: none      Other apps: none              |
|                     |  Sees keys: only in text fields, only while it is the active engine|
|                     |  Never runs in: password fields, PIN fields, lock screen           |
|                     |  Learning: on · 1 204 learned phrases   [Open dictionary]          |
|                     |  [Show in Permissions…]                                            |
|                     +--------------------------------------------------------------------+
|                     | Settings                                                       (5) |
|                     |  Output            (•) Simplified ( ) Traditional                  |
|                     |  Fuzzy syllables   [x] z/zh [x] c/ch [x] s/sh [ ] n/l [ ] in/ing   |
|                     |  Candidates/page   [ 7 ]     Layout (•) horizontal ( ) vertical    |
|                     |  Chinese/English   [ Shift (alone) ]   Full/half width [Shift+Space]|
+---------------------+--------------------------------------------------------------------+
| Active: 拼 Pinyin on English (US) · window scope · learning on           3 engines idle  | (6)
+------------------------------------------------------------------------------------------+
```

1. Header bar. No `[net]` badge is possible.
2. Sidebar, 220 px, `listView`.
3. Engines `tableView`: state is running, idle (no process), off, slow, or crashed; signer shows the trust badge with a word.
4. Engine facts `panel` (F123). The lines are generated from the sandbox policy the broker applied, not from static text.
5. Engine settings, a `propertyGrid` built from the engine's declared settings; key fields use the key-capture field.
6. Status bar.

States: empty (no engine pack installed: "Plain layouts only. Add a language pack to type Chinese, Japanese, Korean and more." with `[Add language pack…]`); loading (pack verification: row shows a `progressBar`); error (engine failed signature: row in `--danger` with "tampered", cannot be enabled); offline (packs install from local media or the configured repository through SY05; this app never fetches); locked (window hidden); read-only (managed systems: sources set by policy show a lock and cannot be removed); narrow (sidebar becomes a `comboBox`; the facts panel moves under the list).

### 5.3 Other views and dialogs

**Candidate window, horizontal** (popup; pinyin).

```
  …我想去 bei jing da xue|                                   <- pre-edit in the app, underlined  (1)
  +--------------------------------------------------------------------------+
  | 1 北京大学  2 北京  3 背景  4 被  5 北  6 倍  7 杯        ‹ 1/9 ›  [⌄]    | (2)
  +--------------------------------------------------------------------------+
  | 拼 · 简 · 半                                                              | (3)
```

1. Pre-edit is drawn by the app's own text widget from the text-input pre-edit string and spans. 2. Candidate `panel` with `button` items; current item has an outline; `‹ ›` page; `[⌄]` expands to a grid. 3. Status strip: engine, script, width. States: no candidates (window hidden, pre-edit only); engine slow (strip shows "…"); annotation on (second line under each candidate).

**Candidate window, vertical with segments** (kana-kanji).

```
  きょうは[いい]てんきですね                 <- segments; [ ] = current segment          (1)
  +----------------------------------+
  | 1  良い                          |
  | 2  いい              ひらがな    | (2)
  | 3  好い                          |
  | 4  善い              learned ✕   | (3)
  | 5  イイ              カタカナ    |
  |                         2 / 3    |
  +----------------------------------+
```

1. Current segment is highlighted in the field; `Shift+Left/Right` resize it; `Left/Right` move between segments. 2. Annotation column. 3. A learned candidate shows a mark; `Ctrl+Delete` forgets it.

**Source switcher** (popup, centre of the output, while `Super` is held).

```
+----------------------------------------------+
|   EN   English (US)                          |
| > 拼   Chinese (Pinyin)                      |
|   あ   Japanese (kana-kanji)                 |
|   DE   German                                |
+----------------------------------------------+
|  Space next · Shift+Space previous · 1–9     |
+----------------------------------------------+
```

`listView` on a layer surface. Releasing `Super` selects. A quick tap of `Super+Space` (under 300 ms) switches without showing the popup.

**Emoji and symbol picker** (popup window, 420×460, at the caret).

```
+----------------------------------------------------------+
| [🔍 thumb_________________________________]   [skin ✋ v] | (1)
+----------------------------------------------------------+
| [🕘][★][😀][🐻][🍎][⚽][🚗][💡][🔣][🏁] | [(^_^)] [Ω] [ipa] | (2)
+----------------------------------------------------------+
| Results for “thumb”                                      |
| [👍] 👎  👍🏽  🤙  🖒  ✊                                    | (3)
|                                                          |
| Frequently used                                          |
|  😀  ❤️  ✅  🎉  🙏  👀  🔥  😂                           |
+----------------------------------------------------------+
| 👍 thumbs up · U+1F44D · :thumbsup:        Enter inserts | (4)
+----------------------------------------------------------+
```

1. Search `lineEdit` (has focus on open) and default skin tone `comboBox`. 2. `tabBar`: recents, favourites, emoji categories, then kaomoji, special characters, IPA. 3. `gridView`, virtualised, 36 px cells (44 px in touch density); the first result is preselected. 4. Details strip `label`. States: empty search result ("No match. Try the English name."), no text field focused (strip says "Enter copies to the clipboard"), font cannot draw emoji (banner; text-style symbols still listed).

**Dictionary page** (content area of the main window).

```
| Language [All v]  Source [All v]  [🔍 filter_______]      Learning [ON ]  Incognito [OFF] | (1)
|  Word          Reading        Lang   Count   Last used     Source                         |
| > 北京大学     beijingdaxue   zh     14      today 14:02   Pinyin                    (2) |
|   良い         いい           ja     3       yesterday     Kana-kanji                     |
|   Odin         —              en     22      today 09:40   Prediction                     |
|   teh → the    —              en     —       —             Autocorrect (user)             |
|  [Add word…] [Delete] [Never suggest] [Export…] [Import…]                                 |
|  Erase learned data: [Last hour] [Last day] [Last week] [Everything…]               (3)  |
|  1 204 entries · 0.4 MB · stored on this device only                                      |
```

1. Filters and the two learning switches (`comboBox`, `lineEdit`, `toggleSwitch`). 2. `tableView` over `SqliteModel` (PLATFORM V8); multi-select; `Delete` removes with an Undo toast. 3. Erase buttons; "Everything…" asks once ("Erase all 1 204 learned entries" / "Keep").

**Snippet editor** (dialog, 560×440).

```
+----------------------------------------------------------+
| Snippet                                                  |
+----------------------------------------------------------+
| Abbreviation  [;sig______]   Trigger [Space or punct. v] |
| Group         [Work      v]  [x] Follow case             |
| Expansion                                                |
| +------------------------------------------------------+ |
| | Kind regards,                                        | |
| | {input:Name}                                         | |
| | {date:long}                                          | |
| +------------------------------------------------------+ |
| Insert placeholder [date v] [time] [clipboard] [cursor]  |
| Apps  (•) all  ( ) only [________]  ( ) not in [______]  |
| Preview: Kind regards, ⟨Name⟩ 12 November 2026           |
|                              [Cancel]  [Save snippet]    |
+----------------------------------------------------------+
```

`lineEdit`, `comboBox`, `checkBox`, `codeEdit` (Plain mode) for the expansion, `radioButton`. Validation on blur: abbreviation unique within its app scope, at least 2 characters, no spaces.

**Layout viewer** (window, 900×360).

```
+------------------------------------------------------------------------------------------+
| [=] Layout — German (ISO)       Level: (•) live  ( ) plain ( ) Shift ( ) AltGr   [Print] |
+------------------------------------------------------------------------------------------+
| [^°′][1!¹][2"²][3§³][4$¼][5%½][6&¬][7/{][8([][9)]][0=}][ß?\][´`¸][   ⌫    ]              |
| [Tab  ][q @][w  ][e €][r  ][t  ][z  ][u  ][i  ][o  ][p  ][ü  ][+*~][ ↵   ]               |
| [Caps   ][a  ][s  ][d  ][f  ][g  ][h  ][j  ][k  ][l  ][ö  ][ä  ][#' ][   ]               |
| [⇧   ][<>|][y  ][x  ][c  ][v  ][b  ][n  ][m µ][,; ][.: ][-_ ][      ⇧      ]             |
| [Ctrl][Sup][Alt][                space                ][AltGr][Sup][Menu][Ctrl]          |
+------------------------------------------------------------------------------------------+
| ■ dead key   ■ compose key   Hover a key for its symbols and code points                 |
+------------------------------------------------------------------------------------------+
```

Keys are `button`s in a grid `panel` (not clickable for input; hover and focus show a `tooltip`). "Live" follows the modifiers physically held. Dead keys carry a mark and a word in the tooltip, not colour alone.

**Add input source** (dialog): language `listView` with search → layout and engine `listView` for that language with a preview of the layout → `[Add source]`. Missing packs show `[Install pack…]` (hand-off to SY05).

### 5.4 Menus

```
Main window [=] (F10)
├─ File ▸ Import dictionary… · Export dictionary… · Import snippets… · Export snippets…
│        · Close Ctrl+W · Quit window Ctrl+Q (the broker keeps running)
├─ Edit ▸ Undo Ctrl+Z · Redo Ctrl+Shift+Z · Delete Delete · Select all Ctrl+A · Find Ctrl+F
├─ View ▸ Layout viewer Super+Alt+L · Emoji picker Super+. · Debug view
├─ Sources ▸ Add input source… Ctrl+N · Manage in Settings… · Per-app sources…
├─ Engines ▸ Restart engine · Disable · Engine help F1 · Show in Permissions…
├─ Privacy ▸ Incognito typing · Erase learned data… · Never-learn apps…
└─ Help ▸ Help F1 · Compose sequences · About

Indicator menu (SH02)
├─ <input sources, current marked>
├─ <engine actions: Chinese/English · Full/half width · Simplified/Traditional …>
├─ Incognito typing                         toggle
├─ Emoji and symbols…                       Super+.
├─ Snippets…                                Super+Alt+;
├─ Layout viewer                            Super+Alt+L
└─ Input method settings…

Candidate context menu
├─ Forget this candidate                    Ctrl+Delete
├─ Pin to top
├─ Copy
└─ Look up in dictionary (emits intent)
```

### 5.5 Keyboard and pointer

| Keys | Action | Keys | Action |
|---|---|---|---|
| `Super+Space` / `Super+Shift+Space` | next, previous source | `Super+Alt+1…9` | source by number |
| `Super+.` or `Super+;` | emoji and symbol picker | `Super+Alt+;` | snippet picker |
| `Super+Alt+L` | layout viewer | `Ctrl+Shift+U` | Unicode hex input |
| `Compose` (chosen key) | start compose | `Super+Alt+Space` (2 s) | safe mode |
| `Space` / `Enter` | convert or pick first; commit | `Esc` | cancel composition |
| `1`–`9`, `0` | pick candidate | `PageUp/PageDown`, `-`/`=` | page |
| `Shift+Left/Right` | resize segment | `Left/Right` | move segment or candidate |
| `Ctrl+Delete` | forget candidate | `Tab` | expand candidate grid |

Keys in the lower half apply only while a composition is open, and belong to the active engine's key map. Global chords are registered with the shell; the broker receives them as actions.

Pointer: click a candidate to pick; scroll to page; right click for the candidate menu. In the picker: click inserts, right click gives Copy, Add to favourites, Show details, Show in Fonts and Characters. Drag and drop: favourites reorder; a snippet CSV or dictionary text file dropped on the main window opens the import preview.

### 5.6 States

| State | Behaviour |
|---|---|
| Plain layout active | No engine process; indicator shows the language code. |
| Engine active, idle | Engine process sleeps; zero wake-ups. |
| Composing | Pre-edit in the field; candidate window if the engine offers candidates. |
| Password or PIN field | Engines bypassed; indicator shows a lock beside the layout code (F011). |
| Sensitive field or private window | Engines run; learning, surrounding text and snippets off; indicator shows the incognito mark. |
| Engine slow or crashed | F016, F017; typing continues on the plain layout. |
| Client without text-input | Switcher marks engine sources unavailable for this window (F019). |
| Greeter and lock screen | Plain layouts only; layout name shown (F079). |
| Pack missing | Source listed with "pack not installed"; selecting it offers install. |
| Broker down | Compositor falls back to its direct key path; the session restarts the broker within 500 ms. |
| Offline | No effect. |

### 5.7 Widgets

| Region | Widget | Note |
|---|---|---|
| Sidebar, lists | `listView`, `tableView` (with `SqliteModel`) | in tree |
| Engine settings | `propertyGrid` | in tree |
| Candidate window | `panel` + `button` items, `label` annotations | popup surface role for input methods: AX04-GAP-2 |
| Emoji grid | `gridView`, `tabBar`, `lineEdit`, `comboBox` | colour emoji and CJK fallback: FUSE-WP-17 |
| Snippet expansion editor | `codeEdit` (Plain) | in tree |
| Layout viewer | grid `panel` of `button`s, `tooltip` | in tree |
| Help | `markdownView` | in tree |
| Key fields | key-capture field | AX01-GAP-3, FUSE-WP-14 |
| Pre-edit in every app | `lineEdit`, `codeEdit`, rich text | spans and caret rectangle: FUSE-WP-59, FUSE-WP-35 |

Gaps:

| ID | Gap | Evidence | Lands in |
|---|---|---|---|
| AX04-GAP-1 | Text-input on Wayland in FUSE: pre-edit with spans, commit, surrounding text, content purpose, caret rectangle | `text_services.hpp` backends are Win32 (IMM32) and X11 (XIM) only (`FUSE:Source/FUSE/Core/include/fuse/platform/text_services.hpp`) | G-01, FUSE-WP-59 |
| AX04-GAP-2 | Compositor side of input methods: an interface that hands text-field keys to one authorised broker, accepts pre-edit and commits from it, and gives it a popup surface tied to the caret rectangle | `textinput_v3.cppm` is a 36-line opcode table; no input-method module seen in `PBSD:pbsd/compositor/` | compositor |
| AX04-GAP-3 | Layout data and key-map compiler for the compositor (XKB keymap text is what Wayland clients expect) | none read | compositor, section 13 |
| AX04-GAP-4 | Emoji names and keywords per language (CLDR annotations) are not in FORGE; only emoji properties are (`FORGE:docs/text-locale.md` Data) | Verified absence in the listed data | this app (generated tables) |
| AX04-GAP-5 | Dictionary word breaking for Thai, Lao, Khmer, Burmese is absent in FORGE (`FORGE:docs/text-locale.md` Limits) | Verified | FORGE-WP-23 or this app's packs |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Input broker | C++23 | `input-broker` helper, capability mode, started at login (`bg.autostart`) | Text-input relay, source state per window, dead keys and compose (built in), snippet matching, panel commits, learn-rule check, engine supervision. Native because it sits in the typing path (speed) and handles the key stream (a secret): PLATFORM section 2 (b), (c) |
| Script engine host | C++23 host + ForgeScript engine code | one `engine-host` process per active script engine, capability mode, `Caps` none | Table, Telex, hangul, transliteration, maths, IPA engines |
| Native engines | C++23 | one process each (`engine-pinyin`, `engine-kana-kanji`), capability mode | Lattice search over large lexicons |
| Dictionary store | C++23 (inside the broker) | `input-broker` | The only writer of the learning store |
| Prediction module | C++23 (inside the broker) | `input-broker` | `text.predict` for AX03 |
| App logic | ForgeScript | app host | Main window, picker, snippet editor, layout viewer, candidate window view model |
| UI | FUSE UI | app host | All surfaces, including the candidate window |

```
 keyboard ─▶ compositor ──(keys of the focused text field)──▶ input-broker ◀──▶ engine process
                 ▲                                              │   ▲            (no files,
                 │ pre-edit, commit, delete-surrounding         │   │             no network)
                 └──────────────────────────────────────────────┘   │
 focused app ◀── text-input events                                  │ read-only lexicon fds
                                                                    │
 app host (ForgeScript + FUSE UI) ◀── candidates, status ───────────┤
   candidate window · picker · snippets · settings ── panel commit ─┘
 AX03 / AX06 / AX05 ── panel commit, text.predict ──▶ input-broker ──▶ learning store (SQLite)
```

Why engines cannot leak keys: an engine process holds exactly one socket (to the broker) and read-only descriptors. Capability mode removes every global namespace (PLATFORM V13, G-06). Its only outputs are pre-edit, candidates, commits and `learn` requests, all of which the user can see (in the field, in the candidate window, in the dictionary view). The broker rejects `learn` records that do not match committed text (F109).

### 6.2 Data model

Stores: `svc.settings` namespaces `org.pbsd.input-methods` and `system.input`; `~/.local/share/org.pbsd.input-methods/learn.db` (SQLite, `schema_version`); `snippets.db`; `~/.config/org.pbsd.input-methods/Compose` (user compose table); read-only packs under the verified system dataset.

| Entity | Fields |
|---|---|
| `input_source` | `id: string` (for example `zh-pinyin@us`), `layout_id: string`, `variant: string?`, `engine_id: string?`, `label: string`, `short: string`, `order: u16` |
| `window_state` (memory) | `window_token: bytes16`, `bundle_id: string`, `source_id: string`, `engine_mode: json`, `composing: bool` |
| `app_source` | `bundle_id: string`, `source_id: string` |
| `engine` | `id: string`, `name: string`, `kind: enum{builtin, script, native}`, `langs: string[]`, `version: string`, `signed_by: string`, `blake3: bytes32`, `enabled: bool`, `settings: json`, `hotkeys: json` |
| `learned` | `id: i64`, `lang: string`, `word: string`, `reading: string?`, `count: u32`, `first_used: i64`, `last_used: i64`, `engine_id: string`, `never_suggest: bool`, `user_added: bool` |
| `bigram` | `lang: string`, `w1: i64`, `w2: i64`, `count: u32`, `last_used: i64` |
| `snippet` | `id: uuid`, `abbrev: string`, `expansion: string`, `format: enum{plain, markdown}`, `trigger: enum{space_punct, tab, immediate}`, `group_id: uuid`, `follow_case: bool`, `apps_mode: enum{all, only, except}`, `apps: string[]`, `uses: u32`, `last_used: i64` |
| `snippet_group` | `id: uuid`, `name: string`, `enabled: bool` |
| `emoji_use` | `sequence: string`, `count: u32`, `last_used: i64`, `favourite: bool`, `fav_order: u16` |
| `emoji_tone` | `base: string`, `tone: u8` |
| `compose_entry` (parsed) | `keys: u32[]`, `result: string`, `source: enum{system, user}` |
| `candidate` (wire) | `text: string`, `label: string`, `annotation: string?`, `flags: u8` (learned, emoji, pinned) |
| `preedit_span` (wire) | `start: u32`, `len: u32`, `style: enum{plain, underline, current, converted, error}` |

Pack formats: `lexicon.bin` (sorted double-array trie plus cost tables, memory-mapped read-only, BLAKE3 in the pack manifest), `table.tbl` (generic table engine text), `layout/*.layout` (layout definitions). Limits enforced by the broker on engine output: pre-edit at most 1 024 bytes, commit at most 4 096 bytes, at most 256 candidates per page message of at most 256 bytes each. Migration is forward only with a copy of the old store.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Text-input interface to the compositor | yes | yes | own code on the compositor's protocol (AX04-GAP-2) | input broker |
| Engine API records | yes | yes | schema compiler output (G-08) | broker and engine, each validating the other's records |
| Compose files (X11 Compose text format) | yes | user file only | own parser | input broker (user file is untrusted: size cap 1 MiB, 20 000 entries) |
| Layout definitions | yes | custom only | own format and parser; or XKB text (section 13) | compositor helper; app host for the viewer |
| Lexicon packs (`lexicon.bin`) | yes | no (built offline by a pack tool) | own code | engine process (signed data, still bounds-checked) |
| Table engine files (`.tbl`) | yes | no | own parser | script engine host |
| Emoji data (`emoji-test.txt`-derived table, annotation tables) | yes | no | generated at build time | app host |
| Snippet import and export (CSV, JSON) | yes | yes | `forge::json`, own CSV reader | app host (ForgeScript) |
| Dictionary import and export (UTF-8 text, tab-separated) | yes | yes | own code | app host; rows pass the broker's validation |
| Legacy encodings in imported dictionaries (GBK, Big5, Shift_JIS, EUC-KR) | yes | no | `forge::text::decode` (Verified, `FORGE:docs/text-locale.md`) | app host |
| `text.predict` interface | yes | yes | schema compiler output | input broker |

### 6.4 Integration

- Intents: handles `pick` for type `text/x-emoji` (an app asks for one emoji or symbol); emits `search` (dictionary look-up of a candidate), `open` (SY01 Keyboard page, SY05 language packs, AD05).
- Services provided: `input-method.panel` (commit text into the focused field; used by AX03, AX05, AX06), `text.predict`, `input.sources` (list, current, set).
- Services used: `svc.settings`, `svc.notify`, `svc.audit`, `svc.clipboard` (picker copy; write only), `svc.model` (optional re-ranking), `svc.a11y:track` (fallback caret position), `svc.portal.file`, `svc.portal.print`, `svc.backup` (rules).
- Other apps: SY01 (source list and keyboard options), SH02 (indicator), SH03 (greeter and lock layouts), SY05 (packs), AX03 (layouts, prediction, panel), AX06 (dictation commits through the panel), AX05 (`{clipboard}` placeholder, quick paste), UT03 (character map), AX01 (candidate reading), AD05 (engine rights view).
- Verified in repos: FUSE has a text-input session API (`beginTextInput`, `endTextInput`, `setTextInputArea`) with Win32 IMM32 and X11 XIM backends, and delivers `TextInput` and `TextComposition` events with a composition caret (`FUSE:Source/FUSE/Core/include/fuse/platform/text_services.hpp`, `events_v2.hpp`, `FUSE:Source/FUSE/UI/Core/include/fuse/ui/ui.hpp`); `fuse::ui::syncTextInput` starts and stops the IME and places the candidate window (`FUSE:docs/ui/README.md`). FORGE has normalisation, grapheme, word and sentence segmentation, script and emoji properties, fuzzy matching, capped regex, legacy encodings and locale date formatting, with Unicode 16.0 and CLDR 46 data compiled in (`FORGE:docs/text-locale.md`). The compositor tree has a `textinput_v3` module naming `zwp_text_input_manager_v3` and `zwp_text_input_v3` (`PBSD:pbsd/compositor/pbsd.compositor.wayland.textinput_v3.cppm`). Everything else here is Proposed.

## 7. Security and privacy

### 7.1 Trust model

An input method sees every character the user types into text fields. On other systems it is an ordinary process with files and network: a perfect keylogger if it turns hostile. Here the access is split and narrowed.

| Party | What it can reach | How it is limited |
|---|---|---|
| Input broker (system-signed, part of this bundle) | Keys of the focused text field; surrounding text the field offers; commit path | Holds the single right `svc.a11y:textinput.broker`, granted to exactly one bundle per session. `net.none`. Writes only its own stores. In the trusted base for typed text; kept small; T2 target. |
| Engine process | Keys the broker forwards while it is the active engine and the field is not a password or PIN field | Capability mode, one socket, read-only data. No files, network, bus, clipboard or clock sources beyond monotonic time. Output bounded and visible. |
| Third-party engine | The same as any engine | Manifest may request no capability; install shows this; signer shown. |
| App host (windows, picker) | Candidates and status to draw; snippets and dictionary through the broker | Does not see raw keys. Picker and snippet inserts are panel commits. |
| Panels (AX03, AX05, AX06) | May commit text into the focused field | Named in the broker's allow list by bundle id and signature; each needs the user-granted `svc.input-method` service right; commits refused if the field did not enable text input or is on a secure surface (only the trusted AX03 instance may type there). |

Rights are granted in SY01 through an SH04 prompt, listed and revocable in AD05, and logged in `svc.audit` like other assistive rights (AX01 section 7.1). The broker has no `tree.read`, no `act`, no `input.synth`: it cannot read or drive other apps' UI beyond the text-input contract a field opts into.

Residual channels and how they are treated: an engine could try to signal through timing or through which candidates it shows; neither reaches outside the machine because no party in the chain has a network capability. An engine could commit text the user did not intend; that is visible in the field and limited to 4 096 bytes per commit.

### 7.2 Capabilities requested

| Capability | Why |
|---|---|
| `fs.app` | Learning store, snippets, user compose table, custom layouts |
| `fs.portal` | Import and export |
| `fs.path:/usr/local/share/pbsd/input:r` | Read-only system packs (lexicons, tables, layouts); path Proposed |
| `net.none` | No network use, for the broker and for every engine |
| `proc.helper:input-broker`, `proc.helper:engine-host`, `proc.helper:engine-pinyin`, `proc.helper:engine-kana-kanji` | Broker and engines as separate processes |
| `svc.a11y` (`textinput.broker`, `track`) | Text-field key routing; fallback caret rectangle |
| `svc.settings`, `svc.notify`, `svc.audit` | Settings, toasts, audit of engine events |
| `svc.clipboard` | Picker "copy" when no field has focus (write only) |
| `svc.model`, `ai.local` | Optional candidate re-ranking; off by default |
| `dev.print` | Print the layout viewer |
| `bg.run`, `bg.autostart` | The broker must run for the whole session |

### 7.3 Threat model

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| Typed text | Malicious or compromised engine | Log keys to disk or network | Engine sandbox: no files, no network (F003) |
| Typed text | Engine | Hide a log in the learning store | Learn rule (F109); every entry visible and erasable |
| Passwords | Any engine | See password keys | Engines bypassed in password and PIN fields (F011); never run on greeter or lock screen (F079) |
| Sensitive text | Learning | Private words resurface as suggestions | Sensitive bypass (F012), incognito, never-learn apps (F114), time-range erase (F111) |
| Other apps | Rogue panel client | Inject text | Panel allow list and service right; refused on secure surfaces |
| The user | Malicious app | Draw a fake candidate window to mislead | Low impact; the real window is a compositor popup tied to the caret; noted |
| Broker | Hostile engine output | Oversized or malformed records | Bounds in section 6.2; schema validation; fuzzing |
| Broker | Hostile user Compose or import file | Parser errors | Size caps; fuzzing; T2 parser target |
| URLs and commands | The user's own input | Invisible or look-alike characters pasted from the picker | Warning in the picker for URL and terminal fields (F096); FORGE has no spoof checking (`FORGE:docs/text-locale.md` Limits), so no automatic detection is promised |
| Snippets | Shoulder attack on stored data | Snippets contain addresses or account numbers | Stored in the user's encrypted home; a note in the editor says not to store secrets (use SE04) |
| Supply chain | Tampered language pack | Altered lexicon | Signed packs verified by SY05; BLAKE3 checked at load |
| Availability | Buggy engine | Typing stalls | Time limit and fallback (F016, F017); safe mode (F126) |

Stored: learned words and counts, snippets, emoji recents, settings. Not stored: keystrokes, composition history, surrounding text. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `system.input.sources` | list<input_source> | from the installer's language choice | Enabled sources (edited in SY01 and here) |
| `scope` | enum{window, app, global} | `window` | Where the current source is remembered |
| `new_window_source` | enum{first, previous} | `first` | |
| `switcher.mru` | bool | false | Most-recently-used order |
| `switcher.popup_delay_ms` | u16 | 300 | Tap shorter than this switches silently |
| `compose.key` | enum{ralt, menu, caps, rctrl, none} | `none` | Compose key |
| `compose.invalid` | enum{beep, literal, drop} | `literal` | |
| `unicode_hex.enabled` | bool | true | `Ctrl+Shift+U` |
| `candidates.font_percent` | u16 | 100 | 80–300 |
| `candidates.layout` | enum{engine, horizontal, vertical} | `engine` | |
| `candidates.per_page` | u8 | 7 | 5–10 |
| `candidates.select_keys` | enum{digits, home_row} | `digits` | |
| `focus_loss` | enum{engine, commit, discard} | `engine` | Composition on focus loss |
| `learning.enabled` | bool | true | Global learning |
| `learning.per_lang` | map<string, bool> | all true | |
| `learning.never_apps` | string[] | SE04, SE05, SY03 bundle ids | |
| `learning.decay_days` | u16 | 90 | 0 = never |
| `learning.backup` | bool | true | Include in backups |
| `incognito` | bool | false | Session toggle; not persisted across login |
| `snippets.enabled` | bool | true | |
| `snippets.default_trigger` | enum | `space_punct` | |
| `emoji.skin_tone` | u8 | 0 (none) | Default tone |
| `emoji.recents_max` | u8 | 50 | 0 = keep none |
| `emoji.inline_colon` | bool | false | F094 |
| `predict.inline` | bool | false | F120 |
| `predict.model_rerank` | bool | false | F065 |
| `engine.<id>.*` | per engine | per engine | Declared by each engine |
| `engine.time_limit_ms` | u16 | 100 | F017 |

## 9. Accessibility and languages

- This app is the language feature of the suite; its own interface is translated first.
- Candidate lists are exposed as lists with position ("2 of 7"); annotations give homophone descriptions for AX01.
- The picker is fully keyboard driven; every emoji has a name in the interface language (falls back to English).
- Pre-edit must be perceivable without colour: spans use underline styles (single, thick, dotted) as well as colour.
- Right-to-left: candidate order and picker tabs mirror; direction marks are insertable and shown labelled.
- CJK glyph variants follow the source language, not the interface language (FUSE-WP-17).
- Vertical text fields: the candidate window goes to the left of the column.
- Large candidate font (up to 300%) for low vision; the window follows AX02 tracking as a popup.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Added latency per key, plain layout | 0 (broker not in the path) |
| Added latency per key, dead keys and compose | under 1 ms |
| Key to pre-edit update, script engines | under 5 ms |
| Key to candidates, pinyin sentence of 20 syllables | under 20 ms at the 95th percentile |
| Kana-kanji conversion of a 40-kana clause | under 30 ms |
| Source switch | under 30 ms warm; engine cold start under 150 ms |
| Picker open to first frame | under 100 ms |
| Emoji search per keystroke | under 5 ms |
| Broker memory | under 15 MB |
| Engine memory | pinyin under 60 MB, kana-kanji under 80 MB (mapped lexicons shared) |
| Idle | zero wake-ups in the broker and every engine |
| Learning store size | capped at 50 MB; oldest low-count rows pruned |

## 11. Testing and proof

Nothing has run. All of this is plan.

- Unit (ForgeScript and C++): compose and dead-key tables; hangul composition against the Unicode algorithm; Telex, VNI rules over a word list; transliteration schemes round trip; table engine; snippet matching, case following and placeholders; learn-rule checks; source scope logic.
- Golden: candidate window and picker renders in each theme and script; pre-edit span rendering in `lineEdit` and `codeEdit`.
- Scripted UI: a fixture app with normal, password, PIN, URL and sensitive fields; type through each engine; assert committed text, that no engine process received keys for password fields, and that nothing was learned from sensitive fields.
- Conversion quality: fixed corpora per language with expected first candidates; tracked as a score, not pass or fail.
- Sandbox tests: an engine that tries to open a file, a socket, the bus; an engine that floods output; an engine that never answers; an engine that sends `learn` for text it did not commit.
- Fuzz: engine API records in both directions; Compose parser; table and layout parsers; lexicon loader; import files.
- Differential: compose results against the libX11 Compose table for the same sequences; hangul against Unicode normalisation (NFC of jamo sequences via `forge::text::normalize`).
- Performance: latency traces per engine on reference hardware.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Engine sandbox policy | An engine process holds no descriptor other than its broker socket and read-only pack files, and is in capability mode before the first key | T1 | Proof over the launcher's descriptor table and policy (FORGE-WP-08, FORGE-WP-27) | Depends on those work packages |
| Password bypass | No key event of a field with purpose password or PIN is ever sent to an engine | T1 | State-machine model of broker routing; refinement by PRISM | — |
| Learn rule | Every row written to `learned` equals a committed string of the same composition, from a non-sensitive field, with learning on | T1 | Invariant proof on the broker's store path | — |
| Broker record parsers and bounds | No runtime errors on any engine or client input | T2 | PRISM bounded checks; fuzzing | — |
| Compose, layout, table parsers | No runtime errors | T2 | PRISM; fuzzing | — |
| Native engines | No runtime errors on any key sequence and any signed lexicon | T2 | PRISM on the lattice code; fuzzing | Size of the search code |
| Script engines | Memory safety by the interpreter; no capabilities | T3 | Interpreter guarantees; sandbox tests | FORGE-WP-01 |
| App logic (ForgeScript) | Memory safety by the interpreter | T3 | Unit tests | FORGE-WP-01 |
| Conversion quality | — | T4 | Corpus scores | Not formal |

## 12. Build plan

### Phase 1: usable core

| ID | Work item | Depends on |
|---|---|---|
| AX04-W01 | Agree the compositor's input-method interface and the broker right | AX04-GAP-2 |
| AX04-W02 | Text-input in FUSE on Wayland (pre-edit spans, commit, purpose, caret rectangle) | AX04-GAP-1, G-01, FUSE-WP-59 |
| AX04-W03 | Input broker: relay, per-window source state, focus rules, password bypass | AX04-W01, FORGE-WP-27, FORGE-WP-28 |
| AX04-W04 | Layout data store, source list shared with SY01, switcher popup, indicator | AX04-GAP-3, SY01, SH02 |
| AX04-W05 | Dead keys, compose (system and user tables), Unicode hex input | AX04-W03 |
| AX04-W06 | Engine API, engine launcher, sandbox, time limits, crash handling | AX04-W03, G-06, FORGE-WP-04 |
| AX04-W07 | Script engine host and one reference engine (Telex) to prove the API | AX04-W06 |
| AX04-W08 | Candidate window | AX04-W06, FUSE-WP-07 |
| AX04-W09 | Emoji and symbol picker: data tables, search, recents, insert and copy | AX04-GAP-4, FUSE-WP-17, AX04-W10 |
| AX04-W10 | Panel connection for the picker, AX03 and AX06 | AX04-W03 |
| AX04-W11 | Main window: sources, engines with facts panel, shortcuts | G-02, G-07 |
| AX04-W12 | Layout viewer | AX04-W04 |
| AX04-W13 | Greeter and lock screen layouts; safe mode | SH03 |

### Phase 2: full feature

| ID | Work item | Depends on |
|---|---|---|
| AX04-W14 | Learning store, learn rule, dictionary page, erase | AX04-W03 |
| AX04-W15 | Lexicon pack format and offline pack tool; language packs through SY05 | SY05 |
| AX04-W16 | Pinyin engine | AX04-W06, AX04-W15, lexicon licence decision (section 13) |
| AX04-W17 | Kana-kanji engine | AX04-W06, AX04-W15, lexicon licence decision |
| AX04-W18 | Hangul engine with hanja | AX04-W07 |
| AX04-W19 | Zhuyin engine | AX04-W16 |
| AX04-W20 | Indic transliteration engines | AX04-W07, G-03 |
| AX04-W21 | Vietnamese VNI and VIQR; Arabic and Hebrew helpers; Thai sequence check; IPA; maths symbols | AX04-W07 |
| AX04-W22 | Generic table engine | AX04-W07 |
| AX04-W23 | Snippets: store, matching, picker, editor, placeholders, import and export | AX04-W10, AX05 |
| AX04-W24 | Prediction service for AX03 | AX04-W14 |
| AX04-W25 | Surrounding text, per-field hints, per-app sources, engine hot keys | AX04-W02 |
| AX04-W26 | Picker: skin tones, kaomoji, special characters, favourites, details | AX04-W09 |
| AX04-W27 | Compose browser, engine help pages, debug view | AX04-W11 |

### Phase 3: advanced

| ID | Work item | Depends on |
|---|---|---|
| AX04-W28 | Cangjie, Quick, Wubi, stroke, Cantonese, double pinyin | AX04-W22 |
| AX04-W29 | Other transliterations (Cyrillic, Greek, Esperanto, Ethiopic and others) | AX04-W22 |
| AX04-W30 | Reconversion; mixed-language typing; Japanese extras | AX04-W25 |
| AX04-W31 | Third-party engine bundles | SY05, AX04-W06 |
| AX04-W32 | Model re-ranking through `svc.model` | G-15, FORGE-WP-30 |
| AX04-W33 | Inline suggestions, `:name:` emoji, autocorrect list, snippet suggestions | AX04-W24 |
| AX04-W34 | Custom layouts, print layout, hardware mismatch view | AX04-W12, G-16 |
| AX04-W35 | Dictionary sync between own devices | OF02 sync engine |

## 13. Open questions and risks

1. **Lexicon licences.** Good pinyin and kana-kanji conversion needs large word lists with frequencies and, for Japanese, connection costs. No source has been chosen or checked here. Each candidate source's licence must allow redistribution with an AGPL-3.0-or-later system and must be recorded in the pack. This is the largest risk to Phase 2.
2. **Layout format.** Wayland clients receive key maps as XKB keymap text, so the compositor needs a producer of that text. Options: (a) own layout format and own keymap writer; (b) ship the xkeyboard-config data (permissive licences) and compile it with libxkbcommon (MIT, C) inside a sandboxed helper; (c) own compiler that reads xkeyboard-config data. Option (b) is fastest; (c) fits the safe-subset rule. Owner to decide. Until then AX04-GAP-3 blocks W04.
3. **Broker in the trusted base.** The broker sees typed text outside password fields. It is native and small by design, but it is trusted code. Alternative: put dead keys and compose in the compositor and start the broker only when an engine source is active. This spec already keeps the broker out of the path for plain layouts (F018).
4. **Compositor module state.** The `textinput_v3` module lists `CommitString`, `SetPreeditString` and `DeleteSurroundingText` as requests on the text-input object; upstream sends those as events to the client. The compositor side needs a design pass before W01 can close.
5. **Engines in ForgeScript: speed.** Script engines must answer in a few milliseconds per key. FORGE-WP-18 (interpreter speed without JIT) decides whether hangul and Telex can stay in script or must become table data driven by native code.
6. **Thai, Lao, Khmer, Burmese.** Word-level features (prediction, snippets by word) need dictionary breaking that FORGE lacks.
7. **Snippet buffer.** The 64-grapheme match buffer is a short in-memory record of typing. It is cleared on focus change and never stored, but it exists; an alternative is to match only on surrounding text the field offers. To review.
8. **Emoji font.** A colour emoji font with a licence that allows bundling must be chosen (FUSE-WP-17 dependency).
9. **Apps outside FUSE.** Compatibility-layer apps (VM03) may speak other IME protocols; bridging them is not planned.
