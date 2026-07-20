// Userland bin smoke tests — logic-only, no full OS required.
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.core;
import pbsd.userland.hosted;
import pbsd.userland.capsicum.helpers;
import pbsd.userland.util.filemode;
import pbsd.userland.echo;
import pbsd.userland.hostname;
import pbsd.userland.domainname;
import pbsd.userland.nproc;
import pbsd.userland.pwd;
import pbsd.userland.cat;
import pbsd.userland.chmod;
import pbsd.userland.mkdir;
import pbsd.userland.rmdir;
import pbsd.userland.ln;
import pbsd.userland.mv;
import pbsd.userland.cp;
import pbsd.userland.kill;
import pbsd.userland.sleep;
import pbsd.userland.true_cmd;
import pbsd.userland.false_cmd;
import pbsd.userland.test;
import pbsd.userland.date;
import pbsd.userland.chflags;
import pbsd.userland.dd;
import pbsd.userland.getfacl;
import pbsd.userland.setfacl;
import pbsd.userland.sync;
import pbsd.userland.uuidgen;
import pbsd.userland.realpath;
import pbsd.userland.kenv;
import pbsd.userland.ls.cmp;
import pbsd.userland.stty.util;
import pbsd.userland.chown;
import pbsd.userland.df;
import pbsd.userland.yes;
import pbsd.userland.basename;
import pbsd.userland.dirname;
import pbsd.userland.uname;
import pbsd.userland.printenv;
import pbsd.userland.env;
import pbsd.userland.which;
import pbsd.userland.time_cmd;
import pbsd.userland.touch;
import pbsd.userland.truncate;
import pbsd.userland.id;
import pbsd.userland.head;
import pbsd.userland.tail;
import pbsd.userland.wc;
import pbsd.userland.tee;
import pbsd.userland.mktemp;
import pbsd.userland.printf;
import pbsd.userland.ifconfig;
import pbsd.userland.mount;
import pbsd.userland.route;
import pbsd.userland.install;
import pbsd.userland.tr;
import pbsd.userland.cut;
import pbsd.userland.sort;
import pbsd.userland.uniq;
import pbsd.userland.comm;
import pbsd.userland.cmp;
import pbsd.userland.diff;
import pbsd.userland.seq;
import pbsd.userland.jot;
import pbsd.userland.calendar;
import pbsd.userland.logger;
import pbsd.userland.script;
import pbsd.userland.join;
import pbsd.userland.paste;
import pbsd.userland.nl;
import pbsd.userland.fold;
import pbsd.userland.split;
import pbsd.userland.expand;
import pbsd.userland.tsort;
import pbsd.userland.rev;
import pbsd.userland.umount;
import pbsd.userland.ping;
import pbsd.userland.du;
import pbsd.userland.find;
import pbsd.userland.xargs;
import pbsd.userland.grep;
import pbsd.userland.sed;
import pbsd.userland.awk;
import pbsd.userland.patch;
import pbsd.userland.gzip;
import pbsd.userland.ssh;
import pbsd.userland.fetch;
import pbsd.userland.tar;
import pbsd.userland.pkg_cmd;
import pbsd.userland.newsyslog;
import pbsd.userland.cron;
import pbsd.userland.pw;
import pbsd.userland.vipw;
import pbsd.userland.find.helpers;
import pbsd.userland.grep.context;
import pbsd.userland.sed.addr;
import pbsd.userland.awk.fields;
import pbsd.userland.xargs.quote;
import pbsd.userland.bzip2;
import pbsd.userland.compress;
import pbsd.userland.cpio;
import pbsd.userland.xz;
import pbsd.userland.curl;
import pbsd.userland.scp;
import pbsd.userland.ftp;
import pbsd.userland.file;
import pbsd.userland.hexdump;
import pbsd.userland.stat;
import pbsd.userland.strings;
import pbsd.userland.cksum;
import pbsd.userland.netstat;
import pbsd.userland.sockstat;
import pbsd.userland.vmstat;
import pbsd.userland.who;
import pbsd.userland.w_cmd;
import pbsd.userland.ldd;
import pbsd.userland.iconv;
import pbsd.userland.service;
import pbsd.userland.services;
import pbsd.userland.sysrc;
import pbsd.userland.periodic;
import pbsd.userland.syslogd;
import pbsd.userland.rcorder;
import pbsd.userland.bectl;
import pbsd.userland.zfs;
import pbsd.userland.zpool;
import pbsd.userland.md5;
import pbsd.userland.fsck;
import pbsd.userland.geom;
import pbsd.userland.newfs;
import pbsd.userland.camcontrol;
import pbsd.userland.rsync;
import pbsd.userland.gpart;
import pbsd.userland.mdconfig;
import pbsd.userland.dumpon;
import pbsd.userland.savecore;
import pbsd.userland.fsck_ffs;
import pbsd.userland.fsck_msdosfs;
import pbsd.userland.newfs_msdos;
import pbsd.userland.tunefs;
import pbsd.userland.growfs;
import pbsd.userland.dmesg;
import pbsd.userland.dumpfs;
import pbsd.userland.fdisk;
import pbsd.userland.bsdlabel;
import pbsd.userland.swapon;
import pbsd.userland.shutdown;
import pbsd.userland.reboot;
import pbsd.userland.kldload;
import pbsd.userland.util;
import pbsd.userland.casper.services;
import pbsd.userland.jail;
import pbsd.userland.jail.param;
import pbsd.userland.libc;
import pbsd.userland.libthr;
import pbsd.userland.rtld;
import pbsd.userland.msun;
import pbsd.userland.capsicum.rights;

#include <cstdint>

