<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Maps

By Odin Loch

| | |
|---|---|
| ID | `IN04` (from CATALOGUE.md) |
| Bundle | `org.pbsd.maps` |
| Category | internet |
| Priority | P2 |
| Backing repos | FUSE (the `mapView` widget is planned in FUSE-WP-48 on the 2D canvas of FUSE-WP-43; neither is built). No map code exists in any repo read. |
| Status | Planned. Nothing built. |

## 1. Purpose

Maps shows vector maps that are stored on the machine, searches them, plans routes and records tracks, all without a network connection. It is for travellers, walkers, cyclists, drivers and anyone who wants to look something up without telling a server where they are. It replaces Marble, GNOME Maps and web map sites.

## 2. Scope

In scope:

- Offline vector maps from OpenStreetMap extracts, packaged per region; a region download manager.
- The map data plan: tile format, schema, style sheets, and the helper that feeds the `mapView` renderer (platform gap G-12).
- Offline search: places, addresses, categories, coordinates; reverse geocoding.
- Offline routing: car, bicycle, foot; public transport where a timetable feed is installed.
- Saved places and lists; GPX, KML and GeoJSON import and export; track recording.
- Measuring distance, area, bearing and elevation profile.
- Layers: terrain shading, contours, optional satellite imagery from a source the user configures.
- Coordinate tools; sharing a place as a `geo:` URI, text or QR code.
- Location only through `dev.location`, with a coarse option; no history unless enabled.

Out of scope:

