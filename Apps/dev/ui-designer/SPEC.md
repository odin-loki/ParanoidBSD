<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# UI Designer

By Odin Loch

| | |
|---|---|
| ID | `DV02` (from CATALOGUE.md) |
| Bundle | `org.pbsd.ui-designer` |
| Category | dev |
| Priority | P2 |
| Backing repos | FUSE (`Source/FUSE/UI/Designer` prototype, `Source/FUSE/UI/Doc` document format) |
| Status | Planned. A prototype exists in FUSE (section 1.1). The app in this spec is not built. |

## 1. Purpose

A visual designer for `.fuseui` documents and their style sheets. App authors and designers use it to lay out windows, dialogs and panels, bind them to a mock view model, check them in every theme, density, width and language, and lint them against the design system. It is the Qt Designer equivalent for the suite and the visual half of DV01.

### 1.1 What exists today (Verified)

Read in `FUSE:Source/FUSE/UI/Designer/include/fuse/ui/designer/designer.hpp`, `Designer/src/designer.cpp`, `Designer/app/designer_main.cpp`, `FUSE:Source/FUSE/UI/tests/test_designer.cpp`, `FUSE:docs/ui/README.md` and `FUSE:docs/plans/FUSE_UI_PLAN.md` (row UI8).

| Area | The prototype does | The prototype does not |
|---|---|---|
| Model | `DesignDocument`: a tree of nodes whose members are kept verbatim as JSON, so bindings, `"on"` actions, `items` and unknown keys survive. Preorder indexing. `insert`, `insertAt`, `remove`, `removeMany`, `reparent`, `move`, `setProp` (with `style.<prop>` addressing), `setStyleSheet`, `ensureIds`, edit batches. | No component or fragment model, no multiple documents, no schema knowledge of which property fits which widget. |
| Undo | Snapshot undo and redo: each step stores the whole document text (`std::vector<std::string> m_undo, m_redo`). | No per-edit commands, no undo history view, no memory cap. |
| Save | Pretty JSON, 2-space indent. Load, save, load is a fixed point (gate `fuse_ui_designer_roundtrip`). Keys starting with `x-` are kept for tools and stripped from the live preview (`previewText()`), because `fuse_ui_doc` rejects them. | It does not keep the original byte layout of a hand-written file: it re-prints the JSON. Comments do not exist in the format. |
| Palette | 12 classes: `panel`, `label`, `button`, `checkbox`, `switch`, `radiobutton`, `slider`, `progressbar`, `spinbox`, `combobox`, `lineedit`, `separator`. Enter adds under the selection; drag to the canvas places at a container and child slot. | The other widgets of PLATFORM.md V3. |
| Containers | `panel`, `scrollarea`, `card` only (`DesignDocument::isContainer`). | Tabs, splitters, docks, grids as containers. |
| Canvas | The document is instantiated through `fuse_ui_doc`, so the canvas is the real runtime rendering. Clicks select instead of operating widgets. Ctrl or Shift click multi-selects. Drag moves the selection to a new container and slot as one undo step. A `.dsel` class draws a 2 px outline. | No resize handles, guides, rulers, zoom, pan, artboards or device frames. |
| Outline | A tree that follows the selection. | No drag in the outline, no rename, no visibility or lock toggles. |
| Properties | A `propertyGrid` with 19 text rows: `id`, `text`, `classes`, `min`, `max`, `value`; `width`, `height`, `padding`, `gap`, `flex-grow`, `flex-direction`; `margin`, `background`, `color`, `border`, `radius`, `font-size`, `align-items`, `justify-content`. Numbers are detected by parse. | No typed editors (colour, enum, token), no per-class property sets, no binding editor. |
| Style | One `codeEdit` for the document's top-level `"style"` string with an Apply button (one undo step). | No external CSS files, no token browser, no selector tools. |
| Toolbar | New, Undo, Redo, Delete, Up, Down. Ctrl+Z, Ctrl+Y. | No Open or Save in the UI: files come from the command line (`fuse_ui_designer [file.fuseui] [--save=out] [--screenshot=png]`). |
| Preview data | A `doc::Model` exists so bindings compile. | No way to edit model values. |
| States, themes, locale, a11y, animation, linting | — | None. |

The plan row UI8 lists one item as not done: the cooker's widget-registration list.

### 1.2 Document format (Verified)

From `FUSE:Source/FUSE/UI/Doc/include/fuse/ui/doc/document.hpp`: a `.fuseui` file is JSON with `"fuseui": 1`, an optional `"style"` sheet string and a `"root"` node. A node has `"class"`, optional `"id"`, `"classes"` (array), `"style"` (object of CSS properties), widget properties, `"on"` (signal name to action name) and `"children"`. Strings may hold `{expr}` (optionally `{expr:N}` for N decimals); `{=name}` is a two-way binding to one model variable. Expressions: numbers, dotted names, `+ - * / %`, unary `-`, comparisons, `&& || !`, parentheses. They compile to a stack bytecode. Binding targets are text, value, checked, disabled, visible, min and max. `Document::reload` keeps nodes whose id and class did not change. A cooked binary form exists (`cook.hpp`, tool `fuse_ui_cook`).

## 2. Scope

In scope: canvas editing of `.fuseui` documents with live FUSE UI rendering; palette of all widgets; outline; typed property inspector; layout tools for all six layout models; CSS editing with tokens; binding editor with a mock view model; state, breakpoint, theme, density, locale and right-to-left previews; accessibility inspector; animation editor; component library; golden-image export; design linting; exact round trip.

Out of scope:

| Item | Owner |
|---|---|
| Writing the ForgeScript behind the UI, debugging, building, packaging | DV01 IDE |
| Version history of documents | DV03 Version Control |
| Icon and illustration drawing | OF08 Draw |
| Raster asset editing | CR01 Image Editor |
| Game HUD and world-panel authoring inside a 3D scene | CR04 Studio |
| Font management | UT03 Fonts and Characters |
| Running assistive technology against the design (the designer inspects the tree; the tools themselves are AX01, AX02) | AX01, AX02 |
| Changing the system design tokens | SY01 Settings (appearance); the token source file is owned by the platform |

## 3. Users and top tasks

Personas:

- **App author**: builds a window quickly, binds it, hands back to the IDE.
- **Designer**: cares about spacing, tokens, states and themes; does not write script.
- **Accessibility reviewer**: checks names, roles, contrast and focus order.
- **Translator or localisation tester**: checks long strings and right-to-left layouts.
- **Test author**: exports golden images for the UI test suite.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a `.fuseui` and see it rendered exactly as at run time | 1 |
| T2 | Add a widget to a container at a chosen position | 1 drag |
| T3 | Change a property of the selection | 2 (click field, type) |
| T4 | Switch a container from box to grid and place children in cells | 3 |
| T5 | Replace a hard-coded colour with a token | 2 |
| T6 | Bind a label to a model value and see it change | 3 |
| T7 | See hover, pressed, focus, checked and disabled states of a control | 1 (state strip) |
| T8 | Preview at narrow, medium and wide breakpoints side by side | 1 |
| T9 | Switch theme, density, locale, right-to-left | 1 each |
| T10 | Find every control without an accessible name | 1 (lint panel) |
| T11 | Fix the tab order | drag in the focus-order overlay |
| T12 | Turn a selection into a reusable component | 2 |
| T13 | Export golden images for all variants | 2 |
| T14 | Save and get a file whose only differences are the edits made | 1 (`Ctrl+S`) |

## 4. Features

Priority: M must, S should, C could. Phase 1, 2, 3 as in section 12.

