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
| OF08-F046 | Fills | None, solid, linear, radial, conic and mesh-free multi-stop gradients with on-canvas handles; pattern fills (built-in hatches and tiles, user patterns from a selection); image fill. | M | 1 |
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
| OF08-F084 | Text-to-diagram language | A documented grammar that accepts the common core of Mermaid syntax for: flowchart (`graph TD`), sequence, class, state, ER, Gantt, mind map, pie is excluded (charts are OF05's). Unsupported statements are reported by line; nothing is silently dropped. | M | 2 |
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
