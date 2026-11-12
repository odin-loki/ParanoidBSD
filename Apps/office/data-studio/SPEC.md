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

## 5. GUI

Layout pattern: workbench with docks (DESIGN_SYSTEM.md section 4). Default density: compact.

### 5.1 Window map

```
Data Studio
├─ Main window
│  ├─ Header bar: menu, title (connection — object), quick open / command palette,
│  │              environment word, [net] badge, pending-transaction badge
│  ├─ Environment stripe (4 px, under the header; colour and a repeated word for production)
│  ├─ Sidebar (F9): Connections and schema tree | Saved queries | History | Files | Jobs
│  ├─ Centre tabs: SQL editor | Table data | Table details | ER diagram | Plan | Diff |
│  │               Parquet inspector | JSON explorer | Server information | Start view
│  ├─ Bottom dock (per editor tab): Results 1..n | Messages | Chart | Plan | Parameters
│  ├─ Inspector (F4): Cell viewer | Row (record view) | Column statistics | Object properties
│  └─ Status bar: session mode, transaction state, rows, time, selection aggregates, jobs
├─ Dialogs
│  ├─ New / edit connection (wizard)        ├─ Export result / table
│  ├─ Connection test                       ├─ Import wizard
│  ├─ Leave read-only mode                  ├─ Copy table to connection
│  ├─ Review and commit changes             ├─ Data diff set-up
│  ├─ Dangerous statement confirmation      ├─ Schema diff set-up
│  ├─ Open SQLite file options              ├─ Table designer
│  ├─ CSV options (shared with OF05)        ├─ Formatter options
│  ├─ Save query                            ├─ History privacy and redaction rules
│  └─ Preferences                           └─ SQLite maintenance
├─ Popovers: column filter, foreign-key peek, column statistics, parameter values,
│            net badge details, session list, NULL / default menu, completion list
└─ Inline bars: read-only bar, disconnected bar, trust bar (external file), stale-metadata bar,
               open-transaction bar, partial-result bar
```

### 5.2 Main window

Shown at 1440×860, wide layout, connected to a production PostgreSQL in read-only mode.

```
+------------------------------------------------------------------------------------------+
| [=] Data Studio — orders-db / public        [Ctrl+P quick open]  (1) PRODUCTION [net] [▁▢✕]|
+==== PRODUCTION · READ-ONLY ==== PRODUCTION · READ-ONLY ==== PRODUCTION · READ-ONLY ======+ (2)
| (3) Connections  [+] | (5) [orders.sql ●][customers ▦][Plan]              [▶ Run][■][⚙]  |
| [/ filter_______]    |  1  SELECT c.name, count(*) AS orders, sum(o.total) AS revenue    |
| ▾ ● orders-db  PROD  |  2  FROM   customers c                                            |
|   ▾ public           |  3  JOIN   orders o ON o.customer_id = c.id                       |
|     ▾ Tables (14)    |  4  WHERE  o.placed_at >= :since                                  |
|       ▸ customers    |  5  GROUP  BY c.name                                              |
|       ▸ orders       |  6  ORDER  BY revenue DESC;|                                      |
|       ▸ order_items  |                                                                   |
|     ▸ Views (3)      +-------------------------------------------------------------------+
|     ▸ Functions (9)  | (6) [Results][Messages][Chart][Plan]   :since [2026-01-01]  [Run] |
| ▸ ○ dev-local  DEV   | +----+----------------------+--------+-------------+              |
| ▸ ▣ app-cache.sqlite | | #  | name            text | orders | revenue num |              |
| (4) Saved queries    | +----+----------------------+--------+-------------+              |
|   ★ Revenue by cust. | |  1 | Acme Pty Ltd         |    412 |  188,204.50 |              |
|   ★ Late orders      | |  2 | Birch & Co           |    377 |  141,090.00 |              |
|   History (Ctrl+H)   | |  3 | NULL                 |     12 |    1,204.75 |              |
|                      | |  … | …                    |      … |           … |              |
+----------------------+-------------------------------------------------------------------+
| (7) Read-only · no transaction · 1,000 of 8,412+ rows · 184 ms · Sum 330,499.25  · Jobs 0 |
+------------------------------------------------------------------------------------------+
```

1. Environment word and `[net]` badge. The word and stripe (2) are drawn by app chrome that the SQL content cannot affect.
3. Connection and schema tree (`treeView`), state dot (connected ●, idle ○, file ▣), environment tag.
4. Saved queries and history sections of the sidebar (also full tabs).
5. Centre tabs; `●` marks unsaved text; `[▶ Run]` runs the statement at the caret, `[■]` cancels, `[⚙]` opens tab options (session, limit, time-out).
6. Bottom dock with the result grid (OF09-GAP-1 grid), parameter bar on its tab row.
7. Status bar: mode, transaction, fetched and known rows, duration, aggregates of the selection, job count.

States: empty (start view: recent connections and files, "Open SQLite file…", "New connection…", "Explore files…"), loading (tree nodes show a spinner word "loading…"; grid shows arriving rows), error (message tab with the server text and position; red squiggle in the editor), offline or unreachable (disconnected bar on tabs of that connection, cached schema usable, Run disabled with the reason in its tooltip), locked (keyring locked: a bar "Unlock the keyring to connect" that asks the shell prompt), read-only (bar and status word; write actions disabled with a tooltip naming how to leave read-only).

### 5.3 Other views and dialogs

**New connection** (640×600).

```
+-- New connection -----------------------------------------------------------+
| Kind  (•) PostgreSQL ( ) MySQL ( ) MariaDB ( ) SQLite file ( ) Files         |
| Name        [orders-db__________________]   Group [Work v]                   |
| Environment [Production v]  → opens read-only, statement time-out 60 s       |
| Host        [db1.internal.example____]  Port [5432]                          |
| Database    [orders___________]  User [report_ro________]                    |
| Password    [••••••••••••] (hold to reveal)                                  |
|             (•) Save in keyring  ( ) Ask every time                          |
| TLS         [Verify certificate and host name v]   CA [System store v]       |
| Route       [Clear v]            Tunnel [None v]                             |
| (1) Network access: this app may not reach db1.internal.example:5432 yet.    |
|     [Request access…]  → the system asks you to allow this one host and port |
+------------------------------------------------------------------------------+
| (2) [Test connection]   DNS ok · TCP ok · TLS 1.3 ok · auth ok · PostgreSQL 17|
|                                               [Cancel]   [Save and connect]  |
+------------------------------------------------------------------------------+
```