### 4.1 Documents and round trip

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F001 | Open | `Ctrl+O` through the file portal; `open` and `edit` intents for `.fuseui`; several documents as tabs. | M | 1 |
| DV02-F002 | New | From templates: empty panel, window with header, sidebar and content, three-pane, dialog, wizard page, settings page, utility. Templates follow DESIGN_SYSTEM.md sections 3 and 4. | M | 1 |
| DV02-F003 | Save, save as, save a copy | Atomic write (temporary sibling, `fsync`, rename). | M | 1 |
| DV02-F004 | Round-trip guarantee, level 1 (structure) | Load then save with no edits gives a document equal as JSON values, with member order kept, unknown keys kept and `x-*` keys kept. This is what the prototype's gate checks. | M | 1 |
| DV02-F005 | Round-trip guarantee, level 2 (bytes) | Load then save with no edits gives the identical bytes: original indentation, line endings, number spelling (`1.0` stays `1.0`), string escapes, trailing newline and key order are recorded in a lossless concrete syntax tree. An edit rewrites only the smallest enclosing member; new text follows the file's detected style. | M | 1 |
| DV02-F006 | Unknown content is kept | Unknown widget classes render as a labelled placeholder box and stay in the file; unknown properties appear under "Other" in the inspector as raw JSON. | M | 1 |
| DV02-F007 | Editor metadata | Designer-only data (guides, locked and hidden flags, notes, component links, artboard sizes) is stored either in `x-designer` keys in the document or in a sidecar `<name>.fuseui.design` file; the user chooses per project. Default: sidecar, so shipped documents carry nothing extra. | M | 1 |
| DV02-F008 | Format version | Reads `"fuseui": 1`. A newer version opens read-only with a banner. Migration between versions is offered when FUSE-WP-64 defines it; the original is kept as `.bak`. | M | 2 |
| DV02-F009 | Source view | A split showing the JSON in the shared code editor; edits either side update the other; invalid JSON keeps the last good canvas with an error strip. | M | 1 |
| DV02-F010 | Diff before save | "Review Changes" shows the text diff between the file on disk and what would be written. | S | 2 |
| DV02-F011 | External change | Watches the handle. Unmodified documents reload silently (keeping selection by id); modified ones offer Reload, Keep mine, Compare. | M | 1 |
| DV02-F012 | Autosave and recovery | Unsaved state is journalled to the app data directory every 2 s of idle; restored after a crash. | M | 1 |
| DV02-F013 | Project context | Opening a document inside an app bundle folder (granted through the portal) finds its `style/*.css`, `locale/*.tbl`, `assets/` and sibling documents. Without a folder grant the document opens alone and missing files are listed. | M | 1 |
| DV02-F014 | Cooked check | "Validate for runtime" runs the document through the loader and the cooker in the render helper and lists errors with node paths (`DocError.path`). | S | 2 |
| DV02-F015 | Open in IDE | Hands the document to DV01 at the selected node's source line. | S | 2 |
| DV02-F016 | Recent documents | Last 30, with thumbnails from the last render. | S | 1 |
| DV02-F017 | Untrusted documents | A document with an "untrusted" provenance tag opens with external assets and style files not loaded until the user allows them. Documents contain no code, so rendering is safe; this limits tracking through asset paths. | S | 3 |

### 4.2 Canvas

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F020 | Live rendering | The canvas shows frames rendered by FUSE UI itself from the document, in a render helper process. What is drawn is what loads at run time. | M | 1 |
| DV02-F021 | Design mode and interact mode | Design mode: clicks select, widgets do not operate. Interact mode (`Ctrl+I`): the document behaves as at run time against the mock model; actions are logged, not executed. | M | 1 |
| DV02-F022 | Selection | Click, `Ctrl+click` toggle, `Shift+click` range among siblings, rubber band, `Alt+click` cycles through the stack under the pointer, `Esc` selects the parent, `Tab`/`Shift+Tab` next and previous sibling. | M | 1 |
| DV02-F023 | Selection overlay | Outline, eight resize handles, the node's class and id tag, size readout, padding (green hatch) and margin (orange hatch) overlays, gap markers between children. Drawn by the app over the frame, never by adding classes to the document. | M | 1 |
| DV02-F024 | Move within layout | Dragging a node shows an insertion bar at the target child slot of the container under the pointer; drop reparents as one undo step. | M | 1 |
| DV02-F025 | Resize | Handles set `width`, `height` or flex basis depending on the parent layout; `Shift` keeps aspect; `Alt` resizes from centre; double click on a handle clears the size (auto). | M | 1 |
| DV02-F026 | Snapping | To the spacing scale (2, 4, 8, 12, 16, 24, 32, 48), sibling edges and centres, container padding box, guides. `Ctrl` while dragging turns snapping off. | S | 2 |
| DV02-F027 | Distance measurement | With a selection, hovering another node with `Alt` shows the distances between them in px and as the nearest token. | S | 2 |
| DV02-F028 | Zoom and pan | 10% to 800%; `Ctrl+wheel`, `Ctrl++/-/0`, `Ctrl+1` 100%, `Ctrl+2` fit, `Ctrl+3` fit selection; space-drag or middle-drag pans. Rendering re-rasterises at the zoom scale up to 400%; above that pixels are shown. | M | 1 |
| DV02-F029 | Pixel grid and inspection | Above 600% a pixel grid appears; the status bar shows the colour under the pointer. | C | 3 |
| DV02-F030 | Artboards | One document can be shown on several artboards at once, each with its own size, theme, density, scale, locale and state overrides. Edits apply to the one document. | M | 2 |
| DV02-F031 | Rulers and guides | Rulers in px at the design scale; drag guides from rulers; guides are designer metadata. | S | 2 |
| DV02-F032 | Layout overlay | Toggle to draw every container's layout model: box direction arrows, grid lines and track sizes, anchor lines, flow wrap lines. | S | 2 |
| DV02-F033 | Outline mode | Draws only node borders and ids, to find invisible and overlapping nodes. | S | 2 |
| DV02-F034 | Keyboard nudging | Arrows move among siblings (reorder) in flow layouts and move by 1 px (`Shift` 8 px) in absolute and anchor layouts. | M | 1 |
| DV02-F035 | Copy, cut, paste, duplicate | Clipboard carries the node subtree as `.fuseui` JSON text plus a PNG of its render. Paste gives fresh unique ids and keeps bindings. `Ctrl+D` duplicates. | M | 1 |
| DV02-F036 | Paste style, paste size | Paste only the `style` object or only the size properties onto the selection. | S | 2 |
| DV02-F037 | Wrap and unwrap | Wrap the selection in a panel, scroll area, tab page or splitter; unwrap moves children up and deletes the container. | M | 1 |
| DV02-F038 | Change class | Replace a node's class (for example `label` to `link`) keeping compatible properties and listing the ones dropped. | S | 2 |
| DV02-F039 | Lock and hide | Lock a node against selection on the canvas; hide it in the designer only. Both are metadata; a hidden node still ships. | S | 2 |
| DV02-F040 | Overflow and clipping markers | Nodes whose content overflows or that are clipped by a parent get a marker; text that is elided gets an ellipsis marker. | S | 2 |
| DV02-F041 | Popups and overlays | Menus, tooltips, combo popups and dialogs declared in the document can be pinned open on the canvas for editing. | S | 2 |
| DV02-F042 | Notes | Sticky notes anchored to a node; metadata only; listed in a Notes panel. | C | 3 |

### 4.3 Palette

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F050 | All widgets | Every widget of PLATFORM.md V3, grouped: **Input** (button, toolButton, checkBox, radioButton, toggleSwitch, slider, dial, spinBox, lineEdit, comboBox, dateTimeEdit, colorPicker, fontPicker, calendar); **Display** (label, link, image, separator, progressBar, tooltip, chart, markdownView, videoWidget); **Containers** (panel, scrollArea, splitter, tabBar, tabWidget, dockArea, dockWidget, dialog); **Menus** (menuBar, popupMenu); **Views** (listView, modelListView, treeView, tableView, gridView, propertyGrid); **Editors** (codeEdit, nodeGraph, curveEditor, timeline, viewportWidget, fileDialog). | M | 1 |
| DV02-F051 | Widget metadata | Each class has a descriptor: properties with type and default, signals, whether it takes children, default size, icon, help text. Generated from the widget registration list (the UI8 item not done today) and checked against the runtime at start. | M | 1 |
| DV02-F052 | New platform widgets | Widgets added by FUSE work packages (rich text, canvas, terminal, hex view, header bar, sidebar, status bar, toast, breadcrumb and so on) appear when the registration list has them; no designer release is needed. | S | 2 |
| DV02-F053 | Insert | Drag to canvas or outline; double click or `Enter` inserts into the selected container or after the selected node; type-to-filter (`/`). | M | 1 |
| DV02-F054 | Design system patterns | A second palette tab with ready fragments from DESIGN_SYSTEM.md: header bar, sidebar list, toolbar row, labelled input with hint and error, primary and destructive button pair, empty state, toast, trust badge, secret field, status bar. | M | 2 |
| DV02-F055 | Components tab | Project and user components (4.11). | S | 2 |
| DV02-F056 | Favourites and recent | Pin widgets; the last eight used are at the top. | C | 3 |
| DV02-F057 | Sample data for views | List, tree, table and grid views drop in with mock rows so the layout is visible. | M | 2 |

