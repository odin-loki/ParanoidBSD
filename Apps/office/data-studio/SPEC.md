<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Data Studio

By Odin Loch

| | |
|---|---|
| ID | `OF09` (from CATALOGUE.md) |
| Bundle | `org.pbsd.data-studio` |
| Category | office |
| Priority | P2 |
| Backing repos | FUSE (UI, `fuse_ui_sql::SqliteModel`, `tableView`, `codeEdit`, `chart`), FORGE (ForgeScript) |
| Status | Planned. Nothing built. |

## 1. Purpose

A database browser and query tool. People use it to open a SQLite file and look inside, to query PostgreSQL, MySQL and MariaDB servers they were granted, and to explore CSV, Parquet and JSON files with SQL without importing them anywhere. Production connections are read-only until the user says otherwise, secrets live in the system keyring, and the app can reach only the hosts the user approved one by one. It replaces the role of tools such as DB Browser for SQLite and DBeaver; the Plasma plan (PLATFORM.md V20) had no equivalent.

## 2. Scope

In scope:

- SQLite files: browse, query, edit, create, check, repair helpers.
- PostgreSQL, MySQL and MariaDB over the network, each host by a `net.host` grant.
- Schema browser, ER diagram, object DDL, table designer.
- Table data editor with staged changes and a reviewed commit.
- SQL editor with completion, formatting, parameters, execution plans.
- Result grid with typed cells, filters, export, charts.
- File explorer for CSV, TSV, JSON, JSON Lines and Parquet: query files with SQL.
- Import wizard (files to tables), export, table copy between connections.
- Saved queries, query history, snippets.
- Data diff and schema diff.

Out of scope:

| Item | Owner |
|---|---|
| Spreadsheet work: formulas, pivot tables, formatted reports | OF05 Sheets (hand-off of a result) |
| Free drawing of diagrams; ER diagram export is editable there | OF08 Draw |
| Running a database server, service management | SY04 System Monitor (services), VM02 Jails |
| Notebook-style analysis with code cells | AI04 Notebook |
| Forecast models on a result | AI03 Forecast |
| Password and key management beyond its own connection items | SE04 Passwords and Keys |
| SSH key generation; general SSH client | SE04, SY03 Terminal |
| Database administration at scale: replication set-up, backup scheduling, user-management wizards for servers | not planned (SQL can still be typed) |
| NoSQL servers, cloud warehouses, ODBC/JDBC drivers, vendor accounts | not planned |
| Oracle, SQL Server, DB2 | not planned (no open client protocol library that fits the rules) |

## 3. Users and top tasks

Personas:

- **Developer**: inspects the SQLite file of an app, runs queries against a development PostgreSQL, checks a query plan.
- **Analyst**: explores a 5 GB Parquet or CSV with SQL, charts a result, exports to Sheets.
- **Administrator**: looks at a production database with no risk of changing it, compares two schemas.
- **Support engineer**: opens a database file sent by a customer, in a disposable sandbox.
- **Curious user**: opens the database of a suite app (their own data) to see or export it.

| # | Task | Target steps |
|---|---|---|
| T1 | Open a SQLite file and see its first table | 1 (double click in Files) + 1 click |
| T2 | Run a query and see results | type, `Ctrl+Enter` |
| T3 | Find a column by name across a schema | `Ctrl+P`, type |
| T4 | Query a CSV or Parquet file with SQL | drop file, type query, `Ctrl+Enter` |
| T5 | Edit a cell and commit | edit, `Ctrl+S`, confirm reviewed SQL (3) |
| T6 | Add a PostgreSQL connection with its secret in the keyring | wizard, 5 fields, 1 capability prompt |
| T7 | Look at production safely | 0 extra steps: read-only is the default for connections tagged production |
| T8 | See why a query is slow | `Ctrl+Shift+E` (plan) |
| T9 | Export a result to CSV or Parquet | 3 |
| T10 | Chart a result | 2 |
| T11 | Import a CSV into a new table | wizard, 4 steps |
| T12 | Compare two tables or two schemas | 4 |
| T13 | Re-run a query from last week | `Ctrl+H`, type to filter, `Enter` |
| T14 | See the ER diagram of a schema | 1 |
| T15 | Cancel a long query | `Esc` or Stop |