1. Grant row: shows the state of the `net.host` grant; the button raises the shell prompt. 2. Test results appear as words, not colour alone.

**Table data with staged changes and the commit review** (centre tab, then a 760×520 dialog).

```
+------------------------------------------------------------------------------------------+
| customers   WHERE [country = 'AU'________________] [Apply]   Sort: name ↑    [+ Row] [−] |
+---+----+----------------------+---------+------------------+-----------------------------+
|   | id | name                 | country | email            | updated_at                  |
+---+----+----------------------+---------+------------------+-----------------------------+
|   | 17 | Acme Pty Ltd         | AU      | ap@acme.example  | 2026-10-02 08:14:11+10      |
| ~ | 21 | Birch & Co           | AU      |~bills@birch.exa… | 2026-09-30 17:02:40+10      |
| − | 34 | Cedar Trading        | AU      | hi@cedar.example | 2026-03-12 11:45:09+11      |
| + |  ∅ | Dune Supplies        | AU      | NULL             | DEFAULT                     |
+---+----+----------------------+---------+------------------+-----------------------------+
| 3 pending changes: 1 update · 1 delete · 1 insert        [Discard]  [Review and commit…] |
+------------------------------------------------------------------------------------------+

+-- Review and commit: customers on dev-local -------------------------------------+
| BEGIN;                                                                           |
| UPDATE customers SET email = $1 WHERE id = $2 AND email = $3;   -- 1 row expected|
| DELETE FROM customers WHERE id = $1 AND name = $2 …;            -- 1 row expected|
| INSERT INTO customers (name, country) VALUES ($1, $2);          -- 1 row         |
| COMMIT;                                                                          |
+----------------------------------------------------------------------------------+
| Parameters are bound, not spliced.  [Show values]   If any count differs: roll   |
| back everything.                         [Copy SQL]   [Cancel]   [Commit 3 changes]|
+----------------------------------------------------------------------------------+
```

**Execution plan tab**.

```
+------------------------------------------------------------------------------------------+
| Plan for orders.sql (statement 1)   [Text][>Tree][Graph]   EXPLAIN (ANALYZE off)  [Re-run]|
+------------------------------------------+--------+---------+--------+-------------------+
| Node                                     | Est.rows| Cost    | Width  | Notes             |
+------------------------------------------+--------+---------+--------+-------------------+
| ▾ Sort (revenue DESC)                    |  2,100 | 9,420.1 |   48   |                   |
|   ▾ HashAggregate (c.name)               |  2,100 | 9,300.5 |   48   |                   |
|     ▾ Hash Join (o.customer_id = c.id)   | 84,000 | 7,950.0 |   26   |                   |
|       ▸ Seq Scan on orders o             | 84,000 | 6,112.0 |   12   | ⚠ filter on       |
|       ▸ Hash → Seq Scan on customers c   |  2,100 |    44.0 |   22   |   placed_at, no   |
|                                          |        |         |        |   usable index    |
+------------------------------------------+--------+---------+--------+-------------------+
| EXPLAIN ANALYZE runs the query. In read-only mode it is allowed for SELECT only.         |
+------------------------------------------------------------------------------------------+
```

Tree view is a `treeView` with columns; graph view draws the same tree on the canvas (or `nodeGraph` read-only) with edge width by rows.

**Import wizard, mapping step** (860×600).

```
+-- Import: sales-2026.csv → dev-local.public.sales (new table) ----------------------------+
| (1) Source ✓   (2) Target ✓   (3) > Mapping   (4) Options   (5) Preview and run           |
+-------------------------------------------------------------------------------------------+
| Source column   Sample            → Target column   Type            Transform        Use  |
| order_id        000418227         → order_id        text            —                [x]  |
| date            2026-03-01        → placed_on       date            ISO 8601         [x]  |
| amount          1,204.50          → amount          numeric(12,2)   strip thousands  [x]  |
| sku             MARCH1            → sku             text            —                [x]  |
| notes           =cmd|…            → notes           text            —                [x]  |
| qty             4                 → qty             integer         —                [x]  |
| (none)                            → imported_at     timestamptz     constant now()   [x]  |
+-------------------------------------------------------------------------------------------+
| ⚠ order_id looks numeric but has leading zeros: kept as text.     [Reset] [Load profile…] |
|                                                         [Back]  [Cancel]  [Next]          |
+-------------------------------------------------------------------------------------------+
```

**ER diagram tab**.

```
+------------------------------------------------------------------------------------------+
| ER: public (14 tables)   Detail [Keys only v]   [Layout v] [Fit] [Find table_]  [Export v]|
+------------------------------------------------------------------------------------------+
|   +----------------+            +------------------+           +-------------------+     |
|   | customers      |            | orders           |           | order_items       |     |
|   | 🔑 id          |1──────────<| 🔑 id            |1─────────<| 🔑 id             |     |
|   |    name        |            | ⚷ customer_id    |           | ⚷ order_id        |     |
|   +----------------+            |    placed_at     |           | ⚷ product_id      |>──┐ |
|                                 +------------------+           +-------------------+   │ |
|                                                                +-------------------+   │ |
|                                                                | products          |1──┘ |
|                                                                | 🔑 id             |     |
|                                                                +-------------------+     |
+------------------------------------------------------------------------------------------+
```

Key marks are icons with text alternatives ("primary key", "foreign key"). Drawn on the canvas widget (G-12) in view-and-move mode.

**Data diff tab**.

