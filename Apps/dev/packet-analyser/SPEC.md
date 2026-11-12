<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Packet Analyser

By Odin Loch

| | |
|---|---|
| ID | `DV05` (from CATALOGUE.md) |
| Bundle | `org.pbsd.packet-analyser` |
| Category | dev |
| Priority | P2 |
| Backing repos | none (FORGE and FUSE as platform) |
| Status | Planned. Nothing built. |

## 1. Purpose

Captures and dissects network traffic. Developers debug their protocols with it, administrators find faults, and security staff inspect what the machine sends and receives. It reads and writes pcap and pcapng, filters, follows streams, draws statistics and exports objects. It replaces Wireshark from the Plasma plan (PLATFORM.md V20).

Two things are specific to this OS. First, every packet of local traffic can be attributed to the app that owns its socket, because sockets belong to capability domains with an app identity. Second, the parts are split by risk: capture is a tiny privileged service that does not parse, and dissectors, which parse hostile bytes, run in a sandbox with no capabilities at all.

## 2. Scope

In scope: live capture on local interfaces through the capture service; capture and display filters in a language defined here; pcap and pcapng read and write; dissectors by layer; flows and conversations; stream follow; statistics and graphs; TLS decryption with key logs; offline name resolution; object export; colour rules; expert warnings; per-app attribution; firewall rule creation; remote capture over SSH; scrubbing and anonymising captures before sharing.

Out of scope:

| Item | Owner |
|---|---|
| Blocking, allowing and shaping traffic; pf rule management | SE02 Firewall |
| Live per-app connection list and byte counters without capture | SE02 Firewall, SY04 System Monitor |
| Intrusion detection and alerting | SE01 Security Centre |
| Raw byte editing of payloads | DV04 Hex Editor (hand-off) |
| Decompiling extracted executables | DV07 Reverse Engineering (hand-off) |
| VPN and anonymous routing control | SE03 Anonymity |
| Active scanning, packet injection, replay onto the wire | not planned |
| Wi-Fi monitor-mode control and channel hopping | VM05 Devices (radio control); this app only reads frames if the interface is already in that mode |
| Key and certificate storage | SE04 Passwords and Keys |

## 3. Users and top tasks

Personas:

- **App developer**: sees only their own app's traffic, checks requests and TLS behaviour.
- **Network administrator**: diagnoses loss, retransmission, DNS and DHCP faults.
- **Security analyst**: asks "which app sent this?", inspects unknown flows, exports evidence.
- **Protocol author**: writes a dissector for a new protocol and tests it on captures.
- **Support helper**: receives a scrubbed capture from someone else.

| # | Task | Target steps |
|---|---|---|
| T1 | Start a capture on the default interface | 1 (`Ctrl+E`), plus the system prompt the first time |
| T2 | Capture only one app's traffic | 2 (pick the app, Start) |
| T3 | Open a capture file | 1 |
| T4 | Filter to one conversation | 1 (right click, Conversation filter) |
| T5 | Follow a TCP stream as text | 1 (`Ctrl+Alt+Shift+T`) |
| T6 | See which app sent the selected packet | 0 (App column) |
| T7 | Find retransmissions and resets | 1 (expert panel) |
| T8 | Decrypt TLS with a key log | 2 (add key log, reload) |
| T9 | Export the files transferred over HTTP | 2 |
| T10 | Create a firewall rule that blocks the selected flow | 2 (context menu, confirm in SE02) |
| T11 | Draw throughput over time for a filter | 2 |
| T12 | Scrub addresses and payloads, then save for sharing | 3 |
| T13 | Capture on a remote host over SSH | 3 |
| T14 | Stop and save | 2 (`Ctrl+E`, `Ctrl+S`) |

## 4. Features

Priority: M must, S should, C could. Phase 1, 2, 3 as in section 12.