### 4.4 Outline

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F060 | Tree | Every node: class icon, id or text summary, badges (binding, action, lint, locked, hidden, component instance). | M | 1 |
| DV02-F061 | Drag and drop | Reorder and reparent in the tree; invalid targets (non-containers, own subtree) are refused with a reason tooltip. | M | 1 |
| DV02-F062 | Rename id | `F2`; uniqueness enforced; references in bindings-by-id, focus order, component links and (if a project is open) scripts are listed, and the scripts are offered to DV01 for rename. | M | 1 |
| DV02-F063 | Filter | By class, id, text, "has binding", "has lint issue". | S | 1 |
| DV02-F064 | Multi-select and bulk edit | Select many nodes; the inspector shows shared properties. | M | 1 |
| DV02-F065 | Collapse state | Remembered per document. | S | 1 |
| DV02-F066 | Go to source | Jump from a node to its line in the source view and back. | S | 1 |

### 4.5 Property inspector

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F070 | Typed rows | Editors by property type: bool (toggle), int and float (spin with drag-scrub), string, enum (combo), colour (swatch plus picker), length (number plus unit and token), font, image path, item list, date. | M | 1 |
| DV02-F071 | Sections | Identity (class, id, classes), Content (text, value, items, placeholder), Layout (4.6), Appearance (background, border, radius, shadow, opacity), Text (colour, size, align), Behaviour (disabled, visible, tab index), Signals, Accessibility (name, role, description), Other (unknown keys). | M | 1 |
| DV02-F072 | Value sources | Each row shows where the value comes from: default, style sheet rule (with selector and file), inline `style`, binding. An inline value can be cleared to fall back. | M | 2 |
| DV02-F073 | Token picker | Any colour, length or radius field offers the design tokens first; choosing one writes `var(--name)`. | M | 1 |
| DV02-F074 | Binding toggle | Each bindable row has a `{}` button that opens the binding editor (4.8). Bound rows show the expression and the current mock result. | M | 1 |
| DV02-F075 | Signals | For each signal of the class: the action name (`"on"` map). Completion from the project's `ui::action` ids when a project is open. | M | 1 |
| DV02-F076 | Classes editor | Chips for the node's `classes`; completion from classes defined in the sheets; "new class from inline style" moves the inline `style` into a rule. | M | 2 |
| DV02-F077 | Computed style | Read-only tab listing every computed property and the rule that won, with overridden rules struck through. | S | 2 |
| DV02-F078 | Box model view | Diagram of margin, border, padding and content with editable numbers. | S | 2 |
| DV02-F079 | Raw JSON | A tab showing the node's JSON for direct editing, validated on blur. | S | 1 |
| DV02-F080 | Reset and copy | Per row: reset to default, copy value, copy as CSS. | S | 2 |
| DV02-F081 | Search properties | Filter box over all rows. | S | 1 |
| DV02-F082 | Expression in number fields | Typing `32*2` evaluates on commit. This is a designer convenience and writes the number, not a binding. | C | 3 |

### 4.6 Layout tools

One tool per layout model in `FUSE:docs/ui/README.md` "Styling": `layout` is box, grid, stack, anchor, absolute or flow.

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F090 | Layout switcher | Six buttons on a container; switching keeps children and maps what it can (box order to grid cells row by row, absolute positions to anchors). | M | 1 |
| DV02-F091 | Box | Direction (`flex-direction`), wrap, `gap`, `align-items`, `justify-content` as a 3×3 alignment pad; per child `flex-grow`, `flex-shrink`, `flex-basis`, `align-self`. On canvas: drag the gap, drag a child's grow weight. | M | 1 |
| DV02-F092 | Grid | `grid-template-columns` track editor (fixed px, fraction, auto; add, remove, reorder tracks); children placed by `grid-row`, `grid-column` and spans by dragging across cells; empty cells shown. | M | 2 |
| DV02-F093 | Stack | Children overlap; z-order list; per child alignment in the stack. | M | 2 |
| DV02-F094 | Anchor | Per child `left`, `top`, `right`, `bottom`, `center-x`, `center-y`; canvas shows anchor lines to the parent edges; click a line to pin or free it. | M | 2 |
| DV02-F095 | Absolute | Free position and size; snapping and guides apply; a lint hint says absolute layout does not adapt (4.13). | M | 2 |
| DV02-F096 | Flow | Wrapping inline flow; gap and line alignment; wrap lines shown. | M | 2 |
| DV02-F097 | Size constraints | `min-*`, `max-*`, fixed, fill, hug-content shown as a three-way control per axis. | M | 1 |
| DV02-F098 | Padding and margin | Linked or per-side fields; drag on the canvas overlay; values snap to the spacing scale. | M | 1 |
| DV02-F099 | Align and distribute | Align left, centre, right, top, middle, bottom; distribute; "tidy up" converts a hand-placed absolute group into a box or grid. | S | 2 |
| DV02-F100 | Overflow | `overflow` visible, clip, scroll; scroll containers can be scrolled on the canvas in design mode with `Shift+wheel`. | M | 1 |
| DV02-F101 | Splitter, tab and dock editing | Drag splitter ratios; add, rename, reorder tab pages; arrange dock panels and save the dock layout text. | S | 2 |
| DV02-F102 | Layout explainer | "Why is this node this size?" lists the constraints that decided it (parent layout, grow weights, min and max, content). Needs a layout trace from the render helper. | S | 3 |

### 4.7 CSS and tokens

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F110 | Style sources | The document's inline `"style"` sheet, the app's `style/*.css`, and the system sheet (read-only). Listed in cascade order. | M | 1 |
| DV02-F111 | CSS editor | Shared code editor with the FUSE CSS subset grammar: completion and validation for the selectors (type, `.class`, `#id`, descendant, `>`), pseudo-classes (`:hover :pressed :focus :focus-visible :checked :disabled :active :selected`) and properties the engine supports. Unknown properties are flagged. | M | 1 |
| DV02-F112 | Live apply | Valid edits apply to the canvas within one frame; invalid text keeps the last good sheet. | M | 1 |
| DV02-F113 | Rule list | For the selection: matching rules in cascade order with specificity, source and line; edit in place. | M | 2 |
| DV02-F114 | Selector helper | "Add rule for selection" proposes selectors from the node's class, classes and id, least specific first. | S | 2 |
| DV02-F115 | Token browser | All `:root` tokens grouped as in DESIGN_SYSTEM.md section 2 (colour, type, space, radius, elevation, motion) with dark, light and high-contrast values side by side; search; drag a token onto a node or a field. | M | 1 |
| DV02-F116 | App tokens | Define app-level tokens in the app sheet; warn when one shadows a system token. | S | 2 |
| DV02-F117 | Token usage | For a token: every rule and node using it. For a literal value: every place it appears, with "replace all with token". | S | 2 |
| DV02-F118 | Unused rules | Lists rules that match nothing in any document of the project. | S | 3 |
| DV02-F119 | Colour tools | Picker with contrast readout against the node's effective background; palette of tokens; eyedropper limited to the canvas (no screen capture). | M | 2 |
| DV02-F120 | Gradient and shadow editors | `linear-gradient`, `background-end`, `box-shadow`, `shadow-x/y/blur/color` with visual handles. | S | 2 |
| DV02-F121 | Extract inline styles | Move repeated inline `style` objects into a class rule in the chosen sheet. | S | 2 |

### 4.8 Bindings and mock view model

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F130 | Binding editor | Popover for one property: mode (one-way `{expr}`, two-way `{=name}`, text template with holes), expression field with completion of model names, decimals option (`{expr:N}`), live result, error text from the same compiler the runtime uses. | M | 1 |
| DV02-F131 | Expression checks | Only the verified grammar is accepted (numbers, dotted names, arithmetic, comparisons, `&& || !`, parentheses). Two-way must be a single name. A property that is not a binding target (text, value, checked, disabled, visible, min, max) cannot be bound; the button is disabled with a tooltip. | M | 1 |
| DV02-F132 | Mock model panel | Table of model variables: name, type (number or string, as `doc::Model` has), value editor (number scrub or text). Variables found in bindings are listed automatically. | M | 1 |
| DV02-F133 | Mock data sets | Named sets ("empty", "typical", "long strings", "error") saved in the sidecar file; switch from the toolbar; each artboard can pin a set. | M | 2 |
| DV02-F134 | Import model shape | From the app's view-model ForgeScript module (names and types through DV01's language service) or from a JSON file. | S | 2 |
| DV02-F135 | Two-way feedback | In interact mode, operating a two-way bound control updates the mock variable and every other binding that reads it. | M | 1 |
| DV02-F136 | Binding map | A panel listing every binding: node, property, expression, variables read; filter by variable; unused variables. | S | 2 |
| DV02-F137 | Item model mocks | For list, tree, table and grid views: mock rows from an inline table or a CSV file; column mapping. | S | 2 |
| DV02-F138 | Action log | In interact mode each fired `"on"` action is listed with node and signal arguments. Nothing executes. | M | 1 |
| DV02-F139 | Stress values | One click fills strings with long text, numbers with extremes and lists with 0, 1 and 10 000 rows to show overflow. | S | 3 |