```
+------------------------------------------------------------------------------------------+
| Diff: staging.customers ↔ prod.customers   key: id   [Options…]            [Sync script…]|
| Only in A 12 (+) · Only in B 3 (−) · Changed 41 (~) · Same 20,518    Show [x]+ [x]− [x]~ |
+---+------+----------------------+----------------------------+---------------------------+
|   | id   | name                 | email (A)                  | email (B)                 |
+---+------+----------------------+----------------------------+---------------------------+
| ~ | 21   | Birch & Co           | bills@birch.example        | accounts@birch.example    |
| + | 9001 | Dune Supplies        | NULL                       |                           |
| − | 344  | Elm Partners         |                            | office@elm.example        |
+---+------+----------------------+----------------------------+---------------------------+
```

**Parquet inspector tab**: left `treeView` of the schema; right tabs `[Row groups][Column chunks][Metadata][Preview]`, each a `tableView`; a header line with file size, rows, row groups, created-by, compression summary.

**Leave read-only mode** (480×320): explains what changes, a field to type the connection name, duration `comboBox` (15 min, 1 h, until disconnect), `[Cancel] [Allow writes for 15 minutes]`; the destructive-outline button is not the default.

### 5.4 Menus

```
File      New SQL tab (Ctrl+N) · New connection… (Ctrl+Shift+N) · Open SQLite file… (Ctrl+O) · Open SQL file… · Explore files…
          Open in disposable sandbox… · Save (Ctrl+S: SQL file, or commit staged changes in a data tab) · Save as… (Ctrl+Shift+S)
          Save query… (Ctrl+Alt+S) · Import… · Export… (Ctrl+E) · Print… (Ctrl+P) · Close tab (Ctrl+W) · Quit (Ctrl+Q)
Edit      Undo (Ctrl+Z) · Redo (Ctrl+Shift+Z) · Cut · Copy · Copy as ▸ TSV · CSV · Markdown · JSON · SQL INSERT · IN list
          Paste · Find (Ctrl+F) · Replace (Ctrl+Alt+F) · Format SQL (Ctrl+Shift+F)
          Toggle comment (Ctrl+/) · Upper case keywords · Preferences… (Ctrl+,)
View      Sidebar (F9) ▸ Connections · Saved queries · History (Ctrl+H) · Files · Jobs · Inspector (F4) · Record view (F6)
          Results dock (Ctrl+J) · Split editor · Zoom ▸ · Full screen (F11)
Connection  Connect · Disconnect · Reconnect · Test… · Edit… · Duplicate · Sessions… · Refresh metadata (F5)
          Mode ▸ Read-only · Confirm writes · Manual commit · Auto-commit · Leave read-only… · Network access…
Query     Run statement (Ctrl+Enter) · Run selection or script (Ctrl+Shift+Enter) · Cancel (Esc) · Explain (Ctrl+Shift+E)
          Explain analyse… · Parameters… · Begin (Ctrl+Alt+B) · Commit (Ctrl+Alt+C) · Rollback (Ctrl+Alt+R)
          Row limit ▸ · Time limit ▸ · Open result in new tab · Pin result
Data      Filter… · Clear filters · Sort ▸ · Add row (Ctrl+I) · Delete rows (Ctrl+Delete) · Set NULL (Ctrl+0) · Set default
          Review and commit… · Discard changes · Column statistics · Chart result · Send to Sheets
Schema    Quick open (Ctrl+P) · Table details (F4 on a table) · Show DDL · ER diagram (Ctrl+Shift+D) · Generate SQL ▸
          Table designer… · Search definitions… · Data dictionary…
Tools     Data diff… · Schema diff… · Profile… · Convert file… · Copy table to… · SQLite ▸ Integrity check · Vacuum… · Analyse · Recover…
          History privacy… · Snippets… · Jobs
Help      Data Studio help (F1) · Safety modes · Dialect notes · Keyboard shortcuts · About
```

### 5.5 Keyboard and pointer

Reserved keys keep their meaning with two documented exceptions inside this app: `Ctrl+H` opens history (replace is `Ctrl+Alt+F`), and `Ctrl+S` in a data tab opens "Review and commit" (it never commits without the review). Both can be rebound.

Grid keys: arrows, `Ctrl+Arrow` to edge, `Home/End`, `Ctrl+Home/End`, `PageUp/Down`; `Enter` or `F2` edit; `Esc` cancel edit; `Ctrl+0` NULL; `Ctrl+D` copy value from the cell above (staged); `Space` peek a foreign key; `F6` record view; `Ctrl+Shift+C` copy with headers; `Alt+Down` column filter. Tree keys: type-ahead, `Enter` opens data, `F4` details, `F5` refresh node. Editor: `Ctrl+Space` completion, `Ctrl+Enter` run, `Alt+Up/Down` move line, `Ctrl+D` select next occurrence.

Context menus: connection (connect, mode, edit, test, duplicate, network access, remove); table (open data, details, DDL, generate SQL, import, export, diff with…, designer, truncate…, drop…); column (copy name, statistics, filter, hide, add to query); grid cell (copy, copy as, filter by value, set NULL, peek reference, open viewer, save BLOB); grid header (sort, filter, hide, freeze, statistics, mask); result tab (pin, rename, export, chart, close); history entry (open, copy, save, delete); editor (run, explain, format, copy as string literal for a language).

Drag and drop: table or column from the tree into the editor (inserts the quoted name; with `Shift` a `SELECT` template); files onto the window (SQLite opens; CSV, Parquet, JSON join the file workspace; `.sql` opens in a tab); a result tab out to a new window; rows out as TSV; a table onto another connection (starts "Copy table to…"); ER boxes to move.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Start view with recent items and three actions. |
| Connecting | Tree node shows steps (resolving, connecting, TLS, authenticating); cancel available. |
| Running | Run becomes Stop; elapsed time ticks once a second (no other frames); rows appear as they arrive. |
| Partial result | Bar "Showing the first 1,000 rows — scroll or [Fetch all]". |
| Error | Messages tab with server text, SQLSTATE or error number, position mapped to the editor. |
| Disconnected | Bar on each affected tab; staged edits kept; open transaction reported lost. |
| Offline | Local files and SQLite work; server connections show unreachable. |
| Locked keyring | Bar with unlock action; no secret prompts from the app itself. |
| Read-only | Status word, bar, disabled write actions with reasons. |
| Grant revoked | Connection closes at once; bar "Network access to host was revoked" with a link to request it again. |
| Narrow (<640) | Sidebar becomes an overlay; results dock becomes a tab beside the editor; inspector a sheet. |

