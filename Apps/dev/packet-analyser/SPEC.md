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