### 4.9 Previews: states, breakpoints, themes, density, locale

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F150 | State strip | For the selection: force `:hover`, `:pressed`, `:focus`, `:focus-visible`, `:checked`, `:disabled`, `:active`, `:selected` singly or combined. Forced through a render-helper override, not by editing the document. | M | 1 |
| DV02-F151 | State sheet | One artboard row showing the selected control in every state at once. | S | 2 |
| DV02-F152 | Breakpoints | Presets from DESIGN_SYSTEM.md section 3: narrow (< 640), medium (640 to 1100), wide (> 1100), plus minimum window 480×320 and custom sizes; drag the artboard edge to resize live. | M | 1 |
| DV02-F153 | Side-by-side breakpoints | One click lays out three artboards (narrow, medium, wide). | M | 2 |
| DV02-F154 | Themes | Dark, light, high contrast; per artboard. | M | 1 |
| DV02-F155 | Density | Compact (row 24), normal (32), touch (44). | M | 1 |
| DV02-F156 | Scale | 100, 125, 150, 200, 300%. | M | 1 |
| DV02-F157 | Locale | Pick a `locale/<lang>.tbl`; strings re-text. Pseudo-locales: accented, 40% longer, bracketed (to spot untranslated text). | M | 2 |
| DV02-F158 | Right-to-left | Mirror layout and switch to a right-to-left sample locale. Needs the FULL text profile (G-03) for correct shaping; with MINIMAL text the designer says so on the artboard. | M | 2 |
| DV02-F159 | Reduced motion, large text | Toggles that set the corresponding system flags for the preview. | S | 2 |
| DV02-F160 | Variant matrix | Choose axes (theme × density × breakpoint × locale × data set) and get a grid of artboards; used for review and for golden export. | S | 2 |
| DV02-F161 | Colour-vision simulation | Protanopia, deuteranopia, tritanopia and greyscale filters on an artboard. | S | 3 |
| DV02-F162 | Text profile switch | Render with MINIMAL or FULL text to see the difference. | C | 3 |

### 4.10 Accessibility inspector and animation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F170 | A11y tree | The tree the runtime would publish (from `UiContext::snapshotA11y` in the render helper): role, name, state, value per node; selection is linked to the canvas. | M | 2 |
| DV02-F171 | Names and roles check | Lists controls with no name, images with no text, duplicate names among siblings, roles that do not fit the class. | M | 2 |
| DV02-F172 | Contrast check | Text against effective background per theme: ratio shown; fails under 4.5:1 for text and 3:1 for controls; high-contrast theme checked against 7:1. | M | 2 |
| DV02-F173 | Focus order overlay | Numbered badges in tab order; drag a badge to reorder (writes tab index); shows focus traps and unreachable controls. | M | 2 |
| DV02-F174 | Target size check | Interactive targets under 24 px (44 px in touch density) are flagged. | M | 2 |
| DV02-F175 | Keyboard walk | Step through focus with `Tab` on the canvas and see the focus ring and the announced name and role as text. | S | 2 |
| DV02-F176 | Screen-reader text preview | The string AX01 would speak for the focused node, from the same tree. | S | 3 |
| DV02-F177 | Animation editor | Transitions per property on state change: duration (design system values 80, 140, 200 ms), easing, delay. A `timeline` with a scrubber plays the transition on the canvas. | S | 3 |
| DV02-F178 | Keyframe animations | Keyframes for opacity, position, size and colour edited on `timeline` and `curveEditor`. Depends on the declarative animation format FUSE defines (not in the document format today; Proposed). | C | 3 |
| DV02-F179 | Motion lint | Durations over 200 ms and animations that ignore reduced motion are flagged. | S | 3 |

### 4.11 Components and fragments

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F190 | Create component | Turn a selection into a named fragment file (`components/<name>.fuseui`) with declared parameters (text, bound names, slots). | S | 2 |
| DV02-F191 | Instances | Because the runtime format has no include today (Verified: no such key in `document.hpp`), an instance is an expanded copy in the document plus an `x-designer` or sidecar link to its source and parameter values. "Update instances" re-expands them and shows a diff. A native include is requested in section 13. | S | 2 |
| DV02-F192 | Detach | Break the link; the nodes become plain. | S | 2 |
| DV02-F193 | Library | Project components, user components (app data directory), and the design-system patterns; each with a thumbnail and description. | S | 2 |
| DV02-F194 | Share a library | Export and import a library as a signed archive through the portal. | C | 3 |
| DV02-F195 | Slots | A component marks child positions callers may fill. | C | 3 |

### 4.12 Export and tests

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F200 | Golden-image export | Renders through the CPU rasteriser (the one the FUSE golden gates use) to PNG for each chosen variant; file names encode the variant (`editor@dark-compact-640-en.png`); a `goldens.json` index lists document hash, variant and image hash. | M | 2 |
| DV02-F201 | Golden compare | Loads existing goldens and shows expected, actual and difference per variant; accept one or all. | S | 2 |
| DV02-F202 | Export image | PNG at 1×, 2×, 3× or SVG-free PDF of an artboard for documentation; with or without the overlay. | S | 2 |
| DV02-F203 | Export specification | A Markdown sheet for hand-off: node tree, sizes, tokens used, strings, bindings, actions. | C | 3 |
| DV02-F204 | Copy as | Copy selection as `.fuseui` JSON, as CSS, or as an image. | S | 1 |
| DV02-F205 | Print | An artboard or the variant matrix through `svc.portal.print`. | C | 3 |
| DV02-F206 | Scripted UI test stub | Generates a ForgeScript test skeleton that finds each interactive node by a11y name and checks it exists. | C | 3 |

### 4.13 Design linting

Rules are checked against DESIGN_SYSTEM.md. Each has an id, severity, a fix where one exists and can be suppressed per node with a reason.

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F210 | Lint panel | Table: severity, rule, node, message, fix. Runs on every edit in the background; badge counts in the status bar and outline. | M | 2 |
| DV02-F211 | `token-colour`, `token-space`, `token-radius`, `token-type` | Literal value where a token exists; sizes off the scale; type sizes outside 11, 12, 13, 15, 18, 24. Fix: replace with the nearest token. | M | 2 |
| DV02-F212 | `one-primary` | More than one primary (filled accent) button in a view; a destructive button as default. | M | 2 |
| DV02-F213 | `input-label` | Input without a label above; hint or error not below. | M | 2 |
| DV02-F214 | `status-not-colour-only` | A state shown by colour with no icon or word. | S | 2 |
| DV02-F215 | `contrast`, `target-size`, `a11y-name`, `focus-order` | The accessibility checks of 4.10 as lint rules. | M | 2 |
| DV02-F216 | `anatomy` | Sidebar not 220 px, inspector not 280 px, minimum window under 480×320, sidebar on the right, status bar not at the bottom. | S | 2 |
| DV02-F217 | `radius-border-elevation` | Radius not 4, 8 or 12 by role; border not 1 px `--line`; shadow on a non-elevated surface. | S | 2 |
| DV02-F218 | `motion` | Duration over 200 ms; no reduced-motion fallback. | S | 3 |
| DV02-F219 | `strings` | Hard-coded user-visible text instead of a locale key; missing key in a locale; text that overflows in the 40%-longer pseudo-locale. | M | 2 |
| DV02-F220 | `breakpoint` | Content clipped or overlapping at narrow width; fixed widths wider than 480. | S | 2 |
| DV02-F221 | `menu-shortcut`, `dialog-verbs` | Menu item without a shortcut label; dialog buttons that are not verbs ("OK", "Yes"). | S | 3 |
| DV02-F222 | `absolute-layout` | Absolute layout used for more than decoration. Hint only. | S | 2 |
| DV02-F223 | Lint profile | Rules on or off and severity per project in a `designlint.json`; shared with DV01's hard-coded value lint. | S | 2 |
| DV02-F224 | Lint from the command line | The same rules as a headless task DV01 can run in its build sandbox, output as SARIF. | S | 3 |

