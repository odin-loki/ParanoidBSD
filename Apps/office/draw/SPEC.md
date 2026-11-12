<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Draw

By Odin Loch

| | |
|---|---|
| ID | `OF08` (from CATALOGUE.md) |
| Bundle | `org.pbsd.draw` |
| Category | office |
| Priority | P2 |
| Backing repos | FUSE (UI, draw list, `nodeGraph`), FORGE (ForgeScript, signing) |
| Status | Planned. Nothing built. |

## 1. Purpose

A vector drawing and diagramming program. People use it for flowcharts, network and architecture diagrams, UML, floor plans, icons, posters and technical sketches with real dimensions. It saves OpenDocument Graphics (`.odg`) for multi-page work and SVG for single drawings, reads draw.io and Visio files, and can build a diagram from a few lines of text. Its vector core is also the shape engine of Writer and Slides. It replaces LibreOffice Draw from the Plasma plan (PLATFORM.md V20).

## 2. Scope

In scope:

- Shapes, paths with bezier editing, boolean operations, text, images.
- Layers, groups, symbols (reusable masters with instances) and stencil libraries.
- Connectors that stay attached and route themselves; automatic layout of graphs.
- Grids, guides, snapping, rulers, dimensions and drawing scale.
- Styles, gradients, patterns, markers, effects.
- Multi-page documents with page masters.
- Diagrams as code: a text description becomes a diagram and stays linked to it.
- Import: SVG, draw.io (`.drawio`, mxGraph XML), Visio (`.vsdx`, `.vssx` stencils), ODG, PDF pages as vectors (through OF03's parser helper, C priority), EMF/WMF (C).
- Export: SVG, PDF, PNG, JPEG, WebP, ODG, draw.io, EPS (C).
- Embedding drawings into OF04 Writer and OF06 Slides.

Out of scope:

| Item | Owner |
|---|---|
| Pixel painting, brushes, photo retouching, raster filters | CR01 Image Editor |
| CAD: constraints solver, 3D, DXF/DWG round trip (DXF import is C priority, 2D only) | not planned |
| Page layout of long documents | OF04 Writer |
| Slide shows and animation | OF06 Slides |
| UI layout of `.fuseui` documents | DV02 UI Designer |
| Database reverse engineering to ER diagrams (Draw has ER stencils; live schema diagrams are OF09's) | OF09 Data Studio |
| Charts from data | OF05 Sheets (embedded) |
| Font creation | UT03 Fonts |
| Maps and geodata | IN04 Maps |

## 3. Users and top tasks

Personas:

- **Engineer or admin**: network, rack and architecture diagrams; wants stencils, connectors that stay put, export to SVG and PDF.
- **Developer**: UML and sequence diagrams from text kept beside the code.
- **Analyst**: flowcharts, swimlanes, organisation charts with auto-layout.
- **Designer**: icons, logos, posters; wants precise paths, booleans, gradients, clean SVG.
- **Home user**: a floor plan to scale, a party invitation.
- **Author**: figures embedded in Writer or Slides.

| # | Task | Target steps |
|---|---|---|
| T1 | Drop two shapes and connect them | 3 (drag, drag, drag from port) |
| T2 | Add a shape already connected to the selected one | 1 (click a direction arrow) |
| T3 | Type a label on a shape or connector | select, type |
| T4 | Tidy a messy flowchart | 2 (`Ctrl+Shift+L`, pick layout) |
| T5 | Turn 12 lines of text into a sequence diagram | paste in the code panel, 0 more |
| T6 | Union two shapes | 2 (select both, `Ctrl+Alt+U`) |
| T7 | Edit a curve's nodes | double-click, drag |
| T8 | Export the selection as a transparent PNG at 2× | 3 |
| T9 | Draw a wall of exactly 3.6 m at 1:50 | type `3.6m` while dragging |
| T10 | Open a `.drawio` or `.vsdx` file and keep working | 1 |
| T11 | Change the colour of every "server" symbol | 2 (edit the symbol master) |
| T12 | Put a drawing into a Writer document and edit it later | 2 to insert, 1 to edit |
| T13 | Align and distribute a selection | 2 |
| T14 | Find a stencil shape by name | `/` then type |

## 4. Features

### 4.1 Canvas, pages and navigation

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F001 | Infinite-feel canvas with pages | Each page has a size and sits on a pasteboard. An optional "boundless page" mode grows to fit content (for diagrams). | M | 1 |
| OF08-F002 | Zoom and pan | 1%–25,600%; `Ctrl+wheel` about the pointer; pinch; `Space+drag`, middle-drag; zoom to fit page, width, selection, drawing; `1` = 100%, `2` = fit page, `3` = fit selection. | M | 1 |
| OF08-F003 | Multi-page documents | Add, duplicate, delete, rename, reorder pages; page list panel with thumbnails; different sizes per page; link between pages. | M | 1 |
| OF08-F004 | Page masters | Background pages with borders, title blocks and fields (page number, title, date, scale, author); a page picks a master. | S | 2 |
| OF08-F005 | Page setup | Size presets (ISO A and B series, ANSI, architectural, screen sizes, icon sizes), orientation, margins, bleed, background colour or transparent. | M | 1 |
| OF08-F006 | Units and scale | Document units mm, cm, m, in, ft, pt, px; drawing scale per page (`1:50`, `1 in = 10 ft`); all numeric fields accept units and expressions (`3.6m`, `120mm/2`). | M | 1 |
| OF08-F007 | Rulers | Horizontal and vertical, in document units at the drawing scale, origin movable, selection extents marked. | M | 1 |
| OF08-F008 | Grid | Rectangular with major and minor lines, isometric (30°), dot grid; spacing per page; adapts density with zoom. | M | 1 |
| OF08-F009 | Guides | Drag from rulers; angled guides; guide from object edge; lock, hide, clear; per page and per master. | M | 1 |
| OF08-F010 | Snapping | To grid, guides, object bounds and centres, path nodes, midpoints, intersections, tangents and perpendiculars, connection points, page edges and margins; each kind toggled in a snap bar; snap indicator names what was hit; `Alt` suspends. | M | 1 |
| OF08-F011 | Smart guides | Alignment and equal-spacing hints with distance labels during move and resize. | M | 1 |
| OF08-F012 | View modes | Normal, outline (wireframe), pixel preview (as rasterised at an export scale), print preview with page tiles. | S | 2 |
| OF08-F013 | Multiple views | Two windows or split panes on one document at different zooms. | S | 2 |
| OF08-F014 | Navigator | Small overview with a draggable viewport rectangle. | S | 2 |
| OF08-F015 | Large documents | Tiled rendering and spatial index so 100,000 objects pan at frame rate; off-screen objects cost nothing to paint. | M | 2 |

### 4.2 Shapes and drawing tools

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F016 | Primitive shapes | Rectangle (per-corner radius), ellipse (arc, pie, chord), polygon (3–100 sides), star (inner radius, rounding), line, arrow, spiral. Live parameters stay editable until converted to a path. | M | 1 |
| OF08-F017 | Preset shape library | The ODF/OOXML preset geometry set (about 190 shapes: block arrows, callouts, brackets, flowchart, stars and banners, maths symbols) with adjust handles. Shared with OF06 and OF04. | M | 1 |
| OF08-F018 | Pen tool | Click for corners, drag for smooth nodes, `Alt` to break a handle, close by clicking the start, continue an open path, rubber-band preview. | M | 1 |
| OF08-F019 | Pencil and smoothing | Freehand drawing fitted to beziers with a smoothing setting; pen pressure may drive stroke width (variable-width stroke) when a pen is present. | S | 2 |
| OF08-F020 | Polyline and arc tools | Straight segments with typed lengths and angles; three-point arc; rounded polyline. | M | 1 |
| OF08-F021 | Typed input while drawing | While dragging a shape or segment, typing a length, `w,h`, or `length<angle` sets it exactly. | M | 2 |
| OF08-F022 | Direct selection and node editing | Select nodes and segments; node types corner, smooth, symmetric, auto-smooth; add, delete (keeping shape), join, break, align and distribute nodes; drag segments; numeric node coordinates. | M | 1 |
| OF08-F023 | Path operations | Reverse, close, open, simplify (tolerance), smooth, offset (inset and outset with join style), outline stroke to path, convert shape or text to path, split at node, cut with a knife line, erase part. | M | 2 |
| OF08-F024 | Boolean operations | Union, subtract front, subtract back, intersect, exclude, divide, trim. Non-destructive compound shapes (operands stay editable) and "expand" to a plain path. Robust on overlapping and touching edges. | M | 2 |
| OF08-F025 | Shape builder | Drag across overlapping regions to merge them, `Alt+drag` to remove. | C | 3 |
| OF08-F026 | Transform | Move, scale, rotate, skew, flip; about a chosen origin; numeric transform panel; repeat last transform (`Ctrl+D` duplicate-and-repeat); transform each object about its own centre. | M | 1 |
| OF08-F027 | Align and distribute | Six alignments, distribute centres or gaps, equal spacing by a typed gap, align to selection, key object, page or margins; match width or height. | M | 1 |
| OF08-F028 | Arrange | Z-order within a layer (front, forward, backward, back); move to layer. | M | 1 |
| OF08-F029 | Duplicate and arrays | Duplicate, step and repeat, grid array, radial array, along a path; arrays can stay parametric. | S | 2 |
| OF08-F030 | Corners | Round, chamfer or invert selected corners of any path with a live radius. | S | 2 |
| OF08-F031 | Clipping and masking | Clip a group by a path; opacity mask from a shape or image; release. | S | 2 |
| OF08-F032 | Blend and morph between two shapes | N intermediate steps along a spine. | C | 3 |
| OF08-F033 | Images in drawings | PNG, JPEG, WebP, SVG placed and embedded; crop, mask, opacity; never linked to a URL. Raster editing hands off to CR01 through the `edit` intent. | M | 1 |
| OF08-F034 | Trace bitmap to vectors | Threshold or colour-quantised tracing with corner and smoothness settings, run in the geometry helper. | C | 3 |
| OF08-F035 | Measure tool | Distance, angle, dx, dy between two picked points; area and perimeter of a closed shape; in drawing-scale units. | S | 2 |

### 4.3 Text

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F036 | Text on any shape | Select a shape or connector and type; text area follows the shape; margins, vertical alignment, auto-grow shape to fit text. | M | 1 |
| OF08-F037 | Point text and area text | Click for a line that grows; drag for a box that wraps. | M | 1 |
| OF08-F038 | Character and paragraph formatting | Font, size, weight, style, colour, underline, strike, spacing, alignment, line height, lists, tabs, text direction. | M | 1 |
| OF08-F039 | Text on path | Along any path, with offset, side and spacing; text inside a shape (flow). | S | 2 |
| OF08-F040 | Text styles | Named character and paragraph styles (ODF). | S | 2 |
| OF08-F041 | Fields in text | Page number, page count, date, file name, scale, object data values (`{name}`, `{ip}` from shape data). | S | 2 |
| OF08-F042 | Spell check | Through the dictionary service (G-17). | S | 2 |
| OF08-F043 | Find and replace text | Across pages; also find by shape name, style, data value. | M | 2 |
| OF08-F044 | Formulas in labels | A LaTeX-subset formula object (shared with OF04 and OF07). | C | 3 |
| OF08-F045 | Fonts in export | SVG export keeps text as text with a font fallback list, or converts to outlines by option; PDF embeds subsets. | M | 2 |

### 4.4 Fill, stroke, styles and effects

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F046 | Fills | None, solid, linear, radial and conic multi-stop gradients with on-canvas handles; pattern fills (built-in hatches and tiles, user patterns from a selection); image fill. | M | 1 |
| OF08-F047 | Strokes | Width, colour or gradient, dash patterns (presets and custom), caps, joins, mitre limit, alignment (centre, inside, outside), variable-width profiles. | M | 1 |
| OF08-F048 | Markers | Arrowheads and other end markers (about 40), scale with stroke, custom markers from a shape. | M | 1 |
| OF08-F049 | Multiple fills and strokes | An appearance stack per object (several strokes and fills in order). | C | 3 |
| OF08-F050 | Opacity and blend modes | Object and layer opacity; blend modes normal, multiply, screen, overlay, darken, lighten (those SVG and PDF both express). | S | 2 |
| OF08-F051 | Effects | Drop shadow, inner shadow, blur, glow as SVG filter primitives; rasterised only at export when the target cannot express them. | S | 2 |
| OF08-F052 | Graphic styles | Named styles (fill, stroke, effects, text) with inheritance; a style panel; update a style from the selection; styles travel in ODG and as SVG classes. | M | 2 |
| OF08-F053 | Document themes | A colour palette and font pair that stencil shapes refer to by role (`accent1`, `line`, `surface`), so one switch restyles a diagram; shared theme files with OF04, OF05, OF06. | S | 2 |
| OF08-F054 | Swatches and palettes | Document swatches, global colours (edit once, all users change), palette import and export (GPL palette text format). | S | 2 |
| OF08-F055 | Colour spaces | sRGB working space; Display P3 when the platform colour pipeline has it (FUSE-WP-26); CMYK values kept as annotations for PDF export (C). | S | 3 |
| OF08-F056 | Eyedropper and style copy | Pick colour from anywhere inside the app's windows; copy and paste style. Picking from outside the app goes through the shell colour picker portal (SH04). | S | 2 |
| OF08-F057 | Contrast helper | Shows the contrast ratio of text against its shape fill and warns under 4.5:1. | S | 2 |
| OF08-F058 | Hand-drawn style | A "sketch" rendering option (roughened strokes, hachure fills) as a live effect with a fixed seed. | C | 3 |

### 4.5 Layers, groups and symbols

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F059 | Layers | Per page or shared across pages; show, lock, print flags; colour tag; opacity; reorder; nested layers. | M | 1 |
| OF08-F060 | Layers panel | Tree of layers, groups and objects with names, eye and lock toggles, drag to reorder and re-parent, type to filter, select by click. | M | 1 |
| OF08-F061 | Groups | Group, ungroup, enter a group (isolation mode with the rest dimmed), nested groups. | M | 1 |
| OF08-F062 | Containers | A shape that owns the shapes placed on it (swimlane, zone, rack): they move with it, and it can resize to fit them. | S | 2 |
| OF08-F063 | Symbols | A master object defined once; instances share geometry; per-instance overrides for text, colour roles and data; edit master in place; detach instance. | M | 2 |
| OF08-F064 | Stencil libraries | Built-in sets: basic, arrows, flowchart (ISO 5807), BPMN 2.0, UML 2 (class, sequence, use case, activity, state, component, deployment), ER (Chen, crow's foot), network (generic devices, racks 19-inch in units), cloud-neutral architecture, electrical and electronic (IEC 60617-style symbols), logic gates (IEC and ANSI), floor plan (walls, doors, windows, furniture, to scale), wireframe/UI mock-up, organisation chart, mind map, timeline and Gantt, signs and pictograms. Vendor icon sets are not bundled. | M | 2 |
| OF08-F065 | Stencil panel | Search across libraries (`/`), recent and favourite shapes, drag or click to place, preview with name; libraries can be shown or hidden. | M | 1 |
| OF08-F066 | Custom stencils | Save a selection as a stencil shape with connection points, data fields and resize rules; stencil files (`.pbstencil`, a ZIP of SVG plus JSON metadata) can be exported and imported. | S | 2 |
| OF08-F067 | Stencil signing and trust | Stencil files are data, not code. They are parsed in the I/O helper. Signed stencil packs show their signer; unsigned ones load with an "unverified" badge. | S | 3 |
| OF08-F068 | Shape data | Key-value fields on any object (typed: text, number, date, choice, link); a data panel; data shown in labels by field; export of all shape data as CSV. | S | 2 |
| OF08-F069 | Smart shape behaviour | Declarative per-stencil rules: resize keeping corner radius, stretch regions (9-slice), text placement, rotation limits, minimum size. No scripts inside shapes. | S | 2 |
| OF08-F070 | Select similar | Same fill, stroke, style, symbol, stencil type, layer, or data value. | S | 2 |

### 4.6 Connectors and automatic layout

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F071 | Connector types | Straight, orthogonal (elbow), rounded orthogonal, curved, arc; several waypoints; convertible. | M | 1 |
| OF08-F072 | Attachment | Glue to a named connection point, to a floating point on the outline, or to the shape as a whole (the end slides to the nearest side). Ends show a green mark when glued and red when free. | M | 1 |
| OF08-F073 | Connection points | Default points per shape (sides, corners, centre); add, move and delete custom points; directional points that set the exit side. | M | 1 |
| OF08-F074 | Automatic routing | Orthogonal routing that avoids shapes, keeps a clearance, and re-routes on move; manual waypoints pin a route. Runs in the geometry helper. | M | 2 |
| OF08-F075 | Line jumps | Arc or gap where connectors cross, for one direction. | S | 2 |
| OF08-F076 | Connector labels | Several labels per connector at positions along it (start, middle, end, percentage), with background and rotation options. | M | 1 |
| OF08-F077 | End styles by notation | UML (association, aggregation, composition, generalisation, dependency, realisation), ER crow's foot cardinalities, BPMN flows, network link types, as presets. | S | 2 |
| OF08-F078 | Quick connect | Hover a shape to show four arrows; click adds the last-used shape in that direction, connected; drag from an arrow to draw a connector; drop a stencil on a connector to insert it between two shapes. | M | 1 |
| OF08-F079 | Layout algorithms | Layered (Sugiyama) top-down or left-right, tree (compact, radial, mind map), organisation chart, force-directed, circular, grid, orthogonal compaction, swimlane-aware layered. Parameters: spacing, direction, alignment. Deterministic for a given input and seed. | M | 2 |
| OF08-F080 | Partial and incremental layout | Lay out only the selection; "keep relative order" mode that changes as little as possible; animate the change in 200 ms (off with reduced motion). | S | 2 |
| OF08-F081 | Tidy tools | Straighten connectors, equalise spacing, align ports, remove overlaps, without a full re-layout. | S | 2 |
| OF08-F082 | Sequence, timeline and Gantt helpers | Lifelines that extend together, activation bars, message numbering; timeline shapes bound to dates. | S | 3 |
| OF08-F083 | Validation rules for diagrams | Optional checks per diagram type: unconnected ends, flowchart decision without two exits, duplicate names, orphan shapes. Results list with "go to". | C | 3 |

### 4.7 Diagrams as code

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F084 | Text-to-diagram language | A documented grammar that accepts the common core of Mermaid syntax for: flowchart (`graph TD`), sequence, class, state, ER, Gantt and mind map. Pie charts are excluded (charts belong to OF05). Unsupported statements are reported by line; nothing is silently dropped. | M | 2 |
| OF08-F085 | Code panel | A `codeEdit` panel beside the canvas; the diagram updates 300 ms after typing stops; errors underline the line. | M | 2 |
| OF08-F086 | Code-linked diagram object | The generated diagram is a group that stores its source text. Editing the text regenerates it. Style and manual position overrides are kept by node id where the node still exists. | M | 2 |
| OF08-F087 | Detach | Convert a code-linked diagram to ordinary shapes. | M | 2 |
| OF08-F088 | Diagram to code | For diagrams built from stencil shapes of a supported type, write the text form (lossy; layout is not kept). | C | 3 |
| OF08-F089 | Other text inputs | Graphviz DOT (graph structure and labels; layout by Draw's own algorithms), PlantUML sequence and class subsets (C), CSV edge list (`from,to,label`), indented text outline to tree or mind map. | S | 3 |
| OF08-F090 | Source files | Open a `.mmd` or `.dot` file directly as a diagram; export SVG or PNG from the command line of the app host in headless mode with only the passed file handles (for documentation builds). | S | 3 |
| OF08-F091 | No embedded code | The language is declarative. Click actions, HTML labels and script hooks found in Mermaid sources are rejected and reported. | M | 2 |
| OF08-F092 | Draft from a description | Optional: with `ai.local`, a typed description produces diagram text in the panel for the user to edit. The model sees only the typed description. Off by default. | C | 3 |

### 4.8 Dimensions and technical drawing

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F093 | Dimension objects | Linear (horizontal, vertical, aligned), angular, radius, diameter, arc length, leader with note. Attached to geometry and updated when it changes. | M | 2 |
| OF08-F094 | Dimension styles | Arrow type, text position, precision, unit display, tolerance text, extension line gaps; ISO and architectural presets. | S | 2 |
| OF08-F095 | Drawing scale | Dimensions and rulers read in real-world units at the page scale; viewports at different scales on one page (C). | M | 2 |
| OF08-F096 | Area and length schedule | A table object listing areas and perimeters of tagged shapes (rooms), updated live. | C | 3 |
| OF08-F097 | Wall tool | Double-line walls with thickness that join at corners and tees; doors and windows cut openings when dropped on a wall. | S | 3 |
| OF08-F098 | Hatches | Material hatch patterns by ISO names for sections. | C | 3 |
| OF08-F099 | Angle and length constraints while drawing | Hold `Shift` for 15° steps; lock length or angle in the typed-input box; ortho mode. No persistent constraint solver. | S | 2 |
| OF08-F100 | Coordinate read-out and numeric placement | Status bar shows cursor position; objects can be placed by absolute or relative coordinates (`@100,50`). | S | 2 |

### 4.9 Import and export

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F101 | ODG native | ODF 1.3 Drawing (`.odg`, template `.otg`, flat `.fodg`): pages, layers, shapes, connectors with glue points, styles, text. Extensions (symbols, shape data, code-linked sources, containers) under the `org.pbsd.draw` namespace with plain-shape fallbacks. | M | 1 |
| OF08-F102 | SVG open and save | SVG 1.1 and the widely used parts of SVG 2: paths, basic shapes, text, gradients, patterns, clip paths, masks, markers, filters (subset), `use` and `symbol`, CSS presentation attributes and a small CSS subset. A single-page drawing can use SVG as its working format, with Draw's extra data in a namespaced metadata block. | M | 1 |
| OF08-F103 | SVG safety | `script`, event attributes, `foreignObject`, external references (`href` to URLs, external CSS, fonts, images) and animation elements are not executed or fetched. They are dropped on import and listed in a report. Entities and DTDs are not processed. | M | 1 |
| OF08-F104 | SVG export options | Plain SVG (no private data), minified, with or without text as outlines, decimal precision, `viewBox` only or fixed size, IDs kept or shortened, embedded or separate images, export of the selection, page or all pages. | M | 1 |
| OF08-F105 | PDF export | Vector PDF per page; layers as optional content groups; fonts subset; links; PDF/A-2b option; through the print pipeline (G-16). | M | 2 |
| OF08-F106 | Raster export | PNG, JPEG, WebP; scale factors (1×, 2×, 3×, custom ppi); transparent background; export slices: named areas or objects exported as separate files in one step to a granted folder. | M | 1 |
| OF08-F107 | draw.io import and export | `.drawio` and mxGraph XML (plain, deflated-and-base64 forms): cells, geometry, styles mapped to Draw styles, connectors with waypoints, layers, pages, groups, containers. Stencil references are mapped to Draw stencils by name where known; unknown shapes use the embedded stencil XML geometry when present, else a labelled box. HTML labels are reduced to plain rich text. | M | 2 |
| OF08-F108 | Visio import | `.vsdx`, `.vsdm` (macros ignored), `.vssx`, `.vstx` (OPC package, XML): shapes with geometry sections, masters to symbols, connectors and glue, pages and backgrounds, layers, text, shape data, themes approximated. Formulas in ShapeSheet cells are read as their cached values only. Binary `.vsd` is C priority. | S | 3 |
| OF08-F109 | Other imports | PDF page to vectors (asks OF03's parser helper for a display list; C), EMF/WMF (C), DXF 2D entities (C), Graphviz DOT and Mermaid (see 4.7), image files as embedded images. | C | 3 |
| OF08-F110 | Import report | Every import lists what was approximated, dropped or blocked, with a link to the object. | M | 2 |
| OF08-F111 | Copy and paste formats | Offers: native fragment, SVG, PNG, plain text (labels). Accepts the same, plus images and text. | M | 1 |
| OF08-F112 | Print | Page tiling for large drawings over several sheets with overlap and crop marks; scale to fit; through `svc.portal.print`. | S | 2 |
| OF08-F113 | Batch export | Export all pages, or a list of documents picked through the portal, to a granted folder with a naming pattern. | C | 3 |

### 4.10 Embedding and reuse

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F114 | Embed in Writer and Slides | OF04 and OF06 store a drawing as an ODF embedded object (the ODG sub-document) plus an SVG replacement image. "Edit drawing" sends the `edit` intent with a handle to the sub-document; Draw opens it in an embedded-edit window titled with the host document; Save writes back through the handle and refreshes the replacement image. | M | 3 |
| OF08-F115 | Render service | A bus interface renders an ODG or SVG handle to SVG or PNG at a size, in the I/O helper, for hosts that only need a picture. | S | 3 |
| OF08-F116 | Shared vector core | The geometry, preset shapes, text-on-shape layout and renderer are one source library used by OF04, OF06 and Draw, so a shape looks the same in all three. | M | 1 |
| OF08-F117 | Linked drawings | A host document may link to a drawing file by portal token instead of embedding; the link is refreshed only on user action. | C | 3 |
| OF08-F118 | Drag out | Drag a selection to OF04, OF06, OF02 or Files: delivered as SVG with PNG fallback, or as a native object where the target accepts it. | S | 2 |

### 4.11 Editing support, safety and system integration

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF08-F119 | Undo and history | Unlimited per session; history panel; one step per gesture. | M | 1 |
| OF08-F120 | Autosave and recovery | Command journal every 2 s; recovery after a crash at the last command. | M | 1 |
| OF08-F121 | Local versions | A version per explicit save with thumbnails; restore a page or the document; visual diff between two versions (added, removed, moved, restyled objects highlighted). | S | 3 |
| OF08-F122 | Comments | Pinned notes on the canvas with author and time, resolve, comments panel; stored as ODF annotations. | S | 2 |
| OF08-F123 | Trust bar on open | For files from outside: origin, blocked content (scripts, external references, macros in `.vsdm`), embedded fonts not loaded. | M | 1 |
| OF08-F124 | Disposable open | Open in a throw-away instance with no remembered grants. | S | 2 |
| OF08-F125 | Metadata tools | Properties panel; strip author, software, hidden layers and off-page objects on export; SVG "plain" export removes all private metadata. | M | 2 |
| OF08-F126 | Templates | New from template (flowchart, network, UML, floor plan at 1:50 and 1:100, icon grid, poster sizes); user templates. | S | 2 |
| OF08-F127 | Command palette and shortcuts | Every action; single-key tool shortcuts; rebindable. | M | 1 |
| OF08-F128 | Pen and touch | Pressure, tilt and eraser end for the pencil; barrel button as secondary click; palm rejection from the platform; two-finger pan and pinch (needs G-12 pen input and FUSE-WP-08). | S | 2 |
| OF08-F129 | Search and thumbnails | Text labels and page names offered to `svc.search`; thumbnailer for ODG and SVG runs in the I/O helper. | S | 2 |
| OF08-F130 | Scripted generation | A signed ForgeScript module with no capabilities can create or update shapes through a narrow document API (for diagrams from data). | C | 3 |
| OF08-F131 | Accessibility of diagrams | Reading order, alt text per object and per page, a generated text outline of a diagram (nodes and what they connect to) exported with tagged PDF and as `desc`/`title` in SVG. | M | 2 |
| OF08-F132 | Help | Tool guides, the diagram language reference, stencil catalogue, shortcut list. | M | 1 |

## 5. GUI

Layout pattern: document canvas + inspector, with a tool strip. Default density: compact.

### 5.1 Window map

```
Draw
├─ Document window (tabs for several documents)
│  ├─ Header bar: menu, title, search / command palette, trust badge
│  ├─ Context toolbar (options of the active tool or selection)
│  ├─ Tool strip (left edge, 40 px)
│  ├─ Sidebar (F9): Stencils | Pages | Layers | Symbols | Comments | History
│  ├─ Canvas (rulers, grid, guides, pages, pasteboard)
│  ├─ Inspector (F4): Style | Text | Arrange | Data | Connector | Dimension | Page | Export
│  ├─ Bottom dock: Diagram code | Import report | Validation results
│  ├─ Page tab bar
│  └─ Status bar: tool hint, cursor position, selection size, snap bar, scale, zoom
├─ Embedded-edit window (same layout, host document named in the title, Save returns to host)
├─ Dialogs
│  ├─ New document (template, size, scale)   ├─ Export (SVG, PDF, raster, slices)
│  ├─ Page setup                             ├─ Import options (draw.io, Visio, DOT)
│  ├─ Layout graph                           ├─ Import report
│  ├─ Transform                              ├─ Stencil manager
│  ├─ Array                                  ├─ Save as stencil
│  ├─ Trace bitmap                           ├─ Dimension styles
│  ├─ Offset path                            ├─ Document properties and metadata
│  ├─ Find and replace                       ├─ Versions and visual diff
│  ├─ Print (tiling preview)                 └─ Preferences
├─ Popovers: colour picker, gradient editor, stroke and dash, marker picker, quick-connect
│            arrows, typed-input box, snap settings, zoom menu, node-type menu, stencil search
└─ Inline bars: trust bar, isolation-mode bar, code-linked bar, read-only bar
```

### 5.2 Main window

Shown at 1360×820, wide layout.

```
+------------------------------------------------------------------------------------------+
| [=] Draw — network-plan.odg                [search  Ctrl+K]            (1)[ok] [▁][▢][✕] |
+------------------------------------------------------------------------------------------+
| (2) Fill [■ v] Line [■ v][1.5pt v][—— v] Ends [◁ v][▷ v] | [Align v][Dist v] | [Group]    |
+--+---------------+--------------------------------------------------+--------------------+
|(3)| (4) Stencils  | (5)  0     50    100    150    200    250   mm   | (8) Inspector      |
|[↖]| [/ search___] |   +----------------------------------------------+ [Style][Text][Arr] |
|[▷]| ▾ Network     | 0 |  · · · · · · · · · · · · · · · · · · · · ·   |                    |
|[✎]| [▭][◯][⬡][▤] |   |    +--------+              +--------+        | Style [Device   v] |
|[▭]| [☁][▦][⎚][▥] |50 |    | Router |──────────────| Switch |        | Fill  [■ surface v]|
|[◯]| ▾ Flowchart   |   |    +--------+   (6) 1 Gb   +---o----+        | Line  [■ line    v]|
|[T]| [▭][◇][▱][⬭] |   |         ▲                      │   (7)       | Width [1.5 pt]     |
|[⌐]| ▸ UML         |100|      ◁ [▣] ▷              +---+----+        | Corner [2 mm]      |
|[✂]| ▸ Floor plan  |   |         ▼                 | Server |        | Shadow [ ]         |
|[🔍]| ▸ Electrical  |   |                           +--------+        |                    |
|[✋]|               |150|                                             | Data               |
|   | ▾ Recent      |   +----------------------------------------------+ name [core-sw-1__] |
|   | [▭][▣][☁]    |                                                    | ip   [10.0.0.2___] |
+--+---------------+--------------------------------------------------+--------------------+
| (9) [+] < Overview | Rack A | Floor 2 >                                                  |
+------------------------------------------------------------------------------------------+
| (10) Select: drag to move · X 81.0 Y 96.5 mm · 24×16 mm · Snap [▦][┼][◇][•] · 1:1 · 140% |
+------------------------------------------------------------------------------------------+
```

1. Trust badge. Draw has no network capability, so no `[net]` badge exists in this app.
2. Context toolbar: changes with the tool (pen: node type, close path; text: font; selection: style and arrange).
3. Tool strip: select `V`, direct select `A`, pen `P`, rectangle `R`, ellipse `E`, text `T`, connector `C`, knife `K`, zoom `Z`, hand `H`; long press opens a flyout of related tools (polygon, star, line, arc, pencil, dimension, measure).
4. Stencil panel, 220 px: search field, collapsible libraries as `gridView`s of 32 px shapes with tooltips.
5. Canvas with rulers and a dot grid.
6. A connector label. 7. A glued connector end on a connection point (`o`).
8. Inspector tabs: style, text, arrange, data.
9. Page tabs with add button and page list.
10. Status bar: tool hint, cursor position, selection size, snap toggles (grid, guides, objects, points), drawing scale, zoom.

Quick-connect arrows (`◁ ▷ ▲ ▼`) appear around a hovered or selected shape after 150 ms and for keyboard users on `Alt+Arrow`.

States: empty (new-document chooser with templates), loading (page thumbnails first, then tiles), error (inline bar; import report), offline (always; no difference), locked (encrypted ODG: password sheet), read-only (bar; viewing, export and copy work).

### 5.3 Other views and dialogs

**Node editing** (canvas detail with its context toolbar).

```
+------------------------------------------------------------------------------------------+
| Nodes: [Corner][Smooth][Symmetric][Auto] | [+ Add][− Delete][Join][Break] | X [41.2] Y [18.0]|
+------------------------------------------------------------------------------------------+
|                         ○───────────●────────────○   handles                             |
|                                    ╱ ╲                                                   |
|                    ■──────────────╯   ╰───────────────■                                  |
|                    │  (1) selected smooth node ●        │                                 |
|                    ■────────────────────────────────────■   corner nodes ■               |
+------------------------------------------------------------------------------------------+
```

1. Selected node with two handles. Keys: arrows nudge, `Tab` next node, `Shift+C` cycle node type, `Delete` remove keeping shape, `Ctrl+J` join, `Ctrl+B` break. Each node is an a11y item "node 4 of 9, smooth".

**Diagram code panel** (bottom dock, 1360×260).

```
+------------------------------------------------------------------------------------------+
| Diagram code — linked to group "Login flow"       Type [flowchart v]  [Apply] [Detach]   |
+---------------------------------------------------+--------------------------------------+
|  1  graph TD                                      | (2) Problems                         |
|  2    A[Open login page] --> B{Has account?}      | line 6: "click B call fn()" is not   |
|  3    B -- yes --> C[Enter password]              |   supported: actions are not allowed |
|  4    B -- no  --> D[Sign up]                     |                                      |
|  5    C --> E[Home]                               | Layout [Layered ↓ v] Spacing [12 mm]  |
|  6    click B call fn()                           | [x] Keep my manual positions          |
| (1)                                               | Theme [Document v]                    |
+---------------------------------------------------+--------------------------------------+
```

1. `codeEdit` with a grammar for the diagram language. 2. Problems list (`listView`), layout options for the generated group.

**Layout graph dialog** (560×460).

```
+-- Lay out diagram ----------------------------------------+
| Apply to  (•) Selection (14 shapes) ( ) Page ( ) Container |
| Algorithm [Layered                         v]             |
|   Direction (•)↓ ( )→ ( )↑ ( )←                           |
|   Layer spacing [18 mm]   Node spacing [10 mm]            |
|   Connectors [Orthogonal v]  [x] Route around shapes      |
| [x] Keep relative order where possible                    |
| [ ] Resize containers to fit                              |
| Seed [1]                                                  |
| +------------------ preview --------------------------+   |
| |   [A]→[B]→[C]                                       |   |
| |        ↘[D]                                         |   |
| +-----------------------------------------------------+   |
|                              [Cancel]  [Apply layout]     |
+-----------------------------------------------------------+
```

**Export dialog** (720×540).

```
+-- Export ----------------------------------------------------------------------+
| What   (•) Selection ( ) Page [Overview v] ( ) All pages ( ) Slices (3)        |
| Format [SVG v]   ( SVG · PDF · PNG · JPEG · WebP · ODG · draw.io )             |
+-----------------------------------+--------------------------------------------+
| SVG options                       |  Preview                                   |
| [x] Plain SVG (no private data)   |   +----------------------------------+     |
| [ ] Text as outlines              |   |   [Router]────[Switch]           |     |
| Precision [3] decimals            |   |                  │               |     |
| Size ( ) viewBox only (•) mm      |   |              [Server]            |     |
| Images (•) Embed ( ) Separate     |   +----------------------------------+     |
| IDs   (•) Keep names ( ) Shorten  |   212 × 148 mm · 14 objects · 6.1 kB       |
| [x] Include title and desc (a11y) |                                            |
+-----------------------------------+--------------------------------------------+
| ⚠ 1 effect (blur) is kept as an SVG filter; some viewers ignore it.            |
|                                                   [Cancel]  [Export…]          |
+--------------------------------------------------------------------------------+
```

**Import report** (640×440): `tableView` with Page, Object, Issue, Action taken (approximated, dropped, blocked); filter; `[Go to]`. Shown after draw.io, Visio and SVG imports when anything is listed.

**Layers panel** (sidebar tab).

```
+-- Layers ----------------------------+
| [/ filter____]            [+][🗑]    |
| 👁 🔓 ▾ Labels                 ■     |
| 👁 🔓 ▾ Devices                ■     |
| 👁 🔓    ▸ Group "Core" (4)          |
| 👁 🔓    > Switch core-sw-1          |
| 👁 🔒 ▾ Cabling                ■     |
| ◌ 🔒 ▸ Background (master)           |
+--------------------------------------+
```

`treeView` with two toggle columns; `F2` renames; drag re-parents; `Alt+click` on the eye isolates a layer.

**Stencil manager** (760×520): left `listView` of libraries with show check boxes, signer and trust badge; right `gridView` of the shapes; `[Import pack…] [Export pack…] [New library] [Remove]`.

**Dimension on canvas**: a selected dimension shows grips at both extension-line origins and on the text; the inspector's Dimension tab has style, precision, unit display, override text, and "attached to" with a `[Re-attach]` button.

**Embedded-edit window**: the main window with a bar "Editing a drawing in: report.odt (Writer)   [Save and return] [Discard]"; File menu is reduced to Save to host, Export a copy, Close.

**Versions and visual diff**: two page renders side by side or overlaid, with a legend (added green with `+`, removed red with `−`, changed amber with `~`), and a change list.

### 5.4 Menus

```
File    New… (Ctrl+N) · New from template… · Open… (Ctrl+O) · Open recent ▸ · Open in disposable sandbox…
        Import ▸ SVG… · draw.io… · Visio… · Diagram text… · Image… · Stencil pack…
        Save (Ctrl+S) · Save as… (Ctrl+Shift+S) · Save as template… · Export… (Ctrl+E) · Export again (Ctrl+Shift+E)
        Versions… · Properties… · Page setup… · Print… (Ctrl+P) · Close (Ctrl+W) · Quit (Ctrl+Q)
Edit    Undo (Ctrl+Z) · Redo (Ctrl+Shift+Z) · History… · Cut · Copy · Paste · Paste in place (Ctrl+Shift+V) · Paste style (Ctrl+Alt+V)
        Duplicate (Ctrl+D) · Delete · Select all (Ctrl+A) · Select similar ▸ · Invert selection (Ctrl+Shift+I)
        Find and replace… (Ctrl+F / Ctrl+H) · Preferences… (Ctrl+,)
View    Zoom ▸ In · Out · 100% (1) · Fit page (2) · Fit selection (3) · Fit drawing (4)
        Outline mode (Ctrl+Y) · Pixel preview · Rulers (Ctrl+R) · Grid (Ctrl+') · Guides (Ctrl+;) · Snap ▸ · Smart guides (Ctrl+U)
        Sidebar (F9) ▸ Stencils · Pages · Layers · Symbols · Comments · History · Inspector (F4) · Diagram code (Ctrl+`)
        Navigator · New view · Full screen (F11)
Insert  Shape ▸ · Text (T) · Connector (C) · Image… · Symbol ▸ · Dimension ▸ · Table · Diagram from text…
        Page (Ctrl+Shift+N) · Layer (Ctrl+Alt+L) · Comment (Ctrl+Alt+M) · Field ▸ · Link… (Ctrl+L)
Object  Group (Ctrl+G) · Ungroup (Ctrl+Shift+G) · Enter group (Enter) · Leave group (Esc)
        Bring to front (Ctrl+Shift+]) · Forward (Ctrl+]) · Backward (Ctrl+[) · To back (Ctrl+Shift+[)
        Align ▸ · Distribute ▸ · Transform… (Ctrl+Shift+T) · Flip ▸ · Rotate ▸ · Array…
        Lock (Ctrl+2) · Unlock all (Ctrl+Alt+2) · Hide (Ctrl+3) · Show all (Ctrl+Alt+3)
        Clip ▸ Make (Ctrl+7) · Release (Ctrl+Alt+7) · Mask ▸ · Create symbol (F8) · Detach instance · Save as stencil…
Path    Convert to path (Ctrl+Shift+O) · Outline stroke · Union (Ctrl+Alt+U) · Subtract (Ctrl+Alt+S) · Intersect (Ctrl+Alt+I)
        Exclude (Ctrl+Alt+X) · Divide · Expand compound · Join nodes (Ctrl+J) · Break (Ctrl+B) · Reverse
        Simplify… · Offset… · Round corners… · Trace bitmap…
Diagram Lay out… (Ctrl+Shift+L) · Tidy ▸ · Re-route connectors · Line jumps ▸ · Connection points ▸ Add · Edit
        Container ▸ · Validate · Shape data… · Export shape data…
Text    Font ▸ · Size ▸ · Bold (Ctrl+B in text) · Italic (Ctrl+I) · Align ▸ · Text on path · Convert to outlines · Spelling (F7) · Styles…
Help    Draw help (F1) · Diagram language reference · Stencil catalogue · Keyboard shortcuts · About
```

### 5.5 Keyboard and pointer

Tool keys (single letters when no text is being edited): `V A P N` (pencil) `R E L` (line) `T C K M` (measure) `D` (dimension) `Z H`; `Space` held is the hand; `Esc` returns to select. `Enter` on a shape edits its text; `F2` also. `Tab` and `Shift+Tab` walk objects in reading order. Arrows nudge by 1 unit of the grid subdivision, `Shift` ×10, `Alt` ×0.1. `Alt+Arrow` on a shape triggers quick connect in that direction. `Ctrl+drag` copies, `Shift` constrains, `Alt` resizes from the centre. While dragging, typing digits opens the typed-input box. `X` swaps fill and stroke, `/` sets none, `I` eyedropper. `[` and `]` change stroke width. `Ctrl+Shift+Left/Right` moves between pages (`Ctrl+PageUp/PageDown` also).

Context menus: object (cut, copy, paste, duplicate, order, group, lock, convert, symbol, style, data, link, alt text, export selection); connector (type, reroute, add waypoint, add label, reverse, ends); node (type, delete, break, join); canvas (paste, select all, grid, guides, page setup); layer row; page tab (insert, duplicate, delete, rename, master, move); stencil shape (add to favourites, edit, remove); ruler (units, origin).

Drag and drop: stencil to canvas; stencil onto a connector to insert; stencil onto a shape to replace it keeping connections (`Alt`); files onto the canvas (images embed; SVG, draw.io, Visio, `.mmd`, `.dot` import as a group or as a new page by choice); colour swatch onto an object; layer rows to reorder; selection out as SVG and PNG; pages between documents.

Pointer and pen: double-click a shape to edit text, a path to edit nodes, a group to enter it; pen pressure for pencil width; eraser end erases path parts; two-finger pan, pinch zoom, two-finger rotate of the selection when enabled.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | New-document chooser: templates (`gridView`), recent, "Open…", "Paste diagram text". |
| Loading | Page list first; tiles fill in; large imports show progress with Cancel. |
| Error | Parse error with the part and offset; "Recover readable pages". Geometry helper crash: restart, redo the operation once, then report. |
| Offline | The only state: the app has no network capability. |
| Locked | Password sheet for encrypted ODG. |
| Read-only | Bar with reason; export still works. |
| Isolation mode | Bar "Inside group 'Core' — Esc to leave"; the rest of the page at 30% opacity. |
| Code-linked selection | Bar "Generated from text — edit the text, or Detach". Direct geometry edits are limited to move and style. |
| Narrow (<640) | Tool strip stays; stencil panel and inspector become overlays; context toolbar scrolls. |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Canvas | 2D canvas (**G-12**) | Requirements below |
| Tool strip, context toolbar | `panel`, `toolButton`, `comboBox`, `spinBox` | |
| Stencil panel | `lineEdit` (search), `gridView` | Needs sections and drag out (FUSE-WP-28, FUSE-WP-33) |
| Pages panel, page tabs | `gridView`, `tabBar` | |
| Layers, symbols, history | `treeView`, `listView` | Model-backed tree with toggle columns and drag re-parent (FUSE-WP-28) |
| Inspector | `tabWidget`, `propertyGrid`, `colorPicker`, `fontPicker`, `slider` | Gradient editor is a custom control on the canvas plus a stop list |
| Code panel | `codeEdit` | Custom grammar; error underlines |
| Text editing on canvas | rich text (**G-10**) hosted inside the canvas | |
| Navigator | `viewportWidget` | |
| Dialog previews | canvas in view-only mode | |
| Help | `markdownView` | |

`nodeGraph` (typed pins and bezier links) is not used as the diagram surface: it has no free-form shapes, routing, layers or export. Diagrams are ordinary canvas objects.

**What the canvas (G-12) must do.** This list is Draw's requirement on the platform widget; OF06, OF04, CR01, OF03 and IN04 share parts of it.

| # | Requirement |
|---|---|
| C1 | A scene the app owns; the widget renders it. Retained display items with stable ids, z-order, parent groups and per-item transforms (2×3 affine). |
| C2 | Vector primitives in the draw list: filled and stroked paths (lines, quadratic and cubic beziers, arcs), even-odd and non-zero fill rules, dashes, caps, joins, mitre limit (FUSE-WP-23 lists paths, transforms and gradients). |
| C3 | Paint servers: solid, linear, radial and conic gradients with many stops, pattern tiles, images with opacity. |
| C4 | Clipping by path, opacity groups (off-screen layers), blend modes, blur and shadow filters. |
| C5 | Text runs positioned by the app, with shaping (G-03), on straight baselines and along paths (glyph-by-glyph transforms). |
| C6 | View transform with zoom from 1% to 25,600% and pan in 64-bit document coordinates; pixel snapping for hairlines; anti-aliasing with correct coverage at any zoom. |
| C7 | Tiled, cached rendering with a spatial index; only damaged tiles re-render; tiles render off the UI thread; a budget of 8 ms of main-thread work per frame. |
| C8 | Overlay layer that is not part of the document: selection handles, node handles, guides, smart-guide lines, rubber bands, snapping marks, cursors, drawn in screen space at constant size. |
| C9 | Hit testing: by point with tolerance (fill, stroke, handle), by rectangle (touching or enclosing), returning ids in z-order; done against exact geometry, not bounding boxes. |
| C10 | Input: pointer with sub-pixel positions, wheel and touchpad gestures (pan, pinch, rotate), touch, and pen with pressure, tilt, eraser and barrel buttons (FUSE-WP-08); pointer capture during drags; coalesced events exposed for smooth freehand. |
| C11 | Rulers and grid as optional built-in layers with app-supplied units and scale. |
| C12 | Hosting an in-place editor (rich text, G-10) inside the canvas at an arbitrary transform, at least rotation in 90° steps with free rotation falling back to an unrotated editing box. |
| C13 | Several canvases on one scene (split views, navigator, export preview) without copying it. |
| C14 | Off-screen rendering of a scene region to an image at a given scale, and to a vector target (PDF through G-16; SVG is written by the app). The same code path as on-screen so exports match the view. |
| C15 | Accessibility: the app supplies a tree of items (role, name, bounds, actions); the canvas maps it to the a11y bridge and moves a11y focus with keyboard selection. |
| C16 | Colour: sRGB now; wide gamut when FUSE-WP-26 lands; no change to the scene format. |
| C17 | Zero allocation and zero frames when idle; no steady-state allocation while panning a cached view. |
| C18 | Both backends: CPU rasteriser and Vulkan compute (V1), with identical output within a stated tolerance for goldens. |

Gaps beyond G-12:

- **OF08-GAP-1 Vector core library placement.** A shared, signed source library for geometry and rendering of ODF shapes used by three apps. PLATFORM.md section 3 has no shared-library bundle; each app would carry its own copy of the helper.
- **OF08-GAP-2 Embedded object editing protocol.** `svc.intent` `edit` with a handle to a sub-document inside another app's file, with save-back and replacement-image refresh. PLATFORM.md section 9 lists the intent but not this flow.
- **OF08-GAP-3 Gradient and dash editors** as reusable controls (also wanted by OF06, CR01, DV02).
- G-10, G-03, G-16, G-17 as noted.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Tools as state machines, commands, undo, document model front, stencil catalogue, diagram-language front end, settings, intents |
| UI documents and styles | FUSE UI | app host | Window, panels, dialogs |
| Scene adapter | ForgeScript + typed binding | app host | Mirrors the document into the canvas scene with batched updates (G-02) |
| Vector core | C++23 | helper `vector-core` | Path maths (flattening, offset, simplify), boolean operations, preset shape geometry, hit testing support, text-on-path layout, marker and dash expansion, tracing. Justification: speed |
| Graph layout and routing | C++23 | helper `vector-core` (same process) | Layered, tree, force, circular, orthogonal layouts; obstacle-avoiding connector routing |
| Drawing I/O helper | C++23 | helper `draw-io` | Parse and write ODG, SVG, draw.io, Visio, DOT, EMF/WMF, DXF, stencil packs; build PDF pages; raster export encode; thumbnails; render service. Justification: untrusted bytes |
| Diagram-language parser | ForgeScript | app host | Parses the text grammar to a graph model (memory safe in the interpreter; inputs capped at 1 MiB) |
| Script host for generation | C++23 embedding the ForgeScript engine | helper `macro-host` | Signed scripts with no capabilities |

```
            +----------------------- app host -----------------------+
 pointer, | FUSE UI: canvas (G-12), panels   <->   ForgeScript:       |
 pen, keys| scene (display items)                  document, tools,   |
          |        ^  batched updates              commands, parser   |
          +--------|-----------------------|-----------------|--------+
                   | geometry results      | jobs + handles  | render/1 (bus)
                   v                       v                 v
            +---------------+        +----------+     OF04 Writer, OF06 Slides
            |  vector-core  |        | draw-io  |     (edit intent, replacement image)
            | booleans,     |        | parsers, |
            | layout, route |        | writers  |
            +---------------+        +----------+
              no files, no net        only passed file handles, no net
```

### 6.2 Data model

| Entity | Fields |
|---|---|
| `Document` | `id: uuid`, `pages: [PageId]`, `masters: [PageId]`, `layers_shared: [Layer]`, `styles: [Style]`, `symbols: [Symbol]`, `swatches: [Swatch]`, `theme: Theme`, `units: enum`, `meta: Metadata`, `stencil_refs: [StencilRef]`, `opaque_parts: [Blob]`, `revision: u64` |
| `Page` | `id: u32`, `name`, `size: (w, h)` in µm (`i64`), `scale: Rational`, `master: PageId?`, `background: Paint`, `grid: Grid{kind, spacing, subdiv}`, `guides: [Guide{pos, angle}]`, `layers: [Layer]`, `export_slices: [Slice{name, rect, formats}]` |
| `Layer` | `id`, `name`, `visible`, `locked`, `printable`, `opacity`, `colour_tag`, `children: [NodeId]` |
| `Node` (object) | `id: u64`, `name`, `kind: enum{shape, path, text, image, group, connector, symbol_instance, dimension, container, compound, code_diagram, table}`, `transform: [f64; 6]`, `style_ref`, `overrides: StyleDelta`, `text: RichText?`, `text_box: TextFrame`, `connection_points: [Port{id, pos_rel, dir?}]`, `data: [Field{key, type, value}]`, `alt: {title, desc}`, `locked`, `hidden`, `link?`, `children: [NodeId]`, kind payload |
| `Path` payload | `subpaths: [SubPath{closed, nodes: [PathNode{p, h_in, h_out, type}]}]`, `fill_rule` |
| `Shape` payload | `preset: text`, `params: [f64]` (live adjust values) |
| `Connector` payload | `kind: enum{straight, ortho, rounded, curved, arc}`, `start: End{node?, port?, floating_pos, marker}`, `end: End`, `waypoints: [Point]`, `pinned: bool`, `labels: [Label{t, offset, text}]`, `jump: enum` |
| `Dimension` payload | `kind`, `anchors: [AnchorRef{node, feature}]`, `offset`, `style_ref`, `override_text?` |
| `Symbol` | `id`, `name`, `root: NodeId`, `override_slots: [Slot{key, kind}]`, `resize_rule` |
| `CodeDiagram` payload | `language: enum`, `source: text`, `layout: LayoutParams`, `node_overrides: {source_id → {pos?, style?}}` |
| `Style` | `id`, `name`, `parent?`, `fills: [Paint]`, `strokes: [Stroke{paint, width, dash, cap, join, align, profile}]`, `markers`, `effects: [Effect]`, `opacity`, `blend`, `text: TextStyle` |
| `Stencil` | `library`, `id`, `name`, `keywords`, `geometry: Node`, `ports`, `data_schema`, `resize_rule`, `theme_roles` |

Coordinates are integers in micrometres in the model (exact, no drift) and `f64` inside transforms and the geometry helper.

Stores: recovery journal `~/.local/share/org.pbsd.draw/recovery/<doc-id>.journal`; versions under `versions/` with `versions.sqlite`; user stencils and templates under `~/.local/share/org.pbsd.draw/stencils/` and `templates/`; `app.sqlite` (recent documents as portal tokens, favourites, export presets, `schema_version`); tile and thumbnail cache in `~/.cache/org.pbsd.draw/`. Built-in stencils are read-only in the bundle under `assets/stencils/`.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| ODG, OTG, FODG (ODF 1.3) | yes | yes | Own ZIP and strict XML readers (shared source with OF05, OF06); zlib (zlib licence) | `draw-io` |
| SVG 1.1 / SVG 2 subset, SVGZ | yes | yes | Own parser and writer; own CSS-subset reader | `draw-io` |
| draw.io / mxGraph XML | yes | yes | Own code; inflate via zlib | `draw-io` |
| Visio VSDX, VSSX, VSTX, VSDM | yes | no | Own OPC and VisioML mapping | `draw-io` |
| Visio VSD (binary), EMF, WMF, DXF 2D | yes (C) | no | Own code | `draw-io` |
| Mermaid-compatible diagram text | yes | partly | Own grammar | app host (ForgeScript) |
| Graphviz DOT | yes | yes (structure) | Own grammar. Graphviz itself (EPL-2.0) is not used | app host (ForgeScript) |
| Stencil pack `.pbstencil` | yes | yes | ZIP of SVG plus `stencil.json`; detached Ed25519 signature | `draw-io` |
| PDF | via OF03 helper (C) | yes | Print pipeline (G-16) | spooler; OF03's parser sandbox for import |
| PNG, JPEG, WebP | yes | yes | Platform image pipeline (FUSE-WP-27) | platform image helper |
| GPL palette text | yes | yes | Own code | app host |
| Render service `org.pbsd.draw.render/1` | — | — | Bus schema: `render(handle, page, format, size) → handle` | `draw-io` |

### 6.4 Integration

- Intents handled: `open`, `edit` (including embedded-edit from OF04 and OF06), `view`, `new`, `print`. Emitted: `share`, `print`, `edit` (image to CR01), `open` (links).
- MIME view/edit: `application/vnd.oasis.opendocument.graphics`, `…graphics-template`, `…graphics-flat-xml`, `image/svg+xml`. Import: `application/vnd.jgraph.mxfile`, `application/vnd.ms-visio.drawing.main+xml` (and stencil, template variants), `text/vnd.mermaid`, `text/vnd.graphviz`, `image/png`, `image/jpeg`, `image/webp`. Export: `image/svg+xml`, `application/pdf`, `image/png`, `image/jpeg`, `image/webp`, `application/vnd.jgraph.mxfile`.
- Services: `svc.settings`, `svc.notify` (long exports), `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.clipboard`, `svc.thumbnail`, `svc.keyring` (signer trust list), `svc.audit`, `svc.a11y`, `svc.backup`, `svc.model` (optional).
- Interface offered: `org.pbsd.draw.render/1`.
- Other apps: OF04 and OF06 (embedding, shared vector core and themes), OF05 (embedded charts appear as replacement images inside drawings), OF09 (ER diagrams exported from a schema open in Draw as ordinary shapes), CR01 (raster edits), OF03 (PDF import), SH04 (screen colour picker), DV02 (may reuse the canvas tools; separate app).
- Backing repos. Verified: FUSE's draw list today has Rect, Shadow, Glyph, Image, Line, PushClip, PopClip, Backdrop and no paths, transforms or multi-stop gradients (FUSE build plan 2.2 citing `FUSE:Source/FUSE/UI/Core/include/fuse/ui/draw_list.hpp:15-24`); no canvas widget exists in `widgets.hpp`; `ui::Event` has no pen or touch type (same section, citing `ui.hpp:32-51`). So Draw cannot be built on today's FUSE; it depends on G-12 and FUSE-WP-23 and FUSE-WP-08. Verified: `nodeGraph`, `codeEdit`, `viewportWidget`, `gridView` exist (`FUSE:docs/ui/README.md`). Verified: ForgeScript signing (V14). Proposed: everything else.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | Journal, versions, user stencils and templates |
| `fs.portal` | Open, save, import, export, batch export to a granted folder |
| `net.none` | The app never uses the network. No optional network capability is declared |
| `proc.helper:vector-core` | Geometry, layout, routing |
| `proc.helper:draw-io` | File parsing and writing, render service |
| `proc.helper:macro-host` | Signed generation scripts |
| `dev.print` | Printing |
| `dev.gpu.compute` (optional, off) | Canvas on the Vulkan backend |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.clipboard`, `svc.thumbnail`, `svc.keyring`, `svc.audit`, `svc.a11y`, `svc.backup` | As in 6.4 |
| `ai.local`, `svc.model` (optional, off) | Draft diagram text from a description |

Assets: drawings (network plans and floor plans are sensitive: they describe infrastructure and buildings), shape data (addresses, host names), the user's other files, hosts that embed drawings.

Adversaries: a crafted SVG (scripts, external references as tracking beacons, entity bombs, filter bombs with huge blur radii), crafted draw.io (HTML labels with scripts, compressed payload bombs), crafted Visio packages, a hostile stencil pack, pathological geometry (millions of nodes, self-intersections that blow up boolean operations), a hostile embedded drawing inside a Writer document.

Mitigations:

| Threat | Mitigation |
|---|---|
| Script and remote content in SVG and draw.io | Never executed, never fetched; removed on import and reported; no web view anywhere in the app |
| Parser bugs | All foreign formats parsed in `draw-io` in capability mode with no network; per-job process; output re-validated by the app (node counts, coordinate ranges) |
| Bombs | Limits: 256 MiB expanded per file, 2 million nodes, nesting depth 256, `use` expansion count 100,000, filter region 16,384 px, blur radius 500 px; inflate ratio cap |
| Geometry blow-up | Boolean and offset operations have operand-size and time budgets; cancellable; the helper is restarted on overrun |
| Stencils as a code path | Stencils are data with declarative rules only; signed packs show a signer; no scripts in shapes |
| Infrastructure details leaking | "Plain SVG" and "strip shape data" export options; pre-share check lists hidden layers, off-page objects, shape data fields, comments, metadata |
| Embedded-edit abuse | The host passes a handle to the sub-document only; Draw gets no access to the host file; the replacement image is produced by `draw-io` |
| Render service abuse | Size caps (16,384 px), time budget, only handles passed by the caller |

Stored: journal, versions, stencils, recent list under the app directories. Leaves the machine: nothing. There is no network code in the app or its helpers.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `new.template` | text | `blank-a4-landscape` | |
| `new.units` | enum | locale (`mm` or `in`) | |
| `new.boundless` | bool | `false` | Boundless page for diagrams |
| `canvas.grid.kind` | enum | `dots` | `lines`, `dots`, `isometric`, `none` |
| `canvas.grid.spacing_mm` / `.subdivisions` | float / int | `10` / `2` | |
| `canvas.snap.grid` / `.guides` / `.objects` / `.points` | bool | `true` / `true` / `true` / `true` | |
| `canvas.snap.tolerance_px` | int | `8` | |
| `canvas.smart_guides` | bool | `true` | |
| `canvas.outline_selected_only` | bool | `false` | |
| `canvas.backend` | enum | `auto` | `auto`, `cpu`, `gpu` |
| `tools.nudge_fine` | float | `0.1` | Multiplier with `Alt` |
| `tools.pencil.smoothing` | int | `50` | 0–100 |
| `tools.pen.rubber_band` | bool | `true` | |
| `connect.default_kind` | enum | `ortho` | |
| `connect.autoroute` | bool | `true` | |
| `connect.clearance_mm` | float | `4` | |
| `connect.line_jumps` | enum | `none` | `none`, `arc`, `gap` |
| `connect.quick_arrows` | bool | `true` | |
| `layout.default` | enum | `layered_down` | |
| `layout.animate` | bool | `true` | Off with reduced motion |
| `code.update_delay_ms` | int | `300` | |
| `export.svg.plain` | bool | `true` | No private metadata |
| `export.svg.precision` | int | `3` | |
| `export.raster.scale` | float | `2.0` | |
| `export.strip_shape_data` | bool | `false` | |
| `import.report.always` | bool | `false` | Show the report even when empty |
| `security.embedded_fonts` | enum | `never` | |
| `recovery.journal_interval_ms` | int | `2000` | |
| `versions.keep` | int | `30` | |
| `ai.diagram_drafts` | bool | `false` | Needs `ai.local` |

## 9. Accessibility and languages

- Everything a pointer can do has a key route: place a stencil (`/`, type, `Enter` drops at the view centre or connected to the selection), connect (`Alt+Arrow`, or select two shapes and `Ctrl+Shift+C`), move and resize by keys with spoken deltas, edit nodes by keys.
- The canvas exposes objects in reading order with role (shape, connector, text, image, group), name, and for connectors "from A to B, label L".
- A generated diagram outline (text list of nodes and their links) is available in a panel and is what the screen reader reads for a page summary.
- Colour is never the only carrier: stencil themes pair colour with shape or label; the contrast helper covers text on fills.
- Right-to-left text in shapes; layouts can run right-to-left; vertical text for CJK (G-03).
- Handles and hit targets scale with the density setting; at touch density handles are 24 px with 44 px hit areas.

## 10. Performance budgets

| Measure | Target |
|---|---|
| Cold start to new-document chooser | 400 ms |
| Open a 5,000-object drawing | 1 s to first full view |
| Pan and zoom frame time | under 8 ms with 100,000 objects (cached tiles), under 16 ms while tiles refill |
| Drag one object among 10,000 with live connector re-routing | under 16 ms per frame |
| Boolean union of two 10,000-node paths | under 200 ms |
| Layered layout of 1,000 nodes and 2,000 edges | under 1 s |
| Code panel update for a 200-line diagram | under 100 ms after the delay |
| SVG export of 10,000 objects | under 1 s |
| Memory | under 120 MiB for an empty document; tile cache capped at 512 MiB |
| Idle | zero frames, zero wake-ups |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: tool state machines, command inverses (do then undo restores the model byte for byte), style inheritance, symbol overrides, unit and scale parsing, diagram grammar (accept and reject tables).
- **Geometry property tests**: boolean identities (`A ∪ A = A`, `A ∩ B ⊆ A`, area of union plus intersection equals sum of areas), offset then inverse offset bounds, flattening error under tolerance, routing never crosses an obstacle when a free route exists.
- **Round trip**: ODG → model → ODG; SVG → model → SVG render-compared; draw.io → model → draw.io structure-compared.
- **Differential render**: the resvg test suite and the W3C SVG 1.1 test suite images as references for the supported subset (used as test data only); LibreOffice Draw as an oracle for ODG; draw.io's own PNG exports of sample files for import fidelity.
- **Golden renders**: every preset shape, marker, dash, gradient kind, each stencil library sheet, each layout algorithm on fixed graphs, both backends.
- **Layout determinism**: same input and seed gives identical coordinates on repeated runs and thread counts.
- **Scripted UI**: the 14 top tasks by keyboard through the a11y tree; pen input replay files.
- **Fuzz**: SVG, CSS subset, path data strings, mxGraph XML and its compressed forms, VSDX parts, DOT and Mermaid text, stencil packs, journal, render-service requests, geometry helper inputs (random and adversarial polygons).
- **Security tests**: an SVG corpus with scripts, external references and bombs must produce no socket, no process, bounded memory and a report line each.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Integer coordinate model and transforms composition | No overflow within the stated page limits | T2 | PRISM range analysis | — |
| Path data and number parsers | No runtime errors on any input | T2 | PRISM plus fuzzing | — |
| ZIP and XML readers | No runtime errors | T2 | Shared with OF05 | — |
| Boolean operations | Result is a valid region; identities hold | T3 with property tests; T2 for absence of runtime errors | PRISM for bounds and termination; property tests for correctness | Exact-predicate geometry proofs are research-grade |
| Orthogonal router | Terminates; output avoids obstacles | T2 for termination and bounds; T3 for quality | PRISM plus tests | — |
| Layout algorithms | Terminate; deterministic | T3 | Tests | Heuristics have no simple specification |
| SVG, mxGraph, VisioML mapping | Memory safe | T3 | Safe subset, sanitisers, fuzzing | Format size |
| Undo system | `undo(do(c, m)) = m` for every command | T1 for the command algebra core | Proof over the command representation; each command's inverse tested | Commands are ForgeScript; the proof covers a model of them |
| Diagram grammar (ForgeScript) | Rejects action and script constructs | T3 | Reject-table tests, fuzzing | — |
| Render service boundary | Only passed handles are touched | T2 | Capability confinement plus PRISM on the request parser | — |

## 12. Build plan

Phase 1 — usable core:

| ID | Work item | Depends on |
|---|---|---|
| OF08-W01 | Canvas requirements review with the FUSE plan (C1–C18) and a prototype scene on `viewportWidget` | G-12, FUSE-WP-23 |
| OF08-W02 | Document model, command layer, undo, journal | G-07, G-08 |
| OF08-W03 | Vector core helper: paths, flattening, preset shapes, markers, dashes, hit geometry | G-06 |
| OF08-W04 | Tools: select, transform, primitives, pen, node editing | OF08-W01, OF08-W02, OF08-W03, G-02 |
| OF08-W05 | Fills, strokes, gradients, inspector | OF08-W04 |
| OF08-W06 | Text on shapes, point and area text | G-10, G-03 |
| OF08-W07 | Grid, guides, rulers, snapping, smart guides, align and distribute | OF08-W04 |
| OF08-W08 | Layers, groups, pages, page tabs, panels | OF08-W02, FUSE-WP-28 |
| OF08-W09 | Connectors with glue, labels, quick connect; stencil panel with the basic and flowchart sets | OF08-W04 |
| OF08-W10 | ODG read and write | ZIP and XML readers (shared with OF05) |
| OF08-W11 | SVG import with the safety rules, SVG and raster export | OF08-W03, FUSE-WP-27 |
| OF08-W12 | Trust bar, recovery, palette, help; vector core packaged for OF06 and OF04 | G-09, OF08-GAP-1 |

Phase 2 — full feature:

| ID | Work item | Depends on |
|---|---|---|
| OF08-W13 | Boolean operations, compound shapes, path operations, clipping and masks | OF08-W03 |
| OF08-W14 | Automatic connector routing, line jumps | OF08-W09 |
| OF08-W15 | Layout algorithms and tidy tools | OF08-W14 |
| OF08-W16 | Diagram language, code panel, code-linked objects | OF08-W15 |
| OF08-W17 | Symbols, full stencil libraries, custom stencils, shape data, containers | OF08-W09 |
| OF08-W18 | Styles, themes, swatches, effects, blend modes | OF08-W05 |
| OF08-W19 | Dimensions, drawing scale, typed input, measure tool | OF08-W07 |
| OF08-W20 | draw.io import and export, import report | OF08-W10 |
| OF08-W21 | PDF export, print with tiling, page masters | G-16 |
| OF08-W22 | Pen and touch input, pencil tool | FUSE-WP-08 |
| OF08-W23 | Comments, metadata tools, templates, search and thumbnails, disposable open, diagram accessibility outline | G-09 |

Phase 3 — advanced:

| ID | Work item | Depends on |
|---|---|---|
| OF08-W24 | Embedding in OF04 and OF06; render service | OF08-GAP-2, OF04, OF06 |
| OF08-W25 | Visio import | OF08-W20 |
| OF08-W26 | DOT and other text inputs, headless export, diagram to code | OF08-W16 |
| OF08-W27 | Wall tool, hatches, area schedule, viewports | OF08-W19 |
| OF08-W28 | Trace bitmap, shape builder, blend, sketch style, appearance stack | OF08-W13 |
| OF08-W29 | Versions with visual diff, stencil signing, batch export | SE04, G-09 |
| OF08-W30 | PDF, EMF/WMF, DXF, VSD imports | OF03 |
| OF08-W31 | Scripted generation, diagram drafts with `svc.model`, validation rules | FORGE signing, G-15 |

## 13. Open questions and risks

- **G-12 is the whole app.** The canvas requirement list (C1–C18) is large and shared by six apps. If the platform canvas lands late, Draw's phase 1 moves with it. The model, I/O and geometry helper can be built and tested without it.
- **Robust boolean operations** are hard. Options: integer-snapped coordinates with exact predicates (slower, reliable) or floating point with tolerances (fast, fragile). The plan assumes the first.
- **Mermaid compatibility** is a moving target with no formal grammar. The plan fixes a documented subset and reports the rest; users may expect more.
- **Visio fidelity.** ShapeSheet formulas are not evaluated; shapes that depend on them import as their last cached geometry.
- **Native format choice.** ODG keeps the suite on ODF and shares code with OF06. SVG is friendlier to other tools. The plan supports both as working formats; features outside SVG's model (pages, connectors, symbols with overrides) live in namespaced metadata that other tools ignore.
- **Shared vector core** has no home in the bundle format (OF08-GAP-1). Three copies of one helper is acceptable at first but means three updates for one fix.
- **Embedded editing flow** (OF08-GAP-2) needs a platform decision shared with OF05's embedding.
- **Stencil content.** Drawing several hundred symbols to a consistent standard is art work, not code; licences of any reused symbol sets must allow redistribution (IEC and ISO symbol standards are paid documents; shapes must be drawn from scratch).
- **Text inside rotated shapes** needs the rich text widget to work under a transform (C12); otherwise editing falls back to an unrotated overlay.
