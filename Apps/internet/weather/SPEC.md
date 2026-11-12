<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Weather

By Odin Loch

| | |
|---|---|
| ID | `IN05` (from CATALOGUE.md) |
| Bundle | `org.pbsd.weather` |
| Category | internet |
| Priority | P3 |
| Backing repos | none. Uses FUSE `chart` (in tree, PLATFORM V3), the planned `mapView` (FUSE-WP-48) for radar, the planned network service (FORGE-WP-21) and the applet framework of SH01 (SH01-F116 to SH01-F121). |
| Status | Planned. Nothing built. |

## 1. Purpose

Weather shows current conditions, hourly and daily forecasts, rain radar and official warnings for places the user chooses. It takes data from open weather services that need no account or key, and it rounds coordinates before any request so the service learns an area, not an address. It replaces KWeather-style applets, GNOME Weather and weather web sites.

## 2. Scope

In scope:

- Forecasts (current, hourly, daily) from open sources, with a choice of provider.
- Locations entered by hand from an offline place list, or from coarse location through `dev.location`.
- Rain radar where open radar data exists; official alerts where an open feed exists.
- Units, offline cache, charts, comparison of providers.
- A panel and desktop applet for SH01.
- Optional routing of requests through SE03 routes.
- Privacy controls: coordinate rounding, request timing, no identifiers.

Out of scope:

| Item | Owner |
|---|---|
| General maps, search of addresses, routing | IN04 Maps (Weather may ask its offline geocoder for place names) |
| Clock, alarms, time zones | UT02 Clocks |
| Calendar events with weather | CO02 Calendar (may read Weather's forecast interface, 6.4) |
| Long-range statistical forecasting | AI03 Forecast |
| Route set-up (VPN, Tor) | SE03 Anonymity |
| The panel, slots and applet hosting | SH01 Desktop Shell |
| Personal weather station hardware | not planned |

## 3. Users and top tasks

Personas:

- **Glancer**: looks at the panel applet: temperature, rain in the next hour, a warning if any.
- **Planner**: checks the week for a trip, compares two places.
- **Outdoor user**: wants wind, gusts, precipitation timing, radar, sunrise and sunset.
- **Private user**: wants forecasts without revealing the home address, optionally over Tor.

| # | Task | Target steps |
|---|---|---|
| T1 | See now and today for the main place | open app or look at the applet (0) |
| T2 | Add a place | `Ctrl+N`, type a name, Enter (3) |
| T3 | See the hourly detail for a day | click the day (1) |
| T4 | Check rain in the next two hours | Radar tab, or the "next 2 h" strip on the overview (0–1) |
| T5 | Read an active warning | click the warning banner (1) |
| T6 | Switch units | `Ctrl+U` cycles metric and imperial (1) |
| T7 | Use approximate current location | Add place > "Use approximate location", shell prompt (2) |
| T8 | Refresh now | `F5` (1) |
| T9 | Change provider for one place | place properties, Provider (3) |
| T10 | Add the applet to the panel | panel > Add applet > Weather (shell flow, SH01 T9) |
| T11 | See what was sent to which server | `[net]` badge or Tools > Request log (1) |

## 4. Features

Priority: M must, S should, C could. Phase: 1 usable core, 2 full feature, 3 advanced.

### 4.1 Locations

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN05-F001 | Offline place search | A bundled gazetteer (cities and towns above a population threshold, with country, region, coordinates, elevation, time zone; candidate source GeoNames, CC BY 4.0, to verify) searched locally. No query text leaves the machine. | M | 1 |
| IN05-F002 | Place list | Ordered list of saved places with a main place. Add, remove (Undo toast), reorder by drag, rename. Limit 50. | M | 1 |
| IN05-F003 | Coordinates entry | Latitude and longitude typed by hand, or pasted `geo:` URI; shown rounded to the active precision at once. | M | 1 |
| IN05-F004 | Pick on a map | A small `mapView` (base world map only) to click a point; the pin snaps to the rounding grid visibly. | S | 2 |
| IN05-F005 | Approximate current location | Uses `dev.location` in coarse mode only. The app never asks for precise location. The fix is rounded again by F021 before use. | M | 2 |
| IN05-F006 | "Here" as a place | Optional dynamic place that follows coarse location; updates only when the rounded cell changes and at most once per hour; off by default. | S | 2 |
| IN05-F007 | Place names from Maps | If IN04 is installed with regions, its offline geocoder (`org.pbsd.maps.geocode`) can name a coordinate or find small villages; needs the user's one-time consent. | C | 3 |
| IN05-F008 | Place properties | Name, coordinates as requested (rounded), elevation, time zone, provider, alert source, radar source, route, notes. | M | 1 |
| IN05-F009 | Elevation handling | Elevation from the gazetteer is rounded to 100 m before it is sent to providers that accept it; otherwise omitted. | S | 2 |
| IN05-F010 | Time zone of the place | Times are shown in the place's zone with the user's zone as an option; zone from the gazetteer or bundled zone polygons. | M | 1 |
| IN05-F011 | Import and export places | JSON file with names and rounded coordinates. | C | 3 |
| IN05-F012 | Share a place's forecast | Copy as text or share a rendered card image; no link to a web service. | C | 3 |

### 4.2 Providers and data

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN05-F013 | Provider plug-ins | Each provider is a ForgeScript module plus a response schema: request builder, parser mapping to the common model (6.2), coverage polygon, terms and attribution text. Shipped with the app; no remote code. | M | 1 |
| IN05-F014 | Open-Meteo | Global forecast API with no key (free for non-commercial use; server software is open source and can be self-hosted; data attribution CC BY 4.0; terms to verify). Default provider. Hourly and daily variables, current conditions, minutely precipitation where offered. | M | 1 |
| IN05-F015 | MET Norway Locationforecast | Global, no key. Requires an identifying application User-Agent, at most 4 decimals in coordinates and honouring `Expires` and `If-Modified-Since` (terms to verify). Second built-in provider. | M | 1 |
| IN05-F016 | US National Weather Service | `api.weather.gov`, no key, public domain data, United States only; forecasts and alerts. | S | 2 |
| IN05-F017 | DWD open data | German weather service open data: forecasts, warnings and radar composites, directly or through an open JSON front end such as Bright Sky (terms and endpoints to verify). | S | 3 |
| IN05-F018 | Self-hosted endpoint | The user may point the Open-Meteo provider at their own server URL; needs a `net.host` grant for that host. | S | 2 |
| IN05-F019 | Provider per place | Default provider globally; override per place. The overview shows the provider name and data age for each place. | M | 1 |
| IN05-F020 | Provider comparison | Side-by-side daily and hourly values from two providers for one place, with differences marked. Makes one extra request per provider, only on demand. | C | 3 |
| IN05-F021 | Coordinate rounding | Before any request, coordinates are snapped to a grid: 0.1° (about 11 km, default), 0.25°, 0.05° or 0.01°. Snapping is to cell centres, so repeated requests are identical. The rounded value is what is stored for the request and shown in the UI. | M | 1 |
| IN05-F022 | No identifiers | No cookies, no API keys, no account, no device id. User-Agent is one fixed string for all users of a release (application name and project contact as provider terms require; nothing about the user or machine). | M | 1 |
| IN05-F023 | Attribution | Provider name, data licence and link are shown under every forecast and in About > Data sources. | M | 1 |
| IN05-F024 | Common model | All providers map to one model: time series with fixed variables and units (SI inside), WMO-style condition codes, per-value "missing" flags. Unknown fields are dropped. | M | 1 |
| IN05-F025 | Model and run information | Where the provider says which numerical model and run time produced the data, show it in details. | C | 3 |
| IN05-F026 | Air quality | Optional: particulate matter, ozone, nitrogen dioxide and a European or US index, where the provider has an open endpoint (Open-Meteo air quality, to verify). Off by default because it is a second request. | C | 3 |
| IN05-F027 | Pollen and UV | UV index from the forecast provider; pollen where open data exists (to verify per region). | C | 3 |
| IN05-F028 | Marine and mountain | Wave height, sea temperature, snow depth, freezing level where offered; shown only for places that have values. | C | 3 |

### 4.3 Forecast views

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN05-F029 | Now card | Temperature, "feels like", condition icon and word, wind speed and direction, gusts, humidity, pressure with trend, cloud cover, visibility, dew point, UV index, time of the data. | M | 1 |
| IN05-F030 | Next hours strip | 24 hourly cells: time, icon, temperature, precipitation probability and amount, wind. Scrolls horizontally; keyboard arrow navigation. | M | 1 |
| IN05-F031 | Precipitation nowcast | When the provider has sub-hourly data: a 2-hour bar strip in 5–15 min steps and a sentence ("Rain starting in about 25 minutes"). | S | 2 |
| IN05-F032 | Daily list | 7–16 days (provider dependent): day, icon, high and low, precipitation sum and probability, wind maximum, sunrise and sunset. | M | 1 |
| IN05-F033 | Day detail | Clicking a day opens its hourly table and charts. | M | 1 |
| IN05-F034 | Charts | `chart` with temperature and feels-like lines, precipitation bars, wind and gust lines, pressure, cloud cover band, humidity; choose which series; range 24 h, 48 h, 7 days. | M | 2 |
| IN05-F035 | Hourly table | `tableView` with all variables as columns, column chooser, copy as CSV. | S | 2 |
| IN05-F036 | Sun and moon | Sunrise, sunset, day length, civil twilight, moon phase, moonrise and moonset: computed locally from the rounded coordinates and date. | S | 2 |
| IN05-F037 | Overview of all places | Grid or list of cards: place, now icon and temperature, today's range, warning mark. | M | 1 |
| IN05-F038 | Compare places | Two or three places side by side on the same time axis. | C | 3 |
| IN05-F039 | Uncertainty | Where the provider gives ensemble spread or probability, show a band on charts and wording ("likely", "possible"). | C | 3 |
| IN05-F040 | Condition icons | One icon set (SVG subset) for day and night, in line, filled and high-contrast variants; every icon has a word beside it. | M | 1 |
| IN05-F041 | Text summary | A generated plain sentence for today and tomorrow from the data by fixed rules (no model): "Cloudy, rain from 15:00, high 12°." | S | 2 |
| IN05-F042 | History of forecasts | Optional: keep past forecasts and later observations for a place to show how the forecast changed. Local only. | C | 3 |

### 4.4 Radar and maps

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN05-F043 | Radar view | `mapView` in Minimal style (world base map bundled; IN04 regions used if installed) with a precipitation raster overlay and a legend. | S | 3 |
| IN05-F044 | Radar sources | Only sources with open data and terms that allow this use: national composites such as DWD (Germany) and NWS/NOAA (United States) are the first candidates; coverage and endpoints to verify. Where none exists, the tab says "No open radar data for this area". | S | 3 |
| IN05-F045 | Animation | Past 1–2 hours and, where offered, forecast frames; play, pause, step, speed; a time slider. Frames are fetched once and cached. Honours reduced motion (no autoplay). | S | 3 |
| IN05-F046 | Radar privacy | Radar requests are for a fixed regional tile or composite, not a point; the request therefore reveals a region. Stated beside the source. Off until the user opens the Radar tab the first time. | M | 3 |
| IN05-F047 | Model precipitation map | Fallback where no radar exists: the provider's gridded precipitation forecast for the area around the place (if an open endpoint exists; to verify). | C | 3 |
| IN05-F048 | Wind and temperature layers | Gridded model layers as coloured overlays. | C | 3 |
| IN05-F049 | Raster decoding | Radar frames (PNG, or the source's own binary formats) are decoded in the image helper; georeferencing from the source metadata. | S | 3 |

### 4.5 Alerts

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN05-F050 | Official warnings | From open feeds in the Common Alerting Protocol (CAP) or provider JSON: NWS (United States), DWD (Germany), MET Norway (Norway); the MeteoAlarm feeds for Europe are a candidate (terms to verify). | S | 2 |
| IN05-F051 | Alert matching | An alert applies when its polygon or area code contains the rounded cell centre. With coarse rounding an alert for a neighbouring area may be shown; it is labelled "near". | S | 2 |
| IN05-F052 | Alert banner and list | Banner on the place with severity colour, icon and word (Minor, Moderate, Severe, Extreme), headline, valid time. Full text, instructions and issuing office in a detail sheet. | S | 2 |
| IN05-F053 | Alert notifications | Through `svc.notify` for Severe and Extreme by default; per place and per severity settings; one notification per alert id; honours do-not-disturb except for Extreme (setting). | S | 2 |
| IN05-F054 | Threshold notifications | User rules on forecast values: frost tonight, rain within 1 h, wind gusts above N, UV above N. Evaluated locally after each refresh. | C | 3 |
| IN05-F055 | Alert language | Shows the language version matching the system if the feed has it, else the original, never machine-translated silently. An optional local translation through `svc.model` is labelled. | C | 3 |
| IN05-F056 | No alert source | Places without an open alert feed say so plainly: "No official warning feed is available here. Do not rely on this app for warnings." | M | 2 |

### 4.6 Refresh, cache and offline

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN05-F057 | Refresh policy | On open if data is older than 30 min; scheduled every 60 min (30 min to 6 h) when the applet is in use or background refresh is on; manual `F5`. | M | 1 |
| IN05-F058 | Server cache rules | Honours `Expires`, `Cache-Control`, `ETag`, `Last-Modified`; never requests before a response expires; 429 and 5xx back off exponentially to 6 h. | M | 1 |
| IN05-F059 | Timing jitter | Scheduled refreshes are spread ±20% and never aligned to the hour, so requests do not form a fingerprint by clock. | M | 2 |
| IN05-F060 | Place batching policy | Default: places are fetched one after another with random gaps of 2–20 s. Option "unlinkable": each place on a separate Tor circuit (needs an SE03 Tor route). | S | 3 |
| IN05-F061 | Offline cache | The last good response per place and provider is kept (parsed model, not raw bytes). Offline, the app shows it with its age in words and greys out hours already past. | M | 1 |
| IN05-F062 | Stale marking | Data older than 3 h shows a `--warn` chip "Updated 5 h ago"; older than 24 h the Now card is replaced by "No current data". | M | 1 |
| IN05-F063 | Metered and battery | No scheduled refresh on metered links or under 15% battery unless allowed. | S | 2 |
| IN05-F064 | Background refresh | Needs `bg.run`; asked when the user adds the applet or turns on alerts. Without it, data refreshes only while the window is open. | M | 2 |
| IN05-F065 | Request log | Table of the last 200 requests: time, host, route, the exact rounded coordinates sent, bytes, status. Proves what left the machine. | M | 1 |
| IN05-F066 | Request budget | Hard cap of requests per day per provider (default 200) so a bug cannot hammer a free service. | M | 1 |

### 4.7 Units and display

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN05-F067 | Unit systems | Metric, imperial, or custom per quantity: temperature °C/°F/K; wind m/s, km/h, mph, knots, Beaufort; pressure hPa, mmHg, inHg; precipitation mm, in; distance km, mi. Default from the locale. | M | 1 |
| IN05-F068 | Time format | 12 or 24 hour from the locale; place time or local time. | M | 1 |
| IN05-F069 | Wind display | Arrow plus compass word (N, NNE, …) plus degrees in details; arrow points where the wind blows to or from (setting, default from). | S | 1 |
| IN05-F070 | Feels-like method | Uses the provider's apparent temperature; if missing, computes wind chill or heat index locally and labels it "computed". | S | 2 |
| IN05-F071 | Theme | Follows the system theme. No animated weather backgrounds. An optional subtle tint of the Now card by condition is off in high contrast. | S | 2 |
| IN05-F072 | Density | Compact shows more hours and days per screen; touch enlarges cells to 44 px. | S | 2 |

### 4.8 Applet

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN05-F073 | Panel applet | An applet entry in this bundle (SH01-F116): shows icon and temperature of the main place in a panel slot; width 56–120 px by panel thickness and density. | S | 2 |
| IN05-F074 | Applet popover | 320×360: Now card, next 6 hours, 4 days, active warning headline, data age, "Open Weather". | S | 2 |
| IN05-F075 | Desktop widget | The same applet in the desktop widget grid (SH01-F080) at 2×2, 4×2 and 4×4 cell sizes. | C | 3 |
| IN05-F076 | Applet has no network | The applet process requests no `net.*` capability. It reads the cached model through `org.pbsd.weather.forecast` from the background refresh instance. If that is not running it shows cached data and its age. | M | 2 |
| IN05-F077 | Applet states | No place set ("Set a place"), stale (age shown), offline, warning active (severity mark and word in the tooltip and a11y name). | S | 2 |
| IN05-F078 | Applet settings | Which place (main or "Here"), show temperature, show condition word, show rain chance. | S | 2 |
| IN05-F079 | Lock screen and notifications summary | None by default; a setting may offer the forecast to SH02's summary through the same interface. | C | 3 |

### 4.9 Network routes and privacy

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN05-F080 | Named hosts only | The manifest lists the exact provider hosts. The fetch helper can reach nothing else. A self-hosted endpoint adds one host by prompt. | M | 1 |
| IN05-F081 | Route choice | Clear, VPN profile or Tor for all requests or per place, from SE03's routes. Fail closed: if the route is down, no request is made and cached data is shown. | S | 2 |
| IN05-F082 | First-run notice | Before the first request the app states: which provider will be contacted, that it will see the network address and the rounded coordinates shown, and how often. One button to proceed, one to choose settings first. | M | 1 |
| IN05-F083 | No location history | The app stores saved places only. Coarse fixes for "Here" are held in memory and replaced; the request log stores rounded cells and can be turned off or cleared. | M | 1 |
| IN05-F084 | Decoy requests | Optional: also request N random populated cells per refresh so the home cell is not singled out. Costs provider load; off by default; capped by F066. To review with providers' terms. | C | 3 |
| IN05-F085 | Manual-only mode | A switch that disables all automatic requests; data changes only on `F5`. | S | 1 |

### 4.10 Settings, accessibility and recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN05-F086 | Preferences | Pages: General, Units, Providers, Privacy and network, Alerts, Applet, Shortcuts. | M | 1 |
| IN05-F087 | Chart data as table | Every chart has a "Show as table" switch and an a11y table behind it. | M | 2 |
| IN05-F088 | Spoken summary | The text summary (F041) is the accessible name of the Now card and of the applet. | M | 2 |
| IN05-F089 | Parser failure handling | If a provider changes its format, the place shows "Could not read data from <provider>" with the cached forecast and an offer to switch provider; nothing crashes. | M | 1 |
| IN05-F090 | Store recovery | The cache is disposable; a corrupt cache is deleted and refetched. Places are in settings and are backed up. | M | 1 |
| IN05-F091 | Command line | `weather --place "Name" --json` prints the cached model; `--refresh` fetches first. For scripts and for the terminal. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Weather window
├─ Header bar: menu, place name, search (places), location indicator (only during a coarse fix), [net] badge
├─ Sidebar (F9): Overview · saved places (icon, temperature, warning mark) · [+ Add place]
├─ Content: tabs Forecast | Charts | Table | Radar | Alerts
│   ├─ Forecast: warning banner, Now card, nowcast strip, hourly strip, daily list, sun and moon, attribution
│   ├─ Charts: series chooser, chart, range switch
│   ├─ Table: hourly table
│   ├─ Radar: mapView with overlay, legend, time slider
│   └─ Alerts: list and detail
├─ Inspector (F4): place properties | day detail
└─ Status bar: data age, provider, route, next refresh
Dialogs and sheets
├─ First-run notice · Add place · Place properties · Alert detail · Provider comparison
├─ Request log · Data sources and licences · Preferences
└─ Shell-drawn: location prompt (approximate only), net.host prompt for a self-hosted endpoint
Applet (separate process, hosted by SH01): panel slot · popover · desktop widget
```

### 5.2 Main window

Shown at 1100×760, Forecast tab.

```
+----------------------------------------------------------------------------------------+
|[=] Weather — Vienna                    [search places ⌘K]          [net] [▁] [▢] [✕]  | (1)
+------------------+---------------------------------------------------------------------+
| Overview         | [Forecast] [Charts] [Table] [Radar] [Alerts 1]                      | (3)
| > Vienna    12°  +---------------------------------------------------------------------+
|   Bergen ! 7°    | ! Moderate · Wind warning · until Thu 18:00              [Details]  | (4)
|   Hobart    19°  +---------------------------------------------------------------------+
|   Here (approx.) |  cl  12°   Cloudy              Wind  ↙ NE 18 km/h, gusts 35          | (5)
|                  |      feels 10°                Humidity 71% · Pressure 1012 hPa ↓    |
| [+ Add place]    |  Today 14° / 6° · Rain from 15:00   UV 2 · Visibility 20 km         |
|                  +---------------------------------------------------------------------+
|                  | Next 2 h  ▁▁▁▂▃▅▆▆▅▃▂▁   Rain starting in about 25 min              | (6)
|                  +---------------------------------------------------------------------+
|                  | 13   14   15   16   17   18   19   20   21   22   23   00   …       | (7)
|                  | cl    cl    rn   rn   rn   cl    cl    ni    ni    ni    ni    ni            |
|                  | 12°  13°  12°  11°  10°  9°   8°   8°   7°   7°   6°   6°           |
|                  | 10%  20%  70%  80%  60%  30%  10%  5%   0%   0%   0%   0%           |
|                  +---------------------------------------------------------------------+
|                  | Thu  rn  14° / 6°   4.2 mm 80%   ↙ 35   su 06:58–16:21              | (8)
|                  | Fri  pc 11° / 4°   0.0 mm 10%   ← 20                               |
|                  | Sat  su 9° / 1°    0.0 mm  0%   ↖ 12                               |
|                  | …                                                                   |
|                  | Data: Open-Meteo (CC BY 4.0) · requested cell 48.25, 16.35          | (9)
+------------------+---------------------------------------------------------------------+
| Updated 14 min ago · Open-Meteo · Route: Clear · Next refresh in about 46 min          | (10)
+----------------------------------------------------------------------------------------+
  (2) sidebar
```

Two-letter codes in the drawings (cl cloud, rn rain, su sun, pc part cloud, ni clear night) stand for the app's own icon set (F040); every icon has a word beside it or in its accessible name.

States: empty (no place), loading (first fetch), error (provider failed), offline (cached, age shown), stale, route down, narrow. No locked or read-only state applies.

1. Header bar. Title carries the place. `[net]` appears only while a request is in flight and lists host and rounded coordinates.
2. Sidebar 220 px, `listView`: place name, icon, temperature, warning mark (`!` plus severity word in the tooltip). Drag to reorder.
3. `tabBar`. The Alerts tab shows a count.
4. Warning banner, `panel` in severity colour with icon and word; `button` Details.
5. Now card, `panel`, 120 px high: display-size temperature (24 pt), condition word, secondary values in two columns.
6. Nowcast strip, 48 px: custom bars on `chart` (bar series), with the sentence beside it. Hidden when the provider has no sub-hourly data.
7. Hourly strip: horizontal `scrollArea` of fixed 56 px cells (`gridView`), 4 rows: time, icon, temperature, precipitation probability.
8. Daily list: `listView` rows 40 px; click opens day detail in the inspector.
9. Attribution and the exact cell requested, as a `link` to Data sources.
10. Status bar: data age in words, provider, route, next refresh.

Narrow (<640): sidebar becomes a place switcher `comboBox` in the header; tabs stay; hourly strip scrolls; daily list unchanged.

### 5.3 Other views and dialogs

#### 5.3.1 First-run notice (dialog, 520×360)

```
+----------------------------------------------------------+
| Before Weather contacts a server                         |
| Weather gets forecasts from Open-Meteo                   |
| (api.open-meteo.com). For each place it will send:       |
|   • coordinates rounded to 0.1° (an area about 11 km     |
|     across), never your address                          |
|   • nothing that identifies you or this computer         |
| The server will see your network address unless you      |
| choose a VPN or Tor route.                               |
| Refresh: about once an hour while the app or applet runs.|
| Route [ Clear v ]   Rounding [ 0.1° v ]                  |
|            [Privacy settings…] [Use manual refresh only] |
|                                           [Continue]     |
+----------------------------------------------------------+
```

Shown once before the first request and again when the provider changes. `dialog`, `comboBox`, `button`.

#### 5.3.2 Add place (dialog, 520×440)

```
+----------------------------------------------------------+
| Add a place                                              |
| [ berg                                                 ] |
| > Bergen, Vestland, Norway            60.4, 5.3          |
|   Bergen, Lower Saxony, Germany       52.8, 10.0         |
|   Bergen op Zoom, Netherlands         51.5, 4.3          |
|   Bergamo, Lombardy, Italy            45.7, 9.7          |
| Search is done on this computer.                         |
+----------------------------------------------------------+
| Or: [Use approximate location]  [Enter coordinates…]     |
|     [Pick on map…]                                       |
| Will request cell: 60.35, 5.35 (0.1° grid)               |
|                               [Cancel] [Add place]       |
+----------------------------------------------------------+
```

`lineEdit`, `listView`, `button`s, `label`. "Use approximate location" raises the shell prompt, which offers Approximate only for this app.

#### 5.3.3 Charts tab (content area, 880×640)

```
+--------------------------------------------------------------------------------------+
| Series [x] Temperature [x] Feels like [x] Precipitation [ ] Wind [ ] Pressure  [24h|48h|7d] |
+--------------------------------------------------------------------------------------+
|  15°┤      ╭──╮                                                              ┤ 4 mm  |
|  10°┤ ╭────╯  ╰───╮          ╭───╮                                           ┤ 2 mm  |
|   5°┤─╯           ╰──────────╯   ╰────                                       ┤       |
|     │        ▂▅▇▅▂                         ▁▂▁                               │       |
|     └──┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬──                                 |
|       12    18    00    06    12    18    00    06                                   |
+--------------------------------------------------------------------------------------+
| Thu 15:00 · 12° · feels 10° · 1.8 mm · 70%                      [Show as table]      |
+--------------------------------------------------------------------------------------+
```

`checkBox` row, `chart` with two value axes, range `tabBar`, readout `label`. Keyboard: Left and Right move the readout hour by hour.

#### 5.3.4 Radar tab (content area, 880×640)

```
+--------------------------------------------------------------------------------------+
| Source: national radar composite (open data) · covers a region, not your place  [i]  |
+--------------------------------------------------------------------------------------+
|                                                                                [+]   |
|            ░░▒▒▓▓▒░                                                            [−]   |
|        ░▒▓██▓▒░        ● Vienna (cell)                                               |
|           ░▒▒░                                                                       |
|                                                   Legend  ░ light ▒ moderate ▓ heavy |
+--------------------------------------------------------------------------------------+
| [◀] [▶ Play] [▶|]   13:00 ────────●──────── 15:00 | forecast → 16:00    Speed [1× v] |
+--------------------------------------------------------------------------------------+
```

`mapView` with a raster overlay, `slider` for time, `toolButton`s, `comboBox`. States: not yet enabled ("Radar makes an extra request to <host> for a regional image. [Enable radar]"), no open source for this area, frames loading, offline (cached frames with their times).

#### 5.3.5 Alert detail (sheet, 560×520)

```
+--------------------------------------------------------------+
| ! Moderate — Wind warning                                    |
| Bergen (near: the alert area borders your rounded cell)      |
| Valid   Wed 20:00 – Thu 18:00 (place time)                   |
| Issued  by the national weather service, Wed 11:42           |
+--------------------------------------------------------------+
| South-westerly gale force 8 with gusts of 30 m/s in exposed  |
| places. …                                                    |
| What to do                                                   |
| Secure loose objects. Avoid exposed coastal roads. …         |
+--------------------------------------------------------------+
| Source: official CAP feed · id 2.49.0.1.578…   [Copy text]   |
|                              [Open source page] [Close]      |
+--------------------------------------------------------------+
```

Text is shown as plain text from the feed (control characters removed). "Open source page" goes to IN01 by intent.

#### 5.3.6 Request log (dialog, 720×460)

```
+----------------------------------------------------------------------------------+
| Requests made by Weather                              [Clear] [Logging: on v]    |
| Time         Host                 Route  Sent (lat, lon)   Status  Bytes         |
| 13:46:12     api.open-meteo.com   Clear  48.25, 16.35      200     6.1 K         |
| 13:46:21     api.met.no           Clear  60.35, 5.35       304     0.3 K         |
| 12:44:03     api.open-meteo.com   Clear  48.25, 16.35      200     6.1 K         |
| Today: 14 requests of 200 allowed for Open-Meteo                                 |
+----------------------------------------------------------------------------------+
```

`tableView`, monospace for coordinates and hosts.

#### 5.3.7 Applet: panel slot and popover

```
 panel slot (normal density, 32 px thick):   [ cl12° ]        with warning:  [ ! cl12° ]

 popover 320×360:
+--------------------------------------+
| Vienna                    14 min ago |
|  cl  12°  Cloudy · feels 10°          |
|  Rain from 15:00 · wind NE 18 km/h   |
+--------------------------------------+
| 14   15   16   17   18   19          |
| 13°  12°  11°  10°  9°   8°          |
| 20%  70%  80%  60%  30%  10%         |
+--------------------------------------+
| Thu  14° / 6°   80%                  |
| Fri  11° / 4°   10%                  |
| Sat   9° / 1°    0%                  |
+--------------------------------------+
| [Open Weather]            [Refresh]  |
+--------------------------------------+
```

"Refresh" asks the background instance to refresh; the applet itself has no network. Slot a11y name: "Weather, Vienna, 12 degrees, cloudy, rain from 15:00, wind warning".

### 5.4 Menus

```
File     Add place… Ctrl+N · Remove place Delete · Place properties… Alt+Enter · Import places… ·
         Export places… · Copy forecast as text Ctrl+Shift+C · Close Ctrl+W · Quit Ctrl+Q
Edit     Undo Ctrl+Z · Copy Ctrl+C · Find place Ctrl+F · Preferences Ctrl+,
View     Sidebar F9 · Inspector F4 · Forecast Ctrl+1 · Charts Ctrl+2 · Table Ctrl+3 · Radar Ctrl+4 ·
         Alerts Ctrl+5 · Units: cycle Ctrl+U · Place time or my time Ctrl+T · Refresh F5 ·
         Zoom in Ctrl++ · Zoom out Ctrl+- · Reset Ctrl+0 · Full screen F11
Place    Next place Ctrl+Tab · Previous place Ctrl+Shift+Tab · Set as main place Ctrl+M ·
         Move up Ctrl+Up · Move down Ctrl+Down · Provider ▸ · Route ▸ · Compare providers…
Tools    Request log Ctrl+L · Data sources and licences · Manual refresh only (toggle) · Clear cache
Help     Weather help F1 · Keyboard shortcuts · About
```

### 5.5 Keyboard and pointer

Keys beyond the menus: `Left` / `Right` move through hours in the hourly strip and charts; `Up` / `Down` through days; `Enter` opens day detail; `Space` plays or pauses radar; `,` and `.` step radar frames; `Esc` closes sheets.

Context menus: place (Set as main, Rename, Properties, Provider, Route, Refresh, Copy forecast, Remove); hour cell and day row (Copy values, Show in chart, Show in table); alert (Details, Copy text, Mute this alert, Notification settings); chart (Series, Range, Show as table, Copy as CSV, Save image).

Drag and drop: places reorder in the sidebar; a `geo:` URI or coordinate text dropped on the sidebar opens Add place with the rounded cell shown; a place dragged out offers plain text (name and rounded coordinates) and a `geo:` URI of the cell centre.

Pointer: wheel over the hourly strip scrolls it horizontally; wheel over the radar zooms; hover on a chart shows the readout.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | "Add a place to see its forecast." One button. No request is possible until a place exists and the first-run notice is accepted. |
| Loading | Skeleton cards for at most 10 s; then error state. Cached data stays visible while refreshing. |
| Error | Card: "Could not get data from Open-Meteo (timeout)." Buttons: Try again, Switch provider. Cached data remains below with its age. |
| Offline | Chip "Offline · showing data from 09:12". Past hours greyed. Radar shows cached frames. |
| Stale | `--warn` chip with the age (F062). |
| Route down | "Route VPN home-vpn is down; no request was made." Link to SE03. |
| No alert feed | Alerts tab states that no official feed exists for this place (F056). |
| Narrow | See 5.2. |

### 5.7 Widgets

| Region | FUSE widget | Note |
|---|---|---|
| Sidebar, daily list, search results, alerts | `listView` / `modelListView` | drag reorder (FUSE-WP-33) |
| Tabs | `tabBar`, `tabWidget` | |
| Now card, banners | `panel`, `label`, `image` (icons), `button` | |
| Hourly strip | `scrollArea` + `gridView` | horizontal virtualised grid |
| Charts, nowcast | `chart` | needs dual axes, bar plus line series, band series, keyboard readout: IN05-GAP-1 (see FUSE-WP-46) |
| Table, request log | `tableView` | |
| Radar | `mapView` with raster overlay and time-indexed frames | FUSE-WP-48; G-12 |
| Time slider | `slider` | |
| Forms | `lineEdit`, `comboBox`, `checkBox`, `radioButton`, `spinBox`, `toggleSwitch`, `dialog` | |
| Applet | applet slot contract of SH01 (SH01-F117), `popover` | out-of-process applet embedding is SH01-F120 |

Gaps: **IN05-GAP-1** chart features listed above; **IN05-GAP-2** a background timer or scheduler service so the refresh instance need not stay resident (same need as IN02-GAP-2); **IN05-GAP-3** the location portal must offer a coarse-only request type that an app can declare, so the prompt never offers precise location to this app (same portal work as IN04-GAP-5); **IN05-GAP-4** a shared, bundled offline gazetteer and time zone polygon data set as a platform data package (UT02 Clocks, CO02 Calendar and IN04 need the same data).

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host (window instance, or windowless background instance) | Places, refresh scheduler, provider modules (request building and mapping), unit conversion, sun and moon maths, alert matching, text summary, rounding. |
| UI | FUSE UI | app host | Section 5. |
| Applet | ForgeScript + FUSE UI | separate app host started by SH01 (out-of-process applet), no network | Slot, popover, widget; reads the model over IPC. |
| Fetch helper | C++23 (safe subset) on the platform protocol helper | on demand, one per route | HTTPS GET to the listed hosts only; conditional requests; returns bytes as a descriptor. |
| Parse helper | C++23 (safe subset) | on demand, no network | JSON and XML (CAP, Atom) parsing with size and depth caps; returns schema-checked records. |
| Image helper | C++23 (safe subset) | on demand, no network | Radar frame decode (PNG and source-specific rasters) to pixels with georeference. |
| Gazetteer reader | ForgeScript over `SqliteModel` | app host | Read-only place search in the bundled data file. |

```
 SH01 panel ── applet process (no net) ──(org.pbsd.weather.forecast)──┐
                                                                      ▼
 app host: logic (ForgeScript) + UI (FUSE) ── cache.db, places in svc.settings
   ├─ rounding ► request builder ► fetch helper [route] ── network service ── provider hosts
   ├─ parse helper (JSON, CAP XML; no net) ► common model
   ├─ image helper (radar frames; no net)
   └─ svc.portal.location (coarse only) · svc.notify (alerts) · org.pbsd.maps.geocode (optional)
```

Rounding happens in app logic before the request builder sees a coordinate; the unrounded value from a coarse fix is dropped at that point. The fetch helper receives only complete URLs for allowed hosts and rejects any other host.

### 6.2 Data model

Places live in `svc.settings` (`org.pbsd.weather/places`). Cache: `~/.cache/org.pbsd.weather/cache.db` (SQLite; disposable; `schema_version` in `meta`). Request log: `~/.local/share/org.pbsd.weather/requests.db`. Gazetteer: read-only file in the bundle `assets/places.db`.

| Entity | Fields |
|---|---|
| `place` (settings) | `id u32`, `name text`, `kind enum{fixed,here}`, `cell_lat f64`, `cell_lon f64` (already snapped), `grid_deg f32`, `elevation_m i16?` (rounded to 100), `tz text`, `country text?`, `provider text?`, `alert_source text?`, `radar_source text?`, `route enum{default,clear,vpn,tor}`, `position u32`, `is_main bool`, `notify_min_severity enum?` |
| `forecast` (cache) | `place_id u32`, `provider text`, `fetched i64`, `expires i64`, `etag text?`, `last_modified text?`, `model_info text?`, `attribution text` |
| `hourly` (cache) | `place_id`, `provider`, `time i64` (UTC), `temp_c f32?`, `apparent_c f32?`, `dew_c f32?`, `rh_pct u8?`, `pressure_hpa f32?`, `wind_ms f32?`, `gust_ms f32?`, `wind_dir_deg u16?`, `cloud_pct u8?`, `precip_mm f32?`, `precip_prob_pct u8?`, `snow_cm f32?`, `visibility_m u32?`, `uv f32?`, `code u8` (condition), `is_day bool` |
| `minutely` (cache) | `place_id`, `provider`, `time i64`, `precip_mm f32`, `step_min u8` |
| `daily` (cache) | `place_id`, `provider`, `date text` (place zone), `tmax_c f32?`, `tmin_c f32?`, `precip_mm f32?`, `precip_prob_pct u8?`, `wind_max_ms f32?`, `gust_max_ms f32?`, `wind_dir_deg u16?`, `uv_max f32?`, `code u8`, `sunrise i64?`, `sunset i64?` |
| `alert` (cache) | `id text`, `source text`, `place_id u32`, `match enum{inside,near}`, `severity enum{minor,moderate,severe,extreme,unknown}`, `urgency text`, `certainty text`, `event text`, `headline text`, `description text`, `instruction text`, `onset i64?`, `expires i64?`, `sender text`, `lang text`, `web text?`, `notified bool`, `muted bool` |
| `radar_frame` (cache) | `source text`, `time i64`, `kind enum{observed,forecast}`, `bounds [f64;4]`, `width u16`, `height u16`, `pixels_path text` |
| `request` (log) | `time i64`, `host text`, `path_template text`, `sent_lat f64?`, `sent_lon f64?`, `route enum`, `status u16`, `bytes u32`, `ms u32` |
| `gazetteer.place` (bundle) | `id u32`, `name text`, `ascii_name text`, `alt_names text`, `country text`, `admin1 text`, `lat f32`, `lon f32`, `elevation_m i16`, `population u32`, `tz text`; FTS5 over names |

Units are SI in the store; conversion happens at display. Condition codes follow the WMO weather interpretation code table mapped to about 28 icons.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| HTTPS GET with conditional headers | yes | — | FORGE-WP-21 protocol helper | fetch helper |
| Open-Meteo forecast and air-quality JSON | yes | — | own schema and mapper | parse helper (JSON), app logic (mapping of checked records) |
| MET Norway Locationforecast 2.0 JSON (GeoJSON-shaped) | yes | — | own schema and mapper | parse helper |
| NWS API JSON-LD / GeoJSON (points, gridpoints, forecast, alerts) | yes | — | own schema and mapper | parse helper |
| Common Alerting Protocol 1.2 XML; Atom feeds of CAP entries | yes | — | own XML parser (shared source with IN02), own CAP mapper | parse helper |
| DWD open data products (forecast and radar formats; exact products to verify) | yes | — | own readers, written per product after verification | parse and image helpers |
| PNG radar frames and tiles | yes | PNG (share card, chart image) | platform image decoders; own PNG writer | image helper |
| Gazetteer (SQLite, built at packaging from the open source data set) | yes | — | SQLite via `SqliteModel` (PLATFORM V8) | app host (trusted, signed bundle file) |
| `geo:` URI | yes | yes | own code (shared source with IN04) | app host |
| Places JSON, CSV export | yes | yes | own code | app host |

### 6.4 Integration

- Intents handled: `view` with a `geo:` URI or place name ("weather here": the coordinate is rounded on receipt). Emitted: `open` (source pages to IN01), `share` (text or card image).
- MIME: export `text/plain`, `text/csv`, `image/png`, `application/json`.
- Services: `svc.settings`, `svc.notify`, `svc.portal.location` (coarse), `svc.portal.file` (import, export), `svc.intent`, `svc.power`, `svc.audit`, `svc.model` (optional alert translation).
- IPC offered: `org.pbsd.weather.forecast` v1, read-only: `places() -> [{id, name}]`, `current(place_id)`, `hourly(place_id, from, to)`, `daily(place_id)`, `alerts(place_id)`, `request_refresh(place_id)`; signal `updated(place_id)`. Callers: this app's applet, SH02 (summary), CO02 (event weather), UT02. Each caller other than the applet needs the user's consent once. The interface never returns unrounded coordinates because the app does not keep any.
- Applet: declared in the manifest under `"applet"` as SH01-F116 specifies (Verified in `Apps/shell/desktop-shell/SPEC.md`, features SH01-F116 to SH01-F121).
- Other apps: SH01 (applet host), SH02 (notifications), IN04 (optional geocoder; shared `mapView`), SE03 (routes), IN01 (links), UT02 and CO02 (consumers).
- Backing repos: none. Verified: `chart` and `SqliteModel` exist in FUSE (PLATFORM V3, V8). Proposed: everything else. Facts about the outside weather services in 4.2 are from general knowledge and are marked "to verify"; their terms must be read before release.

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Cache, request log. |
| `fs.portal` | Import and export of places, saving images. |
| `net.host:api.open-meteo.com:443`, `net.host:air-quality-api.open-meteo.com:443` | Default provider and optional air quality (host names to verify at build time). |
| `net.host:api.met.no:443` | MET Norway forecast and alerts. |
| `net.host:api.weather.gov:443` | NWS forecasts and alerts (United States). |
| `net.host:<dwd or front-end host>:443`, `net.host:<alert feed host>:443`, `net.host:<radar host>:443` | Added when those sources are verified and enabled; each named in the manifest of the release that ships them. |
| `net.host:<self-hosted>:<port>` | Added at run time by prompt (F018). |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Route choice (F081). |
| `dev.location` | Coarse location only (F005); prompt on first use. |
| `proc.helper:fetch`, `:parse`, `:image` | Section 6.1. |
| `svc.settings`, `svc.notify`, `svc.portal.location`, `svc.portal.file`, `svc.intent`, `svc.power`, `svc.audit` | Section 6.4. |
| `svc.model`, `ai.local` | Only for optional alert translation (F055). |
| `bg.run` | Scheduled refresh for the applet and alerts. `bg.autostart` only if the applet is on a panel (started by SH01, so normally not needed). |

Applet process capabilities: `fs.app` (none used), IPC to `org.pbsd.weather.forecast`, nothing else.

Threat model:

| | |
|---|---|
| Assets | Where the user lives, works and travels (place list and request pattern); the daily rhythm visible in request times; the integrity of warnings. |
| Adversaries | The weather provider or anyone reading its logs; a network observer; a provider or path attacker sending malformed or false data; another local app probing the forecast interface. |
| Attack surface | JSON, XML and raster parsers; the HTTP client; alert text shown to the user and sent to notifications; the IPC interface; `geo:` input from other apps. |

Mitigations:

- Coordinates are snapped to a grid before any request (F021); the unrounded value is never stored, logged or sent. Default cell is about 11 km across; forecast quality at that scale is close to the provider's own grid.
- No identifiers of any kind (F022); TLS only; named hosts only; request budget.
- Timing: jitter, no alignment to the hour, expiry honoured, manual-only mode. Multiple places are spaced; unlinkable mode uses separate Tor circuits when available.
- Route choice with fail closed.
- Parsers run in helpers with no network; alert text is treated as hostile plain text (length caps, control and bidi-override characters neutralised) before display or notification.
- Tampering: TLS protects transport. Warnings carry their source and id; the app says when no official feed exists so silence is not read as safety.
- The request log shows exactly what was sent (F065).
- The applet has no network and no location capability.

Residual risks: the provider learns the network address (unless routed) and an 11 km cell per place; a small set of places requested from one address can still identify a household by combination; rounding can place a coastal or mountain location in a cell with different weather (the place properties let the user nudge the cell by one step).

Stored: places (rounded), cache, request log. Sent: GET requests to the listed hosts with rounded coordinates and a fixed User-Agent. Nothing else.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `places` | list of `place` | [] | Section 6.2. |
| `provider.default` | enum open_meteo/met_no/nws/dwd | open_meteo | F019. |
| `provider.open_meteo_url` | string | built-in | F018 self-hosted base URL. |
| `privacy.grid_deg` | enum 0.01/0.05/0.1/0.25 | 0.1 | F021. |
| `privacy.send_elevation` | bool | false | F009. |
| `privacy.request_log` | bool | true | F065. |
| `privacy.unlinkable_places` | bool | false | F060; needs a Tor route. |
| `privacy.decoys` | int 0–5 | 0 | F084. |
| `network.route` | enum clear/vpn/tor | clear | F081. |
| `network.manual_only` | bool | false | F085. |
| `refresh.interval_min` | int 30–360 | 60 | F057. |
| `refresh.background` | bool | false | F064; needs `bg.run`. |
| `refresh.metered` | bool | false | F063. |
| `refresh.daily_budget` | int 20–1000 | 200 | F066, per provider. |
| `units.system` | enum metric/imperial/custom | locale | F067. |
| `units.temperature`, `units.wind`, `units.pressure`, `units.precip`, `units.distance` | enums | by system | F067. |
| `display.time_zone` | enum place/local | place | F068. |
| `display.wind_arrow` | enum from/to | from | F069. |
| `display.days` | int 5–16 | 7 | Daily rows shown. |
| `alerts.enabled` | bool | false | Turns on alert fetching (extra requests). |
| `alerts.notify_min_severity` | enum moderate/severe/extreme | severe | F053. |
| `alerts.break_dnd_for_extreme` | bool | false | F053. |
| `radar.enabled` | bool | false | F046. |
| `radar.autoplay` | bool | false | Forced false with reduced motion. |
| `applet.place` | id or `main` | main | F078. |
| `applet.show` | set{temp,word,rain} | {temp} | F078. |

## 9. Accessibility and languages

- Every icon has a condition word; warnings have severity words; nothing is colour-only.
- The Now card's accessible name is the text summary (F088). Hour cells read as "15:00, rain, 12 degrees, 70 percent chance, 1.8 millimetres".
- Charts have a table alternative (F087) and keyboard readout.
- Radar is visual; its alternative is the nowcast sentence and the hourly precipitation values. Animation never autoplays under reduced motion.
- Condition words, compass points, severity words and summary sentence patterns are string-table entries with plural and gender rules per language; the summary generator uses templates, not concatenation.
- Units and number formats follow the locale; temperatures use a real minus sign; right-to-left layouts mirror the hourly strip direction but keep time increasing in reading order.
- Gazetteer names are searched in local and transliterated forms; display uses the user's language when the data has it.

## 10. Performance budgets

| Metric | Budget |
|---|---|
| Cold start to first frame with cached data | 150 ms |
| Applet first frame | 100 ms; applet memory under 30 MiB |
| Place search | under 30 ms per key press over about 150 000 places |
| Response parse and map | under 20 ms per place |
| Chart frame | 4 ms with 7 days of hourly data and 5 series |
| Radar | 13 frames of 1 000×1 000 pixels decoded in under 1 s; playback at 8 frames per second |
| Network per refresh | one request per place (plus one for alerts and one for air quality if enabled); typical response under 20 KiB |
| Memory | app host 80 MiB; helpers under 48 MiB each and not resident |
| Idle | zero wake-ups between refreshes; the background instance has no UI context |
| Bundle data | gazetteer under 25 MiB |

## 11. Testing and proof

Nothing has run. Targets only.

- Unit: grid snapping (cell centres, poles, antimeridian, idempotence), unit conversions, wind chill and heat index, sun and moon times against published almanac values, alert point-in-polygon matching, scheduler jitter and budget, summary rules, provider mappers on recorded sample responses.
- Golden renders: all tabs, dialogs and the applet in three themes and three densities.
- Scripted UI through the a11y tree: tasks T1–T11 with a mock provider server.
- Fuzz: JSON and XML parsers, each provider schema mapper with mutated responses, CAP documents, radar frame decoders, `geo:` input.
- Contract tests: recorded real responses per provider, replayed; a nightly job in the project's CI (not on user machines) fetches one sample per provider to detect format changes early.
- Privacy tests: property test that no request URL ever contains a coordinate off the grid; packet capture shows only listed hosts; with location denied no fix is requested; applet process opens no socket.
- Differential: forecast values after mapping compared with the provider's own values in a second, independently written mapper for a sample (catches unit errors).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Grid snapping | Output is always a cell centre of the chosen grid; error to input at most half the cell diagonal; idempotent | T1 | PRISM proof on the snapping function (if ForgeScript is supported; else the function moves into a 20-line C++23 helper module) | Floating-point and ForgeScript support in PRISM (to verify) |
| Request builder | Every coordinate in a request passed through snapping | T1 target | Type-level separation (`SnappedCoord` is the only type the builder accepts) plus property tests | Language-level proof support |
| JSON and XML parsers | No runtime errors; bounded memory and depth | T2 | PRISM; fuzz | — |
| Provider mappers | Total on any schema-valid record; units correct | T3 | Unit and differential tests | Semantic correctness is not provable against outside services |
| Fetch helper host allow-list | No connection to a host outside the list | T1 (inherited from FORGE-WP-21 rule matcher) | As FORGE-WP-21 | — |
| Alert text sanitiser | Output has no control or bidi-override characters; length bounded | T1 | PRISM proof of the filter | — |
| Radar decoders | No runtime errors | T2 | PRISM; fuzz | Format complexity per source |
| Sun and moon maths | Domain-safe (no NaN, polar day and night handled) | T2 | PRISM with interval analysis; almanac tests | — |

## 12. Build plan

| ID | Work item | Depends on |
|---|---|---|
| | **Phase 1: usable core** | |
| IN05-W01 | Common model, cache store, places in settings, grid snapping with tests | G-02, G-07, `svc.settings` |
| IN05-W02 | Gazetteer build step and offline place search | IN05-GAP-4, PLATFORM V8 |
| IN05-W03 | Fetch helper with host allow-list, conditional requests, budget, request log | FORGE-WP-21, G-08 |
| IN05-W04 | Parse helper (JSON), Open-Meteo provider module | FORGE-WP-28 |
| IN05-W05 | MET Norway provider module; provider per place | W04 |
| IN05-W06 | Main window: sidebar, Now card, hourly strip, daily list, day detail, states | G-02 |
| IN05-W07 | Units, time zones, first-run notice, attribution, manual-only mode | W06 |
| IN05-W08 | Terms review of both providers; fixed User-Agent text agreed | — |
| | **Phase 2: full feature** | |
| IN05-W09 | Charts tab and hourly table; sun and moon | IN05-GAP-1, FUSE-WP-46 |
| IN05-W10 | Background refresh instance, scheduler with jitter, metered and battery rules | `bg.run`, `svc.power`, IN05-GAP-2 |
| IN05-W11 | `org.pbsd.weather.forecast` interface and the panel applet | SH01 (SH01-F116 to F121), G-08 |
| IN05-W12 | Coarse location, "Here" place, pick on map | `svc.portal.location`, IN05-GAP-3, FUSE-WP-48 |
| IN05-W13 | Alerts: CAP parser, NWS and MET Norway sources, matching, banner, notifications | W03, `svc.notify` |
| IN05-W14 | NWS forecast provider; self-hosted endpoint; route choice | W04, SE03 |
| IN05-W15 | Nowcast strip, text summary, accessibility names | W06 |
| | **Phase 3: advanced** | |
| IN05-W16 | Radar: source verification, image helper, `mapView` overlay, animation | G-12, FUSE-WP-48, W03 |
| IN05-W17 | DWD source (forecast, warnings, radar) after verification; MeteoAlarm after terms check | W13, W16 |
| IN05-W18 | Air quality, UV and pollen, marine and mountain values | W04 |
| IN05-W19 | Provider comparison, compare places, forecast history, uncertainty bands | W09 |
| IN05-W20 | Unlinkable places over Tor circuits; decoy requests after terms review | SE03 |
| IN05-W21 | Desktop widget sizes, threshold notifications, command line, share card | W11 |

## 13. Open questions and risks

1. Provider terms are the main risk. "Free without a key" services have fair-use limits and may restrict commercial distribution; a distribution shipping this app to many users must check each provider's terms and may need a self-hosted or mirrored endpoint. Every provider fact in 4.2 is "to verify".
2. A required identifying User-Agent (MET Norway) identifies the application, not the user; the exact text and contact address must be agreed by the project.
3. Open radar data exists for only some countries; most users will see "no open radar data". A global radar tile service without open terms is deliberately not included.
4. Rounding to 0.1° can be wrong for coasts, valleys and mountains. The nudge control and the finer grids are the answer; the default stays coarse.
5. Decoy requests (F084) add load to free services and may breach terms; kept as "could" until reviewed.
6. The applet needs SH01's out-of-process embedding (SH01-F120), which the FUSE plan says needs a compositor embedding protocol (noted there as Q-05). Until then the applet could run only as an in-process first-party applet (SH01-F119), which puts Weather's applet code in the shell process; acceptable only because the applet has no parser and no network.
7. Whether SE03 offers a Tor route is not established (its backing repo is a consortium mixnet, `ANONYMOUS:README.md`); F060 depends on it.
8. The gazetteer licence (CC BY 4.0 if GeoNames is used) needs attribution in About and a check that redistribution in the bundle is allowed.
9. Alert coverage is partial. The app must never imply it is a complete warning system (F056).
