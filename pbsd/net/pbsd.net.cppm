export module pbsd.net;

import pbsd.core;
export import pbsd.handles;
export import pbsd.net.sockopt;
export import pbsd.net.netinet;
export import pbsd.net.tcp;
export import pbsd.net.udp;
export import pbsd.net.ip;
export import pbsd.net.icmp;
export import pbsd.net.ifnet;
export import pbsd.net.route;
export import pbsd.net.in_pcb;
export import pbsd.net.arp;
export import pbsd.net.ether;
export import pbsd.net.vlan;
export import pbsd.net.mbuf;
export import pbsd.net.bpf;
export import pbsd.net.igmp;
export import pbsd.net.bridge;
export import pbsd.net.pf;
export import pbsd.net.altq;
export import pbsd.net.in6;
export import pbsd.net.gre;
export import pbsd.net.epair;
export import pbsd.net.tun;
export import pbsd.net.tap;
export import pbsd.net.ndp;
export import pbsd.net.lagg;
export import pbsd.net.gif;
export import pbsd.net.loop;
export import pbsd.net.ipfw;
export import pbsd.net.carp;
export import pbsd.net.ip6;
export import pbsd.net.udp6;
export import pbsd.net.tcp6;
export import pbsd.net.sctp;
export import pbsd.net.netmap;
export import pbsd.net.netgraph;
export import pbsd.net.ipsec;
export import pbsd.net.vxlan;
export import pbsd.net.stf;
export import pbsd.net.wg;
export import pbsd.net.pfsync;
export import pbsd.net.dlt;
export import pbsd.net.media;
export import pbsd.net.pfil;
export import pbsd.net.lacp;
export import pbsd.net.clone;
export import pbsd.net.divert;
export import pbsd.net.netgraph.node;
export import pbsd.net.sctp_assoc;
export import pbsd.net.bridge_fdb;
export import pbsd.net.ipfw_rule;
export import pbsd.net.enc;
export import pbsd.net.mroute;
export import pbsd.net.ipdivert;
export import pbsd.net.icmp6;
export import pbsd.net.mld6;
export import pbsd.net.raw6;
export import pbsd.net.dummynet;
export import pbsd.net.ip6_mroute;
export import pbsd.net.pim6;
export import pbsd.net.if_ethersubr;
export import pbsd.net.netisr;
export import pbsd.net.in_rmx;
export import pbsd.net.ip_ecn;
export import pbsd.net.bridgestp;
export import pbsd.net.tcp_timer;
export import pbsd.net.tcp_timer_wheel;
export import pbsd.net.tcp_ecn;
export import pbsd.net.tcp_reass;
export import pbsd.net.tcp_sack;
export import pbsd.net.tcp_syncache;
export import pbsd.net.tcp_timewait;
export import pbsd.net.ip_reass;
export import pbsd.net.ip_options;
export import pbsd.net.in_cksum;
export import pbsd.net.tcp_hpts;
export import pbsd.net.raw_ip;
export import pbsd.net.in_mcast;
export import pbsd.net.ip_encap;
export import pbsd.net.if_llatbl;
export import pbsd.net.vnet;
export import pbsd.net.radix;
export import pbsd.net.ifq;
export import pbsd.net.tcp_fastopen;
export import pbsd.net.rss;
export import pbsd.net.flowtable;
export import pbsd.net.tcp_input;
export import pbsd.net.tcp_output;
export import pbsd.net.in_pcbgroup;
export import pbsd.net.in_proto;
export import pbsd.net.ip_fw_nat;
export import pbsd.net.ether_ifattach;
export import pbsd.net.udp_usrreq;
export import pbsd.net.tcp_usrreq;
export import pbsd.net.ip_input;
export import pbsd.net.ip_output;