### 4.14 Workbench, history, settings, assistance

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV02-F230 | Undo and redo | Command-based (not whole-document snapshots as in the prototype), unlimited within a 256 MiB cap, grouped by gesture; covers document, sheets, mock data and metadata, each tagged so document-only undo is possible. | M | 1 |
| DV02-F231 | History panel | List of steps with names ("Move button1 into toolbar"); click to travel. | S | 2 |
| DV02-F232 | Command palette | `Ctrl+K` over all actions, widgets (insert), tokens and nodes by id. | M | 1 |
| DV02-F233 | Panels | Dockable: Palette, Outline, Inspector, Styles, Tokens, Model, Lint, A11y, History, Source. Layout saved per window. | M | 1 |
| DV02-F234 | Asset picker | Images and icons from the bundle's `assets/`; shows size and whether a 2× variant exists. | S | 2 |
| DV02-F235 | Local AI: describe to layout | Opt-in (`ai.local`): a prompt produces a `.fuseui` fragment that is validated, linted and shown as a preview to insert or discard. | C | 3 |
| DV02-F236 | Local AI: name and label suggestions | Suggests accessible names for unnamed controls; applied only on accept. | C | 3 |
| DV02-F237 | Read-only review mode | Opens a document with all editing off, for reviewers; inspector, lint and a11y still work. | S | 2 |
| DV02-F238 | Keyboard-only editing | Every canvas operation has a keyboard path: select, insert, move among slots, resize by steps, change layout. | M | 2 |

## 5. GUI

### 5.1 Window map

```
UI Designer
├─ Main window (one per project folder or loose document set)
│  ├─ Header bar, toolbar (mode, layout tools, preview axes, zoom)
│  ├─ Left dock: Palette (Widgets · Patterns · Components) · Outline
│  ├─ Centre: document tabs → canvas with artboards │ optional Source split
│  ├─ Right dock: Inspector (Properties · Computed · JSON) · Styles · Tokens
│  └─ Bottom dock: Model · Lint · A11y · Actions · History
├─ Popovers: binding editor, colour picker, token picker, gradient and shadow editor, grid track editor
├─ Dialogs: New document, Variant matrix, Export goldens, Golden compare, Create component, Review changes, Preferences
└─ Sheets (narrow): inspector, lint
```

### 5.2 Main window

Shown at 1440×900, one artboard.

```
+------------------------------------------------------------------------------------------+
| [=] UI Designer — settings.fuseui     [ Search or run a command  Ctrl+K ]      [▁][▢][✕] | (1)
+------------------------------------------------------------------------------------------+
| [Design|Interact] [▭ box][▦ grid][▣ stack][⚓][⊹ abs][↩ flow] | [dark v][compact v][100%v] |
| [medium 900 v] [en v] [RTL ] [data: typical v]   [▶ States]       Zoom [ 100% v ] [Fit]  | (2)
+--------------------+----------------------------------------------+----------------------+
| PALETTE            | settings.fuseui ×   dialog.fuseui            | INSPECTOR       [F4] |
| [Widgets|Patt|Comp]|     0      200     400     600     800   (4) | slider  #vol         |
| [/ filter____]     |   +--------------------------------------+   | [Props|Computed|JSON]|
| v Input            | 0 | Sound                                |   | v Identity           |
|   button           |   | Volume  ┌─────────●───────┐  70%     |   |  id      [vol     ]  |
|   checkBox         |   |         └─slider #vol─────┘ 320×24   |   |  classes [wide ×][+] |
|   slider           |   | [x] Mute        (5)                  |   | v Content            |
|   lineEdit …       |   |                                      |   |  min     [0   ]      |
| v Display          |   |                      [Cancel] [Save] |   |  max     [1   ]      |
|   label …      (3) |   +--------------------------------------+   |  value {=audio.volume}|
+--------------------+       medium · dark · compact · en           |          = 0.70  (7) |
| OUTLINE            |                                          (6) | v Layout             |
| v panel #root      +----------------------------------------------+  width [fill v]      |
|   label            | MODEL  LINT (2)  A11Y  ACTIONS  HISTORY      |  flex-grow [1]       |
|   v panel #row     | audio.volume  number  [ 0.70 ]───●──     (8) | v Appearance         |
|   > slider #vol {} | busy          number  [ 0    ]               |  background          |
|     label          | user.name     string  [ Odin           ]     |   [■ var(--bg-2) v]  |
+--------------------+----------------------------------------------+----------------------+
| 1 selected · slider #vol · 320×24 at 96,48 · ⚠ 2 lint · saved                 x 412 y 96 | (9)
+------------------------------------------------------------------------------------------+
```

1. Header bar. No `[net]` badge ever lights: the app has no network capability.
2. Toolbar rows (`toolButton`, `comboBox`): mode switch, six layout buttons (enabled for containers), preview axes, zoom.
3. Palette (`tabBar` + `treeView`, 220 px) above Outline (`treeView`), split by a `splitter`.
4. Rulers and canvas: a `viewportWidget` showing the artboard frame from the render helper, with the designer overlay drawn on top.
5. Selection overlay: outline, handles, class and id tag, size readout.
6. Artboard caption with its variant; click to change.
7. Inspector (280 px, `propertyGrid` with typed rows). A bound row shows the expression and its mock result.
8. Bottom dock (`tabWidget`, 200 px): mock model `tableView` with inline editors.
9. Status bar: selection summary, lint count, save state, pointer position in design px.

States: empty (no document: New from template tiles, Open, recent thumbnails); loading (artboard skeleton, under 100 ms normally); error (document does not load: source view with the error line, canvas shows the last good render dimmed); offline (always); locked (not applicable); read-only (review mode or read-only handle: lock in the tab, palette disabled, inspector fields disabled but selectable).

### 5.3 Other views and dialogs

#### Binding editor popover (420×300)

```
+--------------------------------------------------+
| Bind slider #vol · value                          |
| (•) Two-way {=name}  ( ) One-way {expr}  ( ) Text |
| Variable  [audio.volume            v]             |
| ------------------------------------------------ |
| Result now:  0.70   (data set: typical)           |
| Reads: audio.volume                               |
| ✓ compiles                                        |
|                         [Remove binding] [Done]   |
+--------------------------------------------------+
```

One-way mode shows an expression `lineEdit` with completion and a decimals `spinBox`. Text mode shows a template field with holes highlighted. Errors appear under the field in `--danger` with the compiler's message. States: valid, error, not bindable (never opened: the `{}` button is disabled).

#### Grid layout editing (canvas detail)

```
        [ 120px ] [   1fr   ] [  auto  ] [+]        (1) track headers
      +---------+-----------+----------+
 [a]  | Name    | [______________________]  (2) spans columns 2–3
      +---------+-----------+----------+
 [a]  | Email   | [_________]| [Verify] |
      +---------+-----------+----------+
 [+]  | · · · · | · · · · · | · · · ·  |   (3) empty cells
      +---------+-----------+----------+
```

1. Click a header to edit the track (fixed, fraction, auto); drag the divider to resize; drag a header to reorder. 2. Drag a child's edge across cells to span. 3. Drop targets. Rows are implicit; the row gutter shows their computed heights.

#### Variant matrix (dialog 800×560, then canvas)

```
+----------------------------------------------------------------------+
| Variant matrix                                                        |
| Axes                                                                  |
|  Theme      [x] dark  [x] light  [x] high contrast                    |
|  Density    [x] compact  [ ] normal  [ ] touch                        |
|  Width      [x] 480  [x] 900  [x] 1440                                |
|  Locale     [x] en  [ ] ar (RTL)  [x] pseudo-long                     |
|  Data set   [x] typical  [x] empty  [ ] error                         |
|  Scale      [x] 100%  [ ] 200%                                        |
| 3 × 1 × 3 × 2 × 2 × 1 = 36 artboards                                  |
|              [Cancel]  [Show on canvas]  [Export goldens…]            |
+----------------------------------------------------------------------+
```

`checkBox` groups. Over 64 artboards the canvas shows them as lazily rendered tiles. States: too many (over 500: export only).

#### Tokens panel with usage (right dock)

```
+----------------------------------------------+
| TOKENS  [search____]  [Colour v]              |
| Name        Dark      Light     HC     Uses   |
| --bg-0      ■ 0E1013  ■ F5F6F8  ■ …     3     |
| --accent    ■ 4C8DFF  ■ 1F5FD6  ■ …    11     |
|>--danger    ■ E5534B  ■ C4302B  ■ …     2     |
| ------------------------------------------   |
| Uses of --danger                              |
|  app.css:14  .delete { border-color }         |
|  node #del   style.color                      |
| Literals close to this token: #E5534B ×1 [Fix]|
+----------------------------------------------+
```

