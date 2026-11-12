<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Printers and Scanners

By Odin Loch

| | |
|---|---|
| ID | `UT06` (from CATALOGUE.md) |
| Bundle | `org.pbsd.print-scan` |
| Category | utils |
| Priority | P2 |
| Backing repos | None for the print path: the base tree has only the classic BSD spooler at `hbsd/src/usr.sbin/lpr` (Verified present; not used). FUSE (UI). FORGE (network stack FORGE-WP-21, `svc.model` for OCR FORGE-WP-29..31: Proposed use) |
| Status | Planned. Nothing built. |

## 1. Purpose

Printers and Scanners sets up printers and scanners, shows queues and jobs, and scans paper to searchable PDF. It is also the specification of the print and scan pipeline behind it (platform gap G-16): PDF is the spool format, printers are driven without vendor drivers over IPP, and scanners over eSCL. Printers are untrusted network devices, so each one is spoken to by a contained backend that can reach that device and nothing else. It replaces the Plasma printer module, print-manager and Skanlite/Skanpage (PLATFORM.md V20).

## 2. Scope

In scope:

- Printer discovery (DNS-SD for IPP Everywhere and AirPrint-class devices, IPP-USB, manual address) and setup.
- Queues and jobs: view, hold, release, reorder, cancel, reprint, history.
- Print options and presets; printer defaults; capabilities read from the device.
- The spooler service (`svc.print`, Proposed) with PDF as the only spool format; print-to-PDF as a built-in printer.
- The contract between apps, the print dialog in SH04, and the spooler.
- Scanner discovery and scanning: eSCL (AirScan) over network and USB, WSD-Scan (could), legacy USB scanners through imported backends (stated below).
- Scan to PDF, PDF/A, PNG, JPEG, TIFF; multi-page, feeder, duplex; clean-up; OCR to a text layer.
- Colour profiles for printers and scanners.
- Supply levels, device status, troubleshooting, test pages.
- Network exposure rules for discovery and for sharing.

Out of scope:

| Item | Owner |
|---|---|
| Drawing the print dialog and the "pick a scanner" prompt | SH04 Portals and Prompts (this spec defines the contract and the option model it shows) |
| Producing the PDF of a document | each app, through the FUSE page layout and PDF writer (FUSE-WP-36, G-16) |
| Viewing, annotating, signing PDFs | OF03 Document Viewer |
| Photographing documents with a camera | ME04 Camera |
| OCR of screenshots | ME05 Screen Capture (shares the OCR service) |
| Firewall rules themselves | SE02 Firewall (this app requests and displays them) |
| USB device authorisation policy | VM05 Devices |
| Display colour calibration and the system colour-management settings | SY01 Settings |
| Fax | not planned |
| 3D printers, label-printer command languages | not planned (label printers that speak IPP work) |
| Printer firmware updates | VM05 Devices (shown as a link) |

## 3. Users and top tasks

Personas:

- **Home user**: one network printer-scanner; wants it to appear and work.
- **Office user**: several printers, held jobs with a PIN, duplex and stapling.
- **Archivist**: scans stacks of paper to searchable PDF/A.
- **Admin**: defines printers for all users, restricts discovery, shares a USB printer.

