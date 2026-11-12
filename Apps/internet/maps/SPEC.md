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