namespace {

int failures = 0;
int g_once_test_ran = 0;

void once_test_init() { g_once_test_ran = 1; }

void expect(bool cond, const char* msg) {
    if (!cond) {
        std::printf("FAIL: %s\n", msg);
        ++failures;
    }
}

void test_echo_parse() {
    using namespace pbsd::userland::bin::echo;

    char a0[] = "-n";
    char a1[] = "hello";
    char* argv_n[] = {a0, a1, nullptr};

    const auto opt = parse_options(argv_n);
    expect(opt.has_value() && opt.value.no_newline, "echo -n flag");

    char b0[] = "hello";
    char* argv_plain[] = {b0, nullptr};
    const auto opt2 = parse_options(argv_plain);
    expect(opt2.has_value() && !opt2.value.no_newline, "echo no flag");

    Options o{};
    o.no_newline = false;
    char c0[] = "foo\\c";
    char* argv_c[] = {c0, nullptr};
    const auto wr = write_args(-1, argv_c, o);
    expect(wr.status == pbsd::Status::Invalid, "echo invalid fd rejected");
}

void test_hostname_display() {
    using namespace pbsd::userland::bin::hostname;
    using pbsd::userland::hosted::cstrcmp;

    char buf[] = "host.example.com";
    const char* full = display_name(buf, DisplayMode::Full);
    expect(cstrcmp(full, "host.example.com") == 0, "hostname full");

    char buf2[] = "host.example.com";
    const char* short_ = display_name(buf2, DisplayMode::Short);
    expect(cstrcmp(short_, "host") == 0, "hostname -s");

    char buf3[] = "host.example.com";
    const char* dom = display_name(buf3, DisplayMode::Domain);
    expect(cstrcmp(dom, "example.com") == 0, "hostname -d");
}

void test_hostname_parse() {
    using namespace pbsd::userland::bin::hostname;

    char prog[] = "hostname";
    char fs[] = "-s";
    char* argv[] = {prog, fs, nullptr};
    const auto p = parse_args(2, argv);
    expect(p.has_value() && p.value.opts.mode == DisplayMode::Short, "hostname parse -s");

    char fd[] = "-d";
    char* bad[] = {prog, fs, fd, nullptr};
    const auto badp = parse_args(3, bad);
    expect(!badp.has_value(), "hostname -s -d conflict");
}

void test_domainname_parse() {
    using namespace pbsd::userland::bin::domainname;

    char prog[] = "domainname";
    char val[] = "example.com";
    char* argv[] = {prog, val, nullptr};
    const auto p = parse_args(2, argv);
    expect(p.has_value() && p.value.set_name != nullptr, "domainname set arg");

    char badf[] = "-x";
    char* badv[] = {prog, badf, nullptr};
    const auto bad = parse_args(2, badv);
    expect(!bad.has_value(), "domainname rejects flags");
}

void test_nproc_logic() {
    using namespace pbsd::userland::bin::nproc;

    expect(apply_ignore(8, 0) == 8, "nproc no ignore");
    expect(apply_ignore(4, 4) == 1, "nproc ignore >= cpus");
    expect(apply_ignore(4, 2) == 2, "nproc ignore subtract");

    const auto ign = parse_ignore("42");
    expect(ign.has_value() && ign.value == 42, "nproc parse ignore");

    char prog[] = "nproc";
    char all[] = "--all";
    char* argv[] = {prog, all, nullptr};
    const auto p = parse_args(2, argv);
    expect(p.has_value() && p.value.all, "nproc --all");
}

void test_pwd_parse() {
    using namespace pbsd::userland::bin::pwd;

    char prog[] = "pwd";
    char p[] = "-P";
    char* argv[] = {prog, p, nullptr};
    const auto opt = parse_args(2, argv);
    expect(opt.has_value() && opt.value.mode == PathMode::Physical, "pwd -P");
}

void test_capsicum_helpers() {
    using namespace pbsd::userland::capsicum;

    expect(limit_stream(-1, StreamLimitFlags::Write) == pbsd::Status::Invalid,
           "capsicum invalid fd");

    const auto rights = stream_rights(StreamLimitFlags::Read | StreamLimitFlags::Write);
    expect(pbsd::has_right(rights, pbsd::CapabilityRights::Read), "stream read right");
    expect(pbsd::has_right(rights, pbsd::CapabilityRights::Write), "stream write right");
}

void test_filemode() {
    using namespace pbsd::userland::util::filemode;

    const auto m = parse_octal_mode("755");
    expect(m.has_value() && m.value == 0755u, "filemode 755");
    expect(apply_mode_template(0100644u, 0755u) == 0100755u, "apply mode template");
}

void test_cat_parse() {
    using namespace pbsd::userland::bin::cat;

    char prog[] = "cat";
    char fl[] = "-benstuv";
    char* argv[] = {prog, fl, nullptr};
    int optind = 0;
    const auto opt = parse_options(2, argv, optind);
    expect(opt.has_value() && opt.value.number_nonblank && opt.value.show_nonprinting,
           "cat -b implies cooked");
    expect(wants_cooked_mode(opt.value), "cat cooked mode");
    expect(is_stdin_path("-"), "cat stdin dash");
}

void test_chmod_parse() {
    using namespace pbsd::userland::bin::chmod;

    char prog[] = "chmod";
    char rf[] = "-R";
    char mode[] = "755";
    char file[] = "x";
    char* argv[] = {prog, rf, mode, file, nullptr};
    const auto opt = parse_args(4, argv);
    expect(opt.has_value() && opt.value.recursive, "chmod -R");
    const auto fts = fts_mode_from_options(opt.value);
    expect(fts.has_value(), "chmod fts mode");
    expect(mode_unchanged(0755u, 0755u, false), "chmod skip unchanged");
}

void test_mkdir_parse() {
    using namespace pbsd::userland::bin::mkdir;

    char prog[] = "mkdir";
    char pm[] = "-p";
    char mm[] = "-m755";
    char dir[] = "a/b/c";
    char* argv[] = {prog, pm, mm, dir, nullptr};
    const auto opt = parse_args(4, argv);
    expect(opt.has_value() && opt.value.parents && opt.value.mode == 0755u, "mkdir -pm755");
    const auto segs = count_path_segments("a/b/c");
    expect(segs.has_value() && segs.value == 3u, "mkdir path segments");
}

void test_rmdir_parse() {
    using namespace pbsd::userland::bin::rmdir;

    char prog[] = "rmdir";
    char pv[] = "-pv";
    char dir[] = "d";
    char* argv[] = {prog, pv, dir, nullptr};
    const auto opt = parse_args(3, argv);
    expect(opt.has_value() && opt.value.parents && opt.value.verbose, "rmdir -pv");

    char path[] = "a/b/c///";
    trim_trailing_slashes(path);
    expect(pbsd::userland::hosted::cstrcmp(path, "a/b/c") == 0, "rmdir trim slashes");
}

void test_ln_helpers() {
    using namespace pbsd::userland::bin::ln;

    expect(same_dir_entry_names("/tmp/foo", "/var/foo"), "ln basename match");
    char out[64];
    expect(join_target_path(out, sizeof(out), "/tmp", "/src/bar"), "ln join target");
    expect(link_char(true) == '-', "ln symlink char");
}

void test_mv_helpers() {
    using namespace pbsd::userland::bin::mv;

    char prog[] = "mv";
    char f[] = "-fn";
    char a[] = "a";
    char b[] = "b";
    char* argv[] = {prog, f, a, b, nullptr};
    const auto opt = parse_args(4, argv);
    expect(opt.has_value() && opt.value.no_clobber, "mv -fn (n wins over f)");
    expect(basename_matches("/tmp/x/", "x"), "mv trailing basename");
}

void test_cp_parse() {
    using namespace pbsd::userland::bin::cp;

    char prog[] = "cp";
    char fl[] = "-a";
    char a[] = "a";
    char b[] = "b";
    char* argv[] = {prog, fl, a, b, nullptr};
    const auto opt = parse_args(4, argv);
    expect(opt.has_value() && opt.value.archive && opt.value.recursive, "cp -a");
    expect(classify_operation(1, false, false) == Operation::DirToDne, "cp classify dne");
}

void test_kill_parse() {
    using namespace pbsd::userland::bin::kill;

    const auto sig = parse_signal_token("TERM");
    expect(sig.has_value() && sig.value == 15, "kill TERM");
    const auto pid = parse_pid("1234");
    expect(pid.has_value() && pid.value == 1234, "kill pid");

    char prog[] = "kill";
    char s[] = "-s";
    char hup[] = "HUP";
    char p[] = "1";
    char* argv[] = {prog, s, hup, p, nullptr};
    const auto opt = parse_args(4, argv);
    expect(opt.has_value() && opt.value.signal == 1, "kill -s HUP");
}

void test_sleep_parse() {
    using namespace pbsd::userland::bin::sleep;

    const auto one = parse_interval("1.5s");
    expect(one.has_value() && one.value > 1.4 && one.value < 1.6, "sleep 1.5s");
    const auto mins = parse_interval("2m");
    expect(mins.has_value() && mins.value == 120.0, "sleep 2m");
    const auto zero = parse_interval("0");
    expect(zero.has_value() && zero.value == 0.0, "sleep zero immediate");
}

void test_true_false() {
    expect(pbsd::userland::bin::true_::exit_code() == 0
               && ok(pbsd::userland::bin::true_::run(0, nullptr).status),
           "true exits 0");
    expect(pbsd::userland::bin::false_::exit_code() == 1, "false exits 1");
}

void test_lexer() {
    using namespace pbsd::userland::bin::test;

    expect(find_op("-eq") == Token::IntEq, "test -eq token");
    expect(find_op("=") == Token::StrEq, "test = token");
    const auto cmp = int_compare("10", "2");
    expect(cmp.has_value() && cmp.value > 0, "test int compare");
    expect(evaluate_string_relation(Token::StrEq, "a", "a"), "test streq");
    expect(is_bracket_invocation("test", "["), "test [ invocation");
}

void test_date_helpers() {
    using namespace pbsd::userland::bin::date;

    const auto iso = lookup_iso8601_part("seconds");
    expect(iso.has_value(), "date iso8601 seconds");
    const auto vary = parse_vary_spec("+1d");
    expect(vary.has_value() && vary.value.op == '+' && vary.value.unit == 'd', "date vary +1d");

    const char* ts = "202607191530.45";
    const auto fields = parse_compact_datetime(ts);
    expect(fields.has_value() && fields.value.year == 126 && fields.value.mon == 6 &&
               fields.value.mday == 19 && fields.value.hour == 15 && fields.value.min == 30 &&
               fields.value.sec == 45,
           "date compact parse");
    expect(days_in_month(126, 1) == 28 || days_in_month(126, 1) == 29, "date feb days");
}

void test_util_fparseln() {
    using namespace pbsd::userland::util;

    char line[] = "hello";
    unsigned len = static_cast<unsigned>(std::strlen(line));
    expect(!strip_line_end(line, len, {}), "fparseln no continuation");

    char esc[] = "foo\\#bar";
    len = static_cast<unsigned>(std::strlen(esc));
    len = unescape_line(esc, len, FPARSELN_UNESCALL, {});
    expect(len == 7 && std::strcmp(esc, "foo#bar") == 0, "fparseln unescape");
}

void test_util_numbers() {
    using namespace pbsd::userland::util;

    char buf[64];
    expect(humanize_number(buf, sizeof(buf), 1536, "B", 1, HN_DIVISOR_1000) > 0,
           "humanize_number");
    long long n = 0;
    expect(expand_number("1k", &n) == 0 && n == 1024, "expand_number 1k");
}

void test_util_secure_path() {
    using namespace pbsd::userland::util;

    auto lstat = [](const char*, PathStat* out) noexcept -> int {
        out->exists = true;
        out->is_regular = true;
        out->world_writable = false;
        out->uid = 0;
        out->gid = 0;
        out->group_writable = false;
        return 0;
    };
    expect(secure_path("/etc/passwd", 0, 0, lstat) == kSecurePathOk,
           "secure_path ok");
}

void test_util_paths() {
    using namespace pbsd::userland::util;

    expect(std::strcmp(getlocalbase(), kDefaultLocalBase) == 0, "getlocalbase default");
    expect(std::strcmp(resolve_localbase("/opt/local", nullptr, false), "/opt/local") == 0,
           "resolve_localbase env");
}

void test_casper_services() {
    using namespace pbsd::userland::casper;
    using namespace pbsd::userland::casper::fileargs;

    expect(validate_service_name(sysctl::kServiceName) == pbsd::Status::Ok, "sysctl service name");
    expect(flags_valid(FA_OPEN | FA_LSTAT), "fileargs flags");
    expect(!flags_valid(0), "fileargs flags reject zero");
}

void test_jail_param() {
    using namespace pbsd::userland::jail;

    JailParam jp{};
    expect(param_init(jp, "name") == pbsd::Status::Ok, "jail param_init");
    expect(param_import(jp, "value") == pbsd::Status::Ok, "jail param_import");

    bool b = false;
    expect(parse_bool("true", b) == pbsd::Status::Ok && b, "jail parse_bool");

    const auto jid = get_jid("42");
    expect(jid.has_value() && jid.value == 42, "jail get_jid numeric");
}

void test_libc_memory() {
    char buf[16]{};
    pbsd::userland::libc::memset(buf, 'A', 8);
    expect(buf[0] == 'A' && buf[7] == 'A' && buf[8] == '\0', "memset");

    pbsd::userland::libc::bzero(buf, sizeof(buf));
    expect(buf[0] == '\0' && buf[15] == '\0', "bzero");

    char src[] = "hello";
    char dst[8]{};
    pbsd::userland::libc::memcpy(dst, src, 6);
    expect(std::strcmp(dst, "hello") == 0, "memcpy");

    char overlap[] = "abcdef";
    pbsd::userland::libc::memmove(overlap + 1, overlap, 5);
    expect(std::strncmp(overlap, "aabcde", 6) == 0, "memmove overlap");
}

void test_chflags_parse() {
    using namespace pbsd::userland::bin::chflags;

    char prog[] = "chflags";
    char rf[] = "-R";
    char fl[] = "hidden";
    char file[] = "x";
    char* argv[] = {prog, rf, fl, file, nullptr};
    const auto opt = parse_args(4, argv);
    expect(opt.has_value() && opt.value.recursive, "chflags -R");

    const auto masks = parse_flags_arg("755");
    expect(masks.has_value() && masks.value.octal, "chflags octal flags");
    const auto sym = parse_flags_arg("hidden");
    expect(sym.has_value() && (sym.value.set & kUfHidden) != 0, "chflags symbolic");
}

void test_dd_parse() {
    using namespace pbsd::userland::bin::dd;

    char a0[] = "if=/dev/zero";
    char a1[] = "of=/dev/null";
    char a2[] = "bs=1k";
    char a3[] = "count=2";
    char* argv[] = {a0, a1, a2, a3, nullptr};
    const auto st = parse_jcl_operands(argv);
    expect(st.has_value() && st.value.ibs == 1024 && st.value.count == 2, "dd jcl parse");

    const auto n = parse_numeric("4k", false);
    expect(n.has_value() && n.value == 4096, "dd numeric 4k");
}

void test_getfacl_parse() {
    using namespace pbsd::userland::bin::getfacl;

    char prog[] = "getfacl";
    char fl[] = "-dnq";
    char file[] = "x";
    char* argv[] = {prog, fl, file, nullptr};
    const auto opt = parse_args(3, argv);
    expect(opt.has_value() && opt.value.numeric && opt.value.omit_header, "getfacl -dnq");
}

void test_setfacl_helpers() {
    using namespace pbsd::userland::bin::setfacl;

    expect(pbsd::userland::hosted::cstrcmp(brand_name(AclBrand::Nfs4), "NFSv4") == 0,
           "setfacl brand nfs4");
    expect(branding_mismatch(AclBrand::Nfs4, AclBrand::Posix), "setfacl brand mismatch");

    char prog[] = "setfacl";
    char b[] = "-b";
    char file[] = "x";
    char* argv[] = {prog, b, file, nullptr};
    const auto opt = parse_args(3, argv);
    expect(opt.has_value() && opt.value.op_count == 1, "setfacl -b");
}

void test_chown_parse() {
    using namespace pbsd::userland::sbin::chown;

    char prog[] = "chown";
    char own[] = "100:200";
    char file[] = "x";
    char* argv[] = {prog, own, file, nullptr};
    const auto opt = parse_args(3, argv, prog);
    expect(opt.has_value() && opt.value.owner.uid == 100 && opt.value.owner.gid == 200,
           "chown uid:gid");
}

void test_df_parse() {
    using namespace pbsd::userland::sbin::df;

    char prog[] = "df";
    char fl[] = "-hik";
    char* argv[] = {prog, fl, nullptr};
    const auto opt = parse_args(2, argv);
    expect(opt.has_value() && opt.value.show_inodes && opt.value.blocksize == BlockSizeMode::Kilo,
           "df -hik");
    expect(fsbtoblk(2, 4096, 512) == 16, "df fsbtoblk");
}

void test_sync_run() {
    expect(ok(pbsd::userland::bin::sync::run().status), "sync run");
}

void test_uuidgen_parse() {
    using namespace pbsd::userland::bin::uuidgen;

    char prog[] = "uuidgen";
    char n[] = "-n";
    char c[] = "3";
    char* argv[] = {prog, n, c, nullptr};
    const auto opt = parse_args(3, argv);
    expect(opt.has_value() && opt.value.count == 3, "uuidgen -n3");

    UuidFields u{};
    u.clock_seq_hi_and_reserved = 0xff;
    u.time_hi_and_version = 0xffff;
    apply_uuid_v4_version(u);
    expect((u.time_hi_and_version & (15 << 12)) == (4 << 12), "uuidgen v4 version bits");
}

void test_realpath_parse() {
    using namespace pbsd::userland::bin::realpath;

    char prog[] = "realpath";
    char q[] = "-q";
    char path[] = "/tmp";
    char* argv[] = {prog, q, path, nullptr};
    const auto opt = parse_args(3, argv);
    expect(opt.has_value() && opt.value.quiet, "realpath -q");
}

void test_kenv_parse() {
    using namespace pbsd::userland::bin::kenv;

    char prog[] = "kenv";
    char l[] = "-l";
    char* argv[] = {prog, l, nullptr};
    const auto opt = parse_args(2, argv);
    expect(opt.has_value() && opt.value.dump == DumpMode::Loader, "kenv -l");
}

void test_ls_cmp() {
    using namespace pbsd::userland::bin::ls::cmp;

    Entry a{"a", {}};
    Entry b{"b", {}};
    a.stat.size = 10;
    b.stat.size = 20;
    expect(sizecmp(a, b) < 0, "ls sizecmp");
    expect(namecmp(a, b) < 0, "ls namecmp");
}

void test_stty_util() {
    using namespace pbsd::userland::bin::stty::util;

    TtyDevice out{1, true};
    TtyDevice err{2, true};
    expect(should_warn_redirect(out, err), "stty redirect warn");
}

void test_libc_string() {
    namespace ulc = pbsd::userland::libc;

    expect(ulc::strlen("hello") == 5, "strlen");
    expect(ulc::strcmp("abc", "abc") == 0, "strcmp equal");
    expect(ulc::strcmp("abc", "abd") < 0, "strcmp less");
    expect(ulc::strncmp("abc", "abd", 2) == 0, "strncmp prefix");

    char dst[16]{};
    ulc::strcpy(dst, "foo");
    expect(std::strcmp(dst, "foo") == 0, "strcpy");
    expect(ulc::strlcpy(dst, "bar", sizeof(dst)) == 3, "strlcpy");
    expect(ulc::strnlen("hello", 3) == 3, "strnlen");

    char catbuf[16] = "foo";
    ulc::strcat(catbuf, "bar");
    expect(std::strcmp(catbuf, "foobar") == 0, "strcat");

    char pad[8]{};
    ulc::strncpy(pad, "abcdef", 4);
    expect(std::strncmp(pad, "abcd", 4) == 0, "strncpy");

    expect(ulc::strchr("hello", 'l') != nullptr, "strchr");
    expect(ulc::strrchr("hello", 'l') != nullptr && *ulc::strrchr("hello", 'l') == 'l',
           "strrchr");
}

void test_libc_memory_compare() {
    namespace ulc = pbsd::userland::libc;

    expect(ulc::memcmp("abc", "abd", 3) < 0, "memcmp");
    const char hay[] = "hello";
    expect(ulc::memchr(hay, 'l', 5) == hay + 2, "memchr");
}

void test_libc_stdlib() {
    namespace ulc = pbsd::userland::libc;

    expect(ulc::atoi("42") == 42, "atoi");
    expect(ulc::atol("99") == 99, "atol");
    expect(ulc::strtol("0xff", nullptr, 0) == 255, "strtol hex auto");
    expect(ulc::strtol("-10", nullptr, 10) == -10, "strtol negative");
    expect(ulc::strtoul("0x10", nullptr, 0) == 16, "strtoul hex");
    expect(ulc::abs(-7) == 7, "abs");
    expect(ulc::labs(-9L) == 9L, "labs");
}

void test_libc_sort() {
    namespace ulc = pbsd::userland::libc;

    int arr[] = {5, 3, 8, 1, 2};
    ulc::qsort(arr, 5, sizeof(int), [](const void* a, const void* b) {
        return *static_cast<const int*>(a) - *static_cast<const int*>(b);
    });
    expect(arr[0] == 1 && arr[4] == 8, "qsort");

    const int sorted[] = {1, 3, 5, 7, 9};
    const int key = 5;
    const void* found = ulc::bsearch(&key, sorted, 5, sizeof(int),
                                     [](const void* a, const void* b) {
                                         return *static_cast<const int*>(a) -
                                                *static_cast<const int*>(b);
                                     });
    expect(found == sorted + 2, "bsearch");
}

void test_libc_gen() {
    namespace ulc = pbsd::userland::libc;

    char path[] = "/usr/local/bin/foo";
    expect(std::strcmp(ulc::dirname(path), "/usr/local/bin") == 0, "dirname");
    char base[] = "/usr/local/bin/foo";
    expect(std::strcmp(ulc::basename(base), "foo") == 0, "basename");
}

void test_libc_wave2_burst_b6() {
    namespace ulc = pbsd::userland::libc;
    namespace ulc_stdio = pbsd::userland::libc::stdio;
    namespace ulc_locale = pbsd::userland::libc::locale;

    expect(ulc::iscntrl('\n') != 0 && ulc::isblank(' ') != 0, "ctype ext");
    expect(ulc_locale::setlocale(ulc_locale::Category::All, nullptr) != nullptr,
           "setlocale default");
    expect(std::strcmp(ulc_locale::localeconv().decimal_point, ".") == 0, "localeconv");
    expect(ulc_locale::mblen("a", 1) == 1, "mblen ascii");
    wchar_t wbuf[4]{};
    expect(ulc_locale::mbstowcs(wbuf, "ab", 4) == 2, "mbstowcs");
    char mbuf[8]{};
    expect(ulc_locale::wcstombs(mbuf, L"x", 8) == 1, "wcstombs");
    expect(std::strcmp(ulc_locale::nl_langinfo(ulc_locale::NlItem::Codeset), "UTF-8") == 0,
           "nl_langinfo");
    expect(ulc_locale::btowc('A') == L'A', "btowc");
    expect(ulc_locale::wctob(L'Z') == 'Z', "wctob");

    ulc_stdio::IoFile fp{};
    fp.fd = 1;
    expect(ulc_stdio::fileno(&fp) == 1, "stdio fileno");
    expect(ulc_stdio::putc('x', &fp) == 'x', "stdio putc");
    expect(ulc_stdio::fputs("hi", &fp) == 2, "stdio fputs");
    expect(ulc_stdio::puts("ok") == 3, "stdio puts");
    char sbuf[8]{};
    expect(ulc_stdio::bounded_copy(sbuf, sizeof(sbuf), "abc") == 3, "stdio bounded_copy");
    expect(std::strcmp(ulc_stdio::errno_message(EINVAL), "Invalid argument") == 0,
           "perror msg");

    ulc_stdio::OpenMode mode{};
    expect(ulc_stdio::parse_fopen_mode("r+", mode) && ulc_stdio::mode_allows_read(mode),
           "fopen mode");

    expect(ulc::atof("3.5") > 3.4 && ulc::atof("3.5") < 3.6, "atof");
    const char* err = nullptr;
    expect(ulc::strtonum("42", 0, 100, &err) == 42 && err == nullptr, "strtonum");

    ulc::GetoptState gst{};
    char prog[] = "x";
    char optv[] = "-v";
    char* gargv[] = {prog, optv, nullptr};
    expect(ulc::getopt(2, gargv, "v", gst) == 'v', "getopt -v");

    char env0[] = "FOO=bar";
    char* environ[] = {env0, nullptr};
    expect(ulc::lookup_env(environ, "FOO") != nullptr, "getenv lookup");

    int hs[] = {5, 3, 8, 1};
    ulc::heapsort(hs, 4, sizeof(int), [](const void* a, const void* b) {
        return *static_cast<const int*>(a) - *static_cast<const int*>(b);
    });
    expect(hs[0] == 1 && hs[3] == 8, "heapsort");

    expect(ulc::imaxdiv(10, 3).quot == 3 && ulc::imaxdiv(10, 3).rem == 1, "imaxdiv");
    expect(ulc::llabs(-5LL) == 5LL, "llabs");
    expect(ulc::lldiv(7LL, 2LL).rem == 1, "lldiv");
    expect(ulc::exit_code(ulc::ExitStatus::Success) == 0, "exit status");
}

void test_libthr_rtld_burst_b6() {
    using namespace pbsd::userland::libthr;
    using namespace pbsd::userland::libthr::symbols;
    using namespace pbsd::userland::rtld;

    Mutex m{};
    expect(mutex_init(m) == pbsd::Status::Ok, "mutex init");
    expect(mutex_lock(m, 1) == pbsd::Status::Ok, "mutex lock");
    expect(mutex_unlock(m, 1) == pbsd::Status::Ok, "mutex unlock");

    Cond cv{};
    expect(cond_init(cv) == pbsd::Status::Ok, "cond init");
    expect(thread_equal(thread_self(), g_main_thread), "thread self");
    OnceFlag once_flag{};
    g_once_test_ran = 0;
    once(once_flag, once_test_init);
    expect(g_once_test_ran == 1, "pthread once");

    TssKey key{};
    expect(key_create(key, nullptr) == pbsd::Status::Ok, "tss key create");
    expect(default_offsets().max_keys == kThreadMaxKeys, "libthr debug offsets");

    expect(is_exported("dlopen"), "rtld exported dlopen");
    expect(is_private_rtld_symbol("_rtld_thread_init"), "rtld private symbol");
    expect(kPublicSymbolCount == 16, "rtld public symbol count");
    expect(kVersionSetCount == 4, "rtld version sets");
    expect(std::strcmp(version_label(SymbolVersion::Fbsd10), "FBSD_1.0") == 0,
           "rtld version label");
    expect(lookup_public("dlsym") != nullptr, "rtld lookup_public");

    DlHandle h{};
    expect(dlopen(h, "libfoo.so", DlMode::Lazy) == pbsd::Status::NotImplemented, "dlopen stub");
    expect(current_version().freebsd_version == kFreeBsdVersion, "rtld version");
    expect(std::strcmp(kDefaultLibPath, "/lib:/usr/lib") == 0, "rtld paths");
}

void test_libc_wave2_burst_b8() {
    namespace ulc = pbsd::userland::libc;

    auto qsort_int_cmp = [](const void* x, const void* y, void*) noexcept -> int {
        return *static_cast<const int*>(x) - *static_cast<const int*>(y);
    };

    expect(ulc::stdc_count_ones_ui(0b1011u) == 3, "stdc_count_ones");
    expect(ulc::ffsll(8LL) == 4, "ffsll");
    expect(ulc::wcslen(L"abc") == 3, "wcslen");
    expect(ulc::wcscmp(L"a", L"b") < 0, "wcscmp");
    expect(ulc::wcsspn(L"abc123", L"abc") == 3, "wcsspn");

    ulc::QueElem a{};
    ulc::QueElem b{};
    ulc::insque(b, &a);
    expect(b.prev == &a && a.next == &b, "insque/remque");

    char l64buf[16]{};
    expect(ulc::a64l("./012") >= 0, "a64l");
    expect(ulc::l64a(42, l64buf, sizeof(l64buf)) == l64buf, "l64a");

    expect(ulc::hash_log2(8) == 3, "hash_log2");
    expect(ulc::wcwidth(L'A') == 1, "wcwidth");
    expect(ulc::getpagesize() == 4096, "getpagesize");
    expect(std::strcmp(ulc::getprogname(), "pbsd") == 0, "getprogname");
    ulc::setprogname("test");
    expect(std::strcmp(ulc::getprogname(), "test") == 0, "setprogname");
    expect(ulc::ldexp(1.0, 3) == 8.0, "ldexp");

    char linkbuf[8]{};
    expect(ulc::bounded_readlink_copy(linkbuf, sizeof(linkbuf), "/tmp") == 4, "freadlink");

    int sorted[] = {3, 1, 2};
    ulc::qsort_r(sorted, 3, sizeof(int), nullptr, +qsort_int_cmp);
    expect(sorted[0] == 1 && sorted[2] == 3, "qsort_r");

    expect(ulc::cap_sandboxed() == false, "cap_sandboxed");
    expect(ulc::uuid_is_nil(ulc::Uuid{}) == true, "uuid_nil");
}

void test_msun_burst_b8() {
    namespace ms = pbsd::userland::msun;

    expect(ms::ceilf(1.2f) == 2.0f, "ceilf");
    expect(ms::floorf(1.8f) == 1.0f, "floorf");
    expect(ms::truncf(-1.8f) == -1.0f, "truncf");
    expect(ms::signbit(-1.0) == true, "signbit");
    expect(ms::isfinite(1.0) == true, "isfinite");
    expect(ms::copysign(1.0, -1.0) == -1.0, "copysign");
    expect(ms::finite(1.0) == 1, "finite");
    expect(ms::lrint(2.4) == 2, "lrint");
    expect(ms::llround(2.6) == 3, "llround");
    int exp = 0;
    expect(ms::frexp(8.0, &exp) == 0.5 && exp == 4, "frexp");
    expect(ms::drem(10.0, 3.0) == 1.0, "drem");
}

void test_libthr_rtld_burst_b8() {
    using namespace pbsd::userland::libthr;
    using namespace pbsd::userland::rtld;

    SpinLock sl{};
    expect(spin_init(sl) == pbsd::Status::Ok, "spin init");
    expect(spin_lock(sl, g_main_thread) == pbsd::Status::Ok, "spin lock");

    Semaphore sem{};
    expect(sem_init(sem, 1) == pbsd::Status::Ok, "sem init");
    expect(sem_wait(sem) == pbsd::Status::Ok, "sem wait");

    Barrier bar{};
    unsigned gen = 0;
    expect(barrier_init(bar, 2) == pbsd::Status::Ok, "barrier init");
    expect(barrier_wait(bar, gen) == pbsd::Status::Busy, "barrier wait partial");

    MutexAttr ma{};
    expect(mutexattr_init(ma) == pbsd::Status::Ok, "mutexattr init");
    expect(mutexattr_gettype(ma) == MutexType::Normal, "mutexattr type");

    RwLock rw{};
    expect(rwlock_init(rw) == pbsd::Status::Ok, "rwlock init");
    expect(rwlock_rdlock(rw) == pbsd::Status::Ok, "rwlock rdlock");

    expect(thread_yield() == pbsd::Status::Ok, "thread yield");
    CpuSet cs{};
    expect(affinity_get(g_main_thread, cs) == pbsd::Status::Ok, "affinity get");

    expect(rtld_lock() == pbsd::Status::Ok, "rtld lock");
    expect(debug_enabled(DebugFlag::Bindings) == false, "rtld debug");
    void* p = rtld_malloc(16);
    expect(p != nullptr, "rtld malloc");
    rtld_free(p);

    RelocEntry rel{};
    rel.info = kRelocRelative;
    expect(reloc_type(rel) == kRelocRelative, "rtld reloc type");

    LibmapEntry map[] = {{"libc.so.7", "libc.so.8"}};
    expect(libmap_lookup(map, 1, "libc.so.7") != nullptr, "rtld libmap");
}

void test_libc_msun_libthr_burst_b8() {
    namespace ulc = pbsd::userland::libc;
    namespace ulm = pbsd::userland::msun;
    using namespace pbsd::userland::libthr;

    expect(ulc::stdc_count_ones_ui(0b10101) == 3, "stdbit count ones");
    expect(ulc::stdc_has_single_bit_ui(8), "stdbit single bit");
    expect(ulc::ffsll(8) == 4, "ffsll");
    expect(ulc::wcslen(L"abc") == 3, "wcslen");
    expect(ulc::wcscasecmp(L"Ab", L"ab") == 0, "wcscasecmp");

    ulc::QueElem a{};
    ulc::QueElem b{};
    ulc::insque(b, &a);
    expect(b.prev == &a && a.next == &b, "insque");
    ulc::remque(b);
    expect(a.next == nullptr, "remque");

    expect(ulc::a64l("./012") >= 0, "a64l");
    char lbuf[8]{};
    ulc::l64a(42, lbuf, sizeof(lbuf));
    expect(lbuf[0] != '\\0', "l64a");

    expect(ulc::strtoq("10", nullptr, 10) == 10, "strtoq");
    expect(ulc::hash_log2(16) == 4, "hash_log2");
    expect(ulc::uuid_is_nil(ulc::Uuid{}), "uuid nil");
    expect(ulc::cap_sandboxed() == false, "cap sandboxed default");
    expect(ulc::merge_group_list(nullptr, 0, 100, 0) == -1, "getgrouplist edge");

    ulc::UtsName un{};
    ulc::fill_default_uname(un);
    expect(std::strcmp(un.sysname, "PBSD") == 0, "uname default");
    expect(ulc::isatty_fd(1), "isatty fd");
    expect(ulc::default_bootfile() != nullptr, "getbootfile");

    expect(ulm::ceilf(1.2f) == 2.0f, "msun ceilf");
    expect(ulm::floorf(1.8f) == 1.0f, "msun floorf");
    int exp = 0;
    expect(ulm::frexp(8.0, &exp) > 0.4 && exp == 4, "msun frexp");
    expect(ulm::scalbnf(1.0f, 3) == 8.0f, "msun scalbnf");
    expect(ulm::truncf(-1.7f) == -1.0f, "msun truncf");
    expect(ulm::signbit(-1.0), "msun signbit");
    expect(ulm::isfinite(1.0), "msun isfinite");
    expect(ulm::fdim(5.0, 2.0) == 3.0, "msun fdim");

    SpinLock sl{};
    expect(spin_init(sl) == pbsd::Status::Ok, "spin init");
    expect(spin_lock(sl, 1) == pbsd::Status::Ok, "spin lock");

    Semaphore sem{};
    expect(sem_init(sem, 1) == pbsd::Status::Ok, "sem init");
    expect(sem_wait(sem) == pbsd::Status::Ok, "sem wait");

    Barrier bar{};
    expect(barrier_init(bar, 2) == pbsd::Status::Ok, "barrier init");
    unsigned gen = 0;
    expect(barrier_wait(bar, gen) == pbsd::Status::Busy, "barrier wait partial");

    MutexAttr ma{};
    expect(mutexattr_init(ma) == pbsd::Status::Ok, "mutexattr init");
    expect(mutexattr_gettype(ma) == MutexType::Normal, "mutexattr type");

    RwLock rw{};
    expect(rwlock_init(rw) == pbsd::Status::Ok, "rwlock init");
    expect(rwlock_rdlock(rw) == pbsd::Status::Ok, "rwlock rdlock");
    expect(thread_detach(2) == pbsd::Status::Ok, "thread detach");
}

void test_util_uucplock() {
    using namespace pbsd::userland::util;

    expect(std::strcmp(uu_lockerr(UU_LOCK_OK), "") == 0, "uu_lockerr ok");
    expect(std::strcmp(uu_lockerr(UU_LOCK_INUSE), "device in use") == 0,
           "uu_lockerr inuse");
}

void test_util_login_times() {
    using namespace pbsd::userland::util;

    const LoginTime lt = parse_lt("mo0900-1700");
    expect((lt.lt_dow & LTM_MON) != 0 && lt.lt_start == 540 && lt.lt_end == 1020,
           "parse_lt monday");

    TmLike tm{};
    tm.tm_wday = 1;
    tm.tm_hour = 10;
    tm.tm_min = 30;
    expect(in_ltm(&lt, &tm) == 2, "in_ltm inside window");
}

void test_capsicum_rights() {
    using namespace pbsd::userland::capsicum::rights;

    const auto table = stream_rights_table();
    expect(pbsd::has_right(table.read, pbsd::CapabilityRights::Read), "rights table read");

    const auto r = rights_for_flags(HelperFlag::Read | HelperFlag::Write);
    expect(pbsd::has_right(r, pbsd::CapabilityRights::Read), "rights_for_flags");
}

void test_jail_name_helpers() {
    using namespace pbsd::userland::jail;

    char buf[64]{};
    expect(make_noname("foo.bar", buf, sizeof(buf)) == pbsd::Status::Ok &&
               std::strcmp(buf, "foo.nobar") == 0,
           "jail noname");

    expect(make_nononame("foo.nobar", buf, sizeof(buf)) == pbsd::Status::Ok &&
               std::strcmp(buf, "foo.bar") == 0,
           "jail nononame");

    expect(make_kvname("host.ip", buf, sizeof(buf)) == pbsd::Status::Ok &&
               std::strcmp(buf, "host") == 0,
           "jail kvname");

    int js = -1;
    expect(parse_jailsys("inherit", js) == pbsd::Status::Ok && js == 2, "jail jailsys");

    const auto jid0 = get_jid("0");
    expect(jid0.has_value() && jid0.value == 0, "jail jid 0");
}

void test_util_mntopts() {
    using namespace pbsd::userland::util::mntopts;

    const MountOption table[] = {
        {"ro", 0x01, false, false},
        {"rw", 0x02, false, true},
        {nullptr, 0, false, false},
    };
    ParseResult out{};
    expect(parse_options("ro", table, out) == pbsd::Status::Ok && out.primary_flags == 0x01,
           "mntopts ro");
    expect(parse_options("norw", table, out) == pbsd::Status::Ok && (out.primary_flags & 0x02) == 0,
           "mntopts norw");
}

void test_casper_registry() {
    using namespace pbsd::userland::casper::services;

    expect(is_known_service("system.dns"), "casper known dns");
    expect(validate_known_service("system.pwd") == pbsd::Status::Ok, "casper validate pwd");
    expect(validate_known_service("system.nope") == pbsd::Status::NotFound,
           "casper unknown service");
}

void test_usr_bin_wave2() {
    using pbsd::userland::hosted::cstrcmp;

    {
        using namespace pbsd::userland::usr_bin::yes;
        char buf[64]{};
        char a0[] = "hello";
        char* argv[] = {a0, nullptr};
        const auto len = build_expletive(buf, sizeof(buf), argv);
        expect(len == 6 && buf[5] == '\n', "yes expletive");
        expect(optimal_buflen(2) == 8190, "yes optimal buflen");
    }
    {
        using namespace pbsd::userland::usr_bin::basename;
        char path[] = "/usr/bin/ls";
        char* base = path;
        expect(cstrcmp(basename_component(base), "ls") == 0, "basename component");
        char prog[] = "basename";
        char fs[] = "-s.c";
        char arg[] = "foo.c";
        char* argv[] = {prog, fs, arg, nullptr};
        int oi = 0;
        const auto p = parse_args(3, argv, oi);
        expect(p.has_value() && p.value.suffix_len == 2, "basename -s");
    }
    {
        using namespace pbsd::userland::usr_bin::dirname;
        char path[] = "/usr/bin/ls";
        dirname_component(path);
        expect(cstrcmp(path, "/usr/bin") == 0, "dirname component");
    }
    {
        using namespace pbsd::userland::usr_bin::uname;
        char prog[] = "uname";
        char fa[] = "-a";
        char* argv[] = {prog, fa, nullptr};
        int oi = 0;
        const auto f = parse_args(2, argv, oi);
        expect(f.has_value() && (f.value & kAll) == kAll, "uname -a");
    }
    {
        using namespace pbsd::userland::usr_bin::printenv;
        char env0[] = "HOME=/tmp";
        char* environ[] = {env0, nullptr};
        expect(lookup_env(environ, "HOME") != nullptr, "printenv lookup");
        expect(lookup_env(environ, "MISSING") == nullptr, "printenv missing");
    }
    {
        using namespace pbsd::userland::usr_bin::env;
        char prog[] = "env";
        char ci[] = "-i";
        char cmd[] = "sh";
        char* argv[] = {prog, ci, cmd, nullptr};
        int oi = 0;
        const auto p = parse_options(3, argv, oi);
        expect(p.has_value() && p.value.clear_env && oi == 2, "env -i");
    }
    {
        using namespace pbsd::userland::usr_bin::which;
        char path[] = "/bin:/usr/bin";
        char* p = path;
        expect(cstrcmp(next_path_elem(p), "/bin") == 0, "which path elem");
        char candidate[64];
        expect(join_candidate(candidate, sizeof(candidate), ".", "ls"), "which join");
    }
    {
        using namespace pbsd::userland::usr_bin::time_;
        char prog[] = "time";
        char hp[] = "-h";
        char cmd[] = "true";
        char* argv[] = {prog, hp, cmd, nullptr};
        int oi = 0;
        const auto p = parse_args(3, argv, oi);
        expect(p.has_value() && p.value.human, "time -h");
    }
    {
        using namespace pbsd::userland::usr_bin::touch;
        expect(timeoffset("+5").has_value() && timeoffset("+5").value == 5, "touch A offset");
    }
    {
        using namespace pbsd::userland::usr_bin::truncate;
        char prog[] = "truncate";
        char fs[] = "-s";
        char sz[] = "1024";
        char file[] = "x";
        char* argv[] = {prog, fs, sz, file, nullptr};
        int oi = 0;
        const auto p = parse_args(4, argv, oi);
        expect(p.has_value() && p.value.got_size, "truncate -s");
    }
    {
        using namespace pbsd::userland::usr_bin::id;
        char prog[] = "whoami";
        char* argv[] = {prog, nullptr};
        int oi = 0;
        const auto p = parse_args(1, argv, prog, oi);
        expect(p.has_value() && p.value.mode == ProgMode::Whoami, "id whoami mode");
    }
    {
        using namespace pbsd::userland::usr_bin::head;
        char prog[] = "head";
        char fn[] = "-n3";
        char* argv[] = {prog, fn, nullptr};
        int oi = 0;
        const auto p = parse_options(2, argv, oi);
        expect(p.has_value() && p.value.count == 3, "head -n");
    }
    {
        using namespace pbsd::userland::usr_bin::tail;
        char prog[] = "tail";
        char fn[] = "-n10";
        char* argv[] = {prog, fn, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.offset == 10
               && p.value.style == ReadStyle::Backward, "tail -n default back");
    }
    {
        using namespace pbsd::userland::usr_bin::wc;
        Options wopt{};
        apply_defaults(wopt);
        expect(wopt.lines && wopt.words && wopt.chars, "wc defaults");
    }
    {
        using namespace pbsd::userland::usr_bin::tee;
        char prog[] = "tee";
        char fa[] = "-a";
        char* argv[] = {prog, fa, nullptr};
        int oi = 0;
        const auto p = parse_options(2, argv, oi);
        expect(p.has_value() && p.value.append, "tee -a");
    }
    {
        using namespace pbsd::userland::usr_bin::mktemp;
        expect(has_trailing_x("/tmp/fooXXXXXX"), "mktemp X suffix");
    }
    {
        using namespace pbsd::userland::usr_bin::printf;
        std::size_t w = 0, pr = 0;
        bool hw = false, hp = false;
        const auto r = parse_directive("%5d", w, pr, hw, hp);
        expect(r.has_value() && hw && w == 5, "printf directive");
    }
}

void test_usr_bin_wave2_burst6() {
    {
        using namespace pbsd::userland::usr_bin::tr;
        char prog[] = "tr";
        char fd[] = "-ds";
        char* argv[] = {prog, fd, nullptr};
        int oi = 0;
        const auto p = parse_options(2, argv, oi);
        expect(p.has_value() && p.value.delete_set && p.value.squeeze, "tr -ds");
        std::uint8_t map[256];
        build_map(map, "abc", "xyz");
        expect(map[static_cast<unsigned>('a')] == 'x', "tr build_map");
    }
    {
        using namespace pbsd::userland::usr_bin::cut;
        const auto m = parse_list("1,3");
        expect(m.has_value() && selected(m.value, 1) && selected(m.value, 3), "cut list");
    }
    {
        using namespace pbsd::userland::usr_bin::sort;
        char prog[] = "sort";
        char fu[] = "-u";
        char fn[] = "-n";
        char* argv[] = {prog, fu, fn, nullptr};
        int oi = 0;
        const auto p = parse_args(3, argv, oi);
        expect(p.has_value() && p.value.unique && p.value.numeric, "sort -un");
    }
    {
        using namespace pbsd::userland::usr_bin::uniq;
        char prog[] = "uniq";
        char fc[] = "-c";
        char* argv[] = {prog, fc, nullptr};
        int oi = 0;
        const auto p = parse_options(2, argv, oi);
        expect(p.has_value() && p.value.count, "uniq -c");
    }
    {
        using namespace pbsd::userland::usr_bin::comm;
        char prog[] = "comm";
        char f1[] = "-1";
        char fa[] = "a";
        char fb[] = "b";
        char* argv[] = {prog, f1, fa, fb, nullptr};
        int oi = 0;
        const auto p = parse_args(4, argv, oi);
        expect(p.has_value() && !p.value.col1, "comm -1");
    }
    {
        using namespace pbsd::userland::usr_bin::cmp;
        char spec[] = "10:20";
        std::int64_t s1 = 0, s2 = 0;
        expect(parse_iskipspec(spec, s1, s2) && s1 == 10 && s2 == 20, "cmp skip spec");
    }
    {
        using namespace pbsd::userland::usr_bin::diff;
        expect(algorithm_from_name("myers").has_value(), "diff algorithm");
    }
    {
        using namespace pbsd::userland::usr_bin::seq;
        expect(is_numeric_token("1.5"), "seq numeric");
    }
    {
        using namespace pbsd::userland::usr_bin::jot;
        expect(is_default_arg("-") && getprec("3.14") == 2, "jot helpers");
    }
    {
        using namespace pbsd::userland::usr_bin::calendar;
        expect(is_month_token("jan"), "calendar month");
    }
    {
        using namespace pbsd::userland::usr_bin::logger;
        expect(priority_from_name("info") == 6, "logger priority");
    }
    {
        using namespace pbsd::userland::usr_bin::script;
        expect(stamp_is_input('i'), "script stamp dir");
    }
    {
        using namespace pbsd::userland::usr_bin::paste;
        char prog[] = "paste";
        char fs[] = "-s";
        char fa[] = "a";
        char fb[] = "b";
        char* argv[] = {prog, fs, fa, fb, nullptr};
        int oi = 0;
        const auto p = parse_args(4, argv, oi);
        expect(p.has_value() && p.value.sequential, "paste -s");
    }
    {
        using namespace pbsd::userland::usr_bin::fold;
        char prog[] = "fold";
        char fw[] = "-w40";
        char* argv[] = {prog, fw, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.width == 40, "fold -w");
    }
    {
        using namespace pbsd::userland::usr_bin::expand;
        Options eopt{};
        expect(parse_tabstops("4,8,12", eopt).has_value() && eopt.stop_count == 3,
               "expand tabstops");
    }
    {
        using namespace pbsd::userland::usr_bin::rev;
        expect(line_length("abc\n") == 3, "rev line length");
    }
    {
        using namespace pbsd::userland::usr_bin::du;
        char prog[] = "du";
        char fh[] = "-h";
        char* argv[] = {prog, fh, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.human, "du -h");
    }
    {
        using namespace pbsd::userland::usr_bin::find;
        expect(parse_primary("-name") == Primary::Name, "find primary");
    }
    {
        using namespace pbsd::userland::usr_bin::xargs;
        char prog[] = "xargs";
        char f0[] = "-0";
        char* argv[] = {prog, f0, nullptr};
        int oi = 0;
        const auto p = parse_options(2, argv, oi);
        expect(p.has_value() && p.value.null_delim, "xargs -0");
    }
    {
        using namespace pbsd::userland::usr_bin::grep;
        expect(match_fixed("hello world", "world", false), "grep fixed");
    }
    {
        using namespace pbsd::userland::usr_bin::sed;
        expect(classify("s") == CmdKind::Substitute, "sed command");
    }
    {
        using namespace pbsd::userland::usr_bin::awk;
        char line[] = "a:b:c";
        char* fp[4];
        expect(split_fields(line, ':', fp, 4) >= 3, "awk split_fields");
    }
    {
        using namespace pbsd::userland::usr_bin::patch;
        HunkHeader hdr{};
        expect(parse_hunk_header("@@ -1,3 +1,4 @@", hdr) && hdr.old_count == 3, "patch hunk");
    }
    {
        using namespace pbsd::userland::usr_bin::gzip;
        const std::uint8_t magic[] = {0x1f, 0x8b};
        expect(gzip_magic(magic, 2), "gzip magic");
    }
    {
        using namespace pbsd::userland::bin::tar;
        expect(op_from_letter('x') == Operation::Extract, "tar op");
    }
    {
        using namespace pbsd::userland::usr_bin::fetch;
        expect(url_has_scheme("https://example.com"), "fetch scheme");
    }
}

void test_sbin_wave2() {
    {
        using namespace pbsd::userland::sbin::ifconfig;
        char prog[] = "ifconfig";
        char fa[] = "-a";
        char* argv[] = {prog, fa, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.all, "ifconfig -a");
    }
    {
        using namespace pbsd::userland::sbin::mount;
        expect(specified_ro("ro,soft") && !specified_ro("noro"), "mount ro detect");
        char prog[] = "mount";
        char fa[] = "-a";
        char* argv[] = {prog, fa, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.all, "mount -a");
    }
    {
        using namespace pbsd::userland::sbin::route;
        expect(keyword_index("flush") >= 0, "route keyword");
        char prog[] = "route";
        char fn[] = "-n";
        char* argv[] = {prog, fn, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.numeric, "route -n");
    }
    {
        using namespace pbsd::userland::sbin::install;
        char* dest = nullptr;
        char buf[64];
        expect(setpath(&dest, "pkg/kernel", buf, sizeof(buf)).status == pbsd::Status::Ok,
               "install setpath");
        expect(count_csv_paths("a,b,c") == 3, "install csv count");
    }
    {
        using namespace pbsd::userland::sbin::umount;
        char prog[] = "umount";
        char fa[] = "-a";
        char* argv[] = {prog, fa, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.all == 1, "umount -a");
    }
    {
        using namespace pbsd::userland::sbin::ping;
        char prog[] = "ping";
        char fc[] = "-c3";
        char host[] = "127.0.0.1";
        char* argv[] = {prog, fc, host, nullptr};
        int oi = 0;
        const auto p = parse_args(3, argv, oi);
        expect(p.has_value() && p.value.count == 3, "ping -c");
    }
    {
        using namespace pbsd::userland::usr_sbin::pkg;
        const auto cmd = subcommand_from_name("install");
        expect(cmd.has_value() && cmd.value == Subcommand::Install, "pkg subcommand");
    }
    {
        using namespace pbsd::userland::usr_sbin::newsyslog;
        expect(conf_line_is_comment("# log"), "newsyslog comment");
    }
    {
        using namespace pbsd::userland::usr_sbin::cron;
        expect(crontab_line_is_comment("# daily"), "cron comment");
    }
    {
        using namespace pbsd::userland::usr_sbin::pw;
        const auto cmd = subcommand_from_name("useradd");
        expect(cmd.has_value() && cmd.value == Subcommand::Useradd, "pw subcommand");
    }
    {
        using namespace pbsd::userland::usr_sbin::vipw;
        expect(passwd_line_valid("root:*:0:0:Charlie:/root:/bin/sh"), "vipw line");
    }
}

void test_usr_bin_wave2_burst8() {
    {
        using namespace pbsd::userland::usr_bin::find::helpers;
        expect(bool_op_from_token("-a") == BoolOp::And, "find.helpers -a");
        expect(is_primary_token("-name"), "find.helpers primary");
    }
    {
        using namespace pbsd::userland::usr_bin::grep::context;
        const auto ctx = parse_context_flag("-C3");
        expect(ctx.has_value() && ctx.value.after == 3 && ctx.value.before == 3, "grep.context -C");
    }
    {
        using namespace pbsd::userland::usr_bin::sed::addr;
        const auto a = parse_address("10");
        expect(a.has_value() && a.value.kind == AddrKind::Line && a.value.line == 10, "sed.addr line");
    }
    {
        using namespace pbsd::userland::usr_bin::awk::fields;
        expect(count_fields("a:b:c", ':') == 3, "awk.fields split");
    }
    {
        using namespace pbsd::userland::usr_bin::xargs::quote;
        expect(needs_quoting("hello world"), "xargs.quote needs");
    }
    {
        using namespace pbsd::userland::usr_bin::bzip2;
        expect(accepts_flag('z'), "bzip2 flag");
    }
    {
        using namespace pbsd::userland::usr_bin::compress;
        expect(accepts_flag('f'), "compress flag");
    }
    {
        using namespace pbsd::userland::usr_bin::cpio;
        expect(mode_from_letter('o') == Mode::CopyOut, "cpio mode");
    }
    {
        using namespace pbsd::userland::usr_bin::xz;
        expect(accepts_flag('d'), "xz flag");
    }
    {
        using namespace pbsd::userland::usr_bin::curl;
        expect(url_has_scheme("https://example.com"), "curl scheme");
    }
    {
        using namespace pbsd::userland::usr_bin::scp;
        char prog[] = "scp";
        char fv[] = "-v";
        char* argv[] = {prog, fv, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.verbose, "scp -v");
    }
    {
        using namespace pbsd::userland::usr_bin::rsync;
        expect(default_scheme_is_http(), "rsync stub");
    }
    {
        using namespace pbsd::userland::usr_bin::ftp;
        char prog[] = "ftp";
        char fv[] = "-v";
        char* argv[] = {prog, fv, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.verbose, "ftp -v");
    }
    {
        using namespace pbsd::userland::usr_bin::hexdump;
        expect(accepts_flag('C'), "hexdump -C allowed");
    }
    {
        using namespace pbsd::userland::usr_bin::stat;
        char prog[] = "stat";
        char fv[] = "-v";
        char* argv[] = {prog, fv, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.verbose, "stat -v");
    }
    {
        using namespace pbsd::userland::usr_bin::strings;
        expect(printable_run("hello123", 4), "strings printable");
    }
    {
        using namespace pbsd::userland::usr_bin::cksum;
        char prog[] = "cksum";
        char fv[] = "-v";
        char* argv[] = {prog, fv, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.verbose, "cksum -v");
    }
    {
        using namespace pbsd::userland::usr_bin::netstat;
        char prog[] = "netstat";
        char fn[] = "-n";
        char* argv[] = {prog, fn, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.verbose == false, "netstat -n");
    }
    {
        using namespace pbsd::userland::usr_bin::sockstat;
        expect(accepts_flag('l'), "sockstat -l allowed");
    }
    {
        using namespace pbsd::userland::usr_bin::iconv;
        expect(charset_is_utf8("UTF-8"), "iconv utf8");
    }
    {
        using namespace pbsd::userland::usr_sbin::service;
        const auto cmd = subcommand_from_name("restart");
        expect(cmd.has_value() && cmd.value == Subcommand::Restart, "service restart");
    }
    {
        using namespace pbsd::userland::usr_sbin::services;
        char prog[] = "services_mkdb";
        char fv[] = "-q";
        char* argv[] = {prog, fv, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.quiet, "services -q");
    }
    {
        using namespace pbsd::userland::usr_sbin::sysrc;
        const auto cmd = subcommand_from_name("get");
        expect(cmd.has_value(), "sysrc get");
    }
    {
        using namespace pbsd::userland::usr_sbin::periodic;
        const auto mode = mode_from_name("daily");
        expect(mode.has_value() && mode.value == RunMode::Daily, "periodic daily");
    }
    {
        using namespace pbsd::userland::usr_sbin::syslogd;
        expect(priority_from_name("info") == 6, "syslogd info");
    }
    {
        using namespace pbsd::userland::sbin::rcorder;
        expect(keyword_is_provide("PROVIDE:"), "rcorder PROVIDE");
    }
    {
        using namespace pbsd::userland::sbin::bectl;
        const auto cmd = subcommand_from_name("list");
        expect(cmd.has_value() && cmd.value == Subcommand::List, "bectl list");
    }
    {
        using namespace pbsd::userland::sbin::zfs;
        const auto cmd = subcommand_from_name("snapshot");
        expect(cmd.has_value() && cmd.value == Subcommand::Snapshot, "zfs snapshot");
    }
    {
        using namespace pbsd::userland::sbin::zpool;
        const auto cmd = subcommand_from_name("scrub");
        expect(cmd.has_value() && cmd.value == Subcommand::Scrub, "zpool scrub");
    }
    {
        using namespace pbsd::userland::sbin::fsck;
        expect(accepts_flag('p'), "fsck -p allowed");
    }
    {
        using namespace pbsd::userland::sbin::geom;
        const auto cmd = subcommand_from_name("list");
        expect(cmd.has_value() && cmd.value == Subcommand::List, "geom list");
    }
    {
        using namespace pbsd::userland::sbin::newfs;
        char prog[] = "newfs";
        char fv[] = "-v";
        char* argv[] = {prog, fv, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.verbose, "newfs -v");
    }
    {
        using namespace pbsd::userland::sbin::camcontrol;
        const auto cmd = subcommand_from_name("devlist");
        expect(cmd.has_value() && cmd.value == Subcommand::Devlist, "camcontrol devlist");
    }
}

void test_sbin_wave2_burst8() {
    {
        using namespace pbsd::userland::sbin::gpart;
        const auto cmd = subcommand_from_name("create");
        expect(cmd.has_value() && cmd.value == Subcommand::Create, "gpart create");
    }
    {
        using namespace pbsd::userland::sbin::mdconfig;
        expect(accepts_flag('a'), "mdconfig -a");
    }
    {
        using namespace pbsd::userland::sbin::dumpon;
        char prog[] = "dumpon";
        char fv[] = "-v";
        char* argv[] = {prog, fv, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.verbose, "dumpon -v");
    }
    {
        using namespace pbsd::userland::sbin::savecore;
        expect(accepts_flag('f'), "savecore -f");
    }
    {
        using namespace pbsd::userland::sbin::fsck_ffs;
        expect(accepts_flag('p'), "fsck_ffs -p");
    }
    {
        using namespace pbsd::userland::sbin::fsck_msdosfs;
        expect(accepts_flag('n'), "fsck_msdosfs -n");
    }
    {
        using namespace pbsd::userland::sbin::newfs_msdos;
        expect(accepts_flag('F'), "newfs_msdos -F");
    }
    {
        using namespace pbsd::userland::sbin::tunefs;
        expect(accepts_flag('n'), "tunefs -n");
    }
    {
        using namespace pbsd::userland::sbin::growfs;
        expect(accepts_flag('N'), "growfs -N");
    }
    {
        using namespace pbsd::userland::sbin::dmesg;
        expect(accepts_flag('a'), "dmesg -a");
    }
    {
        using namespace pbsd::userland::sbin::dumpfs;
        expect(accepts_flag('i'), "dumpfs -i");
    }
    {
        using namespace pbsd::userland::sbin::fdisk;
        expect(accepts_flag('b'), "fdisk -b");
    }
    {
        using namespace pbsd::userland::sbin::bsdlabel;
        expect(accepts_flag('B'), "bsdlabel -B");
    }
    {
        using namespace pbsd::userland::sbin::swapon;
        expect(accepts_flag('a'), "swapon -a");
    }
    {
        using namespace pbsd::userland::sbin::shutdown;
        expect(accepts_flag('r'), "shutdown -r");
    }
    {
        using namespace pbsd::userland::sbin::reboot;
        expect(accepts_flag('n'), "reboot -n");
    }
    {
        using namespace pbsd::userland::sbin::kldload;
        char prog[] = "kldload";
        char fv[] = "-v";
        char* argv[] = {prog, fv, nullptr};
        int oi = 0;
        const auto p = parse_args(2, argv, oi);
        expect(p.has_value() && p.value.verbose, "kldload -v");
    }
}

} // namespace