| # | Task | Target steps |
|---|---|---|
| T1 | Add a network printer that announces itself | 2 (open app, click Add on the found device) |
| T2 | Print from any app to the default printer | 2 (`Ctrl+P`, Print) |
| T3 | Print double-sided, two pages per sheet | 4 |
| T4 | See why a job is not printing | 1 (job row shows the device's reason) |
| T5 | Cancel a job | 1 |
| T6 | Scan one page to PDF | 2 (Scan, Save) |
| T7 | Scan a 20-page stack, both sides, to a searchable PDF | 4 |
| T8 | Add a page to a scan and reorder pages | 2 |
| T9 | Print to PDF | 2 |
| T10 | Check ink or toner | 1 |
| T11 | Add a printer by address | 4 |
| T12 | Reprint yesterday's job | 2 |
| T13 | Print a test page | 2 |

## 4. Features

### 4.1 Printer discovery and setup

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT06-F001 | DNS-SD discovery | Browse `_ipp._tcp`, `_ipps._tcp` (and subtypes `_print`, `_universal`) on the local link through the system discovery service. Found devices list name, make and model, location, address, security (TLS or not), driverless standard (IPP Everywhere, AirPrint-class, Mopria-class). | M | 1 |
| UT06-F002 | Discovery only by capability | The app holds no multicast right by default. Discovery runs when the user opens "Add printer" or enables "Watch for printers"; it is done by `svc.discovery` (Proposed) and shown by the `--net` badge. | M | 1 |
| UT06-F003 | IPP-USB | USB printers that expose the IPP-over-USB interface class are driven by the same IPP backend through `dev.usb:printer`. | M | 2 |
| UT06-F004 | Manual add | Enter host, port and path or a full `ipp://` / `ipps://` address; the backend queries `Get-Printer-Attributes` and shows what it found. | M | 1 |
| UT06-F005 | Capability read | From IPP attributes: document formats, media sizes and types, trays, margins, duplex, colour modes, resolutions, quality, finishing, copies limit, job-password support. No PPD files, ever. | M | 1 |
| UT06-F006 | Format choice | Order of preference when sending: `application/pdf` (if the device accepts it and "trust device PDF" is on), `image/pwg-raster`, `image/urf`, `application/PCLm`. Default is raster produced locally, so the printer's PDF interpreter is not exposed to document content. | M | 1 |
| UT06-F007 | TLS and identity | `ipps` preferred. The printer's certificate is pinned on first use; a changed certificate is a `--danger` state that blocks printing until the user accepts or removes the printer. Plain `ipp` shows a `--warn` "not encrypted on the network". | M | 1 |
| UT06-F008 | Authentication | HTTP Basic or Digest over TLS only; credentials in `svc.keyring`. Refused over plain `ipp` unless the user overrides per printer. | S | 2 |
| UT06-F009 | Name, location, icon | Editable local name and location; duplicate names are prevented. | M | 1 |
| UT06-F010 | Default printer | One default per user; optional "last used". | M | 1 |
| UT06-F011 | Printer defaults | Per printer: paper, tray, sides, colour, quality. | M | 1 |
| UT06-F012 | Test page | A bundled PDF test page (alignment, colour bars, margins, the queue name, time). | M | 1 |
| UT06-F013 | Remove and disable | Disable keeps the entry and holds jobs; remove deletes it and its pinned certificate. | M | 1 |
| UT06-F014 | System-wide printers | An admin can define printers for all users (SH04 admin prompt); users see them read-only. | S | 2 |
| UT06-F015 | Legacy printers | Printers with no driverless standard (old PCL, PostScript or vendor-raster devices; LPD and raw port 9100 endpoints) are listed as "needs a legacy backend". A generic PCL 5/6 and a generic PostScript backend are could-haves; vendor raster languages are not planned. Stated plainly in the UI. | C | 3 |
| UT06-F016 | Printer groups | A named set of equivalent printers; a job goes to the first idle one. | C | 3 |
| UT06-F017 | Temporary printers | "Print once to a found printer" from the print dialog without saving it. | S | 2 |
| UT06-F018 | Detail page | All attributes the device reported, readable and copyable, for support. | S | 2 |
| UT06-F019 | Web interface link | The device's admin page address is shown as text with "Open in Web Browser" (IN01). Not fetched by this app. | S | 2 |
| UT06-F020 | Identify | Ask the device to beep or flash (IPP `Identify-Printer`) to tell two devices apart. | C | 2 |

### 4.2 Queues and jobs

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT06-F030 | Job list | Per printer and combined: document name, app, pages, copies, state, reason, submitted, size. Live. | M | 1 |
| UT06-F031 | Job states | Pending, held, processing (rendering), sending, printing (device reports), completed, cancelled, aborted, with the device's reasons in words ("out of paper in tray 2", "door open", "waiting for PIN at the printer"). | M | 1 |
| UT06-F032 | Cancel, hold, release | Per job or all. Cancel also sends `Cancel-Job` to the device. | M | 1 |
| UT06-F033 | Reorder and move | Change order in a queue; move a pending job to another printer (options re-validated). | S | 2 |
| UT06-F034 | Pause a printer | Pause and resume a queue; jobs wait. | M | 1 |
| UT06-F035 | Job detail | Options used, format sent, bytes, times, device job id, backend log lines for this job. | S | 2 |
| UT06-F036 | Notifications | Done, failed, needs attention, through `svc.notify`, with "Show queue" and "Cancel". Progress badge on the app. | M | 1 |
| UT06-F037 | History | Completed jobs kept as metadata (name, app, pages, time, printer) for `history.days`. Document content is kept only if "keep for reprint" is on. | S | 2 |
| UT06-F038 | Reprint | Reprint a kept job with the same or changed options. Kept spool files are encrypted at rest with a per-user key from `svc.keyring` and expire (default 24 h). | S | 2 |
| UT06-F039 | Private jobs | A job marked private is never kept, its name is replaced by "Private document" in lists, notifications and at the device (`job-name` withheld). | S | 2 |
| UT06-F040 | PIN release | Where the device supports `job-password`, the user sets a PIN and releases the job at the printer. | S | 2 |
| UT06-F041 | Retry and resume | Network loss retries with back-off; a job is never silently dropped; after 3 failures it holds with the reason. | M | 1 |
| UT06-F042 | Offline printing | Jobs to an unreachable printer wait and print when it returns (option per printer; default ask). | S | 2 |
| UT06-F043 | Spool limits | Per-user spool quota and per-job size limit; old kept jobs are removed first. | M | 1 |
| UT06-F044 | Page accounting | Pages printed per printer and per app, locally, for the user's own interest. Clearable. | C | 3 |
| UT06-F045 | Open original | For a kept job, open the spooled PDF in OF03. | C | 3 |

### 4.3 Print options and the dialog contract

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT06-F050 | Option model | One typed option set for every app and printer: `copies`, `collate`, `page_ranges`, `page_set` (all, odd, even), `reverse`, `media` (size, type, source), `orientation`, `sides`, `colour_mode`, `quality`, `resolution`, `scaling` (fit, fill, none, percent), `n_up` and layout order, `booklet`, `borderless`, `margins`, `finishings` (staple, punch, fold, positions), `output_bin`, `job_hold`, `job_password`, `private`, `banner`. | M | 1 |
| UT06-F051 | Constraint solving | Options the device does not support are hidden or disabled with a reason; conflicting choices (duplex with a media type that forbids it) are resolved with a visible message, using the device's constraint attributes where present. | M | 2 |
| UT06-F052 | Dialog contract | App calls `svc.portal.print` with a document source (a PDF handle, or a page-provider callback that renders pages to PDF on demand), a title and optional hints. SH04 draws the dialog, shows a preview from the PDF, returns the user's choice to the spooler. The app never learns which printers exist. | M | 1 |
| UT06-F053 | Preview | The dialog previews exactly what will be sent: the spooler applies n-up, booklet, scaling, ranges and colour mode to the PDF and returns preview pages. | M | 2 |
| UT06-F054 | App-specific options | An app may add a typed panel to the dialog (for example "print notes pages"); those options go back to the app, which regenerates the PDF. | S | 2 |
| UT06-F055 | Presets | Named option sets per printer and global ("Draft duplex", "Photo glossy"). | S | 2 |
| UT06-F056 | Print-to-PDF | A built-in printer. Output goes to a file the user picks through the file portal. Options: PDF version, PDF/A-2b, metadata fields, password (AES-256), image downsampling. No device is involved. | M | 1 |
| UT06-F057 | Imposition engine | n-up (2, 4, 6, 9, 16), booklet (saddle stitch order), posters (tiling across sheets), page borders, done on PDF pages in the transform helper. | S | 2 |
| UT06-F058 | Manual duplex | For simplex printers: print odd pages, prompt to reload, print even pages in the right order for that printer's paper path (asked once, remembered). | S | 3 |
| UT06-F059 | Colour handling | Document colour → device colour through ICC profiles in the rasteriser; rendering intent option; "let the printer manage colour" option. | S | 3 |
| UT06-F060 | Greyscale and economy | Greyscale conversion in the rasteriser; a "save ink" quality where the device offers draft. | S | 2 |
| UT06-F061 | Print a file directly | Drop a PDF, PNG, JPEG or text file on a printer in this app; it goes through the same dialog. Other types are opened in their app first. | S | 2 |
| UT06-F062 | Metadata scrub | Option (default on for printers that take PDF directly) to strip document metadata before sending. | S | 2 |

### 4.4 Scanner discovery and scanning

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT06-F070 | eSCL discovery | Browse `_uscan._tcp` and `_uscans._tcp` through `svc.discovery`; manual address; eSCL over IPP-USB for USB devices. | M | 2 |
| UT06-F071 | Capabilities | From the device's capability document: sources (flatbed, feeder, feeder duplex), sizes, resolutions, colour modes, formats, intents. | M | 2 |
| UT06-F072 | Preview scan | Low-resolution preview of the flatbed; drag a crop frame; auto-detect the page edges. | M | 2 |
| UT06-F073 | Scan settings | Source, colour (colour, grey, black and white), resolution (75–1200 dpi as offered), paper size or custom area, sides, brightness, contrast, threshold, document type hint (text, photo, mixed). | M | 2 |
| UT06-F074 | Presets | Text document (300 dpi grey), Photo (600 dpi colour), Archive (300 dpi colour, PDF/A, OCR), Quick (150 dpi). Editable. | M | 2 |
| UT06-F075 | Multi-page from flatbed | "Add page" appends; a timer option scans every n seconds for hands-free page turning. | M | 2 |
| UT06-F076 | Feeder and duplex | Scan the whole stack; duplex in one pass where the device supports it. | M | 2 |
| UT06-F077 | Manual duplex | For simplex feeders: scan fronts, flip the stack, scan backs; pages are interleaved (with reverse order for the back side). | S | 3 |
| UT06-F078 | Page strip | Thumbnails of all pages: reorder by drag, rotate, delete, insert, rescan one page, split and merge documents. Undo for all. | M | 2 |
| UT06-F079 | Clean-up | Per page or all: auto-crop, deskew, rotate to reading direction (uses OCR), remove blank pages, remove punch holes, despeckle, background whitening, contrast. Each is previewed and reversible until save. | S | 3 |
| UT06-F080 | Split by separator | Split a stack into documents at blank pages or at a barcode separator sheet. | C | 3 |
| UT06-F081 | Save formats | PDF (images compressed JPEG for colour and grey, CCITT G4 or JBIG2-free lossless for bilevel), PDF/A-2b, PNG, JPEG, TIFF (multi-page). File name pattern with date and counter. | M | 2 |
| UT06-F082 | OCR text layer | Recognised text is placed as an invisible layer under the image in the PDF so the file can be searched and copied. Languages chosen from installed OCR models. Runs locally through `svc.model` (or the OCR service shared with ME05). | S | 3 |
| UT06-F083 | OCR review | Low-confidence words are listed; the text of a page can be viewed and exported as plain text or Markdown. | C | 3 |
| UT06-F084 | Scan to app | Hand the result to another app through `svc.intent` `share`, or answer a `scan` intent from an app (OF02 Notes, OF04 Writer, CO01 Mail) with image or PDF handles. | S | 3 |
| UT06-F085 | Scan-to-print (copy) | Scan and send straight to a printer with the usual options. | C | 3 |
| UT06-F086 | Legacy scanners | Scanners without eSCL need device backends. Options, stated in 6.3: import SANE backends (GPL-2 with exception) in a contained backend process, or write own backends per chipset family. Decision is open; until then such scanners are listed as unsupported with the reason. | C | 3 |
| UT06-F087 | WSD-Scan | Could-have second network protocol for devices without eSCL. | C | 3 |
| UT06-F088 | Scanner buttons | "Scan to computer" pushed from the device panel is not accepted (it needs a listening service). Stated. | M | 2 |
| UT06-F089 | Metadata | Title, author, subject, keywords for the saved PDF; scanner make and model are left out by default. | S | 3 |
| UT06-F090 | Encrypted save | Save the scan into an encrypted PDF or hand it to SE05 Vault. | C | 3 |

### 4.5 Colour profiles

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT06-F100 | Printer profiles | Assign an ICC profile per printer and per media type; a generic profile otherwise. | S | 3 |
| UT06-F101 | Scanner profiles | Assign an input ICC profile; scans are tagged with it or converted to sRGB (default). | S | 3 |
| UT06-F102 | Profile import | Import ICC v2/v4 files through the portal; they are validated in the colour helper. | S | 3 |
| UT06-F103 | Scanner characterisation | Scan an IT8 target and build an input profile from its reference file. | C | 3 |
| UT06-F104 | Soft proof hand-off | Document apps that soft-proof (CR01, ME01) read the printer profile through `svc.print`. | C | 3 |

### 4.6 Status, supplies, troubleshooting

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT06-F110 | Device state | Idle, printing, stopped, with reasons, refreshed on demand and while the window shows the device. No background polling when nothing is queued. | M | 1 |
| UT06-F111 | Supply levels | Marker levels, names and colours from IPP (`marker-*` attributes); "low" and "empty" states; shown as bars with words. Levels are what the device claims. | S | 2 |
| UT06-F112 | Low-supply notice | Optional notification when a level crosses a threshold, checked only when a job is sent. | C | 2 |
| UT06-F113 | Paper state | Trays with loaded size and type and fill state where reported. | S | 2 |
| UT06-F114 | Troubleshooter | Step-by-step checks with results: device found on the network, address answers, TLS identity matches, device accepts attributes query, firewall allows the endpoint, format accepted, test page sent, device reports printed. Each failed step says what to do. | M | 2 |
| UT06-F115 | Job failure explanations | Device and backend errors mapped to plain sentences with the raw code available. | M | 1 |
| UT06-F116 | Logs | Per-printer backend log (last 500 lines, no document content, addresses kept local); "Copy for support"; full logs live in AD04. | S | 2 |
| UT06-F117 | Maintenance commands | Only those the device offers through IPP (clean heads, print a quality report where standardised). Others: link to the device's web page. | C | 3 |
| UT06-F118 | Help hand-off | Each failure links to its UT05 troubleshooting page by error id. | S | 2 |

### 4.7 Network exposure and sharing

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT06-F130 | No listening by default | The machine does not accept print or scan jobs from the network and does not announce any service. | M | 1 |
| UT06-F131 | Per-device endpoints | A backend may connect only to the address and port of its device. Adding a printer adds that endpoint to its backend's grant and, visibly, to SE02 Firewall. | M | 1 |
| UT06-F132 | Address changes | If a device's address changes (DHCP), the backend re-resolves through `svc.discovery` by service name and the pinned certificate must match before the grant is updated. | M | 2 |
| UT06-F133 | Discovery control | Settings: discover never, only while adding (default), or on the chosen networks. Discovery is per network profile (home, work, public: off on public). | M | 1 |
| UT06-F134 | Route rules | Printing uses the clear local route only. If the session is forced through a VPN or anonymity route (SE03), local printers are unreachable unless the user allows the local-link exception; the app explains this instead of failing silently. | M | 2 |
| UT06-F135 | Share a printer | Off by default. An admin can share a queue as an IPP Everywhere service on chosen networks to chosen hosts, with TLS and optional authentication; announced over DNS-SD. Incoming jobs are accepted as PDF or raster only, size-limited, and parsed in the transform helper. | C | 3 |
| UT06-F136 | Exposure summary | A panel lists every endpoint this subsystem talks to and anything it listens on, with links to SE02 and AD05. | M | 2 |
| UT06-F137 | What printers learn | A help page and a per-job note state what the device receives: page images or PDF, job name (unless private), user name (replaced by a fixed string by default), host name (not sent). | M | 2 |

### 4.8 Settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| UT06-F140 | Preferences | Section 8. | M | 1 |
| UT06-F141 | Spooler restart safety | Queued jobs and their state survive a spooler crash or a reboot; a job that was mid-send is held with "may have partly printed" rather than resent blindly. | M | 1 |
| UT06-F142 | Scan session recovery | Scanned pages are written to the app's data directory as they arrive; after a crash the unsaved session is offered back. | M | 2 |
| UT06-F143 | Undo | Page-strip edits, clean-up steps, printer removal (5 s toast). | M | 2 |
| UT06-F144 | Audit | Printer add/remove, certificate changes, sharing changes and elevated actions go to `svc.audit`. | S | 2 |
| UT06-F145 | Export and import of printer set-up | Printer definitions (no credentials) as JSON for moving to another machine. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Printers and Scanners
├─ Main window
│  ├─ Sidebar: Printers (each with state), Print to PDF, Scanners, All jobs, History
│  ├─ Printer page (tabs: Jobs | Options | Supplies | Details)
│  ├─ Scanner page (scan workspace)
│  ├─ Inspector: job detail / page settings
│  └─ Status bar
├─ Add device sheet (found devices + manual)
├─ Printer defaults and presets dialog
├─ Certificate changed dialog
├─ Troubleshooter (wizard)
├─ Scan clean-up panel
├─ Save scan dialog
├─ Colour profiles dialog
├─ Sharing dialog (admin)
├─ Exposure summary panel
├─ Print dialog (drawn by SH04; contract here)
└─ Preferences
```

### 5.2 Main window

Shown at 1100×700, a printer selected, Jobs tab.

```
+------------------------------------------------------------------------------------------+
| [=] Printers and Scanners                                         (1)[net]    [_][o][x] |
+-------------+----------------------------------------------------------+-----------------+
| PRINTERS (2)| Office Laser  ● Printing            [Pause] [Test page] (3)| JOB        (7)|
| > Office Laser| Study, 2nd floor · ipps · identity verified ✓        (4)| Report Q3.pdf |
|   ● printing| [Jobs | Options | Supplies | Details]                     | From  Writer   |
|   Photo Inkjet+--------------------------------------------------------+ Pages 12 × 2   |
|   ○ offline | Document          From     Pages  State                   | A4 · duplex    |
|   Print to PDF| > Report Q3.pdf  Writer   12×2   Printing page 5 of 24  | long edge      |
| SCANNERS    |   Invoice 1182    Sheets   1      Pending                 | Greyscale      |
|   Office Laser|  Private document Mail    3      Held: PIN at printer   | Sent as PWG    |
|   (flatbed, |                                                      (5) | raster 600 dpi |
|    feeder)  |                                                          | 14:02 submitted|
| ─────       |                                                          |                 |
| All jobs  3 | Toner  K ▓▓▓▓▓▓▓░░░ 68%                                   | [Cancel job]   |
| History     | Tray 1  A4 plain · loaded    Tray 2  A4 · empty !    (6) | [Hold]         |
| [+ Add…]    |                                                          | [Move to… v]   |
+-------------+----------------------------------------------------------+-----------------+
| 3 jobs · 1 printing                         Discovery: off (only while adding)      (8) |
+------------------------------------------------------------------------------------------+
```

1. `[net]` badge: on while a backend has a connection open; click lists the endpoints (device name, address, port, TLS).
2. Sidebar (`treeView`): each device row has a state dot plus a word. `[+ Add…]` is the primary action when the list is empty.
3. Device header with state and two `toolButton`s.
4. Identity line: protocol and certificate state as a trust badge (`--ok` verified, `--warn` not encrypted, `--danger` identity changed).
5. Job table (`tableView`); state column carries the device's reason in words. Rows can be dragged to reorder.
6. Supplies and trays strip (`progressBar`s with labels); "empty" has an icon and a word.
7. Inspector (`propertyGrid`, read-only) with job actions. Cancel is outlined `--danger`.
8. Status bar: counts and the discovery state (click opens the setting).

States: empty (no printers: "No printers yet. [Add a printer or scanner]"), loading (device query in progress: skeleton rows, "Asking the printer…"), error (device unreachable: banner with "Troubleshoot"), offline (network down: devices greyed, jobs wait), locked (identity changed: page replaced by the certificate dialog's summary and two buttons), read-only (system-defined printers: no Remove, options locked by admin shown with a lock), narrow (sidebar overlay, inspector sheet, supplies strip collapses to icons).

### 5.3 Other views and dialogs

**Add device sheet** (640×480)

```
+-- Add a printer or scanner ----------------------------------------------------+
| Looking on "Home network" for devices…  (stops when you close this)       (1) |
|                                                                               |
|  Office Laser MFP        printer + scanner   ipps, eSCL    192.0.2.14  [Add]  |
|  Photo Inkjet            printer             ipp (no TLS)  192.0.2.31  [Add]  |
|  USB: Label Printer      printer             IPP-USB                   [Add]  |
|                                                                               |
|  Not listed?  Address [ ipps://printer.example:631/ipp/print ] [Look up]  (2) |
|                                                                               |
|  Found devices can see that this computer asked. Nothing else is sent.    (3) |
|                                                              [Close]          |
+-------------------------------------------------------------------------------+
```

1. Discovery runs only while the sheet is open (default policy); the `--net` badge is on. 2. Manual `lineEdit`; "Look up" queries attributes and shows a confirmation card with make, model, capabilities, certificate fingerprint. 3. One plain line on what discovery reveals.

**Print dialog** (drawn by SH04, 860×620; shown here because this spec defines its model)

```
+-- Print "Report Q3" ---------------------------------------------------------------------+
| Printer [Office Laser ● idle v]   Preset [Default v]            +---------------------+ |
| Copies  [ 1 ] [x] Collate         Pages (•) All ( ) [1-3, 8___] |                     | |
| Paper   [A4 v]  Tray [Auto v]     Orientation [Portrait v]      |      preview        | |
| Sides   [Both, long edge v]       Colour [Greyscale v]          |      page 1 of 12   | |
| Per sheet [1 v]  Scale [Fit v]    Quality [Normal v]            |                     | |
| [More options v]                                                |                     | |
|   Booklet [ ]  Reverse [ ]  Borderless [ ]  Staple [None v]     +---------------------+ |
|   [x] Private job   Hold with PIN [____]                          [<] 1 / 12 [>]       |
| 12 pages on 6 sheets                                                                    |
|                                    [Save as PDF…]        [Cancel]   [Print]             |
+------------------------------------------------------------------------------------------+
```

Contract notes: the app that asked sees none of this; the summary line is computed by the spooler; disabled options carry a tooltip with the reason from the constraint solver.

**Scanner page** (1100×700)

```
+------------------------------------------------------------------------------------------+
| Office Laser (scanner)   [Preview] [Scan]  Preset [Text document v]             [Save…] |
+----------------------+-------------------------------------------+-----------------------+
| Source [Feeder v]    |  ┌───────────────────────────────┐        | PAGES (6)         (3) |
| Sides  [Both v]      |  │ ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐     │        | ┌───┐ ┌───┐ ┌───┐     |
| Colour [Grey v]      |  │                           │   │        | │ 1 │ │ 2 │ │ 3 │     |
| Resolution [300 v]   |  │ │   page 3 (selected)   │     │   (2)  | └───┘ └───┘ └───┘     |
| Size   [A4 v]        |  │                           │   │        | ┌───┐ ┌───┐ ┌───┐     |
| [x] Auto-crop        |  │ └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘     │        | │ 4 │ │ 5 │ │ 6 │     |
| [x] Deskew           |  └───────────────────────────────┘        | └───┘ └───┘ └───┘     |
| [x] Drop blank pages |  [↺] [↻] [Crop] [Clean up…]  Zoom [Fit v]  | [Add page] [Rescan]   |
| [ ] OCR  [English v] |                                           | [Delete] [Split here] |
| (1)                  |                                           |                       |
+----------------------+-------------------------------------------+-----------------------+
| Scanning page 7…  ▓▓▓▓▓▓░░░░  [Stop]                 6 pages · about 3.1 MiB as PDF      |
+------------------------------------------------------------------------------------------+
```

1. Settings column (`comboBox`, `checkBox`). 2. Page canvas: an image view with a crop overlay (2D canvas, G-12; fallback `image` with fixed presets). 3. Page strip (`gridView`), drag to reorder, multi-select.

**Save scan dialog** (560×420)

```
+-- Save 6 pages ------------------------------------------------------+
| Name    [ Scan 2026-11-12 1402        ]   in [~/Documents] [Choose…]|
| Format  (•) PDF  ( ) PDF/A-2b  ( ) PNG files  ( ) JPEG  ( ) TIFF    |
| Quality [Balanced v]   about 3.1 MiB                                |
| [x] Searchable text (OCR)   Languages [English, German v]           |
| [ ] Password-protect the PDF                                        |
| Title [__________]  Author [__________]  [ ] Include scanner model  |
|                            [Share…]     [Cancel]  [Save]            |
+---------------------------------------------------------------------+
```

**Certificate changed dialog** (520×300)

```
+-- "Office Laser" is not the device it was ------------------------------+
| The printer at 192.0.2.14 now presents a different identity.           |
|   Was   SHA-256 4c:91:…:7e  (first seen 3 Mar 2026)                    |
|   Now   SHA-256 a0:13:…:d2                                             |
| This happens after a factory reset or firmware change, or when another |
| device is answering in its place. Jobs are held.                       |
|        [Keep jobs held]   [Remove printer]   [Trust the new identity]  |
+------------------------------------------------------------------------+
```

Default is "Keep jobs held". "Trust" is outlined `--danger` and is logged.

**Troubleshooter** (wizard, 640×480)

```
+-- Why is Office Laser not printing? --------------------------------------------+
|  ✓ Found on the network (192.0.2.14)                                           |
|  ✓ Answers on port 631                                                         |
|  ✓ Identity matches                                                            |
|  ✓ Accepts PWG raster at 600 dpi                                               |
|  ✗ Device reports: media-empty (tray 2), door-open                             |
|      Load A4 paper in tray 2 and close the front door, then press Check again. |
|  – Test page not sent                                                          |
|                         [Copy report]   [Check again]   [Send test page]       |
+--------------------------------------------------------------------------------+
```

**Scan clean-up panel** (sheet over the canvas): toggles and sliders (Deskew, Crop margin, Whiten background, Despeckle, Remove holes, Contrast) with before/after split view; `[Apply to this page] [Apply to all] [Reset]`.

**Exposure summary panel**: `tableView` of endpoints (device, address, port, protocol, TLS, last used) and a "Listening" section that says "Nothing" unless sharing is on; links to SE02 and AD05.

### 5.4 Menus

```
File    Add Printer or Scanner… Ctrl+N · Print File… Ctrl+O · Save Scan… Ctrl+S
        Export Scan Text… · Import/Export Printer Set-up… · Close Ctrl+W · Quit Ctrl+Q
Edit    Undo Ctrl+Z · Redo Ctrl+Shift+Z · Select All Pages Ctrl+A · Delete Page Delete
        Rotate Left Ctrl+[ · Rotate Right Ctrl+] · Rename Device F2 · Preferences Ctrl+,
View    Printers Ctrl+1 · Scanners Ctrl+2 · All Jobs Ctrl+3 · History Ctrl+4
        Inspector F4 · Sidebar F9 · Zoom In/Out/Fit Ctrl++ / Ctrl+- / Ctrl+0 · Refresh Status F5
Printer Set as Default Ctrl+D · Pause/Resume Ctrl+Shift+P(→ Ctrl+Alt+P) · Test Page Ctrl+T
        Defaults and Presets… · Colour Profile… · Troubleshoot… · Identify · Disable · Remove…
Job     Cancel Delete · Hold Ctrl+H(→ Ctrl+Shift+H) · Release Ctrl+R · Move To ▸ · Reprint… · Show Details Alt+Enter
Scan    Preview Ctrl+Shift+V · Scan Ctrl+Enter · Add Page Ctrl+Shift+N · Rescan Page
        Clean Up… Ctrl+L · Split Document Here · Recognise Text (OCR) · Copy to Printer…
Tools   Exposure Summary · Sharing… (administrator) · Colour Profiles… · Open Spool Folder Size
Help    Printers and Scanners Help F1 · What a Printer Receives · About
```

Final bindings: Pause/Resume is `Ctrl+Alt+P`; Hold is `Ctrl+Shift+H` (`Ctrl+H` is reserved for replace, `Ctrl+Shift+P` for the palette).

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Ctrl+Enter` | Scan |
| `Ctrl+Shift+N` | Add page |
| `Ctrl+[` / `Ctrl+]` | Rotate page |
| `Delete` | Cancel job (job table) or delete page (page strip) |
| `Ctrl+R` | Release held job |
| `Ctrl+T` | Test page |
| `Ctrl+D` | Set default printer |
| `Alt+Up` / `Alt+Down` | Move job or page earlier / later |

Context menus: printer (Set as Default, Pause, Test Page, Defaults…, Troubleshoot…, Rename, Disable, Remove); job (Cancel, Hold, Release, Move To ▸, Reprint, Details); page thumbnail (Rotate, Rescan, Delete, Insert Scan Before, Split Here, Save This Page As…).

Drag and drop: files onto a printer (print through the dialog); jobs between printers; jobs within a queue; pages within the strip; pages out to SY02 or another app (as PNG or single-page PDF); image or PDF files into the page strip (append to the document).

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | No devices: add prompt. No jobs: "No jobs". No pages: "Put a page in the scanner and press Scan". |
| Loading | Device queries show skeletons; scanning shows a growing page and progress. |
| Error | Device errors inline on the device and the job; spooler not running: `--danger` banner with Start; backend crash: job held with reason. |
| Offline | Network down or route blocked: devices greyed with the reason (no network, blocked by VPN route, blocked by firewall); jobs wait. |
| Locked | Certificate changed (5.3). Admin-locked options show a lock and a tooltip. |
| Read-only | System printers for non-admins. |
| Narrow (<640) | Sidebar overlay; scanner settings collapse into a popover; page strip moves below the canvas. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `treeView` |
| Device page tabs | `tabWidget` |
| Jobs, endpoints, history | `tableView` |
| Supplies | `progressBar`, `label` |
| Options | `propertyGrid`, `comboBox`, `spinBox`, `checkBox`, `radioButton` |
| Page strip | `gridView` |
| Page canvas | 2D canvas (G-12, FUSE-WP-43) or image view (FUSE-WP-44) |
| Print preview (in SH04) | page view (FUSE-WP-45) |
| Wizards and dialogs | `dialog`, `lineEdit` |
| Inspector | `propertyGrid` |

Missing: 2D canvas with a crop overlay (G-12); page view for print preview (FUSE-WP-45); sheets (FUSE-WP-30). The crop overlay falls back to numeric fields until G-12 exists.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | App host | Device list, queues view, scan workspace, troubleshooter |
| UI | FUSE UI | App host | All windows |
| Spooler (`svc.print`, Proposed) | C++23 daemon | System service, per-user spool | Queue state machine, spool store, option validation, starts transform and backend processes, talks to SH04 |
| `pdf-transform` | C++23 | One per job; capability mode | Reads the job PDF, applies ranges, n-up, booklet, scaling; rasterises to PWG Raster, URF or PCLm, or writes a normalised PDF. Fds: PDF in, output out |
| `ipp-backend` | C++23 | One per device connection; capability mode with one socket | IPP/2.0 client over HTTP/1.1 and TLS; attributes, job submission, status, cancel. Reaches one endpoint (or one USB interface) |
| `escl-backend` | C++23 | One per scanner session; capability mode with one socket | eSCL client (HTTP + XML capability and job documents); streams image data out |
| `scan-image` | C++23 | One per scan session; capability mode | Decodes device JPEG/PNG/PDF-wrapped pages, clean-up operations, encodes outputs, assembles PDF with text layer |
| `legacy-backend` (could) | C or C++ | One per device | Wrapper for imported scanner or printer backends (6.3) |
| OCR client | ForgeScript | App host | Sends page images to the OCR model through `svc.model`; receives words with boxes |
| Colour helper | C++23 | With `pdf-transform` and `scan-image` | ICC parsing and transforms |
| Discovery client | ForgeScript | App host | Calls `svc.discovery` (Proposed) |

```
 any app ──svc.portal.print──► SH04 print dialog ──► spooler (svc.print)
   (PDF handle or page provider)        ▲  preview      │ job PDF (spool, per user)
                                        └───────────────┤
                                                        ▼
                                          pdf-transform (cap mode: PDF in, raster/PDF out)
                                                        ▼
                                          ipp-backend (cap mode: one socket to one printer)
                                                        ▼  ipps://printer:631
 UT06 app ──bus──► spooler (queues, devices)       [the printer]

 UT06 app ──► escl-backend (one socket to one scanner) ──image stream──► scan-image ──► OCR (svc.model)
                                                                             └──► file via portal
```

Three rules shape this:

1. **Apps only make PDF.** They never see printers, drivers or the network.
2. **Document content and network access never meet in one process with parsing.** `pdf-transform` parses PDF and has no network; `ipp-backend` has a socket and treats the payload as opaque bytes.
3. **Each device gets its own backend process** that can reach that device only. A hostile printer can attack its own backend and nothing else.

### 6.2 Data model

Stores: spooler state `/var/db/print/state.sqlite` (system; devices, queues) and per-user spool `~/.local/share/org.pbsd.print/spool/` (job PDFs, encrypted when kept); app data `~/.local/share/org.pbsd.print-scan/` (scan sessions, presets, history view cache). Paths are Proposed.

| Entity | Fields |
|---|---|
| `Printer` | `id: u32`, `name`, `location`, `uri: string`, `transport: enum{ipps, ipp, ipp_usb, pdf_file, legacy}`, `make_model`, `uuid: string`, `dnssd_name: string?`, `cert_pin: bytes32?`, `auth_ref: string?` (keyring id), `scope: enum{user, system}`, `enabled: bool`, `paused: bool`, `is_default: bool`, `formats: list<string>`, `send_format: enum{auto, raster, pdf}`, `caps: PrinterCaps`, `defaults: PrintOptions`, `icc_profile: string?`, `offline_queue: bool` |
| `PrinterCaps` | `media: list<Media>`, `sources: list<string>`, `types: list<string>`, `sides: bitset`, `colour_modes: bitset`, `resolutions: list<(u16,u16)>`, `qualities: bitset`, `finishings: list<u16>`, `max_copies: u16`, `job_password: bool`, `constraints: list<Constraint>`, `urf: string?`, `pwg_types: list<string>` |
| `Media` | `name: string` (PWG self-describing name), `width_um: u32`, `height_um: u32`, `margins_um: [u32;4]`, `borderless: bool` |
| `PrintOptions` | fields of F050; each `Option<T>` so "unset" means the printer default |
| `Job` | `id: u64`, `printer_id`, `user: u32`, `app_id: string`, `title: string`, `private: bool`, `options: PrintOptions`, `state: enum{pending, held, processing, sending, printing, completed, cancelled, aborted}`, `reasons: list<string>`, `pages: u32`, `sheets: u32`, `spool_ref: string`, `sent_format: string`, `device_job_id: i32?`, `submitted: i64`, `finished: i64?`, `keep_until: i64?`, `attempts: u8` |
| `Preset` | `id`, `name`, `printer_id: u32?`, `options: PrintOptions` |
| `Scanner` | `id`, `name`, `uri`, `transport: enum{escl, escl_usb, wsd, legacy}`, `make_model`, `cert_pin`, `caps: ScannerCaps`, `icc_profile: string?` |
| `ScannerCaps` | `sources: bitset{flatbed, feeder, feeder_duplex}`, `max_area_um: (u32,u32)` per source, `resolutions: list<u16>`, `colour_modes: bitset`, `formats: list<string>` |
| `ScanSettings` | `source`, `sides`, `colour`, `dpi: u16`, `area_um: [u32;4]`, `brightness: i8`, `contrast: i8`, `threshold: u8`, `intent: enum{text, photo, mixed}` |
| `ScanSession` | `id`, `scanner_id`, `created: i64`, `pages: list<ScanPage>`, `saved: bool` |
| `ScanPage` | `index: u16`, `raw_ref: string`, `ops: list<CleanOp>` (non-destructive), `rotation: u16`, `crop: [f32;4]`, `ocr: OcrPage?` |
| `OcrPage` | `lang: list<string>`, `words: list<{text, box: [f32;4], conf: f32}>`, `model_id: string` |
| `Supply` | `printer_id`, `name`, `kind: enum{toner, ink, drum, waste, other}`, `colour: string`, `level_pct: i8` (−1 unknown), `state: enum{ok, low, empty}` |
| `Endpoint` | `device_id`, `host`, `port: u16`, `tls: bool`, `last_used: i64` |

Clean-up operations are stored as a list and applied at save time, so every step is undoable. Stores carry `meta.schema_version` and migrate forward with a copy kept.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| PDF (job input, from system apps or a user file) | yes | yes (normalised, print-to-PDF, scans) | Platform PDF reader and writer (G-16; the reader is shared with OF03, FUSE-WP-45). Own C++23, Proposed | pdf-transform; scan-image for output |
| PWG Raster (`image/pwg-raster`) | no | yes | own | pdf-transform |
| Apple Raster / URF (`image/urf`) | no | yes | own, from the public format description | pdf-transform |
| PCLm | no | yes | own (PDF-subset of raster strips) | pdf-transform |
| IPP/2.0 (RFC 8010, 8011) and PWG 5100.x attributes | yes | yes | own C++23 encoder and decoder | ipp-backend |
| HTTP/1.1, TLS 1.3 | yes | yes | FORGE network stack (FORGE-WP-21), FORGE crypto (FORGE-WP-22) | ipp-backend, escl-backend |
| IPP-USB | yes | yes | own framing over the USB printer interface class through `svc.portal.usb` | ipp-backend |
| DNS-SD / mDNS | — | — | `svc.discovery` (Proposed, new gap `UT06-GAP-1`); this app never opens a multicast socket | discovery service |
| eSCL (capabilities, scan settings, status: XML over HTTP) | yes | yes | own C++23 with a small non-validating XML reader (no entities, no DTD, depth and size caps) | escl-backend |
| JPEG, PNG, PDF pages from scanners | yes | yes | platform image decoders; own PDF page extraction | scan-image |
| TIFF (multi-page), PDF/A-2b | no | yes | own writers | scan-image |
| ICC profiles v2/v4 | yes | embed | own C++23 colour engine (Proposed; shared with FUSE-WP-26) | colour helper inside pdf-transform / scan-image |
| OCR | — | — | local model through `svc.model` (FORGE-WP-31 model classes; an OCR class must exist: `UT06-GAP-2`) | model service |
| WSD-Scan (SOAP) | yes | yes | own, could | escl-backend variant |
| LPD, raw 9100, PCL 5/6, PostScript generation | no | could | own generic generators | pdf-transform + legacy backend |
| SANE backends (legacy scanners) | — | — | Import of the SANE backends collection: GPL-2-or-later with a linking exception; C; many chipset drivers. If imported: built offline, one contained `legacy-backend` process per device with only its USB device handle. Alternative: own backends per chipset family. Neither is planned before phase 3 | legacy-backend |
| Vendor printer drivers, PPD files, CUPS filters | no | no | Not used and not planned | — |

What must be built (nothing exists in the repos read): the spooler, the IPP client, three raster writers, the PDF transform and rasteriser, the eSCL client, the scan image pipeline, the PDF/A writer with text layer, the discovery service, and an OCR model class. What may be imported: SANE backends, only for legacy scanners. The base tree's `lpr`/`lpd` (`hbsd/src/usr.sbin/lpr`) is not used: it has no PDF, IPP or capability model.

### 6.4 Integration

- Intents handled: `print` (a file dropped on the app or "Print with…"), `scan` (another app asks for a scan; the app returns handles), `open` for `application/pdf` only as "print this file".
- Intents emitted: `share` (scan result), `open` (saved scan in OF03, device web page in IN01), `view` (help pages in UT05).
- Services used: `svc.print` (Proposed, part of G-16), `svc.portal.print`, `svc.portal.usb`, `svc.discovery` (Proposed), `svc.portal.file`, `svc.notify`, `svc.keyring`, `svc.settings`, `svc.audit`, `svc.model`, `svc.intent`, `svc.power` (inhibit sleep while a job is being sent).
- Offers: the `scan` intent provider; through the spooler, `org.pbsd.print.v1` (submit, list own jobs, cancel own jobs) used by SH04.
- Print dialog contract with SH04:
  1. App → `svc.portal.print.request(title, source, hints, app_panel?)`. `source` is a PDF handle or a page provider (`page_count()`, `render(range, paper) → PDF handle`).
  2. SH04 asks the spooler for printers, capabilities, presets and constraint results; draws the dialog.
  3. For each option change SH04 asks the spooler for preview pages. If an app-panel option or the paper size changed, SH04 asks the app to re-render.
  4. On Print, SH04 passes the final PDF handle and `PrintOptions` to the spooler; the app gets back only `{accepted, job_token}` and later completion state for its own job.
  5. On "Save as PDF" the file portal is used and no spooler job is made.
- Links: SH04 (dialog), SE02 (endpoint rules), SE03 (route), VM05 (USB authorisation), AD05 (grants), OF03 (PDF engine, open result), ME05 (shared OCR), SE04 (credentials), UT05 (troubleshooting pages), AD04 (logs), SY01 (default paper size from region).

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Scan sessions, presets |
| `fs.portal` | Save scans; pick files to print; import ICC profiles |
| `net.host:<device>:<port>` (dynamic, one per added device) | Each backend reaches its own device only. Granted when the user adds the device; listed in AD05 and SE02 |
| `net.route:clear` | Local-link devices are reachable only on the clear route |
| `proc.helper:ipp-backend`, `proc.helper:escl-backend`, `proc.helper:pdf-transform`, `proc.helper:scan-image` | Contain device protocols and document parsing (when run for the app's own direct work, such as scanning; the spooler runs its own instances) |
| `dev.print` | Use the print pipeline |
| `dev.usb:printer`, `dev.usb:scanner` (`dev.usb:<class>`) | USB printers and scanners; prompt on first use |
| `svc.portal.print`, `svc.portal.usb`, `svc.portal.file` | Portals |
| `svc.print` (Proposed) | Manage queues and devices |
| `svc.discovery` (Proposed) | Device discovery on request |
| `svc.keyring` | Printer credentials, spool key |
| `svc.notify` | Job and scan notices |
| `svc.settings`, `svc.intent`, `svc.audit`, `svc.power` | Core operation |
| `svc.model`, `ai.local` | OCR; prompted on first use |
| `bg.run` | Finish a long scan or print hand-off after the window closes |

The manifest cannot list device hosts ahead of time. This needs a manifest form for user-added endpoints of a fixed service type; `net.user` is too broad. Proposed: `net.device:ipp`, `net.device:escl`, each instance approved by the add-device step (`UT06-GAP-3`).

Threat model:

| Asset | Adversary | Attack | Mitigation |
|---|---|---|---|
| The host | Hostile or compromised printer or scanner | Malformed IPP, HTTP, TLS, XML or image responses to exploit the client | One backend per device in capability mode with one socket; own bounded parsers; scanner images decoded in `scan-image` with no network; fuzzing |
| The printer's own parser, and through it the network | Hostile document | A PDF crafted to exploit the printer's interpreter | Default sends locally rendered raster, not the PDF (F006) |
| Document content | Someone on the network | Reading jobs in transit | `ipps` preferred; plain `ipp` flagged; certificate pinning; warning before the first unencrypted job |
| Document content | Someone at the printer; the printer's memory | Picking up pages; stored job names | Private jobs, PIN release, withheld job and user names (F039, F040, F137) |
| Document content | Other local users or apps | Reading the spool | Per-user spool, encrypted when kept, short expiry; apps see only their own job state |
| The user's presence and identity | Devices and others on the link | Discovery reveals this machine is asking; IPP reveals user and host names | Discovery only on request and never on public networks; fixed user string; no host name |
| The local network | An app | Using printing as a covert network path, or scanning the LAN for devices | Apps never reach devices; only this app can call discovery; backends reach saved endpoints only |
| Device identity | Attacker answering as the printer | Capturing jobs | Pinned certificate; identity-change dialog holds jobs |
| The machine as a server | Network peers | Sending jobs to a shared queue | Sharing off by default; when on: allow-list, TLS, size limits, contained parsing |
| Scans | OCR service | Learning scan content | OCR is local; pages are passed as handles; nothing stored by the model service |
| Paper | — | Printer tracking dots identify the device | Cannot be prevented; stated on the help page |

Stored: device definitions, pinned certificates, presets, job history metadata, kept spool files (encrypted, expiring), unsaved scan sessions. Leaves the machine: print data to the chosen device only; discovery queries on the local link when the user asks.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `print.default_printer` | string | none | Default queue name |
| `print.use_last_printer` | bool | false | Last used becomes the default |
| `print.default_paper` | enum | from region | A4 or Letter and others |
| `print.send_format` | enum | `raster` | raster, pdf_if_supported |
| `print.raster_dpi` | enum | `device_default` | Resolution for rasterising |
| `print.keep_for_reprint` | bool | false | Keep spool files after printing |
| `print.keep_hours` | u16 | 24 | Expiry of kept jobs |
| `print.history_days` | u16 | 30 | Job metadata retention; 0 keeps none |
| `print.user_name_sent` | string | `user` | Requesting user name sent to devices |
| `print.send_job_name` | bool | true | Send document titles (never for private jobs) |
| `print.warn_unencrypted` | bool | true | Warn before the first job over plain `ipp` |
| `print.offline_queue` | enum | `ask` | ask, queue, fail |
| `print.spool_quota` | bytes | 2 GiB | Per-user spool limit |
| `discovery.mode` | enum | `while_adding` | never, while_adding, on_trusted_networks |
| `discovery.networks` | list<string> | empty | Network profiles where discovery is allowed |
| `scan.default_preset` | string | `Text document` | Preset selected on open |
| `scan.default_format` | enum | `pdf` | pdf, pdfa, png, jpeg, tiff |
| `scan.name_pattern` | string | `Scan %Y-%m-%d %H%M` | File name pattern |
| `scan.auto_crop` | bool | true | Crop to the page |
| `scan.deskew` | bool | true | Straighten pages |
| `scan.drop_blank` | bool | false | Remove blank pages |
| `scan.ocr` | bool | false | Add a text layer by default |
| `scan.ocr_languages` | list<string> | system language | OCR languages |
| `scan.include_device_model` | bool | false | Write scanner model into metadata |
| `supplies.notify_below_pct` | u8 | 0 (off) | Low-supply notice threshold |
| `sharing.enabled` | bool | false | Admin only |

## 9. Accessibility and languages

- Device and job states are words plus icons; supply bars have text values; "empty" is never colour alone.
- The page strip is a list for assistive tools: "page 3 of 6, rotated 90 degrees"; reorder with `Alt+Up/Down`.
- Scan clean-up has numeric controls beside every slider; the crop frame can be set by fields.
- The print dialog's option model is plain typed fields, so SH04 can render it fully for screen readers.
- Paper sizes follow the region (A4 or Letter); units are millimetres or inches by locale.
- OCR languages depend on installed models; right-to-left and CJK text layers must keep reading order (tested per language).
- Device-reported strings (reasons, supply names) are in the device's language; known keywords are mapped to local strings, unknown ones are shown as received.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | 200 ms (from cached device state) |
| Print dialog to first preview page | 300 ms for a PDF handle; depends on the app for a page provider |
| Rasterise A4 at 600 dpi greyscale | Under 300 ms per text page; streamed so the first page is sent before the last is rendered |
| Spooler idle | No wake-ups; no polling of devices with empty queues |
| Job submit to first byte sent | Under 1 s on a reachable printer |
| Memory | App host 80 MiB; pdf-transform bounded by band rendering (64 MiB cap for A4 at 1200 dpi colour); backends 32 MiB each |
| Largest job | 5 000 pages; spool limited by quota |
| Scan display | A 600 dpi A4 colour page appears progressively as data arrives |
| Scan session | 500 pages, pages stored on disk, thumbnails in memory |
| OCR | Shown as a background job per page; speed depends on the model |

## 11. Testing and proof

- **Unit**: option model, constraint solver, imposition maths (n-up, booklet order, poster tiling), manual-duplex ordering, file-name patterns, state machine transitions.
- **Protocol tests**: the IPP client against a software IPP Everywhere printer model written for the test suite (attributes, job life cycle, errors, slow and broken responses); the eSCL client against a software scanner model. Conformance cases from the public IPP Everywhere self-certification list are used as a checklist.
- **Raster checks**: PWG, URF and PCLm outputs decoded by an independent test decoder and compared with the rendered page bitmaps.
- **Golden**: all views; test page raster output per format.
- **Scripted UI**: T1–T13 with the software device models.
- **Fuzz**: IPP decoder, HTTP response parser, eSCL XML reader, PDF reader in `pdf-transform`, scanner image decoders, ICC parser.
- **Hostile device suite**: a test server that sends oversized attributes, endless chunked bodies, wrong lengths, changed certificates, redirect attempts; expected outcome per case.
- **Recovery**: kill the spooler, backends and the app at each job state; no job lost, none duplicated without the "may have partly printed" hold.
- **Hardware matrix**: a recorded list of real devices tried, with results. Empty until hardware is tested.
- **Sandbox**: `pdf-transform` cannot open sockets; backends cannot open files or other hosts (G-06).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Queue state machine | No job is lost; states follow the allowed transitions; a job is sent at most once without user action | T1 | PRISM proof on the transition system | — |
| IPP encoder/decoder | Decode never reads out of bounds; encode/decode round trip on the supported attribute syntaxes | T2 (round trip T1 for the core syntaxes) | PRISM; fuzzing | Proof effort for all syntaxes |
| Raster writers | Headers and band sizes consistent with page geometry; no overflow | T2 | PRISM; independent decoder | — |
| Imposition maths | Every source page appears exactly once in the right slot | T1 | PRISM proof for n-up and booklet | — |
| eSCL XML reader | Bounded depth and size; no entity expansion | T2 | PRISM; fuzzing | — |
| PDF reader and rasteriser | Memory safety | T3 | Safe subset, fuzzing, sandbox | Size and complexity of PDF |
| Endpoint policy | A backend's socket can reach only its device's endpoint | T2 | PRISM on the policy compiler (FORGE-WP-08) plus sandbox tests | Kernel enforcement is trusted |
| Certificate pin check | A changed certificate never passes silently | T2 | PRISM; hostile device suite | — |
| OCR output | None | T4 | Sample sets | Models are not provable |
| Imported SANE backends (if any) | None | T4 | Sandbox only | Third-party C |

Nothing here has been run.

## 12. Build plan

Phase 1: usable printing.

| ID | Work item | Depends on |
|---|---|---|
| UT06-W01 | Spooler service: queue state machine, spool store, bus interface | G-16, G-08, G-09, FORGE-WP-27 |
| UT06-W02 | IPP client and `ipp-backend` (attributes, submit, status, cancel) over TLS with pinning | FORGE-WP-21, FORGE-WP-22, G-06 |
| UT06-W03 | PDF reader and band rasteriser in `pdf-transform`; PWG Raster and URF writers | G-16, FUSE-WP-45 (shared PDF engine), OF03 |
| UT06-W04 | Option model and capability mapping from IPP attributes | UT06-W02 |
| UT06-W05 | Print dialog contract implemented with SH04; print-to-PDF | SH04, FUSE-WP-36 |
| UT06-W06 | App: device list, manual add, queues and jobs, test page, notifications | G-02 |
| UT06-W07 | Discovery service client and Add device sheet | `UT06-GAP-1` |
| UT06-W08 | Per-device endpoint grants and firewall display | `UT06-GAP-3`, FORGE-WP-08, SE02, AD05 |
| UT06-W09 | Software IPP printer model and hostile device suite | UT06-W02 |

Phase 2: full printing, scanning.

| ID | Work item | Depends on |
|---|---|---|
| UT06-W10 | Constraint solver, presets, preview in the dialog, imposition (n-up, booklet) | UT06-W03 |
| UT06-W11 | IPP-USB | VM05, `svc.portal.usb` |
| UT06-W12 | Supplies, trays, troubleshooter, failure explanations | UT06-W02 |
| UT06-W13 | History, reprint with encrypted spool, private jobs, PIN release, authentication | SE04 |
| UT06-W14 | eSCL client and `escl-backend`; software scanner model | UT06-W02 (shared HTTP/TLS) |
| UT06-W15 | Scan workspace: preview, settings, presets, flatbed multi-page, feeder, duplex | G-02, FUSE-WP-44 |
| UT06-W16 | Page strip editing; save to PDF, PNG, JPEG, TIFF; session recovery | UT06-W15 |
| UT06-W17 | Route and network-profile rules; exposure summary; system-wide printers | SE03, SH04 |
| UT06-W18 | PCLm writer; offline queue; move and reorder jobs | UT06-W03 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| UT06-W19 | Clean-up operations and crop overlay | G-12 |
| UT06-W20 | OCR text layer, PDF/A-2b, OCR review | `UT06-GAP-2`, G-15, FORGE-WP-31 |
| UT06-W21 | Colour profiles for printers and scanners; characterisation | FUSE-WP-26 |
| UT06-W22 | `scan` intent for other apps; copy to printer | OF02, OF04, CO01 |
| UT06-W23 | Printer sharing (IPP server) | SE02 |
| UT06-W24 | Legacy backends decision and work (generic PCL/PostScript; SANE import or own) | VM05 |
| UT06-W25 | Manual duplex for print and scan; posters; printer groups; WSD-Scan | UT06-W10 |

## 13. Open questions and risks

- Nothing for printing or scanning exists in the repos read. G-16 is large: a PDF reader and rasteriser, three raster formats, an IPP client and a spooler. It should be planned as platform work with this app as its front end.
- The PDF engine must be shared with OF03 and FUSE-WP-45; who owns it is open.
- There is no discovery service in PLATFORM.md section 8 (`UT06-GAP-1`). It is also needed by IN03, VM04, ME03 (casting) and CO05.
- The manifest has no form for "endpoints the user adds of a fixed device type" (`UT06-GAP-3`). FORGE-WP-08.2 mentions an extension point for local-link multicast; that should be checked against this need.
- No OCR model class is confirmed (`UT06-GAP-2`). Without it, scans are image-only PDFs.
- Legacy devices: importing SANE backends brings GPL C code and a large untested surface; writing own backends is slow. Many users will have only driverless devices; the recommendation is to ship without legacy support and decide from real demand.
- Sending raster by default costs time and bandwidth and loses printer-side fonts; the benefit is that the printer never parses hostile PDF. The per-printer override exists.
- Devices differ in how well they follow IPP Everywhere and eSCL. The hardware matrix is empty; real-device testing is the main schedule risk.
- Whether the spooler is a system daemon or per-user service affects system-wide printers and the lock screen; proposed: one system daemon with per-user spools.
