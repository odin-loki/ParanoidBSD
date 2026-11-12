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