int main() {
    test_echo_parse();
    test_hostname_display();
    test_hostname_parse();
    test_domainname_parse();
    test_nproc_logic();
    test_pwd_parse();
    test_capsicum_helpers();
    test_filemode();
    test_cat_parse();
    test_chmod_parse();
    test_mkdir_parse();
    test_rmdir_parse();
    test_ln_helpers();
    test_mv_helpers();
    test_cp_parse();
    test_kill_parse();
    test_sleep_parse();
    test_true_false();
    test_lexer();
    test_date_helpers();
    test_util_fparseln();
    test_util_numbers();
    test_util_secure_path();
    test_util_paths();
    test_casper_services();
    test_jail_param();
    test_libc_memory();
    test_chflags_parse();
    test_dd_parse();
    test_getfacl_parse();
    test_setfacl_helpers();
    test_chown_parse();
    test_df_parse();
    test_sync_run();
    test_uuidgen_parse();
    test_realpath_parse();
    test_kenv_parse();
    test_ls_cmp();
    test_stty_util();
    test_libc_string();
    test_libc_memory_compare();
    test_libc_stdlib();
    test_libc_sort();
    test_libc_gen();
    test_libc_wave2_burst_b6();
    test_libthr_rtld_burst_b6();
    test_libc_wave2_burst_b8();
    test_msun_burst_b8();
    test_libthr_rtld_burst_b8();
    test_libc_msun_libthr_burst_b8();
    test_util_uucplock();
    test_util_login_times();
    test_capsicum_rights();
    test_jail_name_helpers();
    test_util_mntopts();
    test_casper_registry();
    test_usr_bin_wave2();
    test_usr_bin_wave2_burst6();
    test_usr_bin_wave2_burst8();
    test_sbin_wave2();
    test_sbin_wave2_burst8();

    if (failures == 0) {
        std::printf("userland harness: ALL PASS\n");
        return 0;
    }
    std::printf("userland harness: %d failure(s)\n", failures);
    return 1;
}