export import pbsd.net.udp_input;
export import pbsd.net.in_pcblist;
export import pbsd.net.raw_usrreq;
export import pbsd.net.tcp_log;
export import pbsd.net.cc_cubic;
export import pbsd.net.pf_ioctl;
export import pbsd.net.rip_input;
export import pbsd.net.rip_output;
export import pbsd.net.ip_fastforward;
export import pbsd.net.tcp_rfc1323;
export import pbsd.net.ip6_input;
export import pbsd.net.ip6_output;
export import pbsd.net.tcp_subr;
export import pbsd.net.udp6_usrreq;
export import pbsd.net.frag6;
export import pbsd.net.ip6_forward;
export import pbsd.net.nd6_nbr;
export import pbsd.net.nd6_rtr;
export import pbsd.net.in6_cksum;
export import pbsd.net.in6_proto;
export import pbsd.net.in6_pcb;
export import pbsd.net.scope6;
export import pbsd.net.sctp6_usrreq;
export import pbsd.net.ip6_id;
export import pbsd.net.ip6_fastfwd;
export import pbsd.net.dest6;
export import pbsd.net.route6;
export import pbsd.net.tcp_hostcache;
export import pbsd.net.tcp_lro;
export import pbsd.net.tcp_lro_hpts;
export import pbsd.net.ip_id;
export import pbsd.net.in_fib;
export import pbsd.net.in_fib_algo;
export import pbsd.net.tcp_stats;
export import pbsd.net.altq_cbq;
export import pbsd.net.altq_codel;
export import pbsd.net.altq_fairq;
export import pbsd.net.altq_hfsc;
export import pbsd.net.altq_priq;
export import pbsd.net.altq_red;
export import pbsd.net.altq_rio;
export import pbsd.net.altq_rmclass;
export import pbsd.net.altq_subr;
export import pbsd.net.bpf_buffer;
export import pbsd.net.bpf_filter;
export import pbsd.net.bpf_jitter;
export import pbsd.net.bpf_zerocopy;
export import pbsd.net.debugnet;
export import pbsd.net.debugnet_inet;
export import pbsd.net.if_subr;
export import pbsd.net.route_helpers;
export import pbsd.net.route_rtentry;
export import pbsd.net.rtsock;
export import pbsd.net.rss_config;
export import pbsd.net.toeplitz;
export import pbsd.net.cc;
export import pbsd.net.cc_cdg;
export import pbsd.net.cc_chd;
export import pbsd.net.cc_dctcp;
export import pbsd.net.cc_hd;
export import pbsd.net.cc_htcp;
export import pbsd.net.cc_newreno;
export import pbsd.net.accf_data;
export import pbsd.net.accf_http;
export import pbsd.net.tcp_offload;
export import pbsd.net.icmp6_input;
export import pbsd.net.dummymbuf;
export import pbsd.net.ieee8023ad_lacp;
export import pbsd.net.iflib;
export import pbsd.net.mp_ring;
export import pbsd.net.mppcc;
export import pbsd.net.mppcd;
export import pbsd.net.fib_algo;
export import pbsd.net.nhgrp;
export import pbsd.net.nhgrp_ctl;
export import pbsd.net.nhop;
export import pbsd.net.nhop_ctl;
export import pbsd.net.nhop_utils;
export import pbsd.net.route_ctl;
export import pbsd.net.route_ddb;
export import pbsd.net.route_ifaddrs;
export import pbsd.net.route_subscription;
export import pbsd.net.route_tables;
export import pbsd.net.route_temporal;
export import pbsd.net.slcompress;
export import pbsd.net.ieee80211;
export import pbsd.net.ieee80211_acl;
export import pbsd.net.ieee80211_action;
export import pbsd.net.ieee80211_adhoc;
export import pbsd.net.ieee80211_ageq;
export import pbsd.net.toecore;
export import pbsd.net.tcp_ratelimit;
export import pbsd.net.tcp_log_buf;
export import pbsd.net.siftr;
export import pbsd.net.accf_dns;
export import pbsd.net.accf_tls;
export import pbsd.net.in_debug;
export import pbsd.net.in_jail;
export import pbsd.net.in_rss;
export import pbsd.net.sctp_pcb;
export import pbsd.net.sctp_input;
export import pbsd.net.cc_vegas;
export import pbsd.net.in_core;
export import pbsd.net.in_fib_dxr;
export import pbsd.net.in_gif;
export import pbsd.net.in_kdtrace;
export import pbsd.net.in_prot;
export import pbsd.net.ip_carp;
export import pbsd.net.ip_divert;
export import pbsd.net.ip_gre;
export import pbsd.net.ip_icmp;
export import pbsd.net.ip_mroute;
export import pbsd.net.h_ertt;
export import pbsd.net.sctp_auth;
export import pbsd.net.sctp_output;
export import pbsd.net.sctp_timer;
export import pbsd.net.sctp_usrreq;
export import pbsd.net.sctputil;
export import pbsd.net.pf_table;
export import pbsd.net.pf_osfp;
export import pbsd.net.pf_norm;
export import pbsd.net.pf_lb;
/// Wave 6 — C++23 network stack façade
/// PROVENANCE: hbsd/src/sys/netinet, sys/sys/socket.h, kern/uipc_socket.c
export namespace pbsd::net {

enum class SocketType : unsigned char {
    Stream = 1, // SOCK_STREAM
    Dgram  = 2, // SOCK_DGRAM
    Raw    = 3, // SOCK_RAW
};

enum class AddressFamily : unsigned char {
    Unspec = 0,
    Unix   = 1, // AF_UNIX
    Inet   = 2, // AF_INET
    Inet6  = 28, // AF_INET6
};

struct SocketObject {
    static void release(SocketObject* p) noexcept { (void)p; }
};

using SocketHandle = UniqueHandle<SocketObject>;

struct Endpoint {
    unsigned char addr[16]{};
    unsigned short port{0};
    unsigned char family{static_cast<unsigned char>(AddressFamily::Inet)};
};

class Stack {
public:
    [[nodiscard]] Status init() noexcept {
        up_ = true;
        return Status::Ok;
    }