| Item | Owner |
|---|---|
| Editing OpenStreetMap data | none (the user can open the OSM site in IN01) |
| Live traffic, live vehicle positions | not offered: no open source without tracking |
| Weather layers and radar | IN05 Weather (it uses the same `mapView`) |
| Photo map of the photo library | ME01 Photos (uses `mapView`; may ask Maps for reverse geocoding through an interface in 6.4) |
| Downloading files in general | IN03 Transfers (region downloads use this app's own fetch helper because packages are signed and resumable in a known format) |
| Routes over VPN or Tor | SE03 Anonymity |
| Scanning QR codes | ME04 Camera |
| Fleet tracking, sharing live location with others | not offered |

## 3. Users and top tasks

Personas:

- **Traveller**: downloads a country before a trip, searches addresses, navigates on foot with no data plan.
- **Walker or cyclist**: plans a route with elevation, exports GPX to a device, records the trip.
- **Driver**: wants a route with turn instructions and options to avoid tolls and ferries.
- **Private user**: wants a map with no network at all and location off.
- **Researcher**: loads GeoJSON overlays, measures, exports a map image for a report.

| # | Task | Target steps |
|---|---|---|
| T1 | Find an address | `Ctrl+F`, type, Enter (3) |
| T2 | See what is at a point | right click, "What is here?" (2) |
| T3 | Route from here to a place | select place, `D` (directions), Enter (3) |
| T4 | Download a region | Regions, tick region, Download (3) |
| T5 | Save a place to a list | select, `Ctrl+D`, pick list (3) |
| T6 | Import a GPX track and see its profile | drag file onto the window (1) |
| T7 | Record a walk | `Ctrl+R`, later `Ctrl+R` again (2) |
| T8 | Measure a distance | `M`, click points, Enter (3+) |
| T9 | Share a place | select, `Ctrl+Shift+S`, pick `geo:` or QR (3) |
| T10 | Convert coordinates | Tools > Coordinates, paste (2) |
| T11 | Show my position, coarse | click locate; first time: shell prompt, pick "Approximate" (2) |
| T12 | Switch to the outdoor style with contours | layer button, tick (2) |
| T13 | Print a map of an area | `Ctrl+P`, set frame and scale, Print (3) |

## 4. Features

Priority: M must, S should, C could. Phase: 1 usable core, 2 full feature, 3 advanced.

### 4.1 Map view

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F001 | Vector map display | `mapView` draws vector tiles from installed regions. Zoom 0–20 (data to 14, over-zoom above), fractional zoom, 60 frames per second target while panning. | M | 1 |
| IN04-F002 | World base map | A low-detail world (zoom 0–6: coastlines, countries, large cities, main roads) ships with the app (about 60 MiB) so the map is never blank. | M | 1 |
| IN04-F003 | Pan and zoom | Drag, wheel, pinch, double-click, `+`/`-`, arrow keys, kinetic pan (off with reduced motion), zoom to rectangle with `Shift`+drag. | M | 1 |
| IN04-F004 | Rotation and tilt | Rotate with `Ctrl`+drag or two fingers; `N` resets north-up; tilt to 60 degrees is C. Labels stay upright. | S | 2 |
| IN04-F005 | Styles | Standard, Outdoor (paths, contours, hill shading), Transport (lines and stops), Minimal (for overlays), each in light, dark and high contrast. Style follows the system theme unless fixed. | M | 2 |
| IN04-F006 | Labels | Names in the user's language when the data has `name:<lang>`, else local name; option to show both. Collision handling is deterministic (FUSE-WP-48). | M | 1 |
| IN04-F007 | Label size and density | Setting for label scale (80–200%) and POI density (few, normal, many). | S | 2 |
| IN04-F008 | Scale bar and attribution | Scale bar in metric, imperial or nautical. Attribution "© OpenStreetMap contributors" plus data date always visible; click opens the licence text. | M | 1 |
| IN04-F009 | Compass and controls | Zoom buttons, compass, locate button, layer button at the right edge; each has a shortcut and an accessible name. | M | 1 |
| IN04-F010 | Coordinates under pointer | Status bar shows latitude and longitude in the chosen format, zoom level, and elevation when terrain data is installed. | S | 1 |
| IN04-F011 | Feature selection | Click or `Enter` on a map feature selects it and opens the place card. Overlapping features cycle with repeated clicks or `Tab`. | M | 1 |
| IN04-F012 | "What is here?" | Reverse geocoding of any point: nearest address, containing areas (suburb, city, region, country), nearby features. | M | 2 |
| IN04-F013 | Missing-data hint | Areas outside installed regions show the base map with a hatched overlay and a chip "Detailed map not installed: Get region". | M | 1 |
| IN04-F014 | Multiple windows and tabs | Several map tabs or windows, each with its own view and overlays. | S | 2 |
| IN04-F015 | View history | `Alt+Left` and `Alt+Right` step through previous views in this session (not stored). | S | 1 |
| IN04-F016 | Bookmarked views | Save the current view (centre, zoom, rotation, style, layers) under a name. | S | 2 |
| IN04-F017 | Mini-map | Optional overview inset at the corner. | C | 3 |
| IN04-F018 | Globe at low zoom | Not planned; Web Mercator throughout, with a note in help about area distortion. A projection hook exists in the widget plan (FUSE-WP-48). | C | 3 |

### 4.2 Regions and map data

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F019 | Region packages | One signed package per region with: vector tiles, search index, routing graph per profile, metadata (bounds polygon, data date, sizes, schema versions). Optional add-ons per region: terrain, contours, transit. | M | 1 |
| IN04-F020 | Region catalogue | A signed index file lists regions as a tree (continent, country, state or province) with sizes and dates. The index ships with the app and is refreshed only when the user opens Regions and presses Refresh. | M | 1 |
| IN04-F021 | Region manager | Tree with tick boxes, size, installed date, available date, state; totals; free space. Pick regions on the map by clicking their outline. | M | 1 |
| IN04-F022 | Download | Resumable range requests from the package mirror; signature (Ed25519 over BLAKE3) and size checked before install; install is an atomic rename. | M | 1 |
| IN04-F023 | Mirror choice | The mirror host is a setting (default: the distribution's package mirror, name to be set by SY05's update configuration). The capability names exactly that host. | M | 1 |
| IN04-F024 | Install from file | Install a region package from a file or removable drive through the portal, with the same signature check. Lets a machine with no network have maps. | M | 1 |
| IN04-F025 | Updates | Shows which regions have newer data; update one or all; never automatic unless the user enables it and only on unmetered links. Old data stays in use until the new package verifies. | S | 2 |
| IN04-F026 | Delta updates | Binary diffs between package versions to cut download size. | C | 3 |
| IN04-F027 | Component choice | Per region: map only, map and search, map, search and routing; add-ons separately. | S | 2 |
| IN04-F028 | Storage location | Regions live in the app data directory or in a folder the user picks (for an external drive). Missing drive: regions show as "not available". | S | 2 |
| IN04-F029 | Border continuity | Tiles of adjacent regions are merged at render time; routing graphs carry border nodes so routes cross region borders when both sides are installed. | M | 2 |
| IN04-F030 | Privacy of downloads | The mirror learns which regions were fetched. The Regions view says so and offers the route choice (clear, VPN, Tor) for the download. | M | 2 |
| IN04-F031 | Build a region locally | Advanced: convert an `.osm.pbf` extract picked in the portal into a region package with the same builder the distribution uses; runs in a helper, shows progress, may take hours. | C | 3 |
| IN04-F032 | Data licence view | Per region: OpenStreetMap ODbL notice, data date, elevation and transit data sources and licences. | M | 1 |
| IN04-F033 | Integrity check | "Verify installed regions" re-checks signatures and SQLite integrity; damaged regions are flagged and can be re-downloaded. | S | 2 |

### 4.3 Search and geocoding

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F034 | Search field | One field for names, addresses, categories and coordinates. Results in a list and as pins. Search is local; no key press leaves the machine. | M | 1 |
| IN04-F035 | Name search | Places, streets, POIs by any stored name (local, translated, alternative, old). Prefix matching, accent folding, case folding, script transliteration where the index has it. | M | 1 |
| IN04-F036 | Address search | Street plus house number plus city or postcode in flexible order; house numbers by exact point or interpolation along the street; per-country address patterns from a data table. | M | 2 |
| IN04-F037 | Typo tolerance | Edit distance 1 for words of 4–7 letters, 2 for longer; exact matches rank first. | S | 2 |
| IN04-F038 | Category search | "pharmacy", "drinking water", "bicycle repair": a translated category table maps words to OSM tags. Category chips under the field. | M | 2 |
| IN04-F039 | Search scope and ranking | Ranked by text match, importance (place type, population) and distance from the view centre; "Search this area" restricts to the view. | M | 1 |
| IN04-F040 | Coordinate input | Decimal degrees, degrees-minutes, degrees-minutes-seconds, UTM, MGRS, Open Location Code, geohash, Maidenhead locator; `geo:` URIs; map-site URLs pasted as text (coordinates are extracted, the site is not contacted). | M | 2 |
| IN04-F041 | Suggestions while typing | From the local index after 2 characters, at most 8 rows, under 50 ms. | S | 2 |
| IN04-F042 | Recent searches | Kept only if "Remember searches" is on (default off); clear button. | S | 2 |
| IN04-F043 | Filters | Open now (from `opening_hours`), wheelchair access, distance limit, category. | S | 3 |
| IN04-F044 | Reverse geocoding service | The function behind F012 is also offered to other apps through IPC (6.4) so ME01 can label photos without a network. | S | 3 |
| IN04-F045 | Search across regions | One query runs over all installed regions; results show their region. | M | 2 |
| IN04-F046 | Nothing found | States which regions were searched and offers "Get region" if the view is in an uninstalled area. No online fallback. | M | 1 |

### 4.4 Place details

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F047 | Place card | Name, type, address, coordinates, distance from the reference point, opening hours with "open now", phone, website, wheelchair, fee, operator, cuisine and other common tags as readable rows. | M | 1 |
| IN04-F048 | Opening hours | Parser for the OSM `opening_hours` syntax (common subset first); shows today's hours and the week table; unparsed values are shown raw. | S | 2 |
| IN04-F049 | Actions | Directions to, Directions from, Save, Share, Copy coordinates, Copy address, Measure from here, Open website (IN01), Call (`tel:` intent), Add to contact (CO03). | M | 1 |
| IN04-F050 | All tags | Expandable raw tag table with OSM element type and id; "Copy OSM link" (text only). | S | 2 |
| IN04-F051 | Wikipedia and Wikidata tags | Shown as links that open in IN01 on click; nothing is fetched by Maps. | C | 3 |
| IN04-F052 | Personal note | A note and a colour on any saved place. | S | 2 |
| IN04-F053 | Nearby | List of POIs within 300 m of the selected place, by category. | C | 3 |

### 4.5 Routing

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F054 | Profiles | Car, bicycle, foot. Each respects access tags, one-ways, turn restrictions, barriers, surfaces. Further profiles as data files: road bike, mountain bike, hiking, wheelchair, motorcycle, truck (C). | M | 2 |
| IN04-F055 | Route planning | From, to and up to 20 via points; set by search, by click, by saved place or by current location. Drag the route line to add a via point. | M | 2 |
| IN04-F056 | Alternatives | Up to 3 alternatives with time, distance, climb; select by clicking a line or a row. | S | 2 |
| IN04-F057 | Options | Avoid motorways, tolls, ferries, unpaved roads, steps; prefer quiet roads (bicycle); maximum slope (wheelchair, bicycle). Options do not need a rebuild of the graph (6.5). | M | 2 |
| IN04-F058 | Optimisation choice | Fastest, shortest, or balanced; for bicycle and foot also "least climb". | S | 2 |
| IN04-F059 | Instructions | Turn-by-turn list with distance, street name, manoeuvre icon and word, lane and exit numbers where tagged; click a step to see it on the map. | M | 2 |
| IN04-F060 | Elevation profile | Graph of height along the route with total ascent and descent; hover links graph and map. Needs the terrain add-on. | S | 2 |
| IN04-F061 | Time estimates | From tagged or default speeds per road class and profile; walking uses a slope-aware rule. Stated as estimates; no traffic data. | M | 2 |
| IN04-F062 | Departure or arrival time | Used for transit and for time-dependent access (for example ferries with hours); otherwise informational. | S | 3 |
| IN04-F063 | Round trip | "Loop of about N km from here" for foot and bicycle, with a direction preference. | C | 3 |
| IN04-F064 | Reorder stops | Drag stops in the list; "Optimise order" for up to 12 stops (exact search for small sets, heuristic above 9). | C | 3 |
| IN04-F065 | Public transport | Where a GTFS timetable feed is installed for the area: earliest-arrival and fewest-transfers journeys, walking legs to and from stops, line names and colours, stop times. | S | 3 |
| IN04-F066 | Transit data management | Install a GTFS zip from a file or from a URL the user enters; shows validity dates and licence text from the feed; warns when the feed has expired. | S | 3 |
| IN04-F067 | Route across regions | Works when all regions on the way are installed; otherwise the planner names the missing region. | M | 2 |
| IN04-F068 | Export route | GPX (route and track), GeoJSON, KML, a printable instruction sheet (PDF through print), plain text. | M | 2 |
| IN04-F069 | Save route | Named routes in the library with profile and options, so they can be recomputed after a data update. | S | 2 |
| IN04-F070 | Navigation mode | With location on: follows the position, shows the next manoeuvre, distance and time remaining, re-routes when off the route by more than 40 m for 10 s. Screen kept on through `svc.power`. | S | 3 |
| IN04-F071 | Spoken instructions | Through the system speech service used by AX01; phrases come from string tables per language. | C | 3 |
| IN04-F072 | Isochrones | Area reachable within N minutes for a profile, drawn as a polygon. | C | 3 |

### 4.6 Places, lists and overlays

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F073 | Save a place | `Ctrl+D`: name, list, colour, icon, note. Works for map features and for bare points. | M | 1 |
| IN04-F074 | Lists | Named lists (Favourites, Want to go, custom); show or hide each on the map; colour and icon per list. | M | 1 |
| IN04-F075 | Library sidebar | Tree: lists, routes, tracks, imported layers, bookmarked views. Search within the library. | M | 1 |
| IN04-F076 | Home and work | Optional named places used as routing shortcuts. Stored locally like any place. | S | 2 |
| IN04-F077 | Import overlays | GPX 1.0/1.1 (waypoints, routes, tracks), KML 2.2 and KMZ (placemarks, lines, polygons, folders, basic styles; no network links), GeoJSON (RFC 7946). Parsed in the import helper. | M | 2 |
| IN04-F078 | Overlay styling | Per layer or per feature: colour, width, opacity, marker icon, label field. | S | 2 |
| IN04-F079 | Overlay table | Attribute table of a layer's features (`tableView`); click a row to fly to it; sort and filter. | S | 3 |
| IN04-F080 | Export | Any list, route, track or layer as GPX, KML or GeoJSON; whole library as one archive. | M | 2 |
| IN04-F081 | Edit geometry | Move, add and delete points of own places, routes drawn by hand and tracks; split and join tracks; reverse; simplify (Douglas-Peucker with a tolerance). | S | 3 |
| IN04-F082 | Draw | Add markers, lines, polygons, circles and text notes as an own layer. | S | 3 |
| IN04-F083 | Large overlays | Layers up to 1 million vertices stay smooth: geometry is tiled and simplified per zoom in the import helper. | S | 3 |
| IN04-F084 | Contacts on the map | Addresses from CO03 are shown only when the user opens a contact's address by intent; Maps does not read the address book. | S | 3 |
| IN04-F085 | Encrypted library | The library file may be placed in an SE05 vault; when locked, only the map works. | C | 3 |

### 4.7 Tracks and recording

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F086 | Record a track | `Ctrl+R` starts and stops. Points every 1–60 s or every 5–100 m (setting). The shell's location indicator is on throughout. | S | 3 |
| IN04-F087 | Recording consent | Recording needs precise location; starting it shows what is stored and where. Background recording needs `bg.run`, asked at that moment. | M | 3 |
| IN04-F088 | Pause, resume, segments | Pauses create a new segment; auto-pause when still for 60 s (setting). | S | 3 |
| IN04-F089 | Live statistics | Distance, moving time, average and current speed, ascent, descent, current height. | S | 3 |
| IN04-F090 | Track view | Line coloured by speed, height or slope; profile graph; statistics; per-segment table. | S | 2 |
| IN04-F091 | Track cleaning | Remove outliers, smooth height, trim ends (for privacy before sharing: "hide the first and last 200 m"). | S | 3 |
| IN04-F092 | Crash-safe recording | Points are appended to a journal every 10 s; a crash loses at most that. | M | 3 |
| IN04-F093 | Map matching | Snap a noisy track to the road or path network (hidden Markov model) to produce a clean route. | C | 3 |
| IN04-F094 | Timestamps on export | Option to strip times and device fields from exported tracks. | S | 2 |
| IN04-F095 | Photos along a track | ME01 may ask for a track to place photos by time; the user picks the track in a Maps-drawn picker. | C | 3 |

### 4.8 Measurement and coordinate tools

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F096 | Distance | `M`: click points; shows segment and total length on the WGS 84 ellipsoid (geodesic), with initial bearing per segment. | M | 2 |
| IN04-F097 | Area | Close the shape to get area and perimeter (geodesic polygon area). | S | 2 |
| IN04-F098 | Radius | Circle of a given radius around a point; range rings at steps. | C | 3 |
| IN04-F099 | Units | Metric, imperial, nautical; areas in m², ha, km², acres, square miles. | M | 2 |
| IN04-F100 | Height at a point and profile along a line | Needs terrain add-on; shows data source and stated accuracy. | S | 3 |
| IN04-F101 | Save a measurement | Keeps it as a line or polygon in the library. | S | 3 |
| IN04-F102 | Coordinate converter | Dialog converting between the formats of F040; shows all at once; copy buttons; validity messages. | M | 2 |
| IN04-F103 | Datum note | Everything is WGS 84. Conversion to national grids (as data-driven Helmert and projection parameter sets) is C. | C | 3 |
| IN04-F104 | Sun and time | Sunrise, sunset and local time zone at a point, computed locally from the bundled time zone polygons. | C | 3 |
| IN04-F105 | Grid overlays | Latitude and longitude graticule, UTM grid, MGRS grid as optional layers. | C | 3 |

### 4.9 Layers

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F106 | Layer panel | `popover` with base style, add-on layers and overlay layers; visibility, opacity and order. | M | 2 |
| IN04-F107 | Terrain shading | Hill shading from elevation tiles in the terrain add-on, computed on the device from height data so light direction and strength are settings. | S | 2 |
| IN04-F108 | Contours | Vector contour lines (10 m, 50 m, 100 m by zoom) with height labels, from the contours add-on. | S | 2 |
| IN04-F109 | Satellite imagery | Off and absent by default. The user may add an imagery source by URL template (XYZ, TMS or WMTS) with its attribution text. Needs a `net.host` grant for that host. | S | 3 |
| IN04-F110 | Imagery privacy notice | Before the first tile request: "The imagery server will see which areas you look at." Route choice offered. Tiles are cached; a "use cached only" switch exists. | M | 3 |
| IN04-F111 | Imagery tile handling | Fetched by the fetch helper with no cookies and no referrer, decoded in the image helper, cache limit in MiB, cache clear button. | M | 3 |
| IN04-F112 | Other raster sources | Any XYZ, TMS or WMTS source the user adds (historic maps, cadastral maps), same rules as F109–F111. Local MBTiles or PMTiles raster files open with no network. | C | 3 |
| IN04-F113 | Public transport layer | Lines and stops from OSM route relations, in the Transport style. | S | 2 |
| IN04-F114 | Cycling and hiking routes | Signed route networks from OSM route relations as coloured overlays with reference labels. | S | 2 |
| IN04-F115 | Custom style | Advanced: load a user style sheet (the app's own style format, 6.5) to restyle the vector map. Data only, no script. | C | 3 |

### 4.10 Location and privacy

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F116 | Location off by default | The app never asks for location until the user presses Locate, starts recording or starts navigation. | M | 1 |
| IN04-F117 | Coarse or precise | The shell prompt offers Approximate and Precise. Approximate shows a circle (radius from the portal, typically 1–3 km) and no dot. Routing from an approximate position says so. | M | 1 |
| IN04-F118 | Follow modes | Off, centre on position, centre and rotate to heading (navigation). Any manual pan stops following. | S | 2 |
| IN04-F119 | Location indicator | The header shows the location icon while a location handle is open; clicking it stops location use at once. | M | 1 |
| IN04-F120 | No history | Positions are not stored unless a recording is running. Searches, routes and viewed places are not stored unless "Remember recent" is on. | M | 1 |
| IN04-F121 | No network by default | The app runs with no sockets. Region download, imagery and transit URL install each need their own grant and each shows `[net]`. | M | 1 |
| IN04-F122 | Clear private data | One action clears recent searches, recent places, view history and the imagery cache. | M | 2 |
| IN04-F123 | Manual position | "Set my position here" for routing and distance without the location device. | S | 1 |

### 4.11 Share, print and integration

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F124 | Share a place | `geo:` URI (RFC 5870, with `?q=` label and `u=` uncertainty for coarse positions), coordinates text, Open Location Code, address text, or a QR code of the `geo:` URI drawn locally. | M | 2 |
| IN04-F125 | Coarsen before sharing | Option to round shared coordinates to 3, 2 or 1 decimal places; default is full precision for map features and 3 places for "my position". | M | 2 |
| IN04-F126 | Open `geo:` links | Handles the `geo:` scheme and "show on map" intents with coordinates or an address string (searched locally). | M | 1 |
| IN04-F127 | Map image export | PNG or SVG of the view or a chosen frame at a chosen scale and resolution, with attribution burned in. | S | 2 |
| IN04-F128 | Print | Print dialog with frame, scale (for example 1:25 000), paper size, grid, legend, scale bar; multi-page tiling for large areas; PDF through the print pipeline (G-16). | S | 3 |
| IN04-F129 | Copy map | Copies the view as PNG plus a `geo:` URI as text. | S | 2 |
| IN04-F130 | Search provider | Saved places and lists are offered to `svc.search` (setting, default off). | C | 3 |
| IN04-F131 | Command line | `maps "geo:…"`, `maps --search "text"`, `maps --route from to --profile foot --export out.gpx` (runs headless and exits). | C | 3 |

### 4.12 Settings, accessibility and recovery

| ID | Feature | Detail | Pri | Phase |
|---|---|---|---|---|
| IN04-F132 | Preferences | Pages: General, Map, Units and coordinates, Regions, Routing, Location and privacy, Layers, Shortcuts. | M | 1 |
| IN04-F133 | Feature list alternative | A list of the features visible in the view (name, type, bearing and distance from the centre), as the non-visual way to explore the map (FUSE-WP-48). | M | 2 |
| IN04-F134 | Keyboard map cursor | A crosshair moved by keys; `Enter` selects what is under it; `Shift+arrows` jumps to the next feature in that direction. | M | 2 |
| IN04-F135 | Session restore | View, open panels, current route and overlays return after a restart (if "Remember recent" is on; else only the view). | S | 1 |
| IN04-F136 | Library safety | The library store is journalled; every import can be undone; `svc.backup` covers it. | M | 1 |
| IN04-F137 | Low-power mode | On battery: frame rate cap of 30 during animations, no kinetic pan, hill shading cached not recomputed. | C | 3 |

## 5. GUI

### 5.1 Window map

```
Maps window
├─ Header bar: menu, title, search field, location indicator, [net] badge (only when a grant is live)
├─ Sidebar (F9), one of: Search results · Library (lists, routes, tracks, layers, views) · Directions · Regions
├─ Map (mapView): controls column, scale bar, attribution, coordinates chip, missing-data chip
│   ├─ Place card (bottom sheet or inspector)
│   ├─ Layer panel (popover)
│   ├─ Measure bar (overlay)
│   └─ Navigation banner (overlay, navigation mode)
├─ Inspector (F4): place details | route details | track statistics | layer style
├─ Bottom panel (toggle): elevation profile | overlay attribute table | instructions
└─ Status bar: coordinates, zoom, elevation, data date, recording state
Dialogs and sheets
├─ Region manager · Install region from file · Add imagery source · Install transit feed
├─ Save place · Share place (with QR) · Coordinate converter · Import overlays · Export
├─ Print map · Export image · Route options · Preferences
└─ Shell-drawn: location prompt (Approximate / Precise), file chooser, print, net.host prompt
```

### 5.2 Main window

Shown at 1280×800, directions sidebar open, bottom profile panel open.

```
+----------------------------------------------------------------------------------------+
|[=] Maps        [ search places, addresses, coordinates…      ]  (◎) [▁] [▢] [✕]       | (1)
+---------------------------+------------------------------------------------------------+
| [Search][Library][Route]  |                                                      [+]   | (4)
| ( ) Car (•) Bike ( ) Foot |          ~~~~ river ~~~~                             [−]   |
| A [ Central Station     ] |     ┌──────────┐        ● B                          [N]   |
| + [ via: Old Bridge     ] |     │  park    │      ╱                              [◎]   |
| B [ Hill View Lookout   ] |     └──────────┘    ╱   route line                   [▤]   |
| [+ stop]  [Options…]      |          A ●──────╱                                        |
| ------------------------- |                                                            | (3)
| > 42 min · 11.8 km · ↑210 |   ░░░░ hatched: region not installed  [Get region]         |
|   47 min · 10.9 km · ↑340 |                                                            |
| ------------------------- |  |—— 500 m ——|                © OpenStreetMap · 2026-10    | (5)
| 1 ↑ Head north on Quay St +------------------------------------------------------------+
|   350 m                   | Elevation  ↑210 m ↓40 m                                    |
| 2 ↰ Turn left: Old Bridge |  240┤            ╱‾‾‾╲___╱‾‾‾‾‾‾                           | (6)
|   1.2 km                  |   60┤___╱‾‾‾╲___╱                                          |
| 3 ↱ Turn right: Hill Rd   |     0        4 km        8 km       11.8 km                |
+---------------------------+------------------------------------------------------------+
| 48.20850° N 16.37280° E · z 13.4 · 212 m · Data: 2026-10-06 · Location: off           | (7)
+----------------------------------------------------------------------------------------+
  (2) sidebar
```

States: empty (first run: base map only), loading (tiles of a just-installed region), error (region damaged), offline (normal: nothing changes), locked (library in a vault), read-only (region drive missing), narrow.

1. Header bar. Search `lineEdit` is the centre of the header. `(◎)` is the location indicator, shown only while location is in use. No `[net]` badge unless a network grant is live.
2. Sidebar, 300 px (wider than the standard 220 px because of route steps), `tabBar` with three modes. Directions: profile `radioButton`s, stop `lineEdit`s in a reorderable `listView`, alternatives `listView`, instructions `listView`.
3. `mapView` (FUSE-WP-48). Overlays: route line with casing, stops, alternatives in `--fg-2`, selected place pin, own position dot or coarse circle.
4. Controls column, right edge, 36 px `toolButton`s: zoom in, zoom out, north, locate, layers.
5. Scale bar bottom left; attribution and data month bottom right (a `link`).
6. Bottom panel, 160 px, `chart` for the profile; hover shows a marker on the route.
7. Status bar: coordinates under the pointer or cursor, zoom, height, data date of the region under the centre, location state in words.

Narrow (<640): sidebar becomes a bottom sheet with three heights (peek 96 px, half, full); controls stay; the bottom panel becomes a tab of the sheet.

### 5.3 Other views and dialogs

#### 5.3.1 Region manager (dialog or sidebar page, 820×600)

```
+----------------------------------------------------------------------------------+
| Regions                              Mirror: maps.mirror (set by the system)     |
| Installed 3 · 2.1 GiB        Free 212 GiB         Catalogue: 2026-11-01 [Refresh]|
+--------------------------------------------+-------------------------------------+
| [ filter…              ]                   |   (outline map: click a region)     |
| [ ] ▾ Europe                               |        ┌───┬────┐                   |
| [x]   ▾ Austria          612 M  2026-10 ✓  |        │███│    │                   |
| [x]       Vienna          88 M  2026-10 ✓  |        ├───┼────┤                   |
| [ ]       Tyrol          140 M  —          |        │   │░░░░│  █ installed      |
| [x]   ▸ Czechia          540 M  2026-08 ↑  |        └───┴────┘  ░ selected       |
| [ ] ▸ Oceania                              |                                     |
+--------------------------------------------+-------------------------------------+
| Components [x] Map [x] Search [x] Routing  Add-ons [ ] Terrain [ ] Contours      |
| Route for downloads [ Clear v ]  The mirror will see which regions you fetch.    |
| Selected: +140 MiB download, +310 MiB on disk                                    |
|                         [Install from file…] [Remove…] [Download and install]    |
+----------------------------------------------------------------------------------+
```

`treeView` with check boxes and columns; outline map is a small `mapView` in Minimal style; `checkBox`, `comboBox`, `button`, `progressBar` per row while downloading. Marks: ✓ current, ↑ update available, each with a word in the a11y name. States: catalogue stale (over 60 days: hint), no network grant yet (Download shows the shell prompt first), downloading (row progress, Pause), verifying, signature failed (row in `--danger`, file discarded), drive missing.

#### 5.3.2 Place card (inspector, 280 px, or bottom sheet)

```
+--------------------------------------+
| Hill View Lookout              [✕]   |
| Viewpoint · 1.8 km north-east        |
| Hill Road, 1190 Vienna               |
| 48.26010° N 16.35870° E  [Copy]      |
| Height 484 m                         |
+--------------------------------------+
| [Directions] [Save] [Share]          |
+--------------------------------------+
| Open     24 hours                    |
| Access   yes · wheelchair: limited   |
| Website  example.org/lookout  ↗      |
| Note     [ add a note…            ]  |
| ▸ All tags (9)  ·  OSM node 123456   |
+--------------------------------------+
```

`label`s, `button`s, `link`, `lineEdit`, collapsible `panel` with a two-column `tableView` for tags.

#### 5.3.3 Share place (dialog, 460×420)

```
+--------------------------------------------------+
| Share "Hill View Lookout"                        |
| Precision [ Full v ]  (Full · ~100 m · ~1 km)    |
| geo:48.2601,16.3587?q=Hill%20View%20Lookout      |
|                                        [Copy]    |
|            ┌─────────────────┐                   |
|            │ █▀▀█ ▄ █▀▀█     │                   |
|            │ █  █ ▀ █  █ QR  │                   |
|            │ ▀▀▀▀ █ ▀▀▀▀     │                   |
|            └─────────────────┘                   |
| Also copy: [Coordinates] [Address] [Plus code]   |
|                 [Save QR as PNG…] [Share…] [Done]|
+--------------------------------------------------+
```

The QR code is generated locally (own encoder) and drawn through `image`. "Share…" opens the shell share sheet with the `geo:` URI as text.

#### 5.3.4 Coordinate converter (dialog, 520×400)

```
+----------------------------------------------------------+
| Coordinates                                              |
| Input [ 48°12'30.6"N 16°22'22.1"E                    ]   |
| Read as: degrees, minutes, seconds                       |
+----------------------------------------------------------+
| Decimal degrees     48.20850, 16.37281          [Copy]   |
| Degrees, minutes    48°12.510'N 16°22.369'E     [Copy]   |
| DMS                 48°12'30.6"N 16°22'22.1"E   [Copy]   |
| UTM                 33U 601971 5340510          [Copy]   |
| MGRS                33UXP 01971 40510           [Copy]   |
| Open Location Code  8FWR6952+9X                 [Copy]   |
| Geohash             u2edk85                     [Copy]   |
| Maidenhead          JN88ee                      [Copy]   |
| geo: URI            geo:48.2085,16.37281        [Copy]   |
|                              [Show on map] [Close]       |
+----------------------------------------------------------+
```

`lineEdit` and a `tableView`-like form; values in monospace; an unreadable input shows what formats are accepted.

#### 5.3.5 Layer panel (popover, 320×460)

```
+--------------------------------------+
| Map style                            |
| (•) Standard ( ) Outdoor             |
| ( ) Transport ( ) Minimal            |
| Theme [ Follow system v ]            |
+--------------------------------------+
| Add-on layers                        |
| [x] Hill shading      ▁▃▅ 60%        |
| [x] Contours                         |
| [ ] Cycling routes                   |
| [ ] Hiking routes                    |
| [ ] Satellite (network) [Set up…]    |
| [ ] Graticule                        |
+--------------------------------------+
| My layers                            |
| [x] ● Favourites (12)                |
| [x] ━ Alps trip.gpx       ▁▃▅ 100%   |
| [ ] ▨ survey.geojson                 |
| [Import…]                            |
+--------------------------------------+
```

`radioButton`, `comboBox`, `checkBox`, `slider`, `listView` with drag to reorder.

#### 5.3.6 Measure bar and track recording (overlays on the map)

```
+--------------------------------------------------------------+
| Measure: 3 points · 2.41 km · bearing 048° · area —          |
| [Undo point] [Close shape] [Save as line] [Units v] [Done]   |
+--------------------------------------------------------------+

+--------------------------------------------------------------+
| ● Recording 00:42:10 · 3.8 km · 5.4 km/h · ↑120 m   (◎ precise)|
| [Pause] [Add waypoint] [Stop and save]                       |
+--------------------------------------------------------------+
```

Each is a `panel` docked at the top of the map with `label`s and `button`s. The recording bar is `--danger` outlined with the word "Recording" so it cannot be missed.

#### 5.3.7 Print map (dialog, 720×520)

```
+----------------------------------------------+-------------------------------+
| (preview: map with frame handles)            | Paper   [ A4 v ] [Portrait v] |
|   ┌──────────────────────────────┐           | Scale   [ 1:25 000 v ]        |
|   │                              │           | Pages   2 × 1 (overlap 10 mm) |
|   │        frame                 │           | [x] Scale bar  [x] Legend     |
|   │                              │           | [x] Grid [ UTM v ]            |
|   └──────────────────────────────┘           | [x] Route and instructions    |
|                                              | Style   [ Outdoor, light v ]  |
+----------------------------------------------+-------------------------------+
|                                   [Cancel] [Save as PDF…] [Print…]           |
+------------------------------------------------------------------------------+
```

### 5.4 Menus

```
File     New window Ctrl+N · New tab Ctrl+T · Import… Ctrl+O · Export ▸ (Selection, Route, Track, Library) ·
         Export image… Ctrl+Shift+E · Print… Ctrl+P · Regions… Ctrl+Shift+R · Close Ctrl+W · Quit Ctrl+Q
Edit     Undo Ctrl+Z · Redo Ctrl+Shift+Z · Copy map Ctrl+C · Copy coordinates Ctrl+Shift+C ·
         Paste place or coordinates Ctrl+V · Find Ctrl+F · Search this area Ctrl+Shift+F · Preferences Ctrl+,
View     Sidebar F9 · Inspector F4 · Bottom panel Ctrl+B · Layers L · Style ▸ · Zoom in Ctrl++ ·
         Zoom out Ctrl+- · Zoom to selection Ctrl+0 · North up N · Feature list Ctrl+Shift+L ·
         Map cursor Ctrl+M · Back Alt+Left · Forward Alt+Right · Full screen F11
Place    What is here? W · Save place Ctrl+D · Share… Ctrl+Shift+S · Directions to D · Directions from Shift+D ·
         Set my position here · Open website · Copy address
Route    New route Ctrl+Shift+N · Add stop · Reverse Ctrl+Shift+V · Options… · Alternatives ▸ ·
         Save route Ctrl+S · Export route… · Start navigation Ctrl+Enter · Clear route
Tools    Measure M · Coordinates… Ctrl+G · Record track Ctrl+R · Add waypoint Ctrl+Shift+W · Draw ▸ ·
         Isochrone… · Transit feeds… · Imagery sources… · Verify regions · Clear private data…
Location Show my position Ctrl+L · Follow ▸ · Stop using location
Help     Maps help F1 · Keyboard shortcuts · Data licences · About
```

`Ctrl+G` is "Coordinates" here (go to); "find next" in result lists is `F3`. This overrides the reserved meaning only inside this app and is listed in Settings.

### 5.5 Keyboard and pointer

| Keys | Action | Keys | Action |
|---|---|---|---|
| Arrows | pan by 1/8 of the view (`Shift`: by one view) | `+` / `-` | zoom by 0.5 |
| `Home` | zoom to all overlays or to the route | `1`–`4` | style Standard, Outdoor, Transport, Minimal |
| `Tab` (map cursor on) | next feature under or near the cursor | `Esc` | cancel tool, deselect, close card |
| `Ctrl`+drag | rotate | `Shift`+drag | zoom to rectangle |

Context menu on the map: What is here?, Directions from here, Directions to here, Add stop here, Save place here, Measure from here, Set my position here, Copy coordinates, Share this point, Centre here. On a place row: Show, Directions, Rename, Move to list, Colour, Share, Export, Delete. On a route line: Add via point, Avoid this road (adds a temporary block and re-routes), Show step. On a track: Profile, Statistics, Edit, Trim ends, Export, Delete. On a region row: Download, Update, Verify, Remove, Show on map.

Drag and drop: GPX, KML, KMZ, GeoJSON and region package files onto the window import or install after confirmation; `geo:` text or coordinate text onto the map drops a pin; a place from the sidebar onto a stop field sets it; stops reorder by drag; the route line drags to add a via point; a place dragged out offers `text/uri-list` (`geo:`), `text/plain` and GeoJSON.

Gestures: pinch zoom, two-finger rotate, two-finger vertical drag for tilt (when built), long press as right click.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty (first run) | Base map at world zoom with a card: "Install a region for detailed maps, search and routing." Buttons: Get regions, Install from file. Nothing is requested. |
| Loading | Tiles appear lowest zoom first; no spinner unless a tile takes more than 300 ms. |
| Error | Damaged region: banner with Verify and Re-download. Route not found: reason in words (no road link, missing region, restricted access) and the nearest routable point is offered. |
| Offline | The normal state. Only Regions, imagery and transit install show "needs network". |
| Location denied | Locate button shows a struck-through icon; tooltip explains; "Set my position here" is offered. |
| Location coarse | Circle only; banner "Approximate location" with "Use precise…" (re-prompts through the shell). |
| Locked | Library in a locked vault: sidebar Library shows Unlock; map, search and routing still work. |
| Read-only | Regions on a missing drive are listed as unavailable. |
| Recording | Recording bar always visible; closing the window asks: keep recording in the background, or stop and save. |
| Narrow | See 5.2. |

### 5.7 Widgets

| Region | FUSE widget | Note |
|---|---|---|
| Map | `mapView` | Proposed in FUSE-WP-48 on the 2D canvas FUSE-WP-43; gap G-12 |
| Search and stops | `lineEdit`, suggestion list (`popupMenu`-style) | search field is listed as missing in the FUSE plan |
| Results, alternatives, instructions, layers | `listView` / `modelListView` | drag reorder (FUSE-WP-33) |
| Library, regions | `treeView` | check boxes, columns |
| Profile graph, statistics | `chart` | hover linking to the map: IN04-GAP-2 |
| Attribute table, tags, converter | `tableView` | |
| Popovers, sheets | `popover`, `sheet` (FUSE-WP-30, Proposed) | bottom sheet with three heights: IN04-GAP-3 |
| Forms | `comboBox`, `checkBox`, `radioButton`, `slider`, `spinBox`, `colorPicker`, `dialog` | |
| QR code | `image` | pixels from the app's own encoder |

Gaps: **IN04-GAP-1** the map helper contract with `mapView`: draw-ready vector buffers, style classes, label candidates, hit-test results (FUSE-WP-48.3 names the buffer contract; this spec supplies the producer side in 6.5); **IN04-GAP-2** `chart` hover and range-selection signals with external marker; **IN04-GAP-3** bottom sheet with detents for narrow windows; **IN04-GAP-4** a distribution-side pipeline and mirror that builds and signs region packages from OpenStreetMap, elevation and transit data; **IN04-GAP-5** an interface on `svc.portal.location` for coarse versus precise fixes with a stated radius, and for heading and speed.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Views, library, route requests, region manager, settings, intents, unit and coordinate maths that is not hot. |
| UI | FUSE UI | app host | Section 5; hosts `mapView`. |
| Tile helper | C++23 (safe subset) | one, long-lived, no network | Opens region packages read-only (descriptors passed by the app), decodes vector tiles, applies the style sheet, builds draw buffers and label candidates, answers hit tests, computes hill shading from height tiles. |
| Search helper | C++23 (safe subset) | one, no network | Queries the per-region search indexes (SQLite FTS5 and R*Tree, read-only), ranks and merges results, reverse geocoding, opening-hours evaluation. |
| Route helper | C++23 (safe subset) | one per request batch, no network | Loads routing graphs by memory map, runs the query, builds instructions and the elevation profile, transit journeys, isochrones, map matching. |
| Import helper | C++23 (safe subset) | on demand, no network | Parses GPX, KML, KMZ, GeoJSON, GTFS zip; simplifies and tiles large overlays; writes exports. |
| Fetch helper | C++23 (safe subset) on the platform protocol helper | on demand, one per route | Region catalogue and package downloads; imagery tiles; transit feed download. |
| Image helper | C++23 (safe subset) | on demand, no network | Decodes imagery tiles (PNG, JPEG, WebP) to pixels. |
| Region builder | C++23 (safe subset) | on demand (F031) and on the distribution's build hosts | `.osm.pbf` to region package. |

```
 app host: logic (ForgeScript) + UI (FUSE, mapView) + library.db
   │  draw buffers (shared memory)        region packages (read-only fds)
   ├─ tile helper ◄───────────────────────── ~/.local/share/org.pbsd.maps/regions/*.mapregion
   ├─ search helper ◄──────────────────────┘
   ├─ route helper  ◄──────────────────────┘
   ├─ import helper (GPX, KML, GeoJSON, GTFS)        ├─ image helper
   ├─ fetch helper ── network service ── package mirror | imagery host (only with a grant)
   └─ svc.portal.location (handle only while in use) · svc.intent · svc.portal.print
```

No helper except the fetch helper ever holds a socket. With no grant given, the app has no network process at all.

### 6.2 Data model

Stores: `~/.local/share/org.pbsd.maps/library.db` (SQLite, WAL, `schema_version` in `meta`), `regions/` (packages), `transit/` (converted feeds), `~/.cache/org.pbsd.maps/imagery/`, `~/.config/org.pbsd.maps/` (settings, imagery sources, custom styles).

| Entity | Fields |
|---|---|
| `place` | `id u64`, `list_id u32`, `name text`, `lat f64`, `lon f64`, `ele_m f32?`, `osm_type enum{node,way,relation}?`, `osm_id u64?`, `category text?`, `address text?`, `colour u8`, `icon text`, `note text`, `created i64`, `modified i64` |
| `list` | `id u32`, `name text`, `colour u8`, `icon text`, `visible bool`, `position u32` |
| `route` | `id u64`, `name text`, `profile text`, `options json`, `stops json` (`[{lat,lon,name,place_id?}]`), `geometry blob` (delta-encoded polyline, 1e-6 degrees), `distance_m u32`, `duration_s u32`, `ascent_m u16`, `descent_m u16`, `instructions blob`, `data_date text`, `created i64` |
| `track` | `id u64`, `name text`, `source enum{recorded,imported}`, `started i64?`, `ended i64?`, `distance_m u32`, `moving_s u32`, `ascent_m u16`, `descent_m u16`, `bounds [f64;4]`, `colour u8`, `visible bool` |
| `track_point` | `track_id u64`, `segment u16`, `seq u32`, `lat f64`, `lon f64`, `ele_m f32?`, `time i64?`, `speed_mps f32?`, `hdop f32?` (stored as packed blobs of 512 points) |
| `layer` | `id u32`, `name text`, `kind enum{gpx,kml,geojson,drawn}`, `source_name text`, `feature_count u32`, `style json`, `visible bool`, `opacity u8`, `position u32`, `tiles blob` (simplified per zoom) |
| `feature` | `layer_id u32`, `id u64`, `geom_type enum{point,line,polygon}`, `geometry blob`, `props json` |
| `view_bookmark` | `id u32`, `name text`, `lat f64`, `lon f64`, `zoom f32`, `bearing f32`, `style text`, `layers json` |
| `recent` (only if enabled) | `kind enum{search,place,route}`, `value json`, `time i64` |
| `region` (index of installed packages) | `id text`, `name text`, `version text` (data date), `path text`, `components set{map,search,routing,terrain,contours}`, `size u64`, `sig_ok bool`, `installed i64` |
| `imagery_source` (config) | `id u32`, `name text`, `url_template text`, `scheme enum{xyz,tms,wmts}`, `min_zoom u8`, `max_zoom u8`, `attribution text`, `host text`, `route enum` |
| `recording.journal` | append-only fixed-size records `{time i64, lat f64, lon f64, ele f32, speed f32, hdop f32, flags u8}` |

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Region package `.mapregion` (Proposed, 6.5) | yes | builder only | own code | tile, search, route helpers (each opens only its part) |
| Vector tiles: Mapbox Vector Tile 2.1 encoding (protobuf) | yes | builder | own protobuf reader (fixed schema, about 300 lines) | tile helper |
| Tile archive: PMTiles v3 layout, or an equivalent own single-file index if its terms do not fit (to verify) | yes | builder | own code | tile helper |
| SQLite FTS5 and R*Tree (search index) | yes | builder | SQLite (public domain), already a platform dependency (PLATFORM V8) | search helper |
| Routing graph (own binary format, 6.5) | yes | builder | own code | route helper |
| Height tiles (16-bit grid, lossless, own framing) and contour vector tiles | yes | builder | own code | tile helper |
| `.osm.pbf` | yes (builder) | — | own protobuf reader plus zlib from the base system | region builder |
| GPX 1.0/1.1, KML 2.2, KMZ | yes | yes | own XML parser (shared source with IN02), own ZIP reader (stored and deflate only) | import helper |
| GeoJSON (RFC 7946) | yes | yes | own code | import helper |
| GTFS (zip of CSV) | yes | — | own code | import helper |
| `geo:` URI (RFC 5870) | yes | yes | own code | app host (short, bounded grammar) |
| QR code (ISO/IEC 18004) | — | yes | own encoder | app host |
| PNG, JPEG, WebP imagery tiles; XYZ, TMS, WMTS (KVP and REST tile requests only, no capabilities XML in phase 3) | yes | PNG export | platform image decoders | image helper |
| HTTPS with range requests | yes | — | FORGE-WP-21 protocol helper | fetch helper |
| PDF, SVG, PNG map export | — | yes | print pipeline (G-16), FUSE draw list to SVG (to verify that FUSE-WP-23 can serialise), own PNG writer | app host |

### 6.4 Integration

- Intents handled: `open` and `view` for `geo:` URIs, GPX, KML, KMZ, GeoJSON and `.mapregion` files; "show on map" with coordinates or an address string. Emitted: `open` (websites to IN01, `tel:`), `share`, `print`, `scan` is not used.
- MIME: view and import `application/gpx+xml`, `application/vnd.google-earth.kml+xml`, `application/vnd.google-earth.kmz`, `application/geo+json`, `application/x-pbsd-mapregion`; export the first four plus `image/png`, `image/svg+xml`, `application/pdf`.
- Services: `svc.settings`, `svc.portal.file`, `svc.portal.location`, `svc.portal.print`, `svc.intent`, `svc.search` (opt-in), `svc.power` (navigation and recording), `svc.notify` (downloads done, recording still running), `svc.backup`, `svc.audit`, `svc.clipboard`.
- IPC offered: `org.pbsd.maps.geocode` v1: `reverse(lat, lon) -> {address, areas[]}` and `forward(text, near?) -> results[]`, answered from installed regions only, for ME01, CO03 and IN05; each caller needs the user's one-time consent because queries reveal locations to Maps (which stores nothing).
- Other apps: IN05 (shares `mapView` and may use geocoding for place names), ME01 (photo places, tracks), CO03 (addresses), IN01 (links), SE03 (route for downloads), SE05 (vault), ME04 (none needed), SY05 (mirror configuration).
- Backing repos: none. Verified: FUSE has no map or canvas widget today (`widgets.hpp` list, PLATFORM V3; the FUSE build plan lists both as missing). Proposed: everything else.

### 6.5 Map data plan (tiles, style, search, routing)

**Region package.** A directory or single file `<region>-<date>.mapregion` with a signed `manifest.json` (id, name, bounds polygon, data date, schema versions, per-part size and BLAKE3 hash) and parts: `tiles.pmt`, `search.db`, `graph-car.bin`, `graph-bike.bin`, `graph-foot.bin`, `border.bin`, optional `terrain.pmt`, `contours.pmt`. Parts can be installed separately (F027). Ed25519 signature over the manifest by the distribution's map key; verified by the app host before any helper opens a part.

**Tiles.** Web Mercator pyramid, zoom 0–14, 4096-unit extent, Mapbox Vector Tile geometry encoding. Layer schema: a fixed, documented set (water, land cover, land use, boundaries, roads, rail, buildings, place labels, POIs, transit, routes). An existing open schema should be adopted if its licence allows use without extra terms (Shortbread and OpenMapTiles are the candidates; licences to verify). Names carry `name`, `name:<lang>` for a fixed language list, and `name_latin`. Tiles are compressed with zstd or gzip (to decide by measured size and decode time; zstd would be a third-party BSD-licensed library, gzip uses base-system zlib).

**Renderer.** The tile helper decodes a tile, evaluates the style sheet and emits, per tile and zoom: vertex and index buffers for fills and lines with style class ids, point lists, and label candidates (text, class, anchor, priority). `mapView` validates every count and index (FUSE-WP-48 gate `fuse_ui_map_vector_tile_hostile`), places labels and draws through the canvas. Polygon triangulation and line joins happen in the helper. Style sheets are a small own format (JSON: class → colours as design tokens, widths by zoom stops, dash, label font and halo, min and max zoom, filters on a fixed set of properties). No expression language, so a style cannot loop or run code.

**Search index.** Built from named OSM elements and addresses: table `entry(id, kind, name, alt_names, lat, lon, importance, admin_ids, tags)`, FTS5 over normalised tokens (case-folded, accent-folded, transliterated), R*Tree over points for reverse lookups, `street(id, name, admin_id, geom)` and `housenumber(street_id, number, lat, lon)` plus interpolation ranges, `admin(id, level, name, polygon)` for containment.

**Routing.** Graph per profile with edge attributes (length, class, speed, surface, access flags, toll, ferry, slope bucket, turn restrictions). Query method: a multi-level overlay method with customisable weights is preferred over plain contraction hierarchies, because user options (F057) change weights without a new preprocessing run; bidirectional A* is the fallback for small regions and the reference for tests. The choice is confirmed by a prototype (IN04-W13). Transit uses a round-based public transit algorithm over stop-time tables converted from GTFS. Cross-region routing joins graphs at border nodes listed in `border.bin`.

**Sources and licences.** OpenStreetMap data (ODbL; attribution shown; produced works are not share-alike, derived databases are). Elevation from an open global model (candidates: SRTM, Copernicus DEM GLO-30; licences and attribution text to verify). Transit feeds as published by agencies under their own terms; shown per feed. Packages are built on distribution hosts (IN04-GAP-4), never fetched from third-party tile services.

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Regions, library, cache, settings. |
| `fs.portal` | Import and export files, install a region from a file, external region folder. |
| `net.none` | Default state. The app works fully with no sockets. |
| `net.host:<map mirror>:443` | Region catalogue and packages. Asked when the user first presses Refresh or Download. Host name comes from system configuration (SY05). |
| `net.host:<imagery host>:443` | One per imagery source the user adds (F109). |
| `net.user` | Only for "install transit feed from URL" (F066); asked at that moment. |
| `net.route:clear`, `net.route:vpn`, `net.route:tor` | Route choice for downloads and imagery. |
| `dev.location` | Position, recording, navigation. Prompt on first use with Approximate or Precise; indicator while in use. |
| `dev.print` | Print maps and instruction sheets. |
| `proc.helper:tile`, `:search`, `:route`, `:import`, `:fetch`, `:image`, `:region-build` | Section 6.1. |
| `svc.settings`, `svc.portal.file`, `svc.portal.location`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.power`, `svc.notify`, `svc.backup`, `svc.audit`, `svc.clipboard` | Section 6.4. |
| `bg.run` | Track recording, navigation and region downloads with the window closed. Asked when needed. |

Threat model:

| | |
|---|---|
| Assets | Where the user is and has been; where they plan to go; saved places (home, work); searches; the integrity of map and routing data. |
| Adversaries | A network observer or a mirror operator learning regions of interest; an imagery server learning viewed areas; a hostile GPX, KML, GeoJSON, GTFS or region file; a hostile app using the geocode interface to probe; someone reading a shared track that starts at the user's home. |
| Attack surface | Parsers for vector tiles, package parts, XML, JSON, ZIP, CSV, image tiles; the routing graph loader (memory-mapped untrusted file if a package is sideloaded); the IPC geocode interface; `geo:` URIs from other apps. |

Mitigations: offline by design, so the common tasks send nothing; location only by explicit action, coarse offered first, indicator always shown, no storage unless recording; region packages are signed and each part hash-checked, and helpers still treat them as hostile (bounds-checked readers, no trust in offsets); parsers run in helpers with no network and no file access beyond passed descriptors; KML network links and external entities are ignored; imagery is opt-in per host with a notice, no cookies and a cache; shared coordinates can be rounded and tracks trimmed at both ends; the geocode interface needs per-app consent and rate-limits to 10 queries per second; `geo:` input is range-checked (latitude ±90, longitude ±180, zoom 0–22).

Stored: section 6.2, on the user's disk. Sent: nothing by default. With grants: region file requests to the mirror (reveals regions), tile requests to an imagery host (reveals viewed areas at tile precision), one request for a transit feed URL. Residual: region choice leaks coarse interest; a sideloaded unsigned package is refused, so community-built regions need the local builder (F031) or the user's own signing key added in SE04 (C).

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `map.style` | enum standard/outdoor/transport/minimal | standard | Base style. |
| `map.theme` | enum system/light/dark/high_contrast | system | Map colours. |
| `map.label_language` | enum system/local/both | system | F006. |
| `map.label_scale_pct` | int 80–200 | 100 | F007. |
| `map.poi_density` | enum few/normal/many | normal | F007. |
| `map.kinetic_pan` | bool | true | Off with reduced motion. |
| `map.rotation` | bool | true | F004. |
| `units.distance` | enum metric/imperial/nautical | locale | F099. |
| `units.coordinates` | enum dd/dm/dms/utm/mgrs/olc | dd | Status bar and copy format. |
| `regions.dir` | portal handle | app data | F028. |
| `regions.auto_update` | bool | false | F025. |
| `regions.route` | enum clear/vpn/tor | clear | F030. |
| `routing.profile` | enum car/bike/foot | foot | Default profile. |
| `routing.avoid` | set{motorway,toll,ferry,unpaved,steps} | {} | F057. |
| `routing.optimise` | enum fastest/shortest/balanced | fastest | F058. |
| `routing.alternatives` | int 0–3 | 2 | F056. |
| `routing.voice` | bool | false | F071. |
| `location.default_precision` | enum ask/approximate/precise | ask | What the shell prompt preselects. |
| `location.follow` | enum off/centre/heading | off | F118. |
| `privacy.remember_recent` | bool | false | F042, F120. |
| `privacy.share_precision` | enum full/3dp/2dp/1dp | full | F125 (own position always starts at 3dp). |
| `privacy.strip_times_on_export` | bool | false | F094. |
| `record.interval_s`, `record.min_distance_m` | int 1–60, int 0–100 | 5, 5 | F086. |
| `record.auto_pause` | bool | true | F088. |
| `layers.hillshade_strength_pct`, `layers.hillshade_azimuth` | int 0–100, int 0–359 | 60, 315 | F107. |
| `imagery.cache_mib` | int 0–20000 | 500 | F111. |
| `imagery.cached_only` | bool | false | F110. |
| `search.contribute_places` | bool | false | F130. |

## 9. Accessibility and languages

- The map is visual; the feature list (F133) and the map cursor (F134) are the equivalent path. The list is an a11y table with name, type, direction ("north-east") and distance; activating a row selects the feature.
- Route instructions are a list with full sentences ("In 350 metres turn left onto Old Bridge"), not icons alone; the profile graph has a text summary (total climb, steepest section and where).
- Map styles pass contrast checks for road casings and labels in the high-contrast theme; route alternatives differ by line pattern as well as colour; track colouring by speed has a legend with numbers.
- All controls are at least 36 px on the map (44 px in touch density).
- Names use the user's language where the data has it; scripts need the FULL text profile (G-03); right-to-left labels follow line direction rules in the label placer.
- Units, number formats, coordinate hemispheres and compass words come from string tables; category search words are translated tables, so "Apotheke" finds pharmacies.

## 10. Performance budgets

| Metric | Budget |
|---|---|
| Cold start to first frame (base map) | 300 ms |
| Pan frame at zoom 14 in a dense city | 6 ms in `mapView` (FUSE-WP-48 budget); tile build in the helper under 40 ms per tile |
| Tile cache | 256 MiB of draw buffers, evicted least recently used |
| Search | first results under 100 ms; suggestions under 50 ms; reverse geocode under 30 ms |
| Route, 30 km city, foot | under 300 ms |
| Route, 1 000 km, car | under 2 s with the multi-level method; under 20 s is the limit for the fallback |
| Region size (guide) | a mid-size country about 0.5–1 GiB with search and three routing graphs (to measure) |
| Largest overlay | 1 million vertices, 6 ms pan frame after tiling |
| Recording | under 1% CPU; journal write under 1 ms |
| Memory | app host 150 MiB; tile helper 400 MiB cap; route helper memory-maps graphs, private memory under 300 MiB |
| Idle | zero frames and zero wake-ups with a still map and no location use |

## 11. Testing and proof

Nothing has run. Targets only.

- Unit: coordinate parsing and formatting for every format of F040 (round trips, hemisphere and zone edge cases, poles, antimeridian), geodesic distance and area against published test sets, `geo:` URI grammar, QR encoder against known vectors, opening-hours parser against a corpus, unit conversion.
- Golden renders: each style at fixed views and zooms; label placement goldens; all dialogs in three themes.
- Scripted UI through the a11y tree: tasks T1–T13, with location simulated by a test portal.
- Fuzz: vector tile decoder, package manifest and part readers, routing graph loader, GPX, KML, GeoJSON, GTFS, ZIP, `.osm.pbf` (builder), imagery tile decode, `geo:` URIs, style sheets.
- Differential: routes against a reference open-source routing engine on the same extract (distance within 1%, same road sequence for a sample set); multi-level query against plain Dijkstra on small graphs (must be equal); geocoding against a reference open-source geocoder on an address sample; tile decode against a reference vector tile decoder.
- Privacy tests: packet capture with no grants shows zero packets in all tasks; with location denied no location handle is opened; shared `geo:` strings honour the precision setting; exported tracks honour time stripping.
- Data pipeline tests: a built region for a small test extract is byte-identical across two builds (reproducible).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Vector tile decoder | No runtime errors on hostile tiles; output counts consistent | T2 | PRISM; fuzz | — |
| Draw-buffer builder | Every index is inside its vertex array | T1 | PRISM proof of the emit functions; validated again by `mapView` | — |
| Routing graph loader | All offsets bounds-checked before use | T2 | PRISM; fuzz | — |
| Shortest-path query | Result equals Dijkstra on the same weights | T1 target for the core relaxation; tested for the overlay | PRISM on the core; differential tests | Proof of the multi-level structure is research-grade work |
| Coordinate conversions | Round-trip error bounds; no overflow or domain errors | T1 | PRISM with interval reasoning; test vectors | Floating-point proof support in PRISM (to verify) |
| XML, JSON, ZIP, CSV parsers | No runtime errors; bounded memory | T2 | PRISM; fuzz | — |
| Package signature check | No part is opened unless manifest signature and part hash verify | T1 | PRISM over the install state machine | — |
| Search ranking | No runtime errors | T2 | PRISM; differential | SQLite itself is third-party C: T4 for that part |
| App logic | Memory safety by construction | T3 | ForgeScript guarantees; unit tests | — |

## 12. Build plan

| ID | Work item | Depends on |
|---|---|---|
| | **Phase 1: usable core (view, regions, basic search, places)** | |
| IN04-W01 | Region package format, manifest, signature and part hashes | FORGE-WP-22, FORGE-WP-09 |
| IN04-W02 | Region builder: tiles and name index from `.osm.pbf` (distribution side) | IN04-GAP-4 |
| IN04-W03 | Tile helper: decode, style, draw buffers, hit test | G-12, FUSE-WP-43, FUSE-WP-48, FORGE-WP-28 |
| IN04-W04 | Main window with `mapView`, controls, status bar, base world map | G-02, W03 |
| IN04-W05 | Region manager: catalogue, download, verify, install from file | W01, FORGE-WP-21 |
| IN04-W06 | Search helper: name search, ranking, results list, place card | W02 |
| IN04-W07 | Library: places and lists, save, show, `geo:` intent in | W04 |
| IN04-W08 | Location: prompt, coarse circle and precise dot, indicator, manual position | `svc.portal.location`, IN04-GAP-5 |
| | **Phase 2: full feature (routing, overlays, layers)** | |
| IN04-W09 | Address index, typo tolerance, categories, coordinates input, reverse geocoding | W06 |
| IN04-W10 | Routing graph builder per profile; border nodes | W02 |
| IN04-W11 | Route helper: query, alternatives, options, instructions | W10 |
| IN04-W12 | Directions UI, route line editing, export, saved routes | W11, FUSE-WP-33 |
| IN04-W13 | Prototype and decision: multi-level method versus contraction hierarchies | W10 |
| IN04-W14 | Import helper: GPX, KML, GeoJSON; overlay styling; export | FORGE-WP-28 |
| IN04-W15 | Terrain and contours add-ons; hill shading; elevation profile | W02, W03 |
| IN04-W16 | Styles (Outdoor, Transport, Minimal), themes, label settings, route overlays | W03 |
| IN04-W17 | Measure, coordinate converter, share with QR, image export | W04 |
| IN04-W18 | Feature list and map cursor for keyboard and screen reader | W03, G-04 |
| | **Phase 3: advanced** | |
| IN04-W19 | Track recording, journal, statistics, cleaning, background | W08, `bg.run` |
| IN04-W20 | Navigation mode, re-routing, spoken instructions | W11, W19, AX01 |
| IN04-W21 | Transit: GTFS import, journey planner, transit layer | W14 |
| IN04-W22 | Imagery sources, fetch and image helpers, cache | FORGE-WP-21 |
| IN04-W23 | Print map with scale and paging | G-16 |
| IN04-W24 | Geometry editing, drawing, large overlays, attribute table | W14 |
| IN04-W25 | Geocode interface for other apps | W09, G-08 |
| IN04-W26 | Local region build (F031), delta updates, isochrones, map matching, round trips | W02, W11 |

## 13. Open questions and risks

1. `mapView` and the 2D canvas do not exist (G-12, FUSE-WP-43, FUSE-WP-48). Maps cannot start before them; the buffer contract must be agreed with the FUSE plan (IN04-GAP-1).
2. Someone must build, sign and host region packages (IN04-GAP-4). A planet-wide build needs large machines and a regular schedule. Without it the app has only the base map and sideloaded data.
3. Tile schema and archive format licences (Shortbread, OpenMapTiles, PMTiles) are to verify; an own schema is the fallback and costs style work.
4. The routing method is a real engineering risk: plain contraction hierarchies are simpler but make user options costly; the multi-level method needs partitioning in the builder. Decided in IN04-W13.
5. Address search quality depends on OSM address coverage, which varies by country; the UI must say when a region has few addresses.
6. Elevation data licences and void areas need checking per source.
7. The location portal's coarse mode (radius, snapping) is defined by SH04 and the location service; this spec assumes a radius is reported (IN04-GAP-5).
8. `Ctrl+G` is used for "go to coordinates" against the reserved "find next"; if the design system owner objects, it moves to `Ctrl+J`.
9. SQLite as the search index is third-party C code in a helper; the helper is confined, but a safe-subset index reader would be better later.
10. Map label quality needs design iteration (the FUSE plan says the same in FUSE-WP-48 risks).