### 5.7 Widgets

| Region | FUSE widget | Notes |
|---|---|---|
| Schema tree, plans, JSON tree, Parquet schema | `treeView` | Needs model-backed lazy trees with columns (today: tab-indented text, 64 collapsible rows; FUSE build plan 2.2; FUSE-WP-28) |
| SQL editor | `codeEdit` | Needs grammars, completion popup, multi-cursor, squiggles (FUSE build plan 2.2 lists these as missing; shared with OF01 and DV01) |
| Result and data grids | `tableView` + a typed remote model (**OF09-GAP-1**) | Needs cell and range selection, inline editors, frozen columns, 64-bit rows (FUSE-WP-28). The spreadsheet grid of OF05-GAP-1 would also serve |
| App-store lists (history, saved queries) | `tableView` / `modelListView` over `fuse_ui_sql::SqliteModel` | Direct use of the existing model on the app's own database |
| Charts | `chart` now; chart widget v2 (OF05-GAP-2) for pie, box, heat map, axes | |
| ER diagram, plan graph | 2D canvas (G-12); `nodeGraph` as a fallback for the plan graph | |
| Inspector | `tabWidget`, `propertyGrid`, `codeEdit` (JSON, text viewers) | |
| Dialogs and wizards | `dialog`, `lineEdit`, `comboBox`, `spinBox`, `checkBox`, `radioButton`, `progressBar` | Secret field pattern from DESIGN_SYSTEM.md section 5 |
| Date and time cell editors | `dateTimeEdit`, `calendar` | |
| Help | `markdownView` | |

What exists and where it stops (Verified, `FUSE:Source/FUSE/UI/Sql/include/fuse/ui/sql/sqlite_model.hpp`, `FUSE:Source/FUSE/UI/Sql/src/sqlite_model.cpp`, `FUSE:Source/FUSE/UI/CMakeLists.txt:98-111`, `FUSE:docs/ui/README.md`):

- `SqliteModel` has `open(path)`, `attach(sqlite3*)`, `exec`, `setQuery(select)`, `sortBy(col, ascending)`, `refresh`, `rows()`, `columns()`, `header(col)`, `cell(row, col)`, `model()`, `setWindow(rowsPerFetch)` (default 256), `lastError()`.
- It fetches windows with `LIMIT ?1 OFFSET ?2` around the query and takes the row count from `SELECT count(*)` over it.
- It is single-threaded on the UI thread; cells are strings; row counts are `u32`.
- The bundled SQLite is built with `SQLITE_THREADSAFE=0`, `SQLITE_OMIT_LOAD_EXTENSION=1`, `SQLITE_DQS=0`.

Consequences for this app:

| Limit | Effect | Plan |
|---|---|---|
| `open` takes a path | The app host has no path access; it receives handles from the portal | Use `attach()` with a connection opened through a descriptor-based VFS (Proposed), or run the model in a worker |
| Queries run on the UI thread | A slow query or a `count(*)` on a large table freezes the window | Worker-backed model (OF09-GAP-1) |
| `OFFSET` paging | Deep scroll cost grows with the offset | Keyset paging in the worker |
| String cells | NULL, empty text, numbers and BLOBs cannot be told apart or typed | Typed cell roles in the model |
| `u32` rows | Tables beyond 4.29 billion rows | 64-bit rows in the new model |
| Hostile database files parsed in the UI process | A corrupt file reaches SQLite inside the app host | User databases are opened in the `sqlite-worker` helper |

Gaps:

- **OF09-GAP-1 Typed, asynchronous, worker-backed item model** for `tableView`: windows requested by row range and delivered later; typed values (null, integer, real, text, blob, plus logical types); 64-bit row index; estimated and exact counts; change notifications; edit staging overlay. `SqliteModel` stays for small trusted stores.
- **OF09-GAP-2 Run-time `net.host` grants.** PLATFORM.md section 6 lists `net.host:<host>:<port>` as a manifest capability shown at install. This app learns its hosts from the user later. Needed: a way to declare "user-added named endpoints, each prompted" that is narrower than `net.user`. (OF05's data connections need the same.)
- **OF09-GAP-3 Handle-based SQLite VFS** as a platform library: open a database from a portal descriptor (and its journal and WAL siblings through a directory handle), for every app that opens user SQLite files under Capsicum.
- G-12 for the ER diagram; G-16 for print.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic | ForgeScript | app host | Tabs, connection manager, safety modes, statement splitter and classifier front, staging of edits, history, saved queries, jobs, settings |
| UI documents and styles | FUSE UI | app host | Window, docks, dialogs |
| App store access | FUSE `fuse_ui_sql::SqliteModel` + ForgeScript | app host | The app's own `app.sqlite` (history, saved queries, metadata cache) shown in lists |
| SQL language service | C++23 | helper `sql-lang` | Tokeniser, dialect-aware splitter, statement classifier, completion context parser, formatter. Justification: speed on keystrokes; parses untrusted text. No handles |
| SQLite worker | C++23 + SQLite (public domain) | helper `sqlite-worker`, one per open database | Opens user database files from passed descriptors with the hardening settings; runs queries; streams typed row windows; integrity and recovery tools. Justification: untrusted bytes |
| Network database client | C++23 | helper `db-net`, one per connection | PostgreSQL frontend/backend protocol 3.0 and MySQL/MariaDB client/server protocol; TLS; authentication; cancel; `COPY`; streams typed rows. Holds the secret only in its own memory. Has only the socket rights for its granted host. Justification: holds a secret, parses untrusted bytes |
| File engine | C++23 + SQLite virtual tables | helper `file-engine` | CSV, JSON, Parquet scanning as virtual tables; profiling; conversion; export writers (CSV, JSON, Parquet, ODS, XLSX, SQLite); diff engine. Justification: untrusted bytes, speed |
| Graph layout | C++23 | OF08's `vector-core` source built into `file-engine` | ER and plan layout |
| Script host | C++23 embedding the ForgeScript engine | helper `macro-host` | Signed formatter and exporter modules with no capabilities |

```
                +------------------------- app host --------------------------+
                | FUSE UI  <->  ForgeScript: tabs, modes, staging, history     |
                |   grids <- typed remote model (OF09-GAP-1)   app.sqlite via  |
                |                                              SqliteModel     |
                +---|-------------|----------------|----------------|----------+
        text only   |    fd of a  |      secret id |   file handles |
                    v    database v     + grant    v                v
              +----------+  +---------------+  +---------+   +-------------+
              | sql-lang |  | sqlite-worker |  | db-net  |   | file-engine |
              +----------+  +---------------+  +----|----+   +-------------+
               no handles    one file, no net       | TLS      files, no net
                                                    v
                                 granted host:port only (net.host), secret from svc.keyring
```

The secret path: the app host asks `svc.keyring` to deliver the item to the `db-net` helper's endpoint (Proposed keyring feature: "release to my helper"); if the keyring cannot do that, the app host receives the secret and forwards it once, zeroing its copy.

### 6.2 Data model

App store `~/.local/share/org.pbsd.data-studio/app.sqlite` (WAL, `schema_version` table, forward-only migration with a copy kept):

| Table | Columns |
|---|---|
| `connection` | `id TEXT PRIMARY KEY` (uuid), `name TEXT`, `kind TEXT` (`sqlite`, `postgres`, `mysql`, `mariadb`, `files`, `memory`), `group_id TEXT`, `env_tag TEXT`, `colour TEXT`, `host TEXT`, `port INTEGER`, `database TEXT`, `username TEXT`, `secret_ref TEXT` (keyring item id), `secret_policy TEXT` (`keyring`, `ask`), `tls_mode TEXT`, `tls_ca_token TEXT`, `tls_pin_sha256 BLOB`, `client_cert_ref TEXT`, `route TEXT`, `tunnel_json TEXT`, `file_token TEXT` (portal token), `default_mode TEXT`, `session_json TEXT`, `record_history INTEGER`, `created INTEGER`, `last_used INTEGER` |
| `connection_group` | `id`, `name`, `parent_id`, `position` |
| `saved_query` | `id`, `name`, `description`, `folder_id`, `tags TEXT`, `dialect`, `connection_id`, `sql TEXT`, `params_json`, `chart_json`, `created`, `modified` |
| `history` | `id INTEGER PRIMARY KEY`, `time INTEGER`, `connection_id`, `env_tag`, `statement TEXT` (after redaction), `class TEXT`, `duration_ms INTEGER`, `rows INTEGER`, `outcome TEXT`, `error TEXT`, `plan_ref TEXT` |
| `history_fts` | FTS5 index over `history.statement` |
| `snippet` | `id`, `name`, `dialect`, `body`, `builtin INTEGER` |
| `workspace` / `tab` | `workspace(id, name, layout_json, active)`; `tab(id, workspace_id, kind, connection_id, title, buffer_ref, state_json, position)` |
| `meta_object` | `connection_id`, `kind`, `schema`, `name`, `parent`, `ddl TEXT`, `comment`, `extra_json`, `fetched INTEGER` |
| `meta_column` | `connection_id`, `schema`, `table_name`, `ordinal`, `name`, `type`, `nullable`, `default_expr`, `key_role`, `fk_target`, `comment` |
| `er_layout` | `connection_id`, `schema`, `table_name`, `x`, `y`, `collapsed` |
| `import_profile` | `id`, `name`, `source_kind`, `config_json` |
| `redaction_rule` | `id`, `pattern`, `replacement`, `enabled` |
| `job` | `id`, `kind`, `state`, `progress`, `started`, `finished`, `summary`, `log_ref` |

Other stores: unsaved buffers and staged edits in `~/.local/share/org.pbsd.data-studio/recovery/` (journal records with a BLAKE3 chain); result spill files and file indexes in `~/.cache/org.pbsd.data-studio/` (columnar chunks, deleted on tab close and at start); secrets only in `svc.keyring`.

In-memory entities: `Session{id, connection_id, mode, in_transaction, started, server_version, pid}`; `ResultSet{id, columns: [Column{name, db_type, logical_type, nullable?}], row_count_known: u64?, fetched: u64, complete: bool, spill: FileRef?}`; `StagedEdit{table, key: [Value], kind: enum{update, insert, delete}, old: map, new: map}`; `Value` = tagged union `Null | Bool | Int64 | UInt64 | Float64 | Decimal{text} | Text | Bytes | Date | Time | Timestamp{micros, tz?} | Interval | Uuid | Json | Array | Other{type_name, text}`.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| SQLite database file | yes | yes | SQLite (public domain), vendored, built with extension loading omitted, defensive configuration | `sqlite-worker` |
| PostgreSQL protocol 3.0 (simple and extended query, `COPY`, cancel, SCRAM-SHA-256) | yes | yes | Own client in C++23. libpq (PostgreSQL Licence) is not used: C code outside the safe subset, and it reads environment variables and files such as `.pgpass` by itself | `db-net` |
| MySQL / MariaDB client/server protocol (text and binary result sets, prepared statements) | yes | yes | Own client in C++23. MariaDB Connector/C (LGPL-2.1) is not used for the same reasons | `db-net` |
| TLS 1.2 / 1.3 | yes | yes | Platform TLS library (which one is a platform decision; Unverified) | `db-net` |
| SSH (tunnel) | yes | yes | Platform SSH facility through SE04's agent (Unverified); else not built | `db-net` |
| CSV, TSV | yes | yes | Own code (shared source with OF05) | `file-engine` |
| JSON, JSON Lines | yes | yes | Own code | `file-engine` |
| Parquet | yes | yes | Own reader and writer (shared source with OF05); Zstandard (BSD-3-Clause) and own Snappy and gzip codecs | `file-engine` |
| gzip, Zstandard streams | yes | yes | zlib (zlib licence), Zstandard | `file-engine` |
| ODS, XLSX (values only) | no | yes | Own writers (shared source with OF05) | `file-engine` |
| SQL text | yes | yes | Own tokeniser, splitter, classifier, formatter | `sql-lang` |
| SVG, PNG, PDF (diagrams, charts) | no | yes | Own SVG writer; platform image and print pipelines | app host / spooler |
| ODG (ER diagram to Draw) | no | yes | Own writer (shared source with OF08) | `file-engine` |
| Helper IPC `org.pbsd.data-studio.rows/1` | — | — | Bus schema (G-08): typed row windows in shared memory | — |

### 6.4 Integration

- Intents handled: `open` and `view` for SQLite, Parquet, CSV, JSON and `.sql` files; `new`. Emitted: `share`, `print`, `open` (send to OF05), `edit` (ER diagram to OF08, BLOB to DV04).
- MIME view/edit: `application/vnd.sqlite3`, `application/sql`. View: `application/vnd.apache.parquet`, `text/csv`, `text/tab-separated-values`, `application/json`, `application/x-ndjson`. Export: those plus `application/vnd.oasis.opendocument.spreadsheet`, `application/vnd.openxmlformats-officedocument.spreadsheetml.sheet`, `text/markdown`, `text/html`, `image/svg+xml`, `image/png`, `application/pdf`. Sheets (OF05) stays the default handler for `text/csv`; Data Studio is offered in "Open with".
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search` (saved query names only), `svc.keyring`, `svc.clipboard`, `svc.audit`, `svc.a11y`, `svc.backup`, `svc.power`, `svc.model` (optional).
- Other apps: OF05 (results hand-off; shared CSV and Parquet code), OF08 (ER diagram editing; shared layout code), SE04 (client certificates, SSH keys), AD05 (network grants per host), SE02 (the firewall shows the same endpoints), SE03 (routes), DV04 (BLOBs), AD04 (reads the audit lines), SY02 (opens files).
- Backing repos. Verified: the `SqliteModel` facts in 5.7. Verified: `tableView` has virtualised rows, header-click sort, column resize and single-row selection only (FUSE build plan 2.2 citing `widgets.hpp:125-132,151-159,171-178`). Verified: `vendor/sqlite` is used by `fuse_ui_sql` (`FUSE:Source/FUSE/UI/CMakeLists.txt:100-105`). Verified: FORGE has an OS layer with a keystore and IPC (PLATFORM.md V18); its API was not read for this spec. Proposed: all helpers and protocols above.

## 7. Security and privacy

| Capability | Why |
|---|---|
| `fs.app` | App store, recovery, cache |
| `fs.portal` | Database files, data files, SQL files, exports, CA certificates |
| `net.none` | Default. The app host itself never opens a socket |
| `net.host:<host>:<port>` (one per server connection, requested at run time; OF09-GAP-2) | Database servers and, for tunnels, SSH hosts the user added. Used only by `db-net` |
| `net.route:<clear\|vpn\|tor>` (optional) | Bind a connection to a route |
| `proc.helper:sql-lang`, `proc.helper:sqlite-worker`, `proc.helper:db-net`, `proc.helper:file-engine`, `proc.helper:macro-host` | Helpers of 6.1 |
| `dev.print` | Printing |
| `svc.keyring` | Connection secrets |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.search`, `svc.clipboard`, `svc.audit`, `svc.a11y`, `svc.backup`, `svc.power` | As in 6.4 |
| `ai.local`, `svc.model` (optional, off) | Explain and draft SQL |

Assets: database credentials; the data in the databases (often the most sensitive data a person or organisation has); the integrity of production data; query history (reveals schema and sometimes values); the user's files.

Adversaries: a crafted SQLite, Parquet, CSV or JSON file; a malicious or compromised database server (hostile protocol messages, oversized fields, a request for client files, credential downgrade); a network attacker between client and server; a pasted SQL script that does more than it seems; the user's own slip on production; another local app wanting the secrets or history.

Mitigations:

| Threat | Mitigation |
|---|---|
| Accidental writes on production | Read-only default at server session and client classifier; timed, typed, audited exit; dangerous-statement guard; reviewed commits; visible environment word and stripe |
| Classifier bypass (function with side effects in a `SELECT`) | Server-side read-only transaction is the backstop; the UI says a read-only database role is the only complete protection and shows the connected role's write rights where they can be read |
| Credential theft | Secrets only in `svc.keyring`; held only by `db-net`; never in definitions, history, logs, exports or crash data; secret fields follow the system pattern |
| Downgrade and interception | TLS with verification by default; weak authentication methods warned or refused; no cleartext password without verified TLS; pinning option |
| Hostile server | Own protocol parser with length checks, in a helper with one socket and no file access; `LOAD DATA LOCAL` never honoured; field and message size caps (1 GiB hard, 64 MiB default per field); result spill limits |
| Hostile files | Parsed only in `sqlite-worker` and `file-engine` in capability mode; SQLite defensive mode, `trusted_schema` off, no extensions, limits, progress-handler time-out; disposable open |
| Hostile SQL text | Never run without a user action; history and saved queries open in a tab, never auto-run; no URL scheme is registered |
| Exfiltration | The only network reach is the granted hosts; grants listed and revocable; exports go through the file portal |
| History leakage | Parameter values not stored; redaction rules; per-connection "no history"; history not indexed by system search; local store only |
| Formula injection in exports | CSV, ODS and XLSX exports escape leading `= + - @` by default |
| Shoulder surfing and screen sharing | Masked sensitive columns; secret fields hidden |
| Model misuse | Model sees schema names and typed text only; never rows; never runs SQL |

Stored: connection definitions (no secrets), history, saved queries, metadata cache, layouts in `app.sqlite`; recovery journal; cache spill files (deleted on close). Leaves the machine: only traffic to granted database hosts over TLS, started by user action; application name sent to servers is the fixed string `pbsd-data-studio` with no user, host or version detail.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `safety.default_mode.production` | enum | `read_only` | |
| `safety.default_mode.staging` | enum | `manual_commit` | |
| `safety.default_mode.other` | enum | `confirm_writes` | |
| `safety.leave_readonly_minutes` | int | `15` | |
| `safety.guard_dangerous` | bool | `true` | Cannot be turned off for production |
| `safety.sqlite_external_readonly` | bool | `true` | Files with outside provenance open read-only |
| `safety.backup_before_write` | bool | `true` | SQLite files |
| `connect.tls_default` | enum | `verify_full` | |
| `connect.statement_timeout_s.production` | int | `60` | |
| `connect.idle_disconnect_min` | int | `15` | |
| `connect.auto_reconnect_readonly` | bool | `true` | |
| `editor.dialect_default` | enum | `sqlite` | |
| `editor.completion` | bool | `true` | |
| `editor.format.keyword_case` | enum | `upper` | |
| `editor.format.indent` | int | `2` | |
| `editor.run_on_ctrl_enter` | enum | `statement` | `statement`, `selection_or_statement` |
| `result.window_rows` | int | `1000` | |
| `result.memory_mib` | int | `512` | Before spilling to cache |
| `result.null_text` | text | `NULL` | Display only |
| `result.date_format` | enum | `iso` | `iso`, `locale` |
| `result.mask_patterns` | list | `password, passwd, secret, token, api_key, ssn` | Column-name patterns |
| `export.csv.escape_formulas` | bool | `true` | |
| `export.csv.encoding` | enum | `utf-8` | |
| `history.enabled` | bool | `true` | |
| `history.keep_days` / `history.keep_entries` | int | `90` / `50000` | |
| `history.store_parameters` | bool | `false` | |
| `history.production_default` | enum | `ask` | `record`, `no_record`, `ask` when a production connection is created |
| `files.csv.safe_typing` | bool | `true` | Ambiguous values stay text |
| `files.cache_mib` | int | `2048` | Cache directory budget |
| `diff.numeric_epsilon` | float | `0` | |
| `ai.sql_help` | bool | `false` | Needs `ai.local` |
| `view.density` | enum | `compact` | |

## 9. Accessibility and languages

- Grids expose table, row, column header and cell roles with the type and state in the name ("revenue, number, 188,204.50", "email, null", "changed, was bills@…"). Needs the table roles missing from today's a11y model (FUSE build plan 2.2).
- The environment tag is a word in the header and in the status bar, announced on tab change ("production, read-only").
- Plans, ER diagrams and charts each have a text or table equivalent (plan tree table; ER as a list of tables and relations; chart data table).
- Staged-change marks are characters (`+`, `−`, `~`) with colour, never colour alone.
- SQL keywords are not translated; UI strings are. Data is shown as stored: no locale conversion of numbers in cells unless the display option is on; bidi text in cells is isolated per cell (G-03).
- Every drag action has a key route (copy name to editor, move ER box by arrows, reorder columns from the header menu).

## 10. Performance budgets

| Measure | Target |
|---|---|
| Cold start to start view | 400 ms |
| Open a SQLite file and list tables | 150 ms after the file handle arrives (10,000 tables: 500 ms) |
| First rows of a table view | 100 ms local, network round trip plus 50 ms remote |
| Scroll in a 100-million-row keyed table | each window under 50 ms (keyset paging) |
| Result streaming | 1 million rows × 10 columns received and spilled in under 5 s on a LAN |
| Grid frame time | under 8 ms |
| Completion popup | under 30 ms from keystroke, from the cache |
| CSV scan in the file engine | 150 MB/s; Parquet column scan 1 GB/s of decoded data on 8 cores |
| Cancel | server acknowledges within 500 ms; UI responsive at all times |
| Memory | app host under 120 MiB with 10 tabs; each helper under 64 MiB idle |
| Idle | zero frames; no polling of servers; keep-alive only if set per connection |

## 11. Testing and proof

Test plan (targets; nothing has run):

- **Unit**: statement splitter and classifier per dialect (large accept and class tables, including dollar quotes, nested comments, `DELIMITER`, CTEs with data-modifying parts, `SELECT … INTO`, `EXPLAIN ANALYZE`), type mapping tables, staged-edit to SQL generation, redaction rules, keyset paging query builder.
- **Protocol tests**: `db-net` against real PostgreSQL and MariaDB/MySQL servers of several versions in jails (test infrastructure only); recorded-session replay for regression; every authentication method; cancel; large fields; `COPY`.
- **Hostile server tests**: a scripted fake server sends malformed lengths, endless rows, wrong sequence numbers, authentication downgrade requests and `LOCAL INFILE` requests; the helper must fail closed.
- **Safety tests**: on a read-only connection every write-class and unknown-class statement in a corpus is refused before sending, and when forced through a test hook is rejected by the server.
- **Differential**: file-engine query results on CSV and Parquet compared with the same data loaded into SQLite tables; Parquet reader compared with reference files from the Apache `parquet-testing` data set (test data only).
- **Fuzz**: PostgreSQL and MySQL message parsers, SQL tokeniser and splitter, CSV, JSON, Parquet footer and pages, SQLite files through the worker (with SQLite's own fuzz corpus as seeds), IPC row windows, journal.
- **Golden renders**: grid with every type and NULL, staged marks, plan tree, ER diagram, dialogs, production chrome, at two scales and three themes.
- **Scripted UI**: the 15 top tasks through the a11y tree against a local SQLite file and a test PostgreSQL.
- **Performance**: section 10 budgets with fixed data sets (TPC-H-shaped generated data at scale 1 and 10 as files and tables).

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Statement classifier | Never classes a writing statement of the corpus grammar as read; unknown is treated as write | T1 for the decision function over the token grammar it defines | PRISM proof that the function returns `read` only on the listed productions; corpus tests | Server-side semantics (functions with side effects) are outside any client proof |
| Read-only gate | No statement reaches the socket in read-only mode unless classed `read` or a fixed session command | T1 | Proof over the send path state machine | — |
| PostgreSQL and MySQL message parsers | No runtime errors on any byte stream; bounded allocation | T2 | PRISM plus fuzzing | — |
| SCRAM-SHA-256 client | Follows RFC 5802 and 7677 message flow; rejects a wrong server signature | T2, T1 for the state machine | PRISM; test vectors from the RFCs | Crypto primitives are platform code |
| Staged-edit SQL generator | Generated statements touch only keyed rows; all values bound | T1 | Proof over the generator: no value reaches statement text | — |
| CSV, JSON, Parquet readers | No runtime errors | T2 | Shared with OF05; PRISM plus fuzzing | — |
| SQLite (third party) | — | T4 | Confinement in the worker, hardening settings | Not our code (SQLite's own testing is extensive but is not a PRISM result) |
| TLS library (third party) | — | T4 | Confinement in `db-net` | Platform choice |
| Diff engine | Reports every differing key for sorted inputs | T1 for the merge core | Proof of the merge invariant | Hash-bucket prefilter is T3 |
| App logic (ForgeScript) | Memory safe by the language | T3 | Unit and UI tests | Interpreter is trusted base |

## 12. Build plan

Phase 1 — usable core (SQLite first):

| ID | Work item | Depends on |
|---|---|---|
| OF09-W01 | App store schema; lists over `SqliteModel` (history, saved queries) | G-07, G-02 |
| OF09-W02 | `sqlite-worker` helper with hardening, descriptor-based open, typed row windows | G-06, OF09-GAP-3 |
| OF09-W03 | Typed asynchronous item model and grid (OF09-GAP-1) | FUSE-WP-13, FUSE-WP-28 |
| OF09-W04 | Main window, connection tree for SQLite, table details, DDL, SQLite file information | OF09-W02, OF09-W03 |
| OF09-W05 | `sql-lang` helper: tokeniser, splitter, classifier, highlighting data | G-08 |
| OF09-W06 | SQL editor tabs, run, cancel, multiple results, messages, row limit | OF09-W05, code editor work shared with OF01 |
| OF09-W07 | Table data view with filter and sort bar (read-only) | OF09-W03 |
| OF09-W08 | Result copy formats, find, export to CSV, JSON, SQL | OF09-W03 |
| OF09-W09 | Safety modes, dangerous-statement guard, environment chrome | OF09-W05 |
| OF09-W10 | History, saved queries, quick open, generate SQL, recovery | OF09-W01 |
| OF09-W11 | Trust bar, external-file read-only default, palette, help | G-09 |

Phase 2 — servers, editing, files:

| ID | Work item | Depends on |
|---|---|---|
| OF09-W12 | Run-time `net.host` grants and the connection wizard | OF09-GAP-2, AD05, SH04 |
| OF09-W13 | `db-net`: PostgreSQL client, TLS, SCRAM, cancel, cursors | Platform TLS decision, G-06 |
| OF09-W14 | `db-net`: MySQL and MariaDB client | OF09-W13 |
| OF09-W15 | Keyring integration, secret release to helper, idle lock | G-09, SE04 |
| OF09-W16 | Server schema browser, metadata cache, completion | OF09-W13, OF09-W05 |
| OF09-W17 | Inline editing, staged changes, reviewed commit, record view, cell viewers | OF09-W07 |
| OF09-W18 | Execution plans (text, tree), parameters, formatter, diagnostics, transactions | OF09-W06 |
| OF09-W19 | `file-engine`: CSV, JSON, Parquet virtual tables; file workspace; Parquet inspector; JSON explorer | Shared CSV and Parquet sources with OF05 |
| OF09-W20 | Import wizard, bulk load paths, export to Parquet, ODS, XLSX | OF09-W19 |
| OF09-W21 | Charts on results; column statistics; masking | OF05 chart widget (OF05-GAP-2) |
| OF09-W22 | ER diagram with layout and export | G-12, OF08 layout code |
| OF09-W23 | Audit lines, net badge details, jobs, leave-read-only flow, history privacy | G-09 |

Phase 3 — advanced:

| ID | Work item | Depends on |
|---|---|---|
| OF09-W24 | Data diff, file diff, sync script | OF09-W19 |
| OF09-W25 | Schema diff and migration script | OF09-W16 |
| OF09-W26 | Table designer, maintenance actions, SQLite recovery tools | OF09-W17 |
| OF09-W27 | Profile, convert, copy between connections, compressed inputs | OF09-W20 |
| OF09-W28 | SSH tunnel, routes | SE04, SE03 |
| OF09-W29 | Plan graph and compare, query builder, related rows, inferred relations | OF09-W18, OF09-W22 |
| OF09-W30 | Server information, roles view, data dictionary | OF09-W16 |
| OF09-W31 | SQL help with `svc.model`; scripted formatters and exporters | G-15, FORGE signing |

## 13. Open questions and risks

- **Run-time host grants (OF09-GAP-2).** Without a platform answer the app must either declare `net.user` (too broad for a tool that touches production) or cannot reach servers at all. This is the main platform dependency.
- **Own protocol clients.** Writing PostgreSQL and MySQL clients is bounded work but must track server versions and authentication changes. The alternative (libpq, Connector/C in the helper) is less work and puts large C libraries, with their environment and file reading, into the process that holds the secret.
- **TLS and SSH libraries** are platform decisions not read here. `db-net` cannot be built before them.
- **Classifier limits.** A client cannot know whether a function call writes. The spec is honest about it in the UI; real safety on production needs a read-only database role, which the app encourages in the wizard.
- **`SqliteModel` path.** It serves the app's own lists as it is. For user databases it needs descriptor-based open and must not run on the UI thread; whether FUSE extends it or the app supplies a new model is undecided (OF09-GAP-1).
- **SQLite as the file query engine.** Row-at-a-time virtual tables will be slower than a vectorised engine on wide aggregations. A dedicated columnar executor could come later; DuckDB (MIT) would fit the purpose but is a large C++ code base outside the safe subset, so it is not proposed.
- **Parquet nested types** are shown in the inspector but only flat and simple-list columns are queryable at first.
- **Overlap with OF05** on CSV and Parquet import is deliberate; the parsing sources are shared so behaviour matches.
- **Secret release to a helper** needs a keyring feature that may not exist; the fallback passes the secret through the app host once.
- **Encrypted SQLite variants** (SQLCipher and the like) are common in app stores; not supported, and adding one means another third-party cipher layer.