### 4.1 Live capture

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F001 | Capture service | Capture is done by a small system service (section 6.1, DV05-GAP-1). It opens the BPF device, installs a verified filter, locks the descriptor and copies packets into a shared-memory ring. It never parses a packet. This app holds no raw-socket or BPF right. | M | 1 |
| DV05-F002 | Capture grant | The first capture on an interface raises a shell prompt: which interface, whose traffic (this app's, a named app's, this user's apps, everything on the interface), for how long (once, this session, always). "Everything" needs an administrator. Grants are listed and revocable in AD05. | M | 1 |
| DV05-F003 | Interface list | Name, description, link type, addresses, link state, a 30 s sparkline of packet rate (from the service's counters, no capture needed), whether a grant exists. Includes loopback, `pflog0`, tunnel and bridge interfaces. | M | 1 |
| DV05-F004 | Start, stop, restart | `Ctrl+E` toggles; `Ctrl+R` restarts (discarding or keeping packets by choice). Several interfaces at once, each a pcapng interface block. | M | 1 |
| DV05-F005 | Capture filter | Entered in the filter language (section 6.3), compiled to a BPF program by the filter compiler, verified, then handed to the service. The compiled program is shown on request. | M | 1 |
| DV05-F006 | Scope by app | "Only these apps" is enforced in the capture service against the socket-owner table (4.11), not by a display filter, so other apps' packets never reach this app. | M | 2 |
| DV05-F007 | Snapshot length | Per interface: full packet or first N bytes (default 262 144; "headers only" presets 96 and 128). | M | 1 |
| DV05-F008 | Promiscuous mode | Off by default; turning it on is part of the grant prompt because it shows other machines' traffic. | M | 1 |
| DV05-F009 | Ring and loss | Ring size setting (default 64 MiB). Kernel and ring drops are counted, shown in the status bar and recorded in the file's interface statistics block. | M | 1 |
| DV05-F010 | Stop conditions | After N packets, N bytes, a duration, or when a display-filter expression first matches (trigger). | S | 2 |
| DV05-F011 | Ring-buffer files | Write to a set of files rotated by size or time, keeping the last N; for long captures. | S | 2 |
| DV05-F012 | Capture to file only | No live dissection, lowest overhead; packets are written by the capture store helper and dissected when the file is opened. | S | 2 |
| DV05-F013 | Live update | The packet list grows while capturing; auto-scroll toggle (`Ctrl+Shift+L`); dissection keeps up or shows "n packets behind". | M | 1 |
| DV05-F014 | Timestamps | Source and precision from BPF (nanosecond where the kernel gives it); clock noted in the interface block. | M | 1 |
| DV05-F015 | Direction | Inbound or outbound per packet where BPF reports it; a column and filter field `frame.dir`. | S | 1 |
| DV05-F016 | Firewall log capture | Capturing on `pflog0` gives each logged packet its pf rule number, action and reason (4.12). | S | 2 |
| DV05-F017 | Capture profiles | Named sets of interfaces, filter, snapshot length, stop conditions, app scope. | S | 2 |
| DV05-F018 | Background capture | With `bg.run` granted: capture continues with the window closed; a notification and a system indicator are shown the whole time. Off by default. | C | 3 |
| DV05-F019 | Capture indicator | While any capture runs, the shell shows a system-wide indicator (as for camera and microphone) naming this app and the interface. | M | 1 |
| DV05-F020 | Other link types | USB (`usbdump` format) and Bluetooth HCI through the same service; 802.11 with radiotap when the interface already delivers it. | C | 3 |
| DV05-F021 | Wireless details | For radiotap frames: channel, signal, noise, rate columns. No monitor-mode control. | C | 3 |

### 4.2 Files

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F030 | Read pcapng | Section header, interface description, enhanced packet, simple packet, name resolution, interface statistics, decryption secrets, custom blocks; several sections and interfaces; both byte orders; unknown blocks kept and written back unchanged. | M | 1 |
| DV05-F031 | Read pcap | Classic format, microsecond and nanosecond magic, both byte orders. | M | 1 |
| DV05-F032 | Write pcapng and pcap | Default save format is pcapng. Saving to pcap warns about what is lost (several interfaces, comments, attribution, secrets). | M | 1 |
| DV05-F033 | Compressed files | Read and write gzip and zstd capture files by streaming; decompression in the archive helper of UT01 with an output cap. | S | 2 |
| DV05-F034 | Huge files | Files are indexed (offset, length, timestamp per packet) by the capture store; packets are dissected lazily. Supported: 2^32 packets or 1 TiB per file. | M | 1 |
| DV05-F035 | Save subsets | All, displayed, selected, marked, a range; with or without dependent packets (the rest of a reassembled stream). | M | 1 |
| DV05-F036 | Merge | Merge several files by timestamp or append; interface blocks are kept apart. | S | 2 |
| DV05-F037 | Split | By packet count, size, time interval or flow. | S | 3 |
| DV05-F038 | Time shift and edit | Shift all or selected timestamps by a value; set a reference packet. Changes the saved copy only. | S | 2 |
| DV05-F039 | Comments | Per file, per interface and per packet, stored as pcapng comment options; a comments panel. | S | 2 |
| DV05-F040 | File properties | Format, size, packet count, time span, interfaces, link types, drops, hash (SHA-256, BLAKE3), capture application and OS fields, whether scrubbed. | M | 1 |
| DV05-F041 | Other imports | Hex dump text (classic offset layout) to packets with a chosen link type; `snoop`; ERF. | C | 3 |
| DV05-F042 | Open by intent and recent | `open` for capture MIME types; recent list of 30 with "missing" flag. | M | 1 |
| DV05-F043 | Provenance | Captures received from elsewhere open like any other: dissection is always sandboxed. Exported objects inherit an "untrusted, from capture" tag. | M | 2 |
| DV05-F044 | Autosave of live captures | A live capture is always backed by a temporary pcapng in the cache directory; after a crash the app offers to recover it. Temporary captures are deleted on clean exit unless saved. | M | 1 |

### 4.3 Packet list, detail and bytes

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F050 | Packet list | Columns: number, time, source, destination, protocol, length, app, info. Virtualised; any field can be a column; sort; resize; column presets. | M | 1 |
| DV05-F051 | Time formats | Absolute (local or UTC), since start, since previous captured, since previous displayed, since reference; precision seconds to nanoseconds. | M | 1 |
| DV05-F052 | Detail tree | Protocol layers as a tree of fields with value, raw bytes range and a one-line summary; expand state remembered per protocol. | M | 1 |
| DV05-F053 | Bytes pane | Hex view (FUSE-WP-41) of the frame; selecting a field highlights its bytes and the reverse; tabs for reassembled and decrypted data. | M | 1 |
| DV05-F054 | Generated fields | Fields computed by analysis (stream index, sequence analysis, response time, checksum status) are shown in brackets and are filterable. | M | 1 |
| DV05-F055 | Apply as filter, column, colour | From any field: "Filter: selected", "not selected", "and", "or"; "Add as column"; "Colour by this". | M | 1 |
| DV05-F056 | Mark, ignore | Mark packets (`Ctrl+M`); ignore packets (excluded from dissection state); next and previous mark `Ctrl+Shift+N/B`. | M | 1 |
| DV05-F057 | Go to | Packet number (`Ctrl+L`), time, the request for this response, the first packet of the stream, linked packets (double click a link field). | M | 1 |
| DV05-F058 | Related packets | The list gutter draws brackets linking request and response, segments of a reassembled message, and duplicate ACKs. | S | 2 |
| DV05-F059 | Find packet | By display filter, hex bytes, string (in info, in detail text, in bytes; encodings), regex; `Ctrl+F`, next `Ctrl+G`. | M | 1 |
| DV05-F060 | Copy | Summary line, field value, field as filter, bytes as hex, as text, as a C array; whole detail tree as text or JSON. | M | 1 |
| DV05-F061 | Checksums | Validation of IP, TCP, UDP, ICMP checksums is optional per protocol (offloading makes outbound checksums look wrong); bad ones raise a note, not an error, by default. | M | 1 |
| DV05-F062 | Decode as | Force a dissector for a port, stream or payload type for this session or as a saved rule. | M | 2 |
| DV05-F063 | Layout | Three panes stacked or side by side; any pane hidden; packet in its own window. | S | 1 |
| DV05-F064 | Open bytes in Hex Editor | Sends the frame, a field or a reassembled payload to DV04 as a read-only buffer. | S | 2 |
| DV05-F065 | Packet diagram | A classic header-box drawing of the selected packet's layers with field widths. | C | 3 |

### 4.4 Filters

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F070 | Display filter bar | One language for display and capture (section 6.3). Completion of fields, values and enum names; syntax colours; green when valid, red with the parser message when not; applied on `Enter`. | M | 1 |
| DV05-F071 | Filter fields | Every dissected field has a typed name (`ip.src`, `tcp.flags.syn`, `dns.qry.name`, `app.id`). A field browser lists them with type and description. | M | 1 |
| DV05-F072 | Saved filters and buttons | Named filters; chosen ones become buttons beside the bar. | S | 1 |
| DV05-F073 | Filter history | Last 50. | S | 1 |
| DV05-F074 | Conversation filters | One click builds the filter for the selected packet's Ethernet, IP, TCP, UDP or app conversation. | M | 1 |
| DV05-F075 | Filter builder | A form for people who do not know the syntax: field, operator, value rows combined with all or any; shows the resulting text. | S | 2 |
| DV05-F076 | Capture-filter check | The bar says whether the expression can be compiled to BPF (capture filter) or only evaluated after dissection (display filter), and why. | M | 1 |
| DV05-F077 | Macros | Named sub-expressions with parameters (`$private_net(ip.addr)`). | C | 3 |
| DV05-F078 | Filter speed | Filters compile to a small bytecode evaluated over each packet's field table in the dissect helper; a 1 million packet refilter meets the budget in section 10. | M | 1 |

### 4.5 Dissectors

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F080 | Dissection sandbox | All dissectors run in the dissect helper: no file, network, device or process capability; it receives packet bytes through a read-only shared-memory window and returns field tables. A crash loses nothing but the current batch; the helper restarts and the offending packet is marked "dissector failed". | M | 1 |
| DV05-F081 | Dissector framework | Own code in safe-subset C++23. Each dissector declares its fields (name, type, display base, description), its parent bindings (EtherType, IP protocol, port, heuristic) and a `dissect(span, context) -> consumed` function over a bounds-checked reader. | M | 1 |
| DV05-F082 | Link layer, phase 1 | Ethernet II, 802.1Q and 802.1ad VLAN, LLC and SNAP, ARP, BSD loopback (`DLT_NULL`), raw IP, pflog. | M | 1 |
| DV05-F083 | Network layer, phase 1 | IPv4 with fragment reassembly, IPv6 with extension headers and fragments, ICMP, ICMPv6 with neighbour discovery, IGMP, MLD. | M | 1 |
| DV05-F084 | Transport layer, phase 1 | TCP with stream reassembly and sequence analysis (retransmission, fast retransmission, out of order, duplicate ACK, zero window, window update, keep-alive, lost segment, reset), options (MSS, SACK, timestamps, window scale, fast open); UDP. | M | 1 |
| DV05-F085 | Application layer, phase 1 | DNS (and mDNS, LLMNR), DHCPv4, DHCPv6, HTTP/1.x with chunked and content decoding, TLS records and handshake (1.2 and 1.3, extensions, certificates summary), NTP, SSH (banner, key exchange messages), syslog. | M | 1 |
| DV05-F086 | Link and tunnels, phase 2 | LLDP, STP, PPP, PPPoE, Linux cooked capture (for imported files), EAPOL, 802.11 with radiotap, IPsec `enc` link type, GRE, IP-in-IP, VXLAN, Geneve, MPLS, ESP and AH headers, WireGuard message headers, OpenVPN headers, IKEv2. | S | 2 |
| DV05-F087 | Transport, phase 2 | QUIC (header, version negotiation, Initial decryption with the published salts, 1-RTT with key log), SCTP, DCCP, UDP-Lite. | S | 2 |
| DV05-F088 | Web, phase 2 | HTTP/2 (frames, HPACK), HTTP/3 (QPACK), WebSocket, gRPC framing, JSON and XML body trees, DNS over TLS and HTTPS when decrypted. | S | 2 |
| DV05-F089 | Mail, files, directory, phase 2 | SMTP, IMAP, POP3, FTP, TFTP, SMB2 and SMB3, NFS with ONC RPC, LDAP, Kerberos, RADIUS, SNMP. | S | 2 |
| DV05-F090 | Real-time and messaging, phase 2 | RTP, RTCP, SIP, SDP, STUN and TURN, MQTT, XMPP, IRC, Matrix federation over HTTP (as HTTP plus JSON). | S | 2 |
| DV05-F091 | Routing and discovery, phase 2 | BGP, OSPF, RIP, VRRP and CARP, SSDP, NetBIOS name service, pfsync. | S | 2 |
| DV05-F092 | Remote access and peer-to-peer, phase 2 | RDP (connection sequence), RFB (VNC), Telnet, BitTorrent (peer wire, DHT), Tor cell framing (headers only). | C | 2 |
| DV05-F093 | Databases and middleware, phase 3 | PostgreSQL, MySQL, Redis, MongoDB wire, AMQP, Kafka, memcached, CoAP. | C | 3 |
| DV05-F094 | Industrial and other, phase 3 | Modbus TCP, DNP3, IEC 104, CAN over UDP encapsulations, USB device classes (HID, mass storage), Bluetooth L2CAP, ATT, RFCOMM. | C | 3 |
| DV05-F095 | Generic encodings | ASN.1 BER and DER, protobuf wire format (with an optional schema file), CBOR, MessagePack, as sub-dissectors usable from "Decode as". | S | 2 |
| DV05-F096 | Heuristic dissection | Dissectors may claim a payload by content when no port matches; heuristics run in a fixed order and can be turned off per protocol. | S | 2 |
| DV05-F097 | Protocol preferences | Per protocol: ports, reassembly on or off, checksum validation, desegmentation, limits (maximum reassembled size, default 64 MiB). | M | 1 |
| DV05-F098 | Enable and disable protocols | A list with search; disabling stops a dissector from being called. | M | 1 |
| DV05-F099 | Declarative dissectors | A protocol described in the structure template language of DV04 (same grammar plus a `protocol` header binding it to a port or parent field), evaluated by the template engine inside the dissect helper. For user protocols. | S | 3 |
| DV05-F100 | Scripted dissectors | ForgeScript dissectors in an engine inside the dissect helper sandbox (no capabilities, step and heap limits). Slower than native; for prototypes. Needs a `bytes` type (FORGE-R-041). | C | 3 |
| DV05-F101 | Dissector limits | Per packet: fuel, recursion depth 64 (tunnels in tunnels), field count 100 000, time 50 ms; over a limit the packet is marked and dissection of that packet stops. | M | 1 |
| DV05-F102 | Dissector test harness | "Test dissector" runs a chosen dissector over a capture and reports failures, unparsed trailing bytes and field statistics; used by DV01 projects for the user's own protocols. | S | 3 |

### 4.6 Flows, conversations and stream follow

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F110 | Conversations | Tables per layer (Ethernet, IPv4, IPv6, TCP, UDP, QUIC, app): endpoints, packets and bytes each way, start, duration, rates; filter, colour, follow from a row. | M | 1 |
| DV05-F111 | Endpoints | Per address: packets, bytes, sent and received, resolved name, app (for local endpoints). | M | 1 |
| DV05-F112 | Flow table | Live table of 5-tuple flows with state (open, closing, reset, idle), app, bytes, retransmission count, TLS server name, HTTP host. Sortable; the default view for "what is talking". | M | 2 |
| DV05-F113 | Follow stream | TCP, UDP, TLS (decrypted), HTTP, HTTP/2 stream, QUIC stream, WebSocket: conversation shown with sides coloured and labelled; view as text (encoding choice), hex dump, raw, C arrays, YAML-like; show one side or both; find in stream; save. | M | 1 |
| DV05-F114 | Stream navigation | Stream number spinner; "next stream"; the list is filtered to the stream. | M | 1 |
| DV05-F115 | Sequence (ladder) diagram | Hosts as columns, messages as arrows in time order, for the displayed packets or one flow; click an arrow to select the packet. | S | 2 |
| DV05-F116 | TCP stream graphs | Time-sequence (Stevens and tcptrace styles), throughput, round-trip time, window scaling. | S | 2 |
| DV05-F117 | Request and response pairing | HTTP, DNS, DHCP, NTP, SMB, RPC: response time field and a table of pairs with latency. | S | 2 |
| DV05-F118 | Flow labels | User-given names and notes for flows, stored as pcapng comments on the first packet. | C | 3 |

### 4.7 Statistics and graphs

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F120 | Capture summary | Packets, bytes, average rates, time span, drops, per interface. | M | 1 |
| DV05-F121 | Protocol hierarchy | Tree of protocols with packet and byte counts and percentages; apply as filter. | M | 1 |
| DV05-F122 | I/O graph | Packets, bytes or bits per interval over time; several series each with its own display filter, colour and style (line, bar, dots); field aggregates (sum, min, max, average, count of a field); interval 1 ms to 10 min; click to jump. | M | 2 |
| DV05-F123 | Packet lengths | Histogram by size range. | S | 2 |
| DV05-F124 | DNS statistics | Query types, response codes, top names, response times, failures. | S | 2 |
| DV05-F125 | HTTP statistics | Requests by host and path, status codes, methods, response times, sizes. | S | 2 |
| DV05-F126 | TLS statistics | Versions, cipher suites, server names, certificate issuers, handshake failures. | S | 2 |
| DV05-F127 | DHCP, ICMP, ARP statistics | Message types; ARP conflicts and storms. | C | 3 |
| DV05-F128 | Per-app statistics | Packets, bytes, flows, remote endpoints, protocols and first-seen time per app. | M | 2 |
| DV05-F129 | Service response time | For request and response protocols: min, max, average, percentiles per operation. | S | 3 |
| DV05-F130 | Top talkers over time | Stacked area of the top N endpoints or apps. | S | 3 |
| DV05-F131 | Export statistics | Any table as CSV or JSON; any graph as PNG or as its data. | S | 2 |
| DV05-F132 | Live statistics | All of the above update during capture at up to 2 Hz. | S | 2 |

### 4.8 TLS and other decryption

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F140 | Key log files | Reads the NSS key log format (`CLIENT_RANDOM`, TLS 1.3 traffic secrets, QUIC secrets). Added per capture through the portal; the dissect helper receives the parsed secrets, not the file. | M | 2 |
| DV05-F141 | Embedded secrets | Reads and writes pcapng decryption secrets blocks. Saving with secrets embedded asks first and marks the file as containing keys. | S | 2 |
| DV05-F142 | TLS decryption | TLS 1.2 and 1.3 record decryption for AEAD suites (AES-GCM, ChaCha20-Poly1305) and CBC suites with the key log; decrypted application data feeds HTTP/1.x, HTTP/2 and other sub-dissectors. Cryptography from the platform crypto library, running inside the dissect helper. | M | 2 |
| DV05-F143 | QUIC decryption | Initial packets always (keys derive from the connection id); handshake and 1-RTT with key log secrets. | S | 2 |
| DV05-F144 | Key log from your own app | DV01's run configuration can ask the app host to write a key log for the app being debugged into a file the developer picked; this app offers "Use key log from the IDE session". Works only for apps run under DV01. Proposed; needs a host switch that is off outside debug runs. | S | 3 |
| DV05-F145 | WireGuard and IPsec | Decryption with keys the user supplies from SE04 (explicit pick; this app never browses keys). | C | 3 |
| DV05-F146 | WPA | 802.11 decryption with a supplied passphrase or PMK when the handshake is in the capture. | C | 3 |
| DV05-F147 | Decryption status | A panel listing each encrypted flow: decrypted, no keys, keys do not match, unsupported suite. | S | 2 |
| DV05-F148 | Secrets hygiene | Secrets are held in memory in the dissect helper only for the session, never written to settings, and wiped on close. The scrubber (4.13) always removes secrets blocks. | M | 2 |
| DV05-F149 | Certificate view | Server and client certificates in handshakes shown as a decoded tree; "Open in Passwords and Keys" for inspection there; export as DER or PEM. | S | 2 |

### 4.9 Name resolution (offline)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F150 | No lookups | The app never sends a DNS query or any other packet to resolve a name. `net.none` is the default and resolution needs no network. | M | 1 |
| DV05-F151 | Names from the capture | Address-to-name map built from DNS, mDNS, LLMNR, NetBIOS, DHCP host names, TLS server names and HTTP hosts seen in the capture; each mapping shows which packet gave it. | M | 1 |
| DV05-F152 | Name blocks | Reads and writes pcapng name resolution blocks. | S | 2 |
| DV05-F153 | Hosts files | User-supplied hosts and `ethers` files per profile. | S | 2 |
| DV05-F154 | Shipped tables | MAC vendor prefixes (OUI), IP protocol numbers, port-to-service names, EtherTypes; bundled, dated, updated only with the app. | M | 1 |
| DV05-F155 | Local machine names | Local interface addresses resolve to "this computer"; local sockets resolve to app names through attribution. | M | 2 |
| DV05-F156 | Address classes | Private, link-local, multicast, loopback, documentation, carrier-grade NAT ranges are labelled. | S | 1 |
| DV05-F157 | Location database | If the user supplies a local IP-to-location or IP-to-AS database file (MaxMind DB format), country and AS columns and a map hand-off to IN04 become available. Nothing is fetched. | C | 3 |
| DV05-F158 | Resolution toggles | MAC, network and transport name resolution each on or off (`Ctrl+Shift+N` cycles); raw values are always one click away. | M | 1 |

### 4.10 Export objects, colour rules, expert warnings

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F160 | Export objects | Lists files carried in HTTP (1.x, 2, 3), SMB, TFTP, FTP data, IMAP and SMTP attachments (MIME parts): packet, host, type, size, name. Save selected or all through the portal. | M | 2 |
| DV05-F161 | Safe object export | Names are sanitised (no path separators, no leading dots, length limit); files are written into the chosen folder handle only; each gets the "untrusted, from capture" tag; nothing is opened automatically. | M | 2 |
| DV05-F162 | Object preview | Text and images preview through sandboxed thumbnailers (`svc.thumbnail`); hashes shown; "Open in Hex Editor". | S | 2 |
| DV05-F163 | Export packets | Dissections as plain text, CSV (columns), JSON (full field trees), PDF through print; packet bytes as hex dump or C arrays. | M | 2 |
| DV05-F164 | Export keys and certificates | TLS session keys seen via key logs as a filtered key log for just the displayed flows; certificates as DER. | S | 3 |
| DV05-F165 | Colour rules | Ordered list: name, display filter, foreground and background from the token palette; first match wins; shipped rules for errors, resets, retransmissions, DNS, ARP, ICMP, HTTP, TLS, this-app. Every rule also puts a short tag in a "Mark" column so colour is never the only signal. | M | 1 |
| DV05-F166 | Temporary colours | Colour a conversation with one of ten colours for the session (`Ctrl+1` … `Ctrl+0`). | S | 2 |
| DV05-F167 | Expert warnings | Dissectors attach notes with severity (chat, note, warning, error) and group (sequence, checksum, malformed, protocol, security, deprecated, reassembly). A panel lists them grouped with counts; the status bar shows the highest severity. | M | 1 |
| DV05-F168 | Security notes | Built-in checks: cleartext credentials (HTTP basic, FTP, Telnet, POP3, IMAP login), TLS below 1.2, weak suites, expired or self-signed certificates in handshakes, DNS answers that look like rebinding, ARP conflicts, unexpected listening responses. | S | 2 |
| DV05-F169 | Malformed packet handling | A packet a dissector cannot parse is marked "malformed" at the failing field with the reason; the remaining bytes are shown as data. | M | 1 |

### 4.11 Per-app attribution

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F170 | Socket-owner table | For live captures the capture service joins each packet with the local socket that sent or will receive it and that socket's owner: app id, capability domain id, helper name if any, route (clear, VPN, Tor). The source of truth is the platform's network service and kernel socket ownership (DV05-GAP-2). | M | 2 |
| DV05-F171 | App column and fields | `app.id`, `app.name`, `app.helper`, `app.domain`, `app.route`, `app.signed_by` as fields: usable in the list, filters, colour rules and statistics. | M | 2 |
| DV05-F172 | Attribution in files | Stored in pcapng as a custom option on each enhanced packet block (compact index) plus a custom block mapping indexes to app records. Other tools ignore unknown custom options, so the file stays readable elsewhere. | M | 2 |
| DV05-F173 | Capability context | For a selected packet: the capability that allowed it (`net.host:…`, `net.user`), when it was granted, and a link to the grant in AD05. | S | 2 |
| DV05-F174 | Unattributed traffic | Packets with no local socket (forwarded, bridged, other hosts in promiscuous mode, kernel-originated) are labelled by kind: `kernel`, `forwarded`, `remote`, `unknown`. | M | 2 |
| DV05-F175 | First-contact marks | Packets that are the first contact of an app with a host (from `svc.audit`) are flagged. | S | 3 |
| DV05-F176 | Attribution privacy | Which apps the user runs is personal data. Scope "this app only" captures hide all other app names from this app. The scrubber removes attribution by default (4.13). | M | 2 |
| DV05-F177 | App view | A sidebar mode listing apps seen, each with flows, endpoints and bytes; selecting an app filters everything. | M | 2 |
| DV05-F178 | Denied connections | Connection attempts refused by capability policy or by pf (from `pflog0` and `svc.audit`) appear as synthetic "blocked" rows in the flow table with app and reason, even when no packet left the machine. | S | 3 |

### 4.12 Firewall integration

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F180 | Create rule from packet | Context menu: "Block this flow", "Block this host", "Block this port", "Block this app to this host", "Allow only…". Builds a rule draft (app, direction, protocol, addresses, ports, interface) and hands it to SE02 by intent. SE02 shows it, the user confirms there. This app changes no rules. | M | 2 |
| DV05-F181 | Rule preview | Before hand-off: the draft in SE02's rule wording and how many captured packets it would have matched. | S | 2 |
| DV05-F182 | Matched rule display | For `pflog` packets: rule number, anchor, action (pass, block), reason and direction as fields; "Show this rule in Firewall". | S | 2 |
| DV05-F183 | Blocklist check | Offline check of displayed endpoints against the blocklists SE02 holds (query by intent, answer per address); matches are flagged. | C | 3 |
| DV05-F184 | DNS policy view | For DNS packets: whether SE02's DNS policy would allow the name. | C | 3 |

### 4.13 Remote capture and privacy scrubbing

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F190 | Remote capture over SSH | Connects to a host the user types (`net.user`; host key and keys through SE04), runs a capture command there (the remote capture service's client if present, else `tcpdump -U -w -` with the given filter) and streams pcapng or pcap back. The SSH traffic of the session itself is excluded by an automatic filter. | S | 3 |
| DV05-F191 | Remote interface list | Queried over the same session. | S | 3 |
| DV05-F192 | Remote profile | Host, user, interface, filter, command template; stored without secrets. | S | 3 |
| DV05-F193 | Remote limits | Rate and size caps so a busy remote link cannot flood the session; drops reported. | S | 3 |
| DV05-F194 | Scrub wizard | Produces a new file; the original is untouched. Steps: scope, addresses, payloads, identities, review, save. | M | 2 |
| DV05-F195 | Address anonymising | IPv4 and IPv6: prefix-preserving keyed mapping (Crypto-PAn style) so subnets stay recognisable, or mapping into documentation ranges; MAC addresses: keep vendor prefix or randomise all; options to keep private ranges, multicast and loopback as they are. Checksums are recomputed. | M | 2 |
| DV05-F196 | Payload removal | Truncate after a chosen layer (keep up to transport headers; keep application headers of named protocols; keep all). Lengths are preserved in headers; removed bytes are noted in a packet comment. | M | 2 |
| DV05-F197 | Identity removal | Blank or hash: DNS names, TLS server names, HTTP hosts, URLs, cookies, authorisation headers, user agents, email addresses, DHCP host names, SSH banners, certificate subjects. Each is a tick box with a count of occurrences found. | M | 2 |
| DV05-F198 | Metadata removal | Removes app attribution, comments, name resolution blocks, secrets blocks, interface names and descriptions, capture host and OS fields. Attribution and secrets are always removed unless explicitly kept. | M | 2 |
| DV05-F199 | Time handling | Keep, shift to start at zero, or shift by a random offset; relative timing is preserved. | S | 2 |
| DV05-F200 | Consistency and key handling | The same original value maps to the same replacement throughout the file. The mapping key is random per scrub and discarded; "keep mapping" saves a key file to a place the user picks so later captures can be scrubbed consistently. | M | 2 |
| DV05-F201 | Scrub report | Counts of each change and a list of what was kept; a residual scan flags strings that still look like addresses, names, tokens or keys in remaining payloads. | M | 2 |
| DV05-F202 | Scrub profiles | "Share with support" (addresses mapped, payloads cut after transport, identities hashed), "Publish" (strictest), custom. | S | 2 |
| DV05-F203 | Scrubbed mark | The output carries a section comment "scrubbed" with the profile name and no key material. | S | 2 |

### 4.14 Workbench, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV05-F210 | Profiles | Named sets of columns, colour rules, filters, protocol preferences, layout; switch from the status bar. | S | 2 |
| DV05-F211 | Command palette | `Ctrl+K`: actions, saved filters, fields (insert into the bar), statistics windows. | M | 1 |
| DV05-F212 | Time reference and deltas | Set a reference packet (`Ctrl+T`); delta between two selected packets in the status bar. | S | 1 |
| DV05-F213 | Explain packet (local AI) | Opt-in: sends the selected packet's field tree (not other packets) to `svc.model` for a plain-language explanation. | C | 3 |
| DV05-F214 | Draft a filter (local AI) | Opt-in: a sentence becomes a filter expression, shown for review and validated by the parser. | C | 3 |
| DV05-F215 | Print | Summary lines, details or bytes for selected packets through `svc.portal.print`. | C | 3 |
| DV05-F216 | Helper recovery | Dissect helper or capture store crash: restart with back-off; capture continues in the service and no packets are lost from the ring up to its size. | M | 1 |
| DV05-F217 | Screen-reader support | Packet rows read as "number, time, source to destination, protocol, app, info, mark"; the detail tree and bytes are in the a11y tree; expert severity is spoken. | M | 2 |
| DV05-F218 | Audit | Capture start and stop, scope, interface, promiscuous flag and remote captures are written to `svc.audit`. | M | 1 |

## 5. GUI

### 5.1 Window map

```
Packet Analyser
├─ Main window
│  ├─ Header bar, toolbar (capture controls, filter bar, saved-filter buttons)
│  ├─ Sidebar: Interfaces · Apps · Flows · Saved filters · Profiles
│  ├─ Content: start page │ packet list / detail tree / bytes (three panes) │ flow table
│  ├─ Inspector: packet attribution, capability context, expert notes for the selection
│  └─ Bottom dock: Expert · Conversations · Endpoints · Protocol hierarchy · I/O graph · Decryption · Comments
├─ Windows: Follow stream · Sequence diagram · TCP stream graphs · Statistics tables · Packet in its own window
├─ Dialogs: Capture options, Capture filter check, Decode as, Protocol preferences, Colour rules, Key logs,
│           Export objects, Export packets, Scrub wizard, Remote capture, Merge, File properties, Preferences
├─ Popovers: field browser, filter completion, name source, rule draft preview
└─ Sheets (narrow): detail tree, inspector
```

### 5.2 Main window

Shown at 1440×900 during a live capture scoped to this user's apps.

```
+------------------------------------------------------------------------------------------+
| [=] Packet Analyser — em0 (live)      [ Search or run a command  Ctrl+K ]  ◉REC [▁][▢][✕] | (1)
+------------------------------------------------------------------------------------------+
| [■ Stop] [⟳] [Save]  [ app.id == "org.pbsd.browser" && tls            ✓] [→] [TLS][DNS]  | (2)
+----------------+-------------------------------------------------------------------------+
| INTERFACES     | No.   Time      Source         Destination    Proto  Len  App      Info |
| > em0  ▂▃▅▂ ◉  | 1041  12.0412   192.168.1.20   203.0.113.9    TLS    571  Browser  Clie…|
|   lo0  ▁▁▁▁    | 1042  12.0688   203.0.113.9    192.168.1.20   TLS   1414  Browser  Serv…|
|   pflog0       |>1043  12.0689   203.0.113.9    192.168.1.20   TCP   1414  Browser  [seg…|
| -------------- | 1044  12.0701   192.168.1.20   192.168.1.1    DNS     74  Mail     Quer…|
| APPS       (3) | 1045  12.0702 ⚠ 192.168.1.20   198.51.100.7   TCP     66  Feeds    [Ret…|
|  Browser  812  |                                                                     (4) |
|  Mail      96  +---------------------------------------+---------------------------------+
|  Feeds     41  | v Frame 1043: 1414 bytes on em0       | 0000  52 54 00 12 34 56 00 1c …  |
|  kernel    12  | v Ethernet II  00:1c:… → 52:54:…      | 0010  05 78 9a 2b 40 00 36 06 …  |
| -------------- | v Internet Protocol v4  203.0.113.9 → | 0020 [01 14 01 bb c3 5a 8f 31 …] |
| FLOWS          | v Transmission Control Protocol       | 0030  …                      (6) |
|  ⇄ 203.0.113.9 |     Source port: 443                  +---------------------------------+
|    :443 Browser|   > Sequence: 2921 (relative)     (5) | APP  Browser (org.pbsd.browser) |
|  ⇄ 192.168.1.1 |     [Stream index: 7]                 | via  net.user · route clear (7) |
|    :53  Mail   |     [Reassembled in: 1046]            | [Block this flow…] [Follow]     |
+----------------+---------------------------------------+---------------------------------+
| EXPERT  CONVERSATIONS  ENDPOINTS  HIERARCHY  I/O GRAPH  DECRYPTION                    (8) |
| ⚠ Warning  Sequence   Retransmission (suspected)        3   › 1045, 1102, 1340           |
+------------------------------------------------------------------------------------------+
| ◉ em0 · 1 045 packets · 1 045 shown · 0 dropped · scope: my apps · profile Default   (9) |
+------------------------------------------------------------------------------------------+
```

1. Header bar. `◉REC` is this app's capture chip, mirrored by the system-wide indicator. The `[net]` badge appears only for remote capture.
2. Toolbar: start or stop, restart, save `toolButton`s; filter `lineEdit` with validity mark; apply; saved-filter buttons.
3. Sidebar (220 px): interfaces `listView` with sparklines and a capture dot; apps `listView` with packet counts; flows `listView`.
4. Packet list (`tableView`, virtualised, 64-bit rows); the App column and a Mark column with tags and severity icons.
5. Detail tree (`treeView`); generated fields in brackets.
6. Bytes pane (hex view, FUSE-WP-41) with the selected field highlighted.
7. Attribution card: app, the capability that allowed the socket, route, actions.
8. Bottom dock (`tabWidget`, 160 px).
9. Status bar: capture state, counts, drops (turns `--warn` with an icon when not zero), scope, profile.

States: empty (start page: interface list with sparklines, "Open a capture file", recent files, a note on what the capture grant means); loading (file indexing progress; first packets appear at once); error (file truncated: packets up to the break are shown with a banner; capture refused: the prompt's denial is stated with "Open Permissions"); offline (normal; remote capture unavailable with reason); locked (session lock stops live captures unless "keep capturing while locked" was part of the grant; the window content is hidden by the shell); read-only (files opened from snapshots or read-only handles: saving is "Save As").

### 5.3 Other views and dialogs

#### Capture options (dialog 820×560)

```
+------------------------------------------------------------------------------+
| Capture options                                                               |
| Interface   Traffic   Link type   Snap len   Promisc   Filter                 |
| [x] em0     ▂▃▅▂      Ethernet    [262144]   [ ]       [tcp || udp       ✓]   |
| [ ] lo0     ▁▁▁▁      Loopback    [262144]   —         [                  ]   |
| [ ] pflog0  ▁▂▁▁      pflog       [   160]   —         [                  ]   |
+------------------------------------------------------------------------------+
| Whose traffic                                                                 |
|  ( ) Only this app's          (•) Apps I choose: [Browser ×][Mail ×] [+]      |
|  ( ) All my apps              ( ) Everything on the interface (administrator) |
| Stop after  [ ] [1000] packets  [ ] [100] MiB  [x] [10] min  [ ] when [____]  |
| Output      (•) Temporary  ( ) File [________] [Pick…]  [ ] Ring of [5] files |
| Filter compiles to BPF: 14 instructions                           [Show]  (1) |
|                                              [Cancel]  [Start capture]        |
+------------------------------------------------------------------------------+
```

1. Opens the compiled program listing. Starting may raise the shell's capture prompt; the dialog says so when no matching grant exists. States: filter not compilable to BPF (the row explains which part needs dissection and offers "apply as display filter instead").

#### Follow stream (window 900×640)

```
+------------------------------------------------------------------------------+
| Follow TLS stream 7 (decrypted) — Browser ⇄ 203.0.113.9:443                   |
| GET /api/items?page=2 HTTP/1.1                                    ← client    |
| Host: example.org                                                             |
| Accept: application/json                                                      |
|                                                                               |
| HTTP/1.1 200 OK                                                   ← server    |
| Content-Type: application/json                                                |
| {"items":[…]}                                                                 |
+------------------------------------------------------------------------------+
| Show [Both v]  As [UTF-8 v]  Stream [ 7 ]◂▸   Find [________]                 |
| 2 client packets · 3 server packets · 4.1 KiB      [Filter out] [Save as…]    |
+------------------------------------------------------------------------------+
```

Sides are marked by a margin label and tint (not tint alone). View choices: text encodings, hex dump, raw, C arrays. Click a line to select its packet in the main window.

#### I/O graph (bottom dock, expanded or own window)

```
+------------------------------------------------------------------------------+
| I/O GRAPH   interval [1 s v]   y [bits/s v]   [x] log   [+ series]            |
| 8M ┤              ▄                                                           |
| 4M ┤      ▂▄▆    ███▆▂       ── all                                           |
|  0 ┼▁▂▃▅▇███▇▅▃▂▁█████▇▅▃▂▁  ── app.id == "org.pbsd.browser"                  |
|    0 s        10 s        20 s  ·· tcp.analysis.retransmission (count)        |
| Series                 Filter                              Style   Colour     |
| [x] all                                                    line    ■          |
| [x] Browser            app.id == "org.pbsd.browser"        line    ■          |
| [x] Retransmissions    tcp.analysis.retransmission         dots    ■          |
+------------------------------------------------------------------------------+
```

`chart` plus a series `tableView`. Series differ by line style as well as colour.

#### Scrub wizard, step "Identities" (dialog 760×560)

```
+--------------------------------------------------------------------------+
| Scrub for sharing   1 Scope ✓  2 Addresses ✓  3 Payloads ✓  ▶4 Identities |
|                     5 Review  6 Save                                      |
+--------------------------------------------------------------------------+
| Found in this capture                          Action                     |
| [x] DNS names                    214           [Hash v]                   |
| [x] TLS server names              37           [Hash v]                   |
| [x] HTTP hosts and URLs           12           [Blank v]                  |
| [x] Cookies and authorisation      9           [Blank v]                  |
| [x] User agents                    5           [Blank v]                  |
| [x] Email addresses                2           [Blank v]                  |
| [x] DHCP host names                1           [Blank v]                  |
| [x] App attribution             all            Remove (always, unless ▾)  |
| [x] Decryption secrets            1 block      Remove (always)            |
+--------------------------------------------------------------------------+
| Profile [Share with support v]              [Back]  [Cancel]  [Next]      |
+--------------------------------------------------------------------------+
```

Step 5 shows the report and residual scan (strings that still look sensitive, each with "show packet"); step 6 saves through the portal. The original file is never modified.

#### Key logs and decryption status (dialog 720×460)

```
+----------------------------------------------------------------------+
| TLS and QUIC keys for this capture                                    |
| Key log files                                                         |
|   browser-keys.log   1 204 secrets   [Remove]            [Add file…]  |
|   Embedded in file   none                                             |
| [ ] Embed secrets when saving (the file will then contain keys)       |
+----------------------------------------------------------------------+
| Flow                         Version  Suite              Status       |
| 203.0.113.9:443  Browser     TLS 1.3  AES_128_GCM        ✓ decrypted  |
| 198.51.100.7:443 Feeds       TLS 1.3  CHACHA20_POLY1305  — no keys    |
| 192.0.2.4:8443   Mail        TLS 1.2  ECDHE-RSA-AES256…  ✕ keys do not match |
+----------------------------------------------------------------------+
|                                                   [Close]  [Reload]  |
+----------------------------------------------------------------------+
```

#### Create firewall rule (popover 420×260)

```
+----------------------------------------------------+
| Block this flow                                     |
| App        Feeds (org.pbsd.feeds)                   |
| Direction  out                                      |
| To         198.51.100.7  port 443  TCP              |
| On         em0                                      |
| Would have matched 41 packets in this capture.      |
| The rule is created in Firewall after you confirm   |
| it there.                 [Cancel] [Open in Firewall]|
+----------------------------------------------------+
```

Other choices in a `comboBox` at the top: this host, this port, this app to any host.

#### Flow table (content view, `Ctrl+3`)

```
+------------------------------------------------------------------------------------------+
| FLOWS  [filter________]  [state v] [app v]                                   128 flows   |
| App      Local            Remote               Proto  State   ↑ bytes  ↓ bytes  Retx  SNI |
| Browser  :51514           203.0.113.9:443      TLS1.3 open    4.1 K    1.2 M    0     example.org |
| Mail     :40022           192.0.2.4:993        TLS1.2 idle    812      9.6 K    0     mail.example |
| Feeds    :51720           198.51.100.7:443     TCP    reset   264      0        3     —   |
| Feeds    —                198.51.100.9:443     —      blocked 0        0        —     —  ⛔ pf rule 12 |
+------------------------------------------------------------------------------------------+
```

"blocked" rows are synthetic (F178). Double click filters the packet list to the flow.

### 5.4 Menus

| Menu | Items |
|---|---|
| File | Open… (`Ctrl+O`) · Open Recent ▸ · Merge… · Import from Hex Dump… · Save (`Ctrl+S`) · Save As… (`Ctrl+Shift+S`) · Export Specified Packets… · Export Dissections ▸ Text, CSV, JSON · Export Objects ▸ HTTP, SMB, TFTP, FTP, Mail · Scrub for Sharing… (`Ctrl+Shift+X`) · File Properties (`Ctrl+Alt+Shift+C`) · Print… (`Ctrl+P`) · Close (`Ctrl+W`) · Quit (`Ctrl+Q`) |
| Edit | Copy ▸ Summary, Value, As Filter, Bytes (`Ctrl+C` copies the summary) · Find Packet… (`Ctrl+F`) · Find Next, Previous (`Ctrl+G`, `Ctrl+Shift+G`) · Mark or Unmark (`Ctrl+M`) · Mark All Displayed · Next, Previous Mark (`Ctrl+Shift+N`, `Ctrl+Shift+B`) · Ignore Packet (`Ctrl+D`) · Set Time Reference (`Ctrl+T`) · Time Shift… · Packet Comment… · Profiles… · Preferences (`Ctrl+,`) |
| View | Command Palette (`Ctrl+K`) · Packets (`Ctrl+1`) · Start Page (`Ctrl+2`) · Flows (`Ctrl+3`) · Packet List, Details, Bytes (toggles) · Layout ▸ · Time Format ▸ · Name Resolution ▸ MAC, Network, Transport · Colour Rules… · Colourise (toggle) · Auto-Scroll (`Ctrl+Shift+L`) · Expand All (`Ctrl+Right`) · Collapse All (`Ctrl+Left`) · Show Packet in New Window · Sidebar (`F9`) · Inspector (`F4`) · Reload (`F5`) · Zoom (`Ctrl++/-/0`) · Full Screen (`F11`) |
| Go | Go to Packet… (`Ctrl+L`) · Linked Packet · First, Last (`Ctrl+Home`, `Ctrl+End`) · Next, Previous in Conversation (`Ctrl+.`, `Ctrl+Shift+.`) · Back, Forward (`Alt+Left/Right`) |
| Capture | Options… (`Ctrl+Shift+E`) · Start or Stop (`Ctrl+E`) · Restart (`Ctrl+R`) · Capture Filters… · Check Filter… · Remote Capture… · Refresh Interfaces |
| Analyse | Apply as Filter ▸ · Prepare as Filter ▸ · Conversation Filter ▸ · Display Filters… · Enabled Protocols… (`Ctrl+Shift+P`) · Decode As… · Follow ▸ TCP (`Ctrl+Alt+Shift+T`), UDP (`Ctrl+Alt+Shift+U`), TLS (`Ctrl+Alt+Shift+S`), HTTP (`Ctrl+Alt+Shift+H`), QUIC · Keys and Decryption… · Expert Information · Create Firewall Rule… (`Ctrl+Shift+F`) · Open Bytes in Hex Editor |
| Statistics | Capture Summary · Protocol Hierarchy · Conversations · Endpoints · Per-App · Packet Lengths · I/O Graph · Sequence Diagram · TCP Stream Graphs ▸ · DNS · HTTP · TLS · Service Response Time ▸ |
| Help | Handbook (`F1`) · Filter Language Reference · Field Browser · Keyboard Shortcuts · About |

### 5.5 Keyboard and pointer

- Packet list: `Up/Down`, `PageUp/Down`, `Home/End`; `Enter` focuses the detail tree; `Space` toggles mark; `Tab` cycles list, details, bytes.
- Detail tree: `Left/Right` collapse and expand; `Shift+Right` expands the subtree; `Enter` on a link field jumps.
- Filter bar: `Ctrl+/` focuses it; `Ctrl+Space` completion; `Esc` clears; `Up/Down` history.
- Packet context menu: Mark, Ignore, Set Time Reference, Packet Comment, Apply as Filter ▸, Conversation Filter ▸, Colourise Conversation ▸, Follow ▸, Decode As, Create Firewall Rule ▸, Show App in Permissions, Copy ▸, Open Bytes in Hex Editor, Show in New Window.
- Field context menu: Expand Subtree, Apply as Column, Apply as Filter ▸, Prepare as Filter ▸, Colour with Filter, Copy ▸, Export Bytes, Protocol Preferences, Decode As, Go to Linked Packet, Field Help.
- Sidebar context menus: interface (Start, Options, Details); app (Filter to App, Colour, Statistics, Open in Permissions, Create Rule); flow (Filter, Follow, Colour, Create Rule, Label).
- Drag and drop: a capture file onto the window (open; with a capture open, ask merge or new window); a field onto the filter bar (adds `field == value`); a field onto the list header (new column); a key log file onto the window (add after confirm); column headers to reorder; an exported object dragged to Files (saves through the portal).

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Start page with interfaces, recent files, Open. |
| Loading | Index progress; list usable for the indexed part; statistics show "partial". |
| Capturing | `◉REC` chip, live counters, Stop is the primary action; closing the window asks Stop and keep, Stop and discard, or (with `bg.run`) Keep capturing. |
| Error | Truncated or corrupt file: readable part shown. Dissector failure: packet marked, helper restarted. Capture service unavailable: banner with the reason. |
| Offline | Normal. |
| Locked | Live capture pauses unless the grant allows capture while locked. |
| Read-only | Read-only handle: Save As only. |
| Grant revoked during capture | Capture stops at once (SI-6); banner "Capture permission was revoked"; captured packets are kept. |
| Narrow (< 640 px) | Columns reduce to No., Time, Proto, App, Info; detail and bytes become tabs under the list; sidebar overlay; bottom dock as a sheet. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Packet list, flow table, conversations, endpoints, expert, statistics tables | `tableView` with `SortFilterProxy` and cell delegates (FUSE-WP-28 for 64-bit row counts) |
| Detail tree, protocol hierarchy, field browser | `treeView` |
| Bytes pane | hex view (FUSE-WP-41), read-only |
| Filter bar | `lineEdit` with completion popup and syntax colours (DV05-GAP-3) |
| I/O graph, lengths, per-app charts | `chart` |
| Sequence diagram, TCP graphs, packet diagram | `viewportWidget` with app drawing (DV05-GAP-4) |
| Sidebar lists and sparklines | `listView` with a delegate |
| Follow stream | code editor (FUSE-WP-37) read-only, or `tableView` for hex dump mode |
| Dialogs, wizard | `dialog`, `checkBox`, `radioButton`, `comboBox`, `spinBox`, `lineEdit`, `progressBar`, `tabWidget` |
| Docks | `dockArea`, `dockWidget`, `splitter` |

| ID | Missing | Note |
|---|---|---|
| DV05-GAP-1 | System capture service (`svc.capture`): privileged BPF holder, verified filters, per-app scoping, shared-memory ring, capture indicator, grant prompt | Platform; not in PLATFORM.md section 8 |
| DV05-GAP-2 | Socket-owner feed: for each local socket, its app identity, domain, route and granting capability, available to the capture service; a kernel or network-service interface | Platform; relates to the network service (FORGE-WP-21) and SE02 |
| DV05-GAP-3 | Single-line editor with syntax colours and inline completion | Could be the code editor in one-line mode (FUSE-WP-37) |
| DV05-GAP-4 | Zoomable 2D plot surface for ladder and time-sequence graphs with hit testing | `chart` covers the I/O graph only |
| DV05-GAP-5 | A capture capability name in the manifest grammar (proposed `net.capture`) so installs can show "can capture network traffic" | PLATFORM.md section 6 has no such row |

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App shell and view models | ForgeScript + FUSE UI | app host | Windows, tables, filter bar, profiles, colour rules, statistics presentation, wizards |
| Capture service (platform, DV05-GAP-1) | C++23, T1 and T2 targets | system service, privileged only to open `/dev/bpf`; after set-up in capability mode with: the locked BPF descriptors, the ring memory, its bus endpoint | Opens BPF on granted interfaces, sets snapshot length, installs the verified filter, sets `BIOCLOCK`, applies per-app scoping from the socket-owner feed, copies packets plus owner index into the ring, counts drops. Does not parse beyond the fixed offsets needed for the flow key |
| Filter compiler | C++23 | library in the dissect helper (display filters) and in the capture service (capture filters, with the verifier) | Parses the filter language; emits display-filter bytecode or a classic BPF program; verifies BPF programs |
| Capture store `capture-store` | C++23 | bundle helper; holds file handles from the portal and the ring handle; no network | Reads the ring, writes the temporary pcapng, indexes packets (offset, length, time, interface), reads and writes pcap and pcapng block framing, merge, split, serves packet bytes by number through shared memory |
| Dissect helper `dissect-helper` | C++23 | bundle helper; **no capabilities**; read-only shared-memory window on packet bytes; CPU and memory limits | All dissectors, reassembly, stream state, decryption, display-filter evaluation, field tables, statistics taps, expert notes, object extraction (returns byte ranges), template and script dissectors |
| Scrubber | C++23, inside `dissect-helper` (needs dissection) writing through `capture-store` | — | Rewrites packets per the scrub plan; recomputes checksums |
| Remote capture client | ForgeScript + system SSH client service | app host / that service | Streams a remote capture into `capture-store` |
| Firewall bridge | ForgeScript | app host | Builds rule drafts; intents to SE02 |

```
 NIC ─ kernel BPF ─▶ [ capture service ]  ◀─ socket-owner feed (network service / kernel)
                          │ shared-memory ring (packets + owner index), drop counters
                          ▼
                    [ capture-store ] ── temp .pcapng, user files (portal handles)
                          │ read-only window: packet bytes by number
                          ▼
                    [ dissect-helper ]   no capabilities; hostile bytes are parsed only here
                          │ field tables, rows, statistics, notes (typed bus records)
                          ▼
 user ─ FUSE UI ─ [ app host ] ── intents ──▶ SE02 (rule), DV04 (bytes), AD05 (grant), SE04 (certificate)
```

Why three processes: the service has privilege and so must be tiny and must not parse; the store has file handles and so must not parse; the dissectors parse and so must have nothing.

### 6.2 Data model

In memory (dissect helper):

| Entity | Fields |
|---|---|
| `PacketRef` | `no: u64`, `ts_ns: i64`, `iface: u16`, `cap_len: u32`, `wire_len: u32`, `dir: in\|out\|unknown`, `owner: u32` (index into `AppRecord`), `flags` (marked, ignored, reference) |
| `Field` | `id: u32` (registered field), `start: u32`, `length: u32`, `value: variant<u64, i64, f64, bytes_ref, str_ref, ipv4, ipv6, mac, time>`, `flags` (generated, hidden, error), `parent: u32` |
| `FieldDef` | `name: string` (`tcp.flags.syn`), `type`, `base`, `description`, `enum_values` |
| `Flow` | `key: {l3, proto, a_addr, a_port, b_addr, b_port, iface}`, `stream_index: u32`, `first_no`, `last_no`, `bytes_ab`, `bytes_ba`, `state`, `owner: u32`, `sni: str_ref`, `retx: u32` |
| `AppRecord` | `app_id: string`, `name`, `helper`, `domain_id: u64`, `route: clear\|vpn\|tor`, `capability: string`, `signer: string`; or `kind: kernel\|forwarded\|remote\|unknown` |
| `ExpertNote` | `no`, `severity`, `group`, `proto: u16`, `text` |
| `NameEntry` | `addr`, `name`, `source: dns\|mdns\|dhcp\|sni\|hosts\|table`, `first_no` |

On disk:

- Captures: pcapng. Attribution uses a custom block (`AppRecord` table) and a custom option on enhanced packet blocks (owner index), both under a private enterprise number to be registered; until then a clearly marked provisional number, and readers accept both.
- App store `~/.local/share/org.pbsd.packet-analyser/`: `profiles/<name>.json` (columns, colour rules, saved filters, protocol preferences, layout), `filters.json`, `decode_as.json`, `remote.json` (no secrets), `hosts/` (user files), `dissectors/` (user templates and scripts), `recent.db` (SQLite: `recent(path_ref, grant_id, opened_at, packets, span_ns)`). Schema field in each JSON; forward-only migration with backup.
- Cache `~/.cache/org.pbsd.packet-analyser/`: temporary live captures, per-file indexes keyed by file hash.
- Nothing about captured traffic is kept after a capture is discarded.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| pcapng block framing | yes | yes | own | `capture-store` (framing only: lengths, types, bounds) |
| pcap classic | yes | yes | own | `capture-store` |
| pcapng options, name blocks, secrets blocks, custom blocks | yes | yes | own | `dissect-helper` |
| Every network protocol (section 4.5) | yes | no | own dissectors; no Wireshark code (GPL-2.0, C) is used | `dissect-helper` |
| NSS key log | yes | filtered export | own | `dissect-helper` |
| gzip, zstd capture files | yes | yes | UT01 archive helper | that helper |
| Filter language text | yes | — | own | app host (syntax colours only) and `dissect-helper` (compile) |
| Classic BPF programs | — | yes | own compiler and verifier | capture service |
| Hosts, `ethers`, OUI and service tables | yes | no | own | `dissect-helper` |
| MaxMind DB (optional, user supplied) | yes | no | own reader | `dissect-helper` |
| Exported objects | — | yes (bytes as found) | — | written by `capture-store` to a portal folder handle |
| CSV, JSON, text exports | no | yes | own | app host |
| SSH (remote capture) | — | — | system SSH client service (section 13) | that service |
| `usbdump`, HCI, ERF, snoop, hex dump text | yes | no | own | `capture-store` (framing), `dissect-helper` (content) |

#### Filter language

One language for display filters and capture filters. A capture filter is an expression that uses only fields at offsets computable without dissection state (see "BPF subset").

```
filter     = or_expr ;
or_expr    = and_expr { ( "||" | "or" ) and_expr } ;
and_expr   = not_expr { ( "&&" | "and" ) not_expr } ;
not_expr   = [ "!" | "not" ] primary ;
primary    = "(" filter ")" | test | field ;                  (* a bare field means "is present" *)
test       = operand relop operand
           | operand "in" "{" range { "," range } "}"
           | operand "contains" operand                        (* bytes or text *)
           | operand "matches" regex ;                         (* linear-time regex *)
relop      = "==" | "!=" | "<" | "<=" | ">" | ">=" | "&" ;     (* "&" is bitwise-and non-zero *)
operand    = field [ slice ] | literal | call ;
field      = ident { "." ident } [ "#" number ] ;              (* "#2" = second occurrence in the packet *)
slice      = "[" number ":" number "]" | "[" number "]" ;      (* offset:length in bytes *)
range      = literal [ ".." literal ] ;
call       = ( "len" | "lower" | "upper" | "count" | "any" | "all" ) "(" operand ")" ;
literal    = number | hex | string | ipv4 [ "/" n ] | ipv6 [ "/" n ] | mac | time | bytes | "true" | "false" ;
bytes      = hex2 { ":" hex2 } ;                               (* de:ad:be:ef *)
```

Semantics:

- Fields are typed. A comparison whose types do not fit is a compile error, not a silent false.
- A field that occurs several times (tunnels) matches if any occurrence matches; `all(f) == v` requires all; `f#2` picks one. `!=` means "present and not equal for every occurrence", avoiding the classic trap where `ip.addr != x` matches almost everything.
- Addresses accept prefixes: `ip.src == 10.0.0.0/8`.
- Pseudo-fields join pairs: `ip.addr`, `tcp.port`, `eth.addr`.
- Attribution fields: `app.id`, `app.name`, `app.helper`, `app.route`, `app.kind`.
- Analysis fields: `tcp.analysis.retransmission`, `frame.time_delta`, `expert.severity >= warning`.
- Evaluation is total: no loops, bounded by the packet's field count.

BPF subset (capture filters): boolean combinations of tests on `eth.*`, `vlan.id`, `ip.*` (fixed header and options length), `ip6.*` (fixed header; next-header chain up to 4 extension headers), `tcp.port`, `tcp.flags.*`, `udp.port`, `icmp.type`, `icmp.code`, `arp.*`, `frame.len`, and raw slices `frame[o:l]`. The compiler emits classic BPF with forward jumps only. The verifier in the capture service rejects any program that is not: bounded length (≤ 4096 instructions), forward-jumping, in-bounds for every load given the snapshot length check, and ending in a return on every path. `app.*` scoping is not BPF; the service applies it after the BPF match.

Examples: `tcp.port == 443 && ip.addr == 192.0.2.0/24`; `dns && dns.qry.name contains "example"`; `app.id == "org.pbsd.mail" && !tls`; `tcp.flags.syn && !tcp.flags.ack && ip.dst in {10.0.0.1 .. 10.0.0.20}`.

### 6.4 Integration

- Intents handled: `open`, `view` for capture types. Emitted: `open` to SE02 (rule draft; blocklist query), DV04 (bytes), AD05 (grant), SE04 (certificate), IN04 (location, optional), DV07 (extracted executable); `share` (offers "Scrub first"); `print`.
- Services: `svc.capture` (Proposed), `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.audit` (write capture events; read first-contact and denial records for the user's own apps), `svc.keyring` (remote capture credentials through SE04's agent), `svc.thumbnail`, `svc.model` (optional).
- IPC used: DV04's `org.pbsd.hex-editor.Templates/1` grammar implementation is linked as a library into the dissect helper (same source, no cross-process call per packet).
- IPC offered: `org.pbsd.packet-analyser.Dissect/1` — dissect a byte buffer with a named dissector and return the field tree (for DV01's dissector test harness and DV04's "decode as packet").
- Backing repos: none. All Proposed. BPF device behaviour (`/dev/bpf`, `BIOCLOCK`, `pflog`) is standard FreeBSD and was not re-verified on HardenedBSD 15-STABLE for this spec.

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Profiles, filters, user dissectors, temporary captures |
| `fs.portal` | Capture files, key logs, hosts files, export folders |
| `proc.helper:capture-store`, `proc.helper:dissect-helper` | Bundle helpers |
| `svc.capture` (Proposed service; manifest name `net.capture` proposed in DV05-GAP-5) | Live capture, prompted per interface and scope |
| `net.none` | Default. Analysis never uses the network |
| `net.user` | Only for remote capture over SSH to hosts the user types |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.audit`, `svc.keyring`, `svc.thumbnail` | As in 6.4 |
| `svc.model`, `ai.local` | Optional explanations and filter drafts; off by default |
| `bg.run` | Only for background capture; off by default |

Assets: the contents of the user's and other users' traffic (passwords, tokens, messages); which apps the user runs and whom they contact; TLS session keys; the integrity of the machine (a capture tool is a classic privilege-escalation target).

Adversaries: a remote attacker who sends crafted packets to exploit a dissector (reachable over the network with no user action during a capture); a hostile capture file; a local app trying to use this app to see other apps' traffic; a person who receives a shared capture; a hostile remote capture host.

| Threat | Mitigation |
|---|---|
| Dissector memory-safety bug exploited by a crafted packet | Dissectors are safe-subset C++23 over a bounds-checked reader, fuzzed, with fuel and depth limits; they run in `dissect-helper` with no capabilities, so a compromise yields only the packets already in view and cannot write files, open sockets or reach the bus beyond its reply channel |
| Privileged capture code exploited | The capture service does not parse; its only input from the app is a BPF program it verifies and a scope request the shell approved; after set-up it runs in capability mode with locked BPF descriptors |
| Malicious BPF program | Verifier (bounds, forward jumps, termination); kernel BPF validation remains as a second check |
| App sees traffic of other apps or users without consent | Scope is enforced in the service; "everything on the interface" and promiscuous mode need an administrator and are named in the prompt; a visible system indicator runs during capture; all captures are audited |
| Covert long-running capture | No background capture without `bg.run` and a persistent indicator; grants expire per the chosen duration |
| Leaking secrets when sharing | Scrub wizard with residual scan; attribution and secrets blocks removed by default; "Share" always offers scrubbing first |
| TLS keys mishandled | Held in helper memory only; never in settings; embedding in files is explicit and marked |
| Exported object is malware or has a hostile name | Sanitised names, written only under the chosen folder handle, tagged untrusted, never opened automatically |
| Decompression or reassembly bombs | Per-stream reassembly cap, content-decoding output cap, helper memory limit |
| Hostile remote capture host | Stream treated as an untrusted capture; SSH host key pinned; no command output is interpreted except as pcap or pcapng |
| Name resolution leaks what is being inspected | No lookups at all (F150) |
| Filter or regex denial of service | Total filter semantics; linear-time regex |

Stored: section 6.2. Leaves the machine: nothing, except SSH traffic for remote capture to the host the user typed. Captures the user saves are the user's files; temporary captures are deleted on exit.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `capture.default_iface` | string | first active non-loopback | Interface for one-key start |
| `capture.scope` | enum | `this_user_apps` | `this_app`, `chosen_apps`, `this_user_apps`, `everything` |
| `capture.snaplen` | int | 262144 | Bytes per packet |
| `capture.promiscuous` | bool | false | Promiscuous mode |
| `capture.ring_bytes` | int | 64 MiB | Shared ring size |
| `capture.autoscroll` | bool | true | Follow new packets |
| `capture.keep_temp_on_exit` | bool | false | Keep unsaved captures |
| `capture.while_locked` | bool | false | Continue when the session locks (also needs the grant) |
| `view.time_format` | enum | `since_start` | Time column format |
| `view.time_precision` | enum | `auto` | Digits |
| `view.layout` | enum | `stacked` | Pane layout |
| `view.colourise` | bool | true | Apply colour rules |
| `resolve.mac`, `resolve.network`, `resolve.transport` | bool | true, true, true | Offline name resolution |
| `dissect.validate_checksums` | bool | false | Checksum validation default |
| `dissect.reassemble_tcp` | bool | true | Stream reassembly |
| `dissect.max_reassembly_bytes` | int | 64 MiB | Per stream |
| `dissect.fuel_per_packet` | int | 1 000 000 | Dissector steps |
| `dissect.mem_bytes` | int | 4 GiB | Dissect helper cap |
| `tls.embed_secrets_on_save` | bool | false | Write secrets blocks |
| `export.sanitise_names` | bool | true | Cannot be turned off; listed for clarity |
| `scrub.default_profile` | string | `Share with support` | Wizard default |
| `scrub.remove_attribution` | bool | true | Default for the wizard |
| `filter.history` | int | 50 | Entries kept |
| `remote.rate_limit_bytes_s` | int | 0 | 0 = no limit |
| `ai.enabled` | bool | false | Local explanations and filter drafts |

## 9. Accessibility and languages

- No information is carried by row colour alone: the Mark column carries the rule tag and the severity icon; the colour rules editor checks contrast of each rule against both themes and warns under 4.5:1.
- Graphs have table equivalents (`Statistics` tables) and series differ by line style.
- The packet list supports type-ahead on the packet number and reads rows in a fixed, short order; verbosity levels (brief, full) for screen readers.
- Live capture can flood announcements; live regions announce only counts every 5 s and expert items of severity warning or above.
- Protocol and field names are not translated; descriptions and UI strings are. Addresses, hex and numbers stay left-to-right in right-to-left UIs.
- All three panes resize and reflow at 300% scale; the bytes pane drops to 8 bytes per row when narrow.

## 10. Performance budgets

| Metric | Budget |
|---|---|
| Cold start to first frame | ≤ 300 ms |
| Open and index a 1 GiB pcapng (about 1 million packets) | first packets ≤ 200 ms; index complete ≤ 3 s |
| First-pass dissection | ≥ 500 000 packets/s per core for Ethernet/IP/TCP headers; ≥ 150 000 packets/s with TLS and HTTP |
| Apply a display filter to 1 million dissected packets | ≤ 1 s |
| Live capture | no drops at 1 Gbit/s of 1500-byte packets to file with a 64 MiB ring; live dissection may lag and catch up |
| Scroll the list | 60 Hz with 100 million rows |
| Select a packet to detail tree and bytes shown | ≤ 20 ms |
| Follow a 100 MiB stream | first screen ≤ 300 ms |
| Memory | app host ≤ 250 MiB; `capture-store` ≤ 128 MiB plus index (16 bytes per packet); `dissect-helper` ≤ 4 GiB cap, about 400 bytes per packet of retained state |
| Largest capture | 2^32 packets or 1 TiB per file |
| Idle (no capture) | zero frames and wake-ups |
| Capture service CPU at 1 Gbit/s | ≤ 15% of one core |

## 11. Testing and proof

Test plan:

- Unit: filter parser and type checker; BPF compiler (each field test) and verifier (accepts all compiler output, rejects a corpus of bad programs); display-filter evaluator; pcap and pcapng readers and writers (round trip keeps unknown blocks byte-identical); scrub mappings (prefix preservation, consistency, checksum repair).
- Dissector goldens: for each protocol a set of capture files with expected field trees as JSON; public sample captures where licences allow, otherwise generated.
- Differential: field values for the phase 1 protocols compared with `tshark -T json` output on the same files (run as an external reference in the test environment only; no code shared); BPF programs compared with `tcpdump -d` semantics by running both over packet corpora.
- Fuzz: every dissector (per-protocol targets and a whole-stack target), reassembly, pcap and pcapng framing, key log parser, filter parser, BPF verifier, MaxMind reader, hex dump importer. Fuzzing runs continuously; every crash becomes a regression capture.
- Capture service: privilege tests (cannot open files, cannot create sockets after set-up), scope tests (packets of an unselected app never appear in the ring), revocation test (capture stops within one ring flush).
- Scrub: after scrubbing with the strict profile, a search for every original address, name and token in the output must find nothing; the output must load in this app and in a reference reader.
- Golden renders of the main window, dialogs and graphs; scripted UI for the 14 top tasks.
- Robustness: kill each helper during capture and during file load.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| BPF verifier (capture service) | Accepts only programs that terminate and never load out of bounds | T1 | Lean model of classic BPF and the verifier; PRISM on the code | — |
| BPF compiler | Compiled program accepts exactly the packets the filter's BPF-subset semantics accept | T1 for the core field set; T3 beyond | Lean semantics for the subset; differential tests | Size of the field set |
| Capture service | No runtime errors; no parsing beyond the flow key; scope filter never passes a packet of an unselected owner | T2 | PRISM; scope tests | Depends on the correctness of the socket-owner feed |
| pcap and pcapng framing (`capture-store`) | No runtime errors on any input; writes are well-formed | T2 | PRISM + fuzz | — |
| Bounds-checked reader used by all dissectors | Never reads outside the packet | T1 | PRISM proof; all dissectors use only this reader (lint rule) | — |
| Dissectors | No runtime errors (T2) for link, IP, TCP, UDP, DNS, DHCP, TLS record layer; safe subset plus fuzz (T3) for the rest | T2 / T3 | PRISM per dissector; fuzz | Number of dissectors |
| Display-filter evaluator | Total; type-safe | T2 | PRISM + property tests | — |
| Scrubber address mapping | Injective and prefix-preserving | T1 | Small pure function; PRISM | — |
| Scrubber as a whole | No original identifier remains for the ticked classes | T3 | Search tests on corpora | Free-form payloads cannot be fully classified |
| App ForgeScript | Engine safety only | T4 | Tests | — |

Nothing here has been run.

## 12. Build plan

Phase 1: usable core.

| ID | Work item | Depends on |
|---|---|---|
| DV05-W01 | Shell: three-pane layout, sidebar, status bar, command palette | G-01, G-02, G-07 |
| DV05-W02 | `capture-store`: pcap and pcapng read and write, index, packet service | FORGE-WP-28.1, G-08 |
| DV05-W03 | `dissect-helper` framework: bounds-checked reader, field registry, limits, restart | FORGE-WP-28.1, G-06 |
| DV05-W04 | Phase 1 dissectors: link, IPv4, IPv6, ICMP, TCP with reassembly and analysis, UDP | DV05-W03 |
| DV05-W05 | Phase 1 application dissectors: DNS, DHCP, HTTP/1.x, TLS records and handshake, NTP, SSH, syslog | DV05-W04 |
| DV05-W06 | Filter language: parser, type checker, display-filter bytecode, filter bar | DV05-W03, DV05-GAP-3 |
| DV05-W07 | Packet list, detail tree, bytes pane, find, marks, columns | FUSE-WP-28, FUSE-WP-41, DV05-W02 |
| DV05-W08 | Capture service and grant flow; BPF compiler and verifier; live capture path | DV05-GAP-1, SH04 (prompt), SH02 (indicator) |
| DV05-W09 | Conversations, endpoints, protocol hierarchy, capture summary, follow stream | DV05-W04 |
| DV05-W10 | Colour rules, expert panel, offline name tables, save subsets | DV05-W07 |

Phase 2: full feature.

| ID | Work item | Depends on |
|---|---|---|
| DV05-W20 | Per-app attribution: owner feed, fields, pcapng storage, app view, per-app statistics | DV05-GAP-2, DV05-W08 |
| DV05-W21 | TLS and QUIC decryption with key logs; decryption status; secrets blocks | DV05-W05, platform crypto |
| DV05-W22 | Phase 2 dissectors (tunnels, QUIC, HTTP/2 and 3, mail, file, directory, real-time, routing) | DV05-W04 |
| DV05-W23 | I/O graph, sequence diagram, TCP graphs, protocol statistics | DV05-GAP-4 |
| DV05-W24 | Export objects and dissections; safe export | DV05-W05, `svc.thumbnail` |
| DV05-W25 | Scrub wizard and scrubber | DV05-W04, DV05-W21 |
| DV05-W26 | Firewall integration: rule drafts, pflog fields | SE02 |
| DV05-W27 | Flow table, request and response pairing, decode as, protocol preferences UI | DV05-W09 |
| DV05-W28 | Compressed files, merge, ring-buffer files, stop conditions, capture profiles, comments | DV05-W02, UT01 |
| DV05-W29 | Security notes, certificate view | DV05-W05, SE04 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| DV05-W40 | Remote capture over SSH | SSH client service (section 13), SE04 |
| DV05-W41 | Declarative and scripted dissectors; dissector test harness; Dissect IPC | DV04 template engine, FORGE-WP-16 and 20 (`bytes`), DV01 |
| DV05-W42 | Phase 3 dissectors (databases, industrial, USB, Bluetooth) | DV05-W22 |
| DV05-W43 | Denied-connection rows, first-contact marks, blocklist and DNS policy checks | SE02, `svc.audit` |
| DV05-W44 | Key log from IDE debug runs; WireGuard, IPsec, WPA decryption | DV01, SE04 |
| DV05-W45 | Background capture, wireless details, other link types | DV05-W08 |
| DV05-W46 | Location database, service response time, top talkers, split, imports | DV05-W23 |
| DV05-W47 | Local AI explain and filter draft | G-15 |

## 13. Open questions and risks

1. **Capture service (DV05-GAP-1).** Nothing in PLATFORM.md section 8 provides capture. It must be a platform service, not a bundle helper, because a helper cannot hold more rights than its app. Its prompt, indicator and grant model need SH02, SH04 and AD05 to agree.
2. **Socket-owner feed (DV05-GAP-2).** Per-app attribution depends on the kernel or the network service exposing socket ownership with app identity for every packet direction, including packets that are dropped by policy. Where that lives (pf tags, a kernel interface, the network service of FORGE-WP-21) is undecided. Races between socket close and late packets need a short history.
3. **Manifest capability for capture (DV05-GAP-5).** Section 6 of PLATFORM.md has no row for it. `svc.capture` works as a service capability; a clearer `net.capture` row is proposed.
4. **Dissector volume.** Wireshark has thousands of dissectors; this plan lists about 120 over three phases, written from the protocol specifications in safe-subset C++23. Coverage will lag for years. Declarative dissectors reduce the cost of the long tail.
5. **No reuse of Wireshark code.** It is GPL-2.0 C and the main historical source of dissector vulnerabilities. Using `tshark` as a differential test oracle only is the plan.
6. **Display-filter compatibility.** The language is close to the familiar one but stricter (`!=` semantics, type errors). An import that translates common foreign filters is possible; full compatibility is not a goal.
7. **Checksum and offload artefacts.** Captures on the sending host show unfilled checksums and large segments; defaults avoid false errors but need testing on the target drivers.
8. **SSH client service.** Shared need with DV01, DV03, SY03, VM04, IN03; not in PLATFORM.md section 8.
9. **Private enterprise number** for the pcapng custom blocks must be registered before files are shared widely.
10. **Unverified.** BPF behaviour (`BIOCLOCK`, direction flags, nanosecond timestamps), `pflog` header layout and `usbdump` on HardenedBSD 15-STABLE; whether Capsicum capability mode permits the BPF ioctls the service needs after set-up.