    [[nodiscard]] bool up() const noexcept { return up_; }

    /// Capability-checked socket creation (SI-2).
    [[nodiscard]] Result<SocketHandle> socket(SocketType type, CapabilityRights want,
                                              LineageId lineage) noexcept {
        if (!up_) {
            return {Status::Invalid, SocketHandle{}};
        }
        CapabilityRights rights = CapabilityRights::Read | CapabilityRights::Write;
        if (type == SocketType::Stream) {
            rights = rights | CapabilityRights::Grant;
        }
        rights = narrow_rights(rights, want);
        if (rights == CapabilityRights::None) {
            return {Status::Denied, SocketHandle{}};
        }
        return {Status::Ok, SocketHandle{reinterpret_cast<SocketObject*>(1), rights, lineage}};
    }

    [[nodiscard]] Status bind(const SocketHandle& sock, const Endpoint& ep) noexcept {
        if (!sock.valid() || !sock.has_right(CapabilityRights::Write)) {
            return Status::Denied;
        }
        (void)ep;
        return Status::Ok;
    }

    [[nodiscard]] Status connect(const SocketHandle& sock, const Endpoint& ep) noexcept {
        if (!sock.valid()
            || !sock.has_right(CapabilityRights::Write)
            || !sock.has_right(CapabilityRights::Read)) {
            return Status::Denied;
        }
        (void)ep;
        return Status::Ok;
    }

    [[nodiscard]] Status send(const SocketHandle& sock, CapabilityRights need_write) noexcept {
        if (!sock.valid() || !sock.has_right(need_write)) {
            return Status::Denied;
        }
        return Status::Ok;
    }

    /// Capsicum boundary: maps sopt_rights checks from uipc_socket.c.
    [[nodiscard]] Status setsockopt(const SocketHandle& sock,
                                    sockopt::Level level, int name,
                                    sockopt::OptDir dir) noexcept {
        if (!sock.valid()) {
            return Status::Invalid;
        }
        if (level == sockopt::Level::Tcp) {
            return tcp::check_option(sock.rights(), static_cast<tcp::Option>(name), dir);
        }
        if (level == sockopt::Level::Udp) {
            return udp::check_option(sock.rights(), static_cast<udp::Option>(name), dir);
        }
        return sockopt::check_socket_option(sock.rights(), level, name, dir);
    }

    [[nodiscard]] Status getsockopt(const SocketHandle& sock,
                                    sockopt::Level level, int name) noexcept {
        return setsockopt(sock, level, name, sockopt::OptDir::Get);
    }

private:
    bool up_{false};
};

} // namespace pbsd::net