## 4. Features

### 4.1 Connections

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F001 | Connection kinds | SQLite file; PostgreSQL; MySQL; MariaDB; in-memory scratch database; file workspace (a set of CSV, Parquet, JSON files). | M | 1 |
| OF09-F002 | Connection manager | List with name, kind, host or file, environment tag, colour, last used, state; groups and folders; search; duplicate; import and export of definitions without secrets. | M | 1 |
| OF09-F003 | Environment tags | `development`, `test`, `staging`, `production`, or custom. The tag sets the window accent stripe, a word in the header ("PRODUCTION"), and the default safety mode. Colour is never the only signal. | M | 1 |
| OF09-F004 | Read-only by default for production | A connection tagged production opens in read-only mode: the session is set read-only on the server (`SET default_transaction_read_only = on` / `SET SESSION TRANSACTION READ ONLY`), and the client refuses statements its classifier does not recognise as reads. Both layers apply. | M | 1 |
| OF09-F005 | Safety modes | Read-only; confirm writes (every writing statement shows a confirmation with the statement and, where it can be had, the affected row estimate); manual commit (auto-commit off, explicit Commit and Rollback buttons, a pending-transaction badge); auto-commit. Default: read-only for production, manual commit for staging, confirm writes otherwise. | M | 1 |
| OF09-F006 | Switching out of read-only | Needs a typed confirmation of the connection name, lasts for a chosen time (15 min default) or until disconnect, shows a countdown, and writes an audit line. | M | 2 |
| OF09-F007 | Dangerous statement guard | `DELETE` or `UPDATE` without `WHERE`, `DROP`, `TRUNCATE`, `ALTER … DROP`, multi-statement scripts with writes: extra confirmation naming the object and the row count where known. Applies in every writable mode. | M | 1 |
| OF09-F008 | Network grant per host | Adding a server connection asks the shell for a `net.host:<host>:<port>` grant. The prompt names the host and port. Without the grant the helper has no route. Grants are listed in the connection's details and in AD05; revoking one disconnects at once. | M | 2 |
| OF09-F009 | Secrets in the keyring | Passwords, client key passphrases and tunnel secrets are stored only in `svc.keyring` under the app's namespace, or asked each time. Connection definitions on disk hold a keyring item id, never a secret. Secret fields are hidden, reveal on hold, and never appear in logs, history or exports. | M | 2 |
| OF09-F010 | Authentication methods | PostgreSQL: SCRAM-SHA-256 (preferred), MD5 (warned as weak), client certificate, `.pgpass` is not read. MySQL/MariaDB: `caching_sha2_password`, `mysql_native_password` (warned), `ed25519` (MariaDB), client certificate. Cleartext password methods are refused unless TLS is verified. | M | 2 |
| OF09-F011 | TLS | On by default with certificate verification (`verify-full` equivalent). Options: system trust store, a pinned CA or leaf certificate picked through the file portal, client certificate and key from SE04. Downgrade to no TLS needs an explicit per-connection setting and shows a permanent warning badge. | M | 2 |
| OF09-F012 | Tunnel through SSH | Optional: connect through an SSH host (itself a separate `net.host` grant) with a key held by SE04's agent; host key shown and pinned on first use. Implemented in the network helper. | S | 3 |
| OF09-F013 | Routes | A connection may be bound to a route (`clear`, `vpn`, `tor`) when the platform allows (`net.route`); shown in the connection details. | C | 3 |
| OF09-F014 | Connection test | Shows DNS result, TCP reachability, TLS details (protocol, certificate subject, fingerprint, expiry), server version, authentication result, each as a line with ok or the failure. | M | 2 |
| OF09-F015 | Session settings | Per connection: default schema or database, search path, statement timeout (default 60 s on production), lock timeout, idle disconnect (default 15 min), keep-alive, application name (fixed string, no user or host name), time zone, initial SQL. | S | 2 |
| OF09-F016 | Several sessions | Each editor tab can use the shared session or its own; a sessions panel lists them with state and running statement; cancel and close per session. | S | 2 |
| OF09-F017 | Reconnect | On a dropped connection: the tab shows "disconnected"; reconnect is manual or automatic for read-only sessions only; a transaction lost in a drop is reported plainly. | M | 2 |
| OF09-F018 | SQLite open options | Read-only (default for files outside the user's own folders and for files with external provenance), read-write, immutable; attach more files to one session; open a copy; WAL files handled; encrypted SQLite variants are not supported and are reported as such. | M | 1 |
| OF09-F019 | SQLite hardening | Opened with defensive mode, `trusted_schema` off, extension loading compiled out, double-quoted string literals off, limits on SQL length, expression depth, attached databases, and a progress handler for cancel and time budgets. | M | 1 |
| OF09-F020 | Open suite app databases | A list of the user's installed apps that declare an exportable store; opening one goes through the file portal like any file and is read-only. No access without the portal grant. | C | 3 |
| OF09-F021 | Disposable open | Open a database file in a throw-away instance: read-only copy in memory or a temporary dataset, no remembered grants, nothing kept on close. | S | 2 |

### 4.2 Schema browser

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F022 | Object tree | Server → databases → schemas → tables, views, materialised views, indexes, sequences, functions and procedures, triggers, types and domains, extensions, foreign tables, partitions; for SQLite: tables, views, indexes, triggers, virtual tables, attached databases. Lazy loading per node; counts on folders. | M | 1 |
| OF09-F023 | Quick open | `Ctrl+P` fuzzy search over all loaded object and column names in the connection; `Enter` opens, `Ctrl+Enter` inserts the name into the editor. | M | 1 |
| OF09-F024 | Table details | Tabs: Columns (name, type, nullable, default, generated, collation, comment), Constraints (primary, unique, check, foreign keys with targets and actions), Indexes (columns, method, unique, partial predicate, size), Triggers, Dependencies (what uses this, what this uses), Statistics (rows estimate, size on disk, last analyse/vacuum where the server has it), DDL. | M | 1 |
| OF09-F025 | DDL view | The `CREATE` statement of any object, from the server's own functions where they exist (`sqlite_schema.sql`, `SHOW CREATE TABLE`, `pg_get_viewdef`, `pg_get_functiondef`) and rebuilt from the catalogue for PostgreSQL tables. Copy, or open in an editor. | M | 1 |
| OF09-F026 | Metadata cache | The catalogue is cached per connection in the app store with a fetch time; refresh per node (`F5`) or whole; completion works offline from the cache. | M | 2 |
| OF09-F027 | Filters on the tree | Show only some schemas; hide system schemas by default; name filter box; favourites pinned on top. | S | 1 |
| OF09-F028 | Object search | Find text inside the definitions of views, functions and triggers; find columns by name or type across the database. | S | 2 |
| OF09-F029 | Server information | Version, settings (read-only list with search), active sessions and locks (where the user has the right), database sizes, extension list. Views only; killing a session is a confirmed action in writable modes. | S | 3 |
| OF09-F030 | Roles and privileges view | List roles and grants per object, read-only. | C | 3 |
| OF09-F031 | SQLite file information | Page size, page count, encoding, journal mode, user version, application id, schema version, freelist pages, `integrity_check` and `foreign_key_check` on request, `PRAGMA` browser with descriptions. | M | 1 |
| OF09-F032 | Comments and documentation | Show object and column comments; edit them (writes `COMMENT ON` in writable modes); generate a Markdown data dictionary of a schema. | S | 3 |

### 4.3 ER diagram

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F033 | Diagram from schema | Tables as boxes with columns (key and type marks), foreign keys as connectors with crow's-foot cardinality, drawn for a schema or a chosen set of tables. | M | 2 |
| OF09-F034 | Automatic layout | Layered or force layout from the shared graph layout code (OF08's vector core); manual moves are kept per connection and schema. | M | 2 |
| OF09-F035 | Detail levels | Names only; keys only; all columns; with types; with indexes. | S | 2 |
| OF09-F036 | Navigation | Zoom, pan, search a table, highlight a table's neighbours, expand "tables related to this one" step by step, jump to table details or data. | M | 2 |
| OF09-F037 | Inferred relations | Optional dotted connectors for likely relations without declared keys (matching names and types, such as `customer_id` → `customer.id`); clearly marked as guesses. | C | 3 |
| OF09-F038 | Export | SVG, PNG, PDF; "Open in Draw" sends an editable ODG through the `edit` intent; Mermaid ER text. | S | 2 |
| OF09-F039 | Diagram for a query | Show the tables and joins a selected query uses. | C | 3 |

### 4.4 Table data editor

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F040 | Data view of a table | Opens the first window of rows at once; rows are fetched in windows while scrolling; total count is estimated first and counted exactly on request. | M | 1 |
| OF09-F041 | Keyset paging | For tables with a primary or unique key, windows are fetched by key range, not `OFFSET`, so deep scrolls stay fast. `OFFSET` is the fallback. | M | 2 |
| OF09-F042 | Filter and sort bar | Per-column filter popovers (equals, contains, range, null, in list), a free `WHERE` box with completion, sort by clicking headers (multi-column with `Shift`). The generated SQL is always shown and can be opened in an editor. | M | 1 |
| OF09-F043 | Inline editing | Edit cells in place for tables with a key (or `rowid`); typed editors: text, number, boolean, date and time pickers, enum dropdown, JSON editor, NULL toggle (`Ctrl+0`), default toggle. | M | 2 |
| OF09-F044 | Staged changes | Edits, inserted rows and deleted rows are staged locally and marked (changed cell `~`, new row `+`, deleted row `−`, each with a colour and the mark). Nothing is sent until Commit. | M | 2 |
| OF09-F045 | Reviewed commit | Commit shows the exact SQL (parameterised `UPDATE`, `INSERT`, `DELETE` by key, including the old values in the `WHERE` for an optimistic check), the row counts, and runs them in one transaction. If any statement affects a different number of rows than expected, the whole transaction rolls back and the conflict is shown. | M | 2 |
| OF09-F046 | Row operations | Add row, duplicate row, delete rows, paste rows from the clipboard (TSV or CSV) as staged inserts, set a column to a value for selected rows. | M | 2 |
| OF09-F047 | Foreign key helpers | A cell holding a foreign key shows a link: peek the referenced row in a popover, jump to it, or pick a value from the referenced table with search. | S | 2 |
| OF09-F048 | Related rows | From a row: list tables that reference it with counts; open them filtered. | S | 3 |
| OF09-F049 | Record view | One row as a form (field per line, large editors for long text and JSON), `F6` toggles; previous and next row. | S | 2 |
| OF09-F050 | Cell viewers | Long text (wrap, find), JSON and JSONB (tree and pretty text), XML (pretty text), binary (hex view with size and detected type; images shown when the bytes are PNG, JPEG or WebP, decoded by the platform image helper), arrays, geometric and network types as text, UUIDs, intervals. | M | 2 |
| OF09-F051 | Binary cell save and load | Save a BLOB to a file or load a file into a staged BLOB through the file portal. "Open in Hex Editor" hands a temporary handle to DV04. | S | 3 |
| OF09-F052 | Table designer | Create and alter tables in a form: columns, types (per dialect), defaults, keys, checks, foreign keys, indexes. Produces a DDL script for review; for SQLite alterations that need a rebuild it generates the documented copy-and-rename procedure inside a transaction. | S | 3 |
| OF09-F053 | Truncate, drop, rename, vacuum, analyse | Each as a menu action that produces reviewed SQL and passes the dangerous-statement guard. | S | 3 |

### 4.5 Query editor

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F054 | Editor tabs | Each tab has a connection, a session mode and a text buffer; unsaved tabs persist across restarts; split view; tab colour follows the environment tag. | M | 1 |
| OF09-F055 | Syntax highlighting per dialect | SQLite, PostgreSQL, MySQL, MariaDB keyword and function sets; strings, dollar-quoted strings, comments, numbers, parameters. | M | 1 |
| OF09-F056 | Completion | Context-aware: keywords, schemas, tables and aliases, columns of tables in scope, functions with signatures, join conditions suggested from foreign keys, `INSERT` column lists, snippets. Works from the metadata cache; never sends a query to the server on a keystroke. | M | 2 |
| OF09-F057 | Run | Run the statement under the caret (`Ctrl+Enter`), the selection, or the whole script (`Ctrl+Shift+Enter`); statement boundaries detected by a dialect-aware splitter (handles strings, comments, dollar quotes, `DELIMITER`). | M | 1 |
| OF09-F058 | Cancel and time limit | `Esc` or Stop cancels on the server (`pg_cancel` request, `KILL QUERY`, SQLite interrupt). A per-tab time limit can be set. | M | 1 |
| OF09-F059 | Row limit | Results are fetched with a window (default 1,000 rows shown, more on scroll) using a server-side cursor or streaming; the app never appends `LIMIT` silently; the status line says "first 1,000 rows, more available". | M | 1 |
| OF09-F060 | Parameters | `:name`, `$1`, `?` placeholders are detected; a parameter bar asks for values with types; values are bound, never spliced into the text; last values remembered per saved query. | M | 2 |
| OF09-F061 | Execution plan | `EXPLAIN` and, in writable or confirmed cases, `EXPLAIN ANALYZE` (wrapped in a rolled-back transaction for writes; refused in read-only mode for writing statements). Views: text; tree table with estimated and actual rows, cost, time, loops, buffers; a graph view with the widest arrows on the largest row flows; warnings (sequential scan on a large table with a selective filter, misestimates over 10×, sort or hash spilling to disk). For SQLite: `EXPLAIN QUERY PLAN` tree. | M | 2 |
| OF09-F062 | Plan compare | Keep plans in history and compare two side by side. | C | 3 |
| OF09-F063 | Formatter | Reformat SQL with options (keyword case, indent, comma position, line width); format selection or file. | S | 2 |
| OF09-F064 | Diagnostics | Parse errors from the server are mapped back to the position in the editor; a local syntax check underlines obvious errors before running; unknown table and column names are marked from the cache. | S | 2 |
| OF09-F065 | Statement classifier | Each statement is classed as read, write, DDL, transaction control, session setting, or unknown, by a local parser. The class drives safety modes. Unknown is treated as a write. Functions with side effects in a `SELECT` cannot be detected; the server-side read-only setting is the backstop. | M | 1 |
| OF09-F066 | Transaction control | Buttons and keys for Begin, Commit, Rollback; a badge while a transaction is open with its age; closing a tab with an open transaction asks. | M | 2 |
| OF09-F067 | Multiple result sets and messages | Each statement of a script gets a result tab or a messages entry (rows affected, notices, timing). | M | 1 |
| OF09-F068 | Script execution options | Stop on first error or continue; run in one transaction; dry run (roll back at the end) for dialects with transactional DDL. | S | 2 |
| OF09-F069 | Query builder | Pick tables, joins (suggested from keys), columns, filters, grouping in a form; it writes SQL into the editor; one way only. | C | 3 |
| OF09-F070 | Snippets and templates | Built-in per dialect (select top rows, count by, find duplicates, table sizes, running queries) and user snippets with tab stops. | S | 2 |
| OF09-F071 | Editor basics | Multi-cursor, find and replace with regex, bracket matching, comment toggle, upper and lower case, go to line, code folding of statements, minimap off by default. Depends on the code editor widget work shared with OF01 and DV01. | M | 1 |
| OF09-F072 | Explain in words | Optional, with `ai.local`: describe a query, or draft SQL from a typed question. The model receives the schema names from the cache and the text; never row data. Drafts are placed in the editor and never run automatically. Off by default; unavailable on production connections unless enabled per connection. | C | 3 |
| OF09-F073 | Open and save SQL files | `.sql` files through the portal; encoding and line ending kept; external change detection. | M | 1 |

### 4.6 Result grid

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F074 | Typed grid | Columns keep their database type; numbers right-aligned with tabular figures; NULL shown as a dim `NULL` word, distinct from empty text; booleans as words; dates in ISO form by default or locale by option; binary as size and prefix. | M | 1 |
| OF09-F075 | Streaming and virtualisation | Rows arrive in chunks and are shown as they come; the grid holds a window in memory and spills to a temporary columnar file in the cache directory above a memory budget (default 512 MiB). 64-bit row counts. | M | 1 |
| OF09-F076 | Selection and copy | Cell, range, row and column selection; copy as TSV, CSV, Markdown table, JSON, SQL `INSERT` statements, `IN (…)` list, with or without headers. | M | 1 |
| OF09-F077 | Client-side sort and filter | On the fetched rows only, labelled as such when the result is not complete; "re-run on the server with this order and filter" wraps the query. | M | 2 |
| OF09-F078 | Column tools | Resize, reorder, hide, freeze left columns, auto-fit, column statistics popover (count, nulls, distinct, min, max, mean, top values) computed over fetched rows or by a generated query. | M | 2 |
| OF09-F079 | Aggregates of selection | Sum, average, count, min, max of selected cells in the status bar. | S | 1 |
| OF09-F080 | Find in results | Text search over fetched rows; highlight matches; next and previous. | M | 1 |
| OF09-F081 | Pin and compare results | Pin a result tab so the next run opens a new one; diff two results (see data diff). | S | 2 |
| OF09-F082 | Transpose and record view | Show one row vertically; transpose small results. | S | 2 |
| OF09-F083 | Export results | CSV and TSV (delimiter, quote, encoding, line ends, header, NULL text, formula-injection escaping on by default), JSON (array or lines), Parquet, SQL inserts, Markdown, HTML table, ODS and XLSX (written by this app's I/O helper, values only), SQLite table in a new or existing file. Export streams from the server; it does not need the rows in the grid. | M | 1 |
| OF09-F084 | Send to Sheets | Hand a result to OF05 as a CSV or Parquet handle through `svc.intent`. | S | 2 |
| OF09-F085 | Edit results | A result from a single table with its key columns present can be edited like a table view (staged changes, reviewed commit). | S | 3 |
| OF09-F086 | Value formatting options | Number grouping, decimal places, time zone for timestamps (shown in the header), binary as hex or base64, max text shown per cell. Display only; export has its own options. | S | 2 |
| OF09-F087 | Sensitive columns | Columns whose names match a configurable list (`password`, `secret`, `token`, `ssn`, …) are masked in the grid until revealed per column; masked on copy by default; the mask state is shown in the header. | S | 2 |

### 4.7 Charts from results

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F088 | Chart tab on a result | Types: bar, stacked bar, line, area, scatter, pie, histogram, box, heat map. Pick X, Y, series and aggregate columns; sensible defaults from column types. | M | 2 |
| OF09-F089 | Aggregation | Group by X with sum, count, average, min, max, distinct count; date bucketing (hour, day, week, month, year); top N with "other". Computed over fetched rows, or by a generated query on request. | M | 2 |
| OF09-F090 | Interaction | Hover values, zoom and pan on numeric and time axes, toggle series, select points to filter the grid. | S | 3 |
| OF09-F091 | Chart export | PNG, SVG, copy image, copy the aggregated data; "Send to Sheets" with data. | S | 2 |
| OF09-F092 | Saved chart with a query | A saved query can carry a chart definition; opening it runs the query (after the usual mode checks) and shows the chart. | S | 3 |
| OF09-F093 | Accessible charts | Text summary and a data table view for every chart. | M | 2 |

### 4.8 File explorer for CSV, Parquet and JSON

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F094 | File workspace | A connection kind that holds files and folders granted through the portal. Each file appears as a table; folders of same-schema files appear as one table (partitioned data sets, `key=value` directory names become columns). | M | 2 |
| OF09-F095 | Query engine for files | SQL over files runs in the file query helper using SQLite with virtual tables that scan CSV, JSON Lines and Parquet directly (column pruning and row-group skipping for Parquet by min/max statistics). No import step; files are never modified. | M | 2 |
| OF09-F096 | CSV understanding | Encoding, delimiter, quote, header and type detection with a preview and overrides, as in OF05's import dialog (shared source). Types default to text when ambiguous. Malformed rows are counted and viewable. | M | 2 |
| OF09-F097 | Parquet inspector | Schema tree (logical and physical types, repetition), row groups with row counts and sizes, column chunk encodings and compression, min/max/null statistics, key-value metadata, created-by string. | M | 2 |
| OF09-F098 | JSON explorer | Tree view with lazy children for large files; path copy; "table from this array" infers columns from objects; JSON Lines as rows; nested fields addressed with `->` operators in SQL. | S | 2 |
| OF09-F099 | Profile a file or table | One click: per column type, null share, distinct count (exact or estimated), min, max, mean, quantiles, most frequent values, length range, a small histogram. Runs as a background job with progress. | S | 3 |
| OF09-F100 | Convert | CSV ↔ Parquet ↔ JSON Lines ↔ SQLite table, streaming, with type choices and compression options. | S | 3 |
| OF09-F101 | Large files | Scans are streamed; memory is bounded; a 50 GB CSV can be queried with a full scan time shown as progress; a sidecar index is never written next to the user's file (indexes, if any, go to the cache directory). | M | 2 |
| OF09-F102 | Compressed inputs | `.gz` and `.zst` CSV and JSON read as streams. | S | 3 |

### 4.9 Import, export and copy

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F103 | Import wizard | Source (CSV, TSV, JSON, JSON Lines, Parquet, another table or query) → target (new or existing table) → column mapping with types, renames, skips, constants and simple transforms (trim, null-if-empty, date format) → options (batch size, on conflict: error, skip, update; transaction per batch or one; truncate first) → preview of the first 100 target rows → run with progress, rate, error log. | M | 2 |
| OF09-F104 | Bulk load paths | PostgreSQL `COPY … FROM STDIN` (text or binary), MySQL/MariaDB multi-row inserts (`LOAD DATA LOCAL` is not used: it lets a server ask the client for files), SQLite prepared inserts in one transaction. | M | 2 |
| OF09-F105 | Rejected rows | Rows that fail are written to a reject file with the reason; the import can stop at N errors. | S | 2 |
| OF09-F106 | Import profiles | Save a wizard configuration; re-run on a new file. | S | 3 |
| OF09-F107 | Export table or schema | Data as files (formats of result export); SQL dump of structure, data or both for SQLite and, as plain statements generated by the app, for server tables (not a replacement for the server's own backup tools, and it says so). | S | 2 |
| OF09-F108 | Copy between connections | Copy a table or query result from one connection to another with type mapping between dialects, shown for review. | S | 3 |
| OF09-F109 | Generate SQL | From a table: `SELECT`, `INSERT`, `UPDATE`, `DELETE`, `CREATE` templates into the editor. | M | 1 |
| OF09-F110 | Test data | Fill a table with generated rows by column rules (sequence, random in range with a seed, pick list, pattern). Development and test tags only. | C | 3 |

### 4.10 Saved queries and history

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F111 | Saved queries | Named queries with description, tags, the connection or dialect they belong to, parameters with defaults, an optional chart. Folders. Stored in the app store, exportable as `.sql` files with a header comment. | M | 1 |
| OF09-F112 | Query history | Every executed statement with time, connection, duration, rows, outcome and error text. Searchable (full text), filterable by connection, date, outcome. Kept 90 days or 50,000 entries by default. | M | 1 |
| OF09-F113 | History privacy | Parameter values are not stored by default (placeholders are). Literal values inside statement text are stored as typed; a per-connection option "do not record history" exists and is suggested for production. A redaction rule list (regex) scrubs matching literals before storage. | M | 2 |
| OF09-F114 | Re-run and reuse | Open a history entry in a new tab (never auto-run); copy; save as a named query; compare two entries' text. | M | 1 |
| OF09-F115 | Result snapshots | Optionally keep the first N rows of a result with a history entry (off by default) for later comparison. | C | 3 |
| OF09-F116 | Workspaces | A named set of open tabs, connections and layout; restore on start; several workspaces. | S | 2 |
| OF09-F117 | Search integration | Saved query names and descriptions are offered to `svc.search`. History and SQL text are not. | S | 2 |

### 4.11 Data diff and schema diff

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F118 | Table data diff | Compare two tables or two query results (same or different connections) by key columns: rows only in A, only in B, changed (with changed cells marked). Streaming merge of two ordered scans; or hash-bucket comparison first to find differing key ranges on large tables. | M | 3 |
| OF09-F119 | Diff view | Summary counts; a merged grid with marks (`+`, `−`, `~`) and filters; side-by-side cell compare for long values; tolerance options (numeric epsilon, trim, case, NULL equals empty). | M | 3 |
| OF09-F120 | Sync script | Generate the SQL that would make B equal A (inserts, updates, deletes by key) for review. It is opened in an editor; it is never run by the diff tool. | S | 3 |
| OF09-F121 | Schema diff | Compare two schemas or a schema against a DDL file: tables, columns, types, defaults, constraints, indexes, views, functions; show differences as a tree; generate a migration script for review, with destructive steps commented out by default. | S | 3 |
| OF09-F122 | File diff | Compare two CSV or Parquet files by key or by row position using the same engine. | S | 3 |
| OF09-F123 | Snapshot diff | Compare a table now with a snapshot kept earlier (a result snapshot or an exported Parquet file). | C | 3 |
| OF09-F124 | Diff export | Summary and differing rows to CSV, Parquet or an HTML report. | S | 3 |

### 4.12 Safety, audit, recovery and system integration

| ID | Feature | Detail | Priority | Phase |
|---|---|---|---|---|
| OF09-F125 | Visible network state | The header `[net]` badge shows while any server connection is open; clicking lists host, port, route, TLS state and session count for each. | M | 2 |
| OF09-F126 | Local audit trail | Mode changes, writes on staging and production, dangerous-statement confirmations, grant requests and export of more than a set number of rows are written to `svc.audit` (statement class and object names, not data). | M | 2 |
| OF09-F127 | Idle lock of connections | After the idle time, sessions close and the secret must be fetched from the keyring again (which follows the session lock). | S | 2 |
| OF09-F128 | Clipboard care | Copies from masked columns or from production connections can be flagged sensitive to `svc.clipboard` so they expire; setting per environment tag. | S | 3 |
| OF09-F129 | Crash recovery | Unsaved editor buffers and staged edits are journalled every 2 s; after a crash they are restored; open transactions are gone and the user is told which tabs had one. | M | 1 |
| OF09-F130 | Background jobs | Imports, exports, profiles and diffs run as jobs with progress in the status bar and `svc.notify`; they survive closing the window while the app stays alive; cancel at any time. | M | 2 |
| OF09-F131 | Keep-awake during jobs | Long jobs may inhibit sleep through `svc.power`, shown in the job row. | C | 3 |
| OF09-F132 | SQLite maintenance tools | `VACUUM`, `VACUUM INTO` a new file, `ANALYZE`, `REINDEX`, integrity check with a readable report, recover readable data from a damaged file into a new file (using SQLite's recovery routine in the worker). | S | 3 |
| OF09-F133 | Backup before write | For SQLite files opened read-write: optional automatic copy (or a dataset snapshot through `svc.backup` where available) before the first write of a session. | S | 2 |
| OF09-F134 | Command palette, shortcuts, help | Every action; rebindable; help pages on safety modes, dialect notes and the file query engine. | M | 1 |
| OF09-F135 | Themes for grids and editor | Follow the system theme; compact density by default; monospace option for the grid. | S | 1 |
| OF09-F136 | Print | Print or PDF a result, a table definition, an ER diagram or a plan through `svc.portal.print`. | C | 3 |
| OF09-F137 | Scripting | Signed ForgeScript modules may add result formatters and exporters through a narrow API with no capabilities. They cannot open connections or run SQL. | C | 3 |