`tableView` with swatch delegates; drag a row onto a node or a field.

#### Accessibility inspector (bottom dock + canvas overlay)

```
+--------------------------------------+---------------------------------------+
| A11Y TREE                            | CHECKS                                |
| v window "Settings"                  | ✕ a11y-name   button #b3 has no name  |
|   v group "Sound"                    | ✕ contrast    label #hint 3.1:1 (dark)|
|     slider "Volume" value 70%        | ⚠ target-size toolButton #x 20×20     |
|   > check box "Mute" unchecked       | ✓ focus-order 7 stops, no trap        |
|     button (no name)            ✕    | [Show focus order] [Keyboard walk]    |
+--------------------------------------+---------------------------------------+
 canvas overlay:  ①Volume  ②Mute  ③(no name)  ④Cancel  ⑤Save      (1)
```

1. Numbered focus badges; drag to reorder. Contrast rows list each failing theme.

#### Export goldens (dialog 720×480)

```
+----------------------------------------------------------------------+
| Export golden images                                                  |
| Document   settings.fuseui  (hash b3:41c0…)                           |
| Variants   36 from the matrix            [Edit matrix…]               |
| Renderer   CPU rasteriser, text profile [FULL v]                      |
| Folder     [tests/golden/settings/                    ] [Pick…]       |
| Existing   30 match · 4 differ · 2 new                [Compare…]      |
| [x] Write goldens.json index   [ ] Overwrite differing images         |
|                                   [Cancel]  [Export 2 new]            |
+----------------------------------------------------------------------+
```

Differing images are never overwritten without the tick or a per-image accept in Compare.

#### Lint panel (bottom dock)

```
+------------------------------------------------------------------------------+
| LINT  ✕3 ⚠5  [severity v] [rule v] [ ] show suppressed                        |
| ✕ one-primary    #save, #apply   Two primary buttons in one view     [—]     |
| ✕ a11y-name      #b3             Button has no accessible name       [Add…]  |
| ⚠ token-colour   #hint           #6F7783 → var(--fg-2)               [Fix]   |
| ⚠ strings        #title          Hard-coded "Sound" → key            [Extract]|
| ⚠ breakpoint     #row            Clipped at 480 px                   [Show]  |
+------------------------------------------------------------------------------+
```

`tableView`; double click selects the node; "Fix all of this rule" in the context menu.

### 5.4 Menus

| Menu | Items |
|---|---|
| File | New… (`Ctrl+N`) · Open… (`Ctrl+O`) · Open Project Folder… · Open Recent ▸ · Save (`Ctrl+S`) · Save As… (`Ctrl+Shift+S`) · Save a Copy… · Review Changes… · Revert · Validate for Runtime · Export ▸ Golden Images…, Image…, Specification… · Print… (`Ctrl+P`) · Open in IDE · Close (`Ctrl+W`) · Quit (`Ctrl+Q`) |
| Edit | Undo (`Ctrl+Z`) · Redo (`Ctrl+Shift+Z`) · Cut, Copy, Paste (`Ctrl+X/C/V`) · Paste Style (`Ctrl+Alt+V`) · Paste Size · Duplicate (`Ctrl+D`) · Delete (`Delete`) · Select All (`Ctrl+A`) · Select Parent (`Esc`) · Select Children (`Enter`) · Find Node… (`Ctrl+F`) · Rename Id (`F2`) · Preferences (`Ctrl+,`) |
| Insert | Widget… (`/`) · Pattern ▸ · Component ▸ · Wrap In ▸ Panel (`Ctrl+G`), Scroll Area, Tab Page, Splitter · Unwrap (`Ctrl+Shift+G`) |
| Object | Layout ▸ Box (`Alt+1`), Grid (`Alt+2`), Stack (`Alt+3`), Anchor (`Alt+4`), Absolute (`Alt+5`), Flow (`Alt+6`) · Align ▸ · Distribute ▸ · Tidy Up · Move Up, Down (`Ctrl+Up/Down`) · Move Out (`Ctrl+Left`) · Move Into Next (`Ctrl+Right`) · Change Class… · Bind Property… (`Ctrl+B`) · Create Component… · Update Instances · Detach · Lock (`Ctrl+L`) · Hide in Designer (`Ctrl+Shift+H`) |
| View | Command Palette (`Ctrl+K`) · Design, Interact (`Ctrl+I`) · Source Split (`Ctrl+E`) · Layout Overlay (`Ctrl+Shift+L`) · Outline Mode (`Ctrl+Shift+O`) · Rulers (`Ctrl+R`) · Guides (`Ctrl+;`) · Snapping (`Ctrl+Shift+;`) · Zoom In, Out, Reset (`Ctrl++/-/0`) · Actual Size (`Ctrl+1`) · Fit (`Ctrl+2`) · Fit Selection (`Ctrl+3`) · Sidebar (`F9`) · Inspector (`F4`) · Panels ▸ · Full Screen (`F11`) |
| Preview | Theme ▸ Dark, Light, High Contrast (`Ctrl+Shift+T` cycles) · Density ▸ · Scale ▸ · Breakpoint ▸ Narrow, Medium, Wide, Minimum, Custom… · Three Breakpoints · Locale ▸ · Right-to-Left (`Ctrl+Shift+R`) · Data Set ▸ · States… (`Ctrl+Alt+S`) · Variant Matrix… (`Ctrl+M`) · Reduced Motion · Colour-Vision ▸ |
| Check | Run Lint (`F8` next issue, `Shift+F8` previous) · Accessibility Checks · Focus Order (`Ctrl+Shift+F`) · Keyboard Walk · Golden Compare… · Lint Profile… |
| Help | Designer Handbook (`F1`) · Design System · Widget Reference · Keyboard Shortcuts · About |

### 5.5 Keyboard and pointer

- Canvas: arrows nudge or reorder (F034); `Shift+arrows` resize by the spacing step; `Ctrl+arrows` move across the tree; `Alt+drag` duplicates; space-drag pans; `Ctrl+wheel` zooms at the pointer; double click enters a container or edits text in place; `Enter` on a text widget edits its text.
- Canvas context menu: Cut, Copy, Paste, Duplicate, Delete, Wrap In ▸, Unwrap, Layout ▸, Bind ▸ (bindable properties), Add Rule for Selection, Create Component, Go to Source, Lock, Hide, Suppress Lint ▸.
- Outline context menu: the same plus Rename Id, Expand All, Collapse All.
- Inspector row context menu: Reset, Copy Value, Copy as CSS, Bind…, Use Token…, Show Winning Rule.
- Drag and drop: palette to canvas or outline; token to node or field; image file from Files onto an `image` node (copies into `assets/` after a confirm when a project folder is granted); node between document tabs (copy); CSS file onto the Styles panel (adds to the project sheet list).
- Touch and pen: two-finger pan and pinch zoom on the canvas; long press for the context menu.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Template tiles, Open, recent thumbnails. |
| Loading | Skeleton artboard; panels usable. |
| Error | Invalid JSON or load error: error strip with `path`, line and column from `DocError`; last good render dimmed; source view focused on the error. |
| Render helper crashed | Canvas shows "Renderer restarted"; the document model is unaffected; auto-restart with back-off. |
| Offline | Not applicable; the app never uses the network. |
| Read-only | Review mode, read-only handle or newer format version: edits disabled, reason in the banner. |
| Missing project files | Banner listing missing sheets, locale tables or assets with "Grant folder…". |
| Narrow (< 640 px) | Left dock becomes an overlay with palette and outline as tabs; inspector becomes a sheet; bottom dock collapses to its tab strip. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Canvas | `viewportWidget` (frames from the render helper; overlay drawn by the app through the widget's paint callback) |
| Rulers | custom drawing in the same viewport (DV02-GAP-1) |
| Palette, outline, a11y tree | `treeView`, `tabBar` |
| Inspector | `propertyGrid` extended with typed rows (bool, int, float, string, enum, colour are Verified kinds; length-with-unit, token, binding and font rows are DV02-GAP-2) |
| Styles, source | `codeEdit` (FUSE-WP-37 for the full engine) |
| Tokens, model, lint, binding map | `tableView` with cell delegates |
| Colour, gradient | `colorPicker`; gradient editor is DV02-GAP-3 |
| Animation | `timeline`, `curveEditor` |
| Docks | `dockArea`, `dockWidget`, `splitter`, `tabWidget` |
| Dialogs, menus | `dialog`, `menuBar`, `popupMenu`, `tooltip`, `checkBox`, `comboBox`, `spinBox`, `lineEdit`, `button`, `toolButton` |
| Golden compare | image compare view (DV02-GAP-4; shared need with DV01 and DV03 image diff) |

| ID | Missing | Note |
|---|---|---|
| DV02-GAP-1 | Ruler and guide strip | Small; can live in the app |
| DV02-GAP-2 | Property grid row kinds: length with unit, token reference, binding, font, image path, item list | Extension of `propertyGrid` |
| DV02-GAP-3 | Gradient and shadow editor | App-level composite |
| DV02-GAP-4 | Image compare (side by side, swipe, difference) | Shared widget wanted |
| DV02-GAP-5 | Render-helper hooks in FUSE: forced pseudo-class state per node, layout trace, hit-test to node id, widget registration list with property metadata | Requirement on FUSE (FUSE-WP-63) |
| DV02-GAP-6 | Popover anchored to a widget | Listed as missing in FUSE_BUILD_PLAN.md |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| Designer shell | ForgeScript + FUSE UI | app host | Windows, docks, commands, panels, overlay drawing, settings |
| Document model | ForgeScript | app host | Lossless concrete syntax tree of the JSON, node tree view over it, command-based undo, id index |
| Style model | ForgeScript | app host | CSS subset parser and printer (lossless), rule index, token table |
| Mock model and data sets | ForgeScript | app host | Variables, data sets, item-model mocks |
| Lint engine | ForgeScript | app host | Rules of 4.13 over the document model plus facts returned by the render helper (computed sizes, colours, a11y tree) |
| Render helper `ui-render` | C++23 (FUSE UI headless: `fuse_ui_doc`, widgets, CPU rasteriser) | bundle helper, no capabilities; receives read-only handles for assets and fonts | Loads the preview text, applies variant settings and forced states, renders frames to shared memory, answers hit-test, geometry, computed-style, a11y-snapshot and layout-trace queries, writes golden PNGs to a passed descriptor |
| Golden codec | part of `ui-render` | same | PNG encode (own code or FUSE's existing writer) |
| Lint CLI | ForgeScript entry in the bundle | started by DV01's build sandbox | Headless lint with SARIF output |

```
 user ─ FUSE UI ─ [ app host: shell · document model · style model · mock model · lint ]
                         │  session bus: batched edits, variant settings, queries
                         ▼
                  [ ui-render helper (no capabilities) ]  one per window; N artboard contexts
                         │  shared-memory frames, JSON-free typed replies
                         ▼
                  viewportWidget in the canvas
```

The document is rendered in a helper, not in the app's own UI context, for three reasons: a broken document cannot break the designer; each artboard needs its own theme, scale and density; and the helper can force states and collect traces without touching the designer's own tree. The prototype renders in-process (Verified); the app changes that.

### 6.2 Data model

Document (in memory):

| Entity | Fields |
|---|---|
| `CstNode` | `kind: object\|array\|string\|number\|bool\|null`, `raw: string` (exact source text of scalars), `leading_ws`, `trailing_ws`, `members[]` (key raw text, colon spacing, value), `dirty: bool` |
| `DesignNode` | `cst: ref<CstNode>`, `cls: string`, `id: string`, `children[]`, `parent`, `meta: NodeMeta` |
| `NodeMeta` | `locked: bool`, `hidden: bool`, `notes[]`, `component: {source: string, params: map}?`, `lint_suppress: {rule: reason}` |
| `Command` | `name: string`, `do`, `undo`, `scope: document\|style\|model\|meta`, `ts: int` |
| `Variant` | `theme`, `density`, `scale: int`, `width: int`, `height: int`, `locale: string`, `rtl: bool`, `data_set: string`, `forced_states: map<id, state[]>` |
| `MockVar` | `name: string`, `is_string: bool`, `number: float`, `text: string` |

Sidecar `<name>.fuseui.design` (JSON, `schema: 1`): `guides[]`, `artboards[]` (list of `Variant` plus position), `data_sets{name: MockVar[]}`, `node_meta{id: NodeMeta}`, `item_mocks{id: rows}`, `matrix` (last axes).

Project file `designlint.json`: `{schema, rules: {id: "error"|"warning"|"hint"|"off"}, anatomy: {...}}`.

`goldens.json`: `[{document: path, doc_hash: blake3, variant: Variant, image: path, image_hash: blake3, renderer: {text_profile, fuse_version}}]`.

App store (`~/.local/share/org.pbsd.ui-designer/`): `recent.db` (SQLite: `recent(path_ref, grant_id, opened_at, thumb BLOB)`), `components/` (user library), `journal/` (autosave), `layouts/`. Schema version in `user_version`; forward-only migration with backup.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `.fuseui` JSON (version 1) | yes | yes | own lossless JSON parser and printer in ForgeScript for editing; `fuse_ui_doc` for rendering | app host (size cap 16 MiB, depth cap 256) and `ui-render` |
| Cooked `.fuseui` | no | validate only | `fuse_ui_cook` code | `ui-render` |
| CSS subset | yes | yes | own lossless parser in ForgeScript; FUSE `StyleEngine` for rendering | app host and `ui-render` |
| Locale tables `.tbl` | yes | yes (extract string) | own | app host |
| Sidecar, lint profile, goldens index (JSON) | yes | yes | own | app host |
| PNG | goldens for compare | goldens, exports | FUSE's PNG code or own | `ui-render` |
| Images referenced by documents | yes | no | platform image decoders through `svc.thumbnail`-class helpers | decoder sandbox, never the app host |
| Fonts | yes | no | FUSE text | `ui-render` |
| CSV (item mocks) | yes | no | own | app host |
| SARIF 2.1.0 | no | yes | own | app host |
| Render protocol (session bus schema `org.pbsd.ui-designer.render/1`) | — | — | schema compiler (G-08) | both ends |

### 6.4 Integration

- Intents handled: `open`, `edit`, `view` for `application/x-fuseui+json`; `new`. Emitted: `edit` to DV01 (source), `open` to UT05 (widget reference), `share`, `print`.
- Services: `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.a11y`, `svc.model` (optional).
- IPC offered: `org.pbsd.ui-designer.Preview` so DV01 can embed the same render helper for its preview panel (one implementation, two apps).
- DV01: rename of ids and action names across scripts; view-model shape import; lint CLI as a build task. DV03: none direct (files are plain text and diff well; DV03's structured `.fuseui` diff uses this app's node-path logic as a library module).
- Backing repo. Verified: everything in section 1.1 and 1.2. Proposed: everything else. The prototype's `DesignDocument` is a reference for behaviour, not reused code, because the app's model is ForgeScript and lossless at byte level.

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Settings, recents, user component library, autosave journal |
| `fs.portal` | Documents and, optionally, the project folder the user grants |
| `proc.helper:ui-render` | The render helper |
| `net.none` | The app has no use for the network |
| `svc.settings`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.a11y` | As in 6.4 |
| `svc.model`, `ai.local` | Optional local suggestions; off by default |

Assets: the user's documents and project files. Adversaries: a hostile `.fuseui`, CSS, image or font file; a hostile component library.

| Threat | Mitigation |
|---|---|
| Document crafted to crash or exhaust the renderer (deep nesting, huge grids, 10 million rows) | Rendered in `ui-render` with no capabilities, memory and time limits; the designer survives a helper crash |
| Hostile image or font | Decoded in decoder sandboxes or in `ui-render`, never in the app host |
| Document refers to paths outside the project (`"image": "../../secret"`) | Assets resolve only beneath the granted project directory handle; other paths show a placeholder and a lint error |
| Actions in a document run code | They cannot: actions are names. Interact mode logs them. No ForgeScript from the project is ever executed by the designer |
| Binding expressions | Compiled by the verified bytecode compiler; no calls, no loops, bounded stack |
| Component library archive | Signature shown; contents are documents and CSS only; extraction through the safe archive path (UT01 rules) |
| Clipboard leaks | Copies carry only the selected subtree; no project paths |
| Lossy save corrupting hand-written files | Byte-level round trip (F005) with a differential fuzz test |

Stored: section 6.2. Leaves the machine: nothing.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `metadata.location` | enum | `sidecar` | `sidecar` or `inline` (`x-designer` keys) |
| `save.style` | enum | `preserve` | `preserve` file formatting or `normalise` (2-space, sorted as written) |
| `canvas.snap` | bool | true | Snapping |
| `canvas.snap_scale` | bool | true | Snap to the spacing scale |
| `canvas.show_rulers` | bool | true | Rulers |
| `canvas.overlay_padding_margin` | bool | true | Hatch overlays on selection |
| `canvas.max_live_artboards` | int | 64 | Above this, tiles render lazily |
| `preview.theme` | enum | `dark` | Default artboard theme |
| `preview.density` | enum | `compact` | Default density |
| `preview.scale` | int | 100 | Default scale |
| `preview.text_profile` | enum | `FULL` | `MINIMAL` or `FULL` (falls back if FULL is not built) |
| `render.mem_bytes` | int | 1 GiB | Render helper memory cap |
| `render.frame_timeout_ms` | int | 2000 | A frame slower than this is abandoned with an error |
| `lint.live` | bool | true | Lint while editing |
| `lint.profile_path` | string | `designlint.json` | Project lint profile |
| `goldens.folder` | string | `tests/golden` | Default export folder (relative to the project) |
| `autosave.interval_ms` | int | 2000 | Journal interval |
| `undo.mem_bytes` | int | 256 MiB | Undo cap |
| `ai.enabled` | bool | false | Local suggestions |

## 9. Accessibility and languages

- The canvas is in the a11y tree as a tree of the document's nodes (class, id, bounds), so a screen-reader user can move through the design, hear each node and edit it through the inspector.
- Every canvas gesture has a keyboard path (F238). Resize and move announce the new size and slot.
- Overlays use shape and pattern as well as colour (hatched padding and margin, numbered focus badges).
- The designer's own UI follows the system locale; the previewed locale is separate and shown on each artboard caption.
- Right-to-left designer UI mirrors the docks; rulers keep their numeric direction and state it.

## 10. Performance budgets

| Metric | Budget |
|---|---|
| Cold start to first frame | ≤ 400 ms |
| Open a 2 000-node document to first render | ≤ 150 ms |
| Property edit to updated artboard | ≤ 16 ms for one artboard; ≤ 100 ms for 9 |
| Drag feedback (insertion bar, handles) | every frame at 60 Hz with a 5 000-node document |
| Largest document | 50 000 nodes; outline and inspector stay responsive (virtualised) |
| Variant matrix | 36 artboards rendered in ≤ 1 s; 500 exported in ≤ 30 s |
| Memory | app host ≤ 250 MiB; render helper ≤ 1 GiB cap |
| Idle | zero frames, zero wake-ups; the render helper sleeps |
| Lint pass on a 2 000-node document | ≤ 50 ms incremental |

## 11. Testing and proof

Test plan:

- Unit (ForgeScript): lossless JSON and CSS parsers and printers; every command and its undo; layout conversion; binding grammar checks; each lint rule with positive and negative fixtures.
- Round trip: every `.fuseui` in the FUSE test data and every document in the suite's bundles must save byte-identical with no edits; a single-property edit must change only that member's bytes.
- Differential: binding compile results equal the runtime's (`Expr::compile`) on a generated corpus; lint contrast numbers equal an independent calculation; designer render equals the FUSE golden gates' render for the same inputs.
- Golden renders of the designer's own UI in three themes and two scales.
- Scripted UI through the a11y tree: the 14 top tasks, keyboard only.
- Fuzz: lossless JSON parser, CSS parser, sidecar loader, render protocol decoder, `ui-render` document loading.
- Robustness: kill the render helper during every operation; the document must be intact.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Lossless JSON printer | `print(parse(x)) == x` for all valid inputs; an edit changes only the target member | T1 | PRISM proof over the C++23 reference implementation of the same algorithm, plus differential fuzz against the ForgeScript one | The shipped code is ForgeScript; the proof covers the reference, the link is by testing |
| Render protocol decoder (in `ui-render`) | No runtime errors on any input | T2 | PRISM + fuzz | — |
| `ui-render` | Safe subset; bounded memory | T3 | Subset plugin, sanitizers, fuzz | Tier of FUSE UI itself |
| Asset path resolver | Never opens outside the granted directory | T2 | PRISM on the resolver; negative tests | — |
| Lint engine, document model (ForgeScript) | Engine safety only | T4 | Tests | Interpreter not modelled |

Nothing here has been run.

## 12. Build plan

Phase 1: usable core.

| ID | Work item | Depends on |
|---|---|---|
| DV02-W01 | Shell: docks, tabs, command palette, settings | G-01, G-02, G-07 |
| DV02-W02 | Lossless JSON model, node tree, command undo, autosave journal | — |
| DV02-W03 | Render helper and render protocol; canvas with one artboard | G-08, FORGE-WP-28.1, FUSE doc library on HardenedBSD (G-18) |
| DV02-W04 | Selection overlay, move, resize, insert, wrap, clipboard | DV02-W02, DV02-W03, DV02-GAP-5 (hit-test) |
| DV02-W05 | Widget metadata from the registration list; full palette | FUSE-WP-63, DV02-GAP-5 |
| DV02-W06 | Typed inspector with token picker | DV02-GAP-2, DV02-W05 |
| DV02-W07 | Outline with drag, rename, filter | DV02-W02 |
| DV02-W08 | Box layout tools, size constraints, padding and margin | DV02-W04 |
| DV02-W09 | CSS editor, style sources, token browser | FUSE-WP-37, DV02-W03 |
| DV02-W10 | Binding editor, mock model, interact mode, action log | DV02-W03 |
| DV02-W11 | Previews: states, theme, density, scale, breakpoints | DV02-GAP-5 (forced states) |
| DV02-W12 | Source split view, external change handling | FUSE-WP-37 |

Phase 2: full feature.

| ID | Work item | Depends on |
|---|---|---|
| DV02-W20 | Grid, stack, anchor, absolute, flow tools; layout overlay; align and distribute | DV02-W08 |
| DV02-W21 | Artboards, variant matrix, rulers, guides, snapping | DV02-W11, DV02-GAP-1 |
| DV02-W22 | Rule list, computed style, value sources, selector helper, extract styles | DV02-W09 |
| DV02-W23 | Locale and right-to-left preview, pseudo-locales | G-03 |
| DV02-W24 | Accessibility inspector and checks | G-04, DV02-W03 |
| DV02-W25 | Lint engine, rules, profile, panel | DV02-W22, DV02-W24 |
| DV02-W26 | Golden export and compare | DV02-W21, DV02-GAP-4 |
| DV02-W27 | Components, patterns palette, library | DV02-W04 |
| DV02-W28 | Project context, DV01 integration (rename, model import, open in IDE), Preview interface for DV01 | DV01 |
| DV02-W29 | Format version handling and migration | FUSE-WP-64 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| DV02-W40 | Animation editor, motion lint | FUSE declarative animation format (Proposed) |
| DV02-W41 | Layout explainer | DV02-GAP-5 (layout trace) |
| DV02-W42 | Lint CLI with SARIF; scripted test stubs | DV02-W25 |
| DV02-W43 | Local AI suggestions | G-15, `svc.model` |
| DV02-W44 | Colour-vision simulation, stress values, notes, library sharing | DV02-W21 |

## 13. Open questions and risks

1. **Byte-level round trip in ForgeScript.** The lossless model must be fast on 50 000 nodes in an interpreter. If it is not, the model moves into a helper and the app keeps a view of it.
2. **No include in the format.** Components are expanded copies with links. A native `"use": "components/x.fuseui"` with parameters would be better. That is a FUSE format change (FUSE-WP-64).
3. **Scoped tokens.** Tokens are global today (Verified: "scoped tokens are deferred"). Per-artboard themes therefore need one UI context per artboard in the helper, which costs memory.
4. **Widget property metadata.** The registration list with property types does not exist yet (UI8 "not done"). Until it does, the palette and inspector need a hand-written descriptor table that can drift from the runtime.
5. **Forced pseudo-class states and layout trace** need hooks in `UiContext` that do not exist (DV02-GAP-5).
6. **Animation.** The document format has no animation or transition keys today. The animation editor waits for FUSE to define them.
7. **Text profile.** Right-to-left and complex-script previews are wrong under the MINIMAL profile. The designer needs FULL text built offline (G-03).
8. **Model types.** `doc::Model` holds numbers and strings only. View models in ForgeScript will have lists and structs (PLATFORM.md section 5, G-02); the mock model must follow whatever G-02 defines.
9. **Unverified.** How FUSE-WP-63 and FUSE-WP-64 scope the designer and format versioning (only their titles were read in the build plan); whether the prototype builds on HardenedBSD.
