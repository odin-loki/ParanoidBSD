# Wave 2 — progressive C++23 userland ports (module-shaped wrappers first).
# Must match nucleus BMI flags (no exceptions/RTTI) while importing pbsd.handles.

set(PBSD_USERLAND_CXX -fno-exceptions -fno-rtti)

function(pbsd_userland_module target)
    target_compile_features(${target} PUBLIC cxx_std_23)
    target_link_libraries(${target} PUBLIC pbsd_handles)
    target_compile_options(${target} PUBLIC ${PBSD_USERLAND_CXX})
endfunction()

# Idempotent module target — parallel burst agents may append duplicate blocks.
function(pbsd_userland_add_library target)
    if(TARGET ${target})
        return()
    endif()
    add_library(${target} ${ARGN})
endfunction()

# Idempotent tool target — parallel burst agents may append duplicate blocks.
function(pbsd_userland_hosted_tool target relpath)
    if(TARGET ${target})
        return()
    endif()
    pbsd_userland_add_library(${target})
    target_sources(${target} PUBLIC FILE_SET CXX_MODULES FILES ${relpath})
    target_link_libraries(${target} PUBLIC pbsd_userland_hosted)
    pbsd_userland_module(${target})
endfunction()

# --- Hosted POSIX syscall surface ---------------------------------------------
if(NOT TARGET pbsd_userland_hosted)
    pbsd_userland_add_library(pbsd_userland_hosted)
    target_sources(pbsd_userland_hosted PUBLIC FILE_SET CXX_MODULES FILES
        hosted/pbsd.userland.hosted.cppm)
    pbsd_userland_module(pbsd_userland_hosted)
endif()

# --- Capsicum (real port from libcapsicum/capsicum_helpers.h) ---------------
pbsd_userland_add_library(pbsd_userland_capsicum_os)
target_sources(pbsd_userland_capsicum_os PUBLIC FILE_SET CXX_MODULES FILES
    capsicum/pbsd.userland.capsicum.os.cppm)
target_link_libraries(pbsd_userland_capsicum_os PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_capsicum_os)

pbsd_userland_add_library(pbsd_userland_capsicum)
target_sources(pbsd_userland_capsicum PUBLIC FILE_SET CXX_MODULES FILES
    capsicum/pbsd.userland.capsicum.rights.cppm
    capsicum/pbsd.userland.capsicum.cppm)
target_link_libraries(pbsd_userland_capsicum PUBLIC pbsd_userland_capsicum_os)
pbsd_userland_module(pbsd_userland_capsicum)

pbsd_userland_add_library(pbsd_userland_capsicum_helpers)
target_sources(pbsd_userland_capsicum_helpers PUBLIC FILE_SET CXX_MODULES FILES
    capsicum/pbsd.userland.capsicum.helpers.cppm)
target_link_libraries(pbsd_userland_capsicum_helpers PUBLIC pbsd_userland_capsicum)
pbsd_userland_module(pbsd_userland_capsicum_helpers)

# --- libcasper + services -----------------------------------------------------
pbsd_userland_add_library(pbsd_userland_casper)
target_sources(pbsd_userland_casper PUBLIC FILE_SET CXX_MODULES FILES
    casper/pbsd.userland.casper.cppm
    casper/pbsd.userland.casper.sysctl.cppm
    casper/pbsd.userland.casper.pwd.cppm
    casper/pbsd.userland.casper.dns.cppm
    casper/pbsd.userland.casper.net.cppm
    casper/pbsd.userland.casper.grp.cppm
    casper/pbsd.userland.casper.syslog.cppm
    casper/pbsd.userland.casper.netdb.cppm
    casper/pbsd.userland.casper.fileargs.cppm
    casper/pbsd.userland.casper.services.cppm)
pbsd_userland_module(pbsd_userland_casper)

# --- freestanding libc helpers (string/memory/stdlib) -------------------------
pbsd_userland_add_library(pbsd_userland_libc)
target_sources(pbsd_userland_libc PUBLIC FILE_SET CXX_MODULES FILES
    libc/pbsd.userland.libc.memory.cppm
    libc/pbsd.userland.libc.string.cppm
    libc/pbsd.userland.libc.string.copy.cppm
    libc/pbsd.userland.libc.string.search.cppm
    libc/pbsd.userland.libc.string.span.cppm
    libc/pbsd.userland.libc.string.find.cppm
    libc/pbsd.userland.libc.string.search.ext.cppm
    libc/pbsd.userland.libc.string.casecmp.cppm
    libc/pbsd.userland.libc.string.concat.cppm
    libc/pbsd.userland.libc.string.stp.cppm
    libc/pbsd.userland.libc.string.bcmp.cppm
    libc/pbsd.userland.libc.string.memext.cppm
    libc/pbsd.userland.libc.string.bit.cppm
    libc/pbsd.userland.libc.string.secure.cppm
    libc/pbsd.userland.libc.string.token.cppm
    libc/pbsd.userland.libc.string.swab.cppm
    libc/pbsd.userland.libc.string.memccpy.cppm
    libc/pbsd.userland.libc.string.strverscmp.cppm
    libc/pbsd.userland.libc.string.memset_explicit.cppm
    libc/pbsd.userland.libc.stdlib.cppm
    libc/pbsd.userland.libc.stdlib.sort.cppm
    libc/pbsd.userland.libc.stdlib.convert.ext.cppm
    libc/pbsd.userland.libc.stdlib.div.cppm
    libc/pbsd.userland.libc.stdlib.rand.cppm
    libc/pbsd.userland.libc.stdlib.strtod.cppm
    libc/pbsd.userland.libc.stdlib.imax.cppm
    libc/pbsd.userland.libc.ctype.cppm
    libc/pbsd.userland.libc.ctype.ext.cppm
    libc/pbsd.userland.libc.locale.cppm
    libc/pbsd.userland.libc.locale.setlocale.cppm
    libc/pbsd.userland.libc.locale.localeconv.cppm
    libc/pbsd.userland.libc.locale.mblen.cppm
    libc/pbsd.userland.libc.locale.mbstowcs.cppm
    libc/pbsd.userland.libc.locale.wcstombs.cppm
    libc/pbsd.userland.libc.locale.nl_langinfo.cppm
    libc/pbsd.userland.libc.locale.btowc.cppm
    libc/pbsd.userland.libc.locale.wctob.cppm
    libc/pbsd.userland.libc.stdio.cppm
    libc/pbsd.userland.libc.stdio.file.cppm
    libc/pbsd.userland.libc.stdio.flags.cppm
    libc/pbsd.userland.libc.stdio.fileno.cppm
    libc/pbsd.userland.libc.stdio.putc.cppm
    libc/pbsd.userland.libc.stdio.getc.cppm
    libc/pbsd.userland.libc.stdio.putchar.cppm
    libc/pbsd.userland.libc.stdio.getchar.cppm
    libc/pbsd.userland.libc.stdio.puts.cppm
    libc/pbsd.userland.libc.stdio.fputs.cppm
    libc/pbsd.userland.libc.stdio.fputc.cppm
    libc/pbsd.userland.libc.stdio.fgetc.cppm
    libc/pbsd.userland.libc.stdio.sprintf.cppm
    libc/pbsd.userland.libc.stdio.vsnprintf.cppm
    libc/pbsd.userland.libc.stdio.perror.cppm
    libc/pbsd.userland.libc.stdio.fflush.cppm
    libc/pbsd.userland.libc.stdio.fopen.cppm
    libc/pbsd.userland.libc.stdio.fread.cppm
    libc/pbsd.userland.libc.stdio.fwrite.cppm
    libc/pbsd.userland.libc.stdlib.atof.cppm
    libc/pbsd.userland.libc.stdlib.strtonum.cppm
    libc/pbsd.userland.libc.stdlib.getopt.cppm
    libc/pbsd.userland.libc.stdlib.getenv.cppm
    libc/pbsd.userland.libc.stdlib.heapsort.cppm
    libc/pbsd.userland.libc.stdlib.mergesort.cppm
    libc/pbsd.userland.libc.stdlib.imaxdiv.cppm
    libc/pbsd.userland.libc.stdlib.llabs.cppm
    libc/pbsd.userland.libc.stdlib.lldiv.cppm
    libc/pbsd.userland.libc.stdlib.exit.cppm
    libc/pbsd.userland.libc.stdlib.system.cppm
    libc/pbsd.userland.libc.gen.cppm
    libc/pbsd.userland.libc.gen.ftok.cppm
    libc/pbsd.userland.libc.stdbit.cppm
    libc/pbsd.userland.libc.string.bit.ext.cppm
    libc/pbsd.userland.libc.string.wchar.copy.cppm
    libc/pbsd.userland.libc.string.wchar.move.cppm
    libc/pbsd.userland.libc.string.wchar.casecmp.cppm
    libc/pbsd.userland.libc.string.wchar.search.cppm
    libc/pbsd.userland.libc.stdlib.insque.cppm
    libc/pbsd.userland.libc.stdlib.tree.cppm
    libc/pbsd.userland.libc.stdlib.a64l.cppm
    libc/pbsd.userland.libc.stdlib.strtoq.cppm
    libc/pbsd.userland.libc.stdlib.reallocf.cppm
    libc/pbsd.userland.libc.gen.assert.cppm
    libc/pbsd.userland.libc.gen.time.cppm
    libc/pbsd.userland.libc.gen.uname.cppm
    libc/pbsd.userland.libc.gen.isatty.cppm
    libc/pbsd.userland.libc.gen.raise.cppm
    libc/pbsd.userland.libc.gen.arc4random.cppm
    libc/pbsd.userland.libc.gen.hostname.cppm
    libc/pbsd.userland.libc.gen.opendir.cppm
    libc/pbsd.userland.libc.gen.dirfd.cppm
    libc/pbsd.userland.libc.gen.getbootfile.cppm
    libc/pbsd.userland.libc.gen.utime.cppm
    libc/pbsd.userland.libc.gen.vis.cppm
    libc/pbsd.userland.libc.uuid.cppm
    libc/pbsd.userland.libc.net.sockatmark.cppm
    libc/pbsd.userland.libc.net.vars.cppm
    libc/pbsd.userland.libc.hash.log2.cppm
    libc/pbsd.userland.libc.gdtoa.ldis.cppm
    libc/pbsd.userland.libc.gen.cap.cppm
    libc/pbsd.userland.libc.gen.getgrouplist.cppm
    libc/pbsd.userland.libc.gen.getprogname.cppm
    libc/pbsd.userland.libc.gen.setprogname.cppm
    libc/pbsd.userland.libc.gen.ldexp.cppm
    libc/pbsd.userland.libc.gen.freadlink.cppm
    libc/pbsd.userland.libc.gen.getpagesize.cppm
    libc/pbsd.userland.libc.gen.getlogin.cppm
    libc/pbsd.userland.libc.gen.getpeereid.cppm
    libc/pbsd.userland.libc.gen.getcwd.cppm
    libc/pbsd.userland.libc.gen.dirname.cppm
    libc/pbsd.userland.libc.gen.basename.cppm
    libc/pbsd.userland.libc.gen.clock.cppm
    libc/pbsd.userland.libc.gen.clock_getcpuclockid.cppm
    libc/pbsd.userland.libc.gen.confstr.cppm
    libc/pbsd.userland.libc.gen.ctermid.cppm
    libc/pbsd.userland.libc.gen.daemon.cppm
    libc/pbsd.userland.libc.gen.devname.cppm
    libc/pbsd.userland.libc.gen.crypt.cppm
    libc/pbsd.userland.libc.gen.dup3.cppm
    libc/pbsd.userland.libc.gen.err.cppm
    libc/pbsd.userland.libc.gen.errlst.cppm
    libc/pbsd.userland.libc.gen.eventfd.cppm
    libc/pbsd.userland.libc.gen.exec.cppm
    libc/pbsd.userland.libc.gen.exect.cppm
    libc/pbsd.userland.libc.gen.fdopendir.cppm
    libc/pbsd.userland.libc.gen.feature_present.cppm
    libc/pbsd.userland.libc.gen.fmtmsg.cppm
    libc/pbsd.userland.libc.gen.fpclassify.cppm
    libc/pbsd.userland.libc.gen.fstab.cppm
    libc/pbsd.userland.libc.gen.ftw.cppm
    libc/pbsd.userland.libc.gen.getdomainname.cppm
    libc/pbsd.userland.libc.gen.getloadavg.cppm
    libc/pbsd.userland.libc.gen.getosreldate.cppm
    libc/pbsd.userland.libc.gen.fdevname.cppm
    libc/pbsd.userland.libc.gen.drand48.cppm
    libc/pbsd.userland.libc.gen.erand48.cppm
    libc/pbsd.userland.libc.gen.cpuset_alloc.cppm
    libc/pbsd.userland.libc.gen.cpuset_free.cppm
    libc/pbsd.userland.libc.gen.memfd_create.cppm
    libc/pbsd.userland.libc.gen.pipe2.cppm
    libc/pbsd.userland.libc.gen.posix_spawn.cppm
    libc/pbsd.userland.libc.gen.pread.cppm
    libc/pbsd.userland.libc.gen.pwrite.cppm
    libc/pbsd.userland.libc.gen.scandir.cppm
    libc/pbsd.userland.libc.gen.setdomainname.cppm
    libc/pbsd.userland.libc.gen.sethostname.cppm
    libc/pbsd.userland.libc.gen.statfs.cppm
    libc/pbsd.userland.libc.gen.sync.cppm
    libc/pbsd.userland.libc.gen.wait3.cppm
    libc/pbsd.userland.libc.gen.wait4.cppm
    libc/pbsd.userland.libc.gen.getusershell.cppm
    libc/pbsd.userland.libc.gen.setusershell.cppm
    libc/pbsd.userland.libc.gen.endusershell.cppm
    libc/pbsd.userland.libc.gen.getttyent.cppm
    libc/pbsd.userland.libc.gen.setttyent.cppm
    libc/pbsd.userland.libc.gen.endttyent.cppm
    libc/pbsd.userland.libc.gen.lockf.cppm
    libc/pbsd.userland.libc.gen.pathconf.cppm
    libc/pbsd.userland.libc.gen.fpathconf.cppm
    libc/pbsd.userland.libc.gen.ttyname.cppm
    libc/pbsd.userland.libc.gen.frexp.cppm
    libc/pbsd.userland.libc.gen.modf.cppm
    libc/pbsd.userland.libc.gen.isinf.cppm
    libc/pbsd.userland.libc.gen.isnan.cppm
    libc/pbsd.userland.libc.gen.fts.cppm
    libc/pbsd.userland.libc.gen.glob.cppm
    libc/pbsd.userland.libc.gen.nftw.cppm
    libc/pbsd.userland.libc.gen.gethostname.cppm
    libc/pbsd.userland.libc.gen.getmntinfo.cppm
    libc/pbsd.userland.libc.gen.getgrent.cppm
    libc/pbsd.userland.libc.gen.getpwent.cppm
    libc/pbsd.userland.libc.gen.getnetgrent.cppm
    libc/pbsd.userland.libc.gen.getutxent.cppm
    libc/pbsd.userland.libc.gen.getvfsbyname.cppm
    libc/pbsd.userland.libc.gen.popen.cppm
    libc/pbsd.userland.libc.gen.sem.cppm
    libc/pbsd.userland.libc.gen.semctl.cppm
    libc/pbsd.userland.libc.gen.sem_new.cppm
    libc/pbsd.userland.libc.gen.sysctl.cppm
    libc/pbsd.userland.libc.gen.sysctlbyname.cppm
    libc/pbsd.userland.libc.gen.sysctlnametomib.cppm
    libc/pbsd.userland.libc.gen.readpassphrase.cppm
    libc/pbsd.userland.libc.gen.setproctitle.cppm
    libc/pbsd.userland.libc.gen.jrand48.cppm
    libc/pbsd.userland.libc.gen.lrand48.cppm
    libc/pbsd.userland.libc.gen.mrand48.cppm
    libc/pbsd.userland.libc.gen.nrand48.cppm
    libc/pbsd.userland.libc.gen.srand48.cppm
    libc/pbsd.userland.libc.gen.seed48.cppm
    libc/pbsd.userland.libc.gen.lcong48.cppm
    libc/pbsd.userland.libc.gen.statvfs.cppm
    libc/pbsd.userland.libc.gen.termios.cppm
    libc/pbsd.userland.libc.gen.timezone.cppm
    libc/pbsd.userland.libc.gen.timespec_get.cppm
    libc/pbsd.userland.libc.gen.timespec_getres.cppm
    libc/pbsd.userland.libc.gen.disklabel.cppm
    libc/pbsd.userland.libc.gen.getcap.cppm
    libc/pbsd.userland.libc.gen.initgroups.cppm
    libc/pbsd.userland.libc.gen.inotify.cppm
    libc/pbsd.userland.libc.gen.kqueue1.cppm
    libc/pbsd.userland.libc.gen.sched_getaffinity.cppm
    libc/pbsd.userland.libc.gen.sched_setaffinity.cppm
    libc/pbsd.userland.libc.gen.siginterrupt.cppm
    libc/pbsd.userland.libc.gen.signal.cppm
    libc/pbsd.userland.libc.gen.sigsetops.cppm
    libc/pbsd.userland.libc.gen.nlist.cppm
    libc/pbsd.userland.libc.gen.pututxline.cppm
    libc/pbsd.userland.libc.gen.pmadvise.cppm
    libc/pbsd.userland.libc.gen.cap_sandboxed.cppm
    libc/pbsd.userland.libc.gen.ulimit.cppm
    libc/pbsd.userland.libc.gen.wordexp.cppm
    libc/pbsd.userland.libc.gen.valloc.cppm
    libc/pbsd.userland.libc.gen.memalign.cppm
    libc/pbsd.userland.libc.gen.ttyslot.cppm
    libc/pbsd.userland.libc.gen.dlfcn.cppm
    libc/pbsd.userland.libc.gen.scandir_b.cppm
    libc/pbsd.userland.libc.gen.opendir2.cppm
    libc/pbsd.userland.libc.gen.setjmperr.cppm
    libc/pbsd.userland.libc.gen.sysconf.cppm
    libc/pbsd.userland.libc.stdlib.strtol.cppm
    libc/pbsd.userland.libc.stdlib.strtoul.cppm
    libc/pbsd.userland.libc.string.strlcpy.cppm
    libc/pbsd.userland.libc.string.strlcat.cppm
    libc/pbsd.userland.libc.net.inet_ntop.cppm
    libc/pbsd.userland.libc.net.inet_pton.cppm
    libc/pbsd.userland.libc.sys.pipe.cppm
    libc/pbsd.userland.libc.sys.wait.cppm
    libc/pbsd.userland.libc.sys.stat.cppm
    libc/pbsd.userland.libc.sys.lstat.cppm
    libc/pbsd.userland.libc.stdlib.qsort_r.cppm
    libc/pbsd.userland.libc.string.wchar.cmp.cppm
    libc/pbsd.userland.libc.string.wchar.set.cppm
    libc/pbsd.userland.libc.string.wchar.span.cppm
    libc/pbsd.userland.libc.locale.wcwidth.cppm
    libc/pbsd.userland.libc.regex.regerror.cppm
    libc/pbsd.userland.libc.regex.cppm
    libc/pbsd.userland.libc.net.hton.cppm
    libc/pbsd.userland.libc.stdio.fseek.cppm
    libc/pbsd.userland.libc.string.strerror.cppm
    libc/pbsd.userland.libc.string.strsignal.cppm
    libc/pbsd.userland.libc.stdlib.abs.cppm
    libc/pbsd.userland.libc.hash.prime.cppm
    libc/pbsd.userland.libc.gdtoa.ldisf.cppm
    libc/pbsd.userland.libc.net.inet_addr.cppm
    libc/pbsd.userland.libc.string.strtok_r.cppm
    libc/pbsd.userland.libc.stdlib.setenv.cppm
    libc/pbsd.userland.libc.stdlib.bsearch.cppm
    libc/pbsd.userland.libc.stdio.fclose.cppm
    libc/pbsd.userland.libc.stdio.ungetc.cppm
    libc/pbsd.userland.libc.stdio.setbuf.cppm
    libc/pbsd.userland.libc.stdio.setvbuf.cppm
    libc/pbsd.userland.libc.stdio.tmpfile.cppm
    libc/pbsd.userland.libc.gen.errno.cppm
    libc/pbsd.userland.libc.hash.crc32.cppm
    libc/pbsd.userland.libc.string.strcoll.cppm
    libc/pbsd.userland.libc.gen.unvis.cppm
    libc/pbsd.userland.libc.gen.strmode.cppm
    libc/pbsd.userland.libc.gen.humanize.cppm
    libc/pbsd.userland.libc.gen.fmtcheck.cppm
    libc/pbsd.userland.libc.gen.parsetime.cppm
    libc/pbsd.userland.libc.gen.fnmatch.cppm
    libc/pbsd.userland.libc.gen.psignal.cppm
    libc/pbsd.userland.libc.gen.sig2str.cppm
    libc/pbsd.userland.libc.gen.getbsize.cppm
    libc/pbsd.userland.libc.gen.siglist.cppm
    libc/pbsd.userland.libc.gen.strtofflags.cppm
    libc/pbsd.userland.libc.gen.stringlist.cppm
    libc/pbsd.userland.libc.gen.nice.cppm
    libc/pbsd.userland.libc.gen.pause.cppm
    libc/pbsd.userland.libc.gen.alarm.cppm
    libc/pbsd.userland.libc.gen.ualarm.cppm
    libc/pbsd.userland.libc.gen.sleep.cppm
    libc/pbsd.userland.libc.gen.usleep.cppm
    libc/pbsd.userland.libc.gen.times.cppm
    libc/pbsd.userland.libc.gen.closedir.cppm
    libc/pbsd.userland.libc.gen.readdir.cppm
    libc/pbsd.userland.libc.gen.rewinddir.cppm
    libc/pbsd.userland.libc.gen.telldir.cppm
    libc/pbsd.userland.libc.gen.seekdir.cppm
    libc/pbsd.userland.libc.gen.getentropy.cppm
    libc/pbsd.userland.libc.gen.arc4random_uniform.cppm
    libc/pbsd.userland.libc.gen.setmode.cppm
    libc/pbsd.userland.libc.gen.syslog.cppm
    libc/pbsd.userland.libc.stdlib.atoi.cppm
    libc/pbsd.userland.libc.stdlib.atol.cppm
    libc/pbsd.userland.libc.stdlib.atoll.cppm
    libc/pbsd.userland.libc.stdlib.qsort.cppm
    libc/pbsd.userland.libc.stdlib.realpath.cppm
    libc/pbsd.userland.libc.stdlib.reallocarray.cppm
    libc/pbsd.userland.libc.stdlib.labs.cppm
    libc/pbsd.userland.libc.stdlib.ldiv.cppm
    libc/pbsd.userland.libc.stdlib.getopt_long.cppm
    libc/pbsd.userland.libc.stdlib.getsubopt.cppm
    libc/pbsd.userland.libc.stdlib.imaxabs.cppm
    libc/pbsd.userland.libc.stdlib.l64a.cppm
    libc/pbsd.userland.libc.stdlib.bsearch_b.cppm
    libc/pbsd.userland.libc.stdlib.abort.cppm
    libc/pbsd.userland.libc.stdio.feof.cppm
    libc/pbsd.userland.libc.stdio.ferror.cppm
    libc/pbsd.userland.libc.stdio.ftell.cppm
    libc/pbsd.userland.libc.stdio.rewind.cppm
    libc/pbsd.userland.libc.stdio.getline.cppm
    libc/pbsd.userland.libc.stdio.clrerr.cppm
    libc/pbsd.userland.libc.net.base64.cppm
    libc/pbsd.userland.libc.net.gai_strerror.cppm
    libc/pbsd.userland.libc.net.if_nametoindex.cppm
    libc/pbsd.userland.libc.sys.open.cppm
    libc/pbsd.userland.libc.sys.read.cppm
    libc/pbsd.userland.libc.sys.write.cppm
    libc/pbsd.userland.libc.sys.close.cppm
    libc/pbsd.userland.libc.sys.accept.cppm
    libc/pbsd.userland.libc.sys.connect.cppm
    libc/pbsd.userland.libc.sys.fcntl.cppm
    libc/pbsd.userland.libc.sys.fsync.cppm
    libc/pbsd.userland.libc.sys.kevent.cppm
    libc/pbsd.userland.libc.sys.mknod.cppm
    libc/pbsd.userland.libc.sys.nanosleep.cppm
    libc/pbsd.userland.libc.sys.openat.cppm
    libc/pbsd.userland.libc.sys.poll.cppm
    libc/pbsd.userland.libc.sys.select.cppm
    libc/pbsd.userland.libc.sys.sendfile.cppm
    libc/pbsd.userland.libc.sys.sigaction.cppm
    libc/pbsd.userland.libc.net.ether_addr.cppm
    libc/pbsd.userland.libc.net.eui64.cppm
    libc/pbsd.userland.libc.net.getnameinfo.cppm
    libc/pbsd.userland.libc.net.getifaddrs.cppm
    libc/pbsd.userland.libc.net.if_nameindex.cppm
    libc/pbsd.userland.libc.net.getifmaddrs.cppm
    libc/pbsd.userland.libc.net.getproto.cppm
    libc/pbsd.userland.libc.net.getservent.cppm
    libc/pbsd.userland.libc.net.getprotoent.cppm
    libc/pbsd.userland.libc.sys.accept4.cppm
    libc/pbsd.userland.libc.sys.clock_nanosleep.cppm
    libc/pbsd.userland.libc.sys.fdatasync.cppm
    libc/pbsd.userland.libc.sys.msync.cppm
    libc/pbsd.userland.libc.sys.ppoll.cppm
    libc/pbsd.userland.libc.sys.lockf.cppm
    libc/pbsd.userland.libc.sys.creat.cppm
    libc/pbsd.userland.libc.sys.closefrom.cppm
    libc/pbsd.userland.libc.sys.brk.cppm
    libc/pbsd.userland.libc.sys.getdents.cppm
    libc/pbsd.userland.libc.sys.setuid.cppm
    libc/pbsd.userland.libc.sys.getuid.cppm
    libc/pbsd.userland.libc.sys.setgid.cppm
    libc/pbsd.userland.libc.sys.getgid.cppm
    libc/pbsd.userland.libc.sys.geteuid.cppm
    libc/pbsd.userland.libc.sys.getegid.cppm
    libc/pbsd.userland.libc.sys.getgroups.cppm
    libc/pbsd.userland.libc.sys.setgroups.cppm
    libc/pbsd.userland.libc.sys.lseek.cppm
    libc/pbsd.userland.libc.sys.ftruncate.cppm
    libc/pbsd.userland.libc.sys.fork.cppm
    libc/pbsd.userland.libc.locale.rpmatch.cppm
    libc/pbsd.userland.libc.locale.tolower.cppm
    libc/pbsd.userland.libc.locale.toupper.cppm
    libc/pbsd.userland.libc.gen.aio_read2.cppm
    libc/pbsd.userland.libc.gen.aio_write2.cppm
    libc/pbsd.userland.libc.gen.arc4random_compat.cppm
    libc/pbsd.userland.libc.gen.basename_compat.cppm
    libc/pbsd.userland.libc.gen.check_utility_compat.cppm
    libc/pbsd.userland.libc.gen.devname_compat11.cppm
    libc/pbsd.userland.libc.gen.dirname_compat.cppm
    libc/pbsd.userland.libc.gen.elf_utils.cppm
    libc/pbsd.userland.libc.gen.fts_compat.cppm
    libc/pbsd.userland.libc.gen.fts_compat11.cppm
    libc/pbsd.userland.libc.gen.ftw_compat11.cppm
    libc/pbsd.userland.libc.gen.getmntinfo_compat11.cppm
    libc/pbsd.userland.libc.gen.glob_compat11.cppm
    libc/pbsd.userland.libc.gen.libc_dlopen.cppm
    libc/pbsd.userland.libc.gen.libc_interposing_table.cppm
    libc/pbsd.userland.libc.gen.nftw_compat11.cppm
    libc/pbsd.userland.libc.gen.pw_scan.cppm
    libc/pbsd.userland.libc.gen.readdir_compat11.cppm
    libc/pbsd.userland.libc.gen.scandir_compat11.cppm
    libc/pbsd.userland.libc.gen.tls.cppm
    libc/pbsd.userland.libc.gen.trivial_getcontextx.cppm
    libc/pbsd.userland.libc.gen.uexterr_format.cppm
    libc/pbsd.userland.libc.gen.uexterr_gettext.cppm
    libc/pbsd.userland.libc.gen.unvis_compat.cppm
    libc/pbsd.userland.libc.gen.utxdb.cppm
    libc/pbsd.userland.libc.net.getaddrinfo.cppm
    libc/pbsd.userland.libc.net.gethostbydns.cppm
    libc/pbsd.userland.libc.net.gethostbyht.cppm
    libc/pbsd.userland.libc.net.gethostbynis.cppm
    libc/pbsd.userland.libc.net.gethostnamadr.cppm
    libc/pbsd.userland.libc.net.getnetbydns.cppm
    libc/pbsd.userland.libc.net.getnetbyht.cppm
    libc/pbsd.userland.libc.net.getnetbynis.cppm
    libc/pbsd.userland.libc.net.getnetnamadr.cppm
    libc/pbsd.userland.libc.net.getprotoname.cppm
    libc/pbsd.userland.libc.net.hesiod.cppm
    libc/pbsd.userland.libc.net.if_indextoname.cppm
    libc/pbsd.userland.libc.net.ip6opt.cppm
    libc/pbsd.userland.libc.net.linkaddr.cppm
    libc/pbsd.userland.libc.net.map_v4v6.cppm
    libc/pbsd.userland.libc.net.name6.cppm
    libc/pbsd.userland.libc.net.nscache.cppm
    libc/pbsd.userland.libc.net.nscachedcli.cppm
    libc/pbsd.userland.libc.net.nsdispatch.cppm
    libc/pbsd.userland.libc.net.nss_compat.cppm
    libc/pbsd.userland.libc.net.rcmd.cppm
    libc/pbsd.userland.libc.net.rcmdsh.cppm
    libc/pbsd.userland.libc.net.rthdr.cppm
    libc/pbsd.userland.libc.net.sctp_sys_calls.cppm
    libc/pbsd.userland.libc.net.sourcefilter.cppm
    libc/pbsd.userland.libc.stdio.asprintf.cppm
    libc/pbsd.userland.libc.stdio.dprintf.cppm
    libc/pbsd.userland.libc.stdio.fcloseall.cppm
    libc/pbsd.userland.libc.stdio.fdopen.cppm
    libc/pbsd.userland.libc.stdio.fgetln.cppm
    libc/pbsd.userland.libc.stdio.fgetpos.cppm
    libc/pbsd.userland.libc.stdio.fgets.cppm
    libc/pbsd.userland.libc.stdio.fgetwc.cppm
    libc/pbsd.userland.libc.stdio.fgetwln.cppm
    libc/pbsd.userland.libc.stdio.fgetws.cppm
    libc/pbsd.userland.libc.stdio.findfp.cppm
    libc/pbsd.userland.libc.stdio.fmemopen.cppm
    libc/pbsd.userland.libc.stdio.fopencookie.cppm
    libc/pbsd.userland.libc.stdio.fprintf.cppm
    libc/pbsd.userland.libc.stdio.fpurge.cppm
    libc/pbsd.userland.libc.stdio.fputwc.cppm
    libc/pbsd.userland.libc.stdio.fputws.cppm
    libc/pbsd.userland.libc.stdio.freopen.cppm
    libc/pbsd.userland.libc.stdio.fscanf.cppm
    libc/pbsd.userland.libc.stdio.fsetpos.cppm
    libc/pbsd.userland.libc.stdio.funopen.cppm
    libc/pbsd.userland.libc.stdio.fvwrite.cppm
    libc/pbsd.userland.libc.stdio.fwalk.cppm
    libc/pbsd.userland.libc.stdio.fwide.cppm
    libc/pbsd.userland.libc.stdio.fwprintf.cppm
    libc/pbsd.userland.libc.stdio.fwscanf.cppm
    libc/pbsd.userland.libc.stdio.getdelim.cppm
    libc/pbsd.userland.libc.stdio.gets.cppm
    libc/pbsd.userland.libc.stdio.gets_s.cppm
    libc/pbsd.userland.libc.stdio.getw.cppm
    libc/pbsd.userland.libc.stdio.getwc.cppm
    libc/pbsd.userland.libc.stdio.getws.cppm
    libc/pbsd.userland.libc.stdio.open_memstream.cppm
    libc/pbsd.userland.libc.stdio.open_wmemstream.cppm
    libc/pbsd.userland.libc.stdio.printf.cppm
    libc/pbsd.userland.libc.stdio.putw.cppm
    libc/pbsd.userland.libc.stdio.putwc.cppm
    libc/pbsd.userland.libc.stdio.putws.cppm
    libc/pbsd.userland.libc.stdio.scanf.cppm
    libc/pbsd.userland.libc.stdio.snprintf.cppm
    libc/pbsd.userland.libc.stdio.sscanf.cppm
    libc/pbsd.userland.libc.stdio.ungetwc.cppm
    libc/pbsd.userland.libc.stdio.vdprintf.cppm
    libc/pbsd.userland.libc.stdio.vfprintf.cppm
    libc/pbsd.userland.libc.stdio.vfscanf.cppm
    libc/pbsd.userland.libc.stdio.vfwprintf.cppm
    libc/pbsd.userland.libc.stdio.vfwscanf.cppm
    libc/pbsd.userland.libc.stdio.vprintf.cppm
    libc/pbsd.userland.libc.stdio.vscanf.cppm
    libc/pbsd.userland.libc.stdio.vsprintf.cppm
    libc/pbsd.userland.libc.stdio.vsscanf.cppm
    libc/pbsd.userland.libc.stdio.vwprintf.cppm
    libc/pbsd.userland.libc.stdio.vwscanf.cppm
    libc/pbsd.userland.libc.stdio.wprintf.cppm
    libc/pbsd.userland.libc.stdio.wscanf.cppm
    libc/pbsd.userland.libc.stdlib.atexit.cppm
    libc/pbsd.userland.libc.stdlib.cxa_thread_atexit.cppm
    libc/pbsd.userland.libc.stdlib.cxa_thread_atexit_impl.cppm
    libc/pbsd.userland.libc.stdlib.hcreate.cppm
    libc/pbsd.userland.libc.stdlib.hcreate_r.cppm
    libc/pbsd.userland.libc.stdlib.hdestroy_r.cppm
    libc/pbsd.userland.libc.stdlib.heapsort_b.cppm
    libc/pbsd.userland.libc.stdlib.hsearch_r.cppm
    libc/pbsd.userland.libc.stdlib.lsearch.cppm
    libc/pbsd.userland.libc.stdlib.memalignment.cppm
    libc/pbsd.userland.libc.stdlib.merge.cppm
    libc/pbsd.userland.libc.stdlib.mergesort_b.cppm
    libc/pbsd.userland.libc.stdlib.ptsname.cppm
    libc/pbsd.userland.libc.stdlib.qsort_r_compat.cppm
    libc/pbsd.userland.libc.stdlib.qsort_s.cppm
    libc/pbsd.userland.libc.stdlib.quick_exit.cppm
    libc/pbsd.userland.libc.stdlib.radixsort.cppm
    libc/pbsd.userland.libc.stdlib.random.cppm
    libc/pbsd.userland.libc.stdlib.recallocarray.cppm
    libc/pbsd.userland.libc.stdlib.remque.cppm
    libc/pbsd.userland.libc.stdlib.set_constraint_handler_s.cppm
    libc/pbsd.userland.libc.stdlib.strfmon.cppm
    libc/pbsd.userland.libc.stdlib.strtoimax.cppm
    libc/pbsd.userland.libc.stdlib.strtold.cppm
    libc/pbsd.userland.libc.stdlib.strtoll.cppm
    libc/pbsd.userland.libc.stdlib.strtoull.cppm
    libc/pbsd.userland.libc.stdlib.strtoumax.cppm
    libc/pbsd.userland.libc.stdlib.strtouq.cppm
    libc/pbsd.userland.libc.stdlib.tdelete.cppm
    libc/pbsd.userland.libc.stdlib.tdestroy.cppm
    libc/pbsd.userland.libc.stdlib.tfind.cppm
    libc/pbsd.userland.libc.stdlib.tsearch.cppm
    libc/pbsd.userland.libc.stdlib.twalk.cppm
    libc/pbsd.userland.libc.string.bcopy.cppm
    libc/pbsd.userland.libc.string.bzero.cppm
    libc/pbsd.userland.libc.string.ffs.cppm
    libc/pbsd.userland.libc.string.ffsl.cppm
    libc/pbsd.userland.libc.string.ffsll.cppm
    libc/pbsd.userland.libc.string.fls.cppm
    libc/pbsd.userland.libc.string.flsl.cppm
    libc/pbsd.userland.libc.string.flsll.cppm
    libc/pbsd.userland.libc.string.memchr.cppm
    libc/pbsd.userland.libc.string.memcmp.cppm
    libc/pbsd.userland.libc.string.memcpy.cppm
    libc/pbsd.userland.libc.string.memmem.cppm
    libc/pbsd.userland.libc.string.memmove.cppm
    libc/pbsd.userland.libc.string.mempcpy.cppm
    libc/pbsd.userland.libc.string.memrchr.cppm
    libc/pbsd.userland.libc.string.memset.cppm
    libc/pbsd.userland.libc.string.memset_s.cppm
    libc/pbsd.userland.libc.string.stpcpy.cppm
    libc/pbsd.userland.libc.string.stpncpy.cppm
    libc/pbsd.userland.libc.string.strcasecmp.cppm
    libc/pbsd.userland.libc.string.strcasestr.cppm
    libc/pbsd.userland.libc.string.strcat.cppm
    libc/pbsd.userland.libc.string.strchr.cppm
    libc/pbsd.userland.libc.string.strchrnul.cppm
    libc/pbsd.userland.libc.string.strcmp.cppm
    libc/pbsd.userland.libc.string.strcpy.cppm
    libc/pbsd.userland.libc.string.strcspn.cppm
    libc/pbsd.userland.libc.string.strdup.cppm
    libc/pbsd.userland.libc.string.strlen.cppm
    libc/pbsd.userland.libc.string.strmode.cppm
    libc/pbsd.userland.libc.string.strncat.cppm
    libc/pbsd.userland.libc.string.strncmp.cppm
    libc/pbsd.userland.libc.string.strncpy.cppm
    libc/pbsd.userland.libc.string.strndup.cppm
    libc/pbsd.userland.libc.string.strpbrk.cppm
    libc/pbsd.userland.libc.string.strrchr.cppm
    libc/pbsd.userland.libc.string.strsep.cppm
    libc/pbsd.userland.libc.string.strspn.cppm
    libc/pbsd.userland.libc.string.strtok.cppm
    libc/pbsd.userland.libc.sys.aio_suspend.cppm
    libc/pbsd.userland.libc.sys.pdfork.cppm
    libc/pbsd.userland.libc.sys.pdwait.cppm
    libc/pbsd.userland.libc.sys.pselect.cppm
    libc/pbsd.userland.libc.sys.readv.cppm
    libc/pbsd.userland.libc.sys.recv.cppm
    libc/pbsd.userland.libc.sys.recvfrom.cppm
    libc/pbsd.userland.libc.sys.recvmsg.cppm
    libc/pbsd.userland.libc.sys.send.cppm
    libc/pbsd.userland.libc.sys.sendmsg.cppm
    libc/pbsd.userland.libc.sys.sendto.cppm
    libc/pbsd.userland.libc.sys.setcontext.cppm
    libc/pbsd.userland.libc.sys.shm_open.cppm
    libc/pbsd.userland.libc.sys.sigprocmask.cppm
    libc/pbsd.userland.libc.sys.sigsuspend.cppm
    libc/pbsd.userland.libc.sys.sigtimedwait.cppm
    libc/pbsd.userland.libc.sys.sigwait.cppm
    libc/pbsd.userland.libc.sys.sigwaitinfo.cppm
    libc/pbsd.userland.libc.sys.swapcontext.cppm
    libc/pbsd.userland.libc.sys.vadvise.cppm
    libc/pbsd.userland.libc.sys.wait3.cppm
    libc/pbsd.userland.libc.sys.wait4.cppm
    libc/pbsd.userland.libc.sys.wait6.cppm
    libc/pbsd.userland.libc.sys.waitid.cppm
    libc/pbsd.userland.libc.sys.waitpid.cppm
    libc/pbsd.userland.libc.sys.writev.cppm
    libc/pbsd.userland.libc.uuid.uuid_compare.cppm
    libc/pbsd.userland.libc.uuid.uuid_create.cppm
    libc/pbsd.userland.libc.uuid.uuid_create_nil.cppm
    libc/pbsd.userland.libc.uuid.uuid_equal.cppm
    libc/pbsd.userland.libc.uuid.uuid_from_string.cppm
    libc/pbsd.userland.libc.uuid.uuid_hash.cppm
    libc/pbsd.userland.libc.uuid.uuid_is_nil.cppm
    libc/pbsd.userland.libc.uuid.uuid_stream.cppm
    libc/pbsd.userland.libc.uuid.uuid_to_string.cppm
    libc/pbsd.userland.libc.string.strnlen.cppm
    libc/pbsd.userland.libc.string.strnstr.cppm
    libc/pbsd.userland.libc.string.strstr.cppm
    libc/pbsd.userland.libc.string.strxfrm.cppm
    libc/pbsd.userland.libc.string.timingsafe_bcmp.cppm
    libc/pbsd.userland.libc.string.timingsafe_memcmp.cppm
    libc/pbsd.userland.libc.string.wcslen.cppm
    libc/pbsd.userland.libc.string.wcscmp.cppm
    libc/pbsd.userland.libc.string.wcscpy.cppm
    libc/pbsd.userland.libc.string.wcsdup.cppm
    libc/pbsd.userland.libc.string.wmemcpy.cppm
    libc/pbsd.userland.libc.string.wmemset.cppm
    libc/pbsd.userland.libc.stdio.getwchar.cppm
    libc/pbsd.userland.libc.stdio.putwchar.cppm
    libc/pbsd.userland.libc.stdio.remove.cppm
    libc/pbsd.userland.libc.stdio.swprintf.cppm
    libc/pbsd.userland.libc.stdio.tmpnam.cppm
    libc/pbsd.userland.libc.stdio.vasprintf.cppm
    libc/pbsd.userland.libc.stdio.makebuf.cppm
    libc/pbsd.userland.libc.stdio.setbuffer.cppm
    libc/pbsd.userland.libc.stdlib._Exit.cppm
    libc/pbsd.userland.libc.sys.compat_stub.cppm
    libc/pbsd.userland.libc.sys.posix2x_fork.cppm
    libc/pbsd.userland.libc.gen.rand48.cppm
    libc/pbsd.userland.libc.gen.once_stub.cppm
    libc/pbsd.userland.libc.gen.xuname.cppm
    libc/pbsd.userland.libc.compat_43.gethostid.cppm
    libc/pbsd.userland.libc.compat_43.getwd.cppm
    libc/pbsd.userland.libc.compat_43.killpg.cppm
    libc/pbsd.userland.libc.compat_43.sethostid.cppm
    libc/pbsd.userland.libc.compat_43.setpgrp.cppm
    libc/pbsd.userland.libc.compat_43.setrgid.cppm
    libc/pbsd.userland.libc.compat_43.setruid.cppm
    libc/pbsd.userland.libc.compat_43.sigcompat.cppm
    libc/pbsd.userland.libc.csu.libc_start1.cppm
    libc/pbsd.userland.libc.gdtoa.hdtoa_impl.cppm
    libc/pbsd.userland.libc.gdtoa.hldtoa_impl.cppm
    libc/pbsd.userland.libc.gdtoa.ldtoa_impl.cppm
    libc/pbsd.userland.libc.gdtoa.glue.cppm
    libc/pbsd.userland.libc.gdtoa.machdep_ldisQ.cppm
    libc/pbsd.userland.libc.gdtoa.machdep_ldisx.cppm
    libc/pbsd.userland.libc.gen.pthread_mutex_init_calloc_cb_stub_stub.cppm
    libc/pbsd.userland.libc.gen.pthread_stubs_impl.cppm
    libc/pbsd.userland.libc.gen.spinlock_stub_impl.cppm
    libc/pbsd.userland.libc.gen.thread_init_impl.cppm
    libc/pbsd.userland.libc.gen.pthread_stubs.cppm
    libc/pbsd.userland.libc.gen.spinlock_stub.cppm
    libc/pbsd.userland.libc.gen.thread_init.cppm
    libc/pbsd.userland.libc.gen.pthread_mutex_init_calloc_cb_stub.cppm
    libc/pbsd.userland.libc.gmon.gmon.cppm
    libc/pbsd.userland.libc.gmon.mcount.cppm
    libc/pbsd.userland.libc.net.inet_cidr_ntop.cppm
    libc/pbsd.userland.libc.net.inet_cidr_pton.cppm
    libc/pbsd.userland.libc.net.inet_lnaof.cppm
    libc/pbsd.userland.libc.net.inet_makeaddr.cppm
    libc/pbsd.userland.libc.net.inet_net_ntop.cppm
    libc/pbsd.userland.libc.net.inet_net_pton.cppm
    libc/pbsd.userland.libc.net.inet_neta.cppm
    libc/pbsd.userland.libc.net.inet_netof.cppm
    libc/pbsd.userland.libc.net.inet_network.cppm
    libc/pbsd.userland.libc.net.inet_ntoa.cppm
    libc/pbsd.userland.libc.net.nsap_addr.cppm
    libc/pbsd.userland.libc.isc.ev_streams.cppm
    libc/pbsd.userland.libc.isc.ev_timers.cppm
    libc/pbsd.userland.libc.nameser.ns_name.cppm
    libc/pbsd.userland.libc.nameser.ns_netint.cppm
    libc/pbsd.userland.libc.nameser.ns_parse.cppm
    libc/pbsd.userland.libc.nameser.ns_print.cppm
    libc/pbsd.userland.libc.nameser.ns_samedomain.cppm
    libc/pbsd.userland.libc.nameser.ns_ttl.cppm
    libc/pbsd.userland.libc.net.ntoh.cppm
    libc/pbsd.userland.libc.nls.msgcat.cppm
    libc/pbsd.userland.libc.regex.engine.cppm
    libc/pbsd.userland.libc.regex.regcomp.cppm
    libc/pbsd.userland.libc.regex.regexec.cppm
    libc/pbsd.userland.libc.regex.regfree.cppm
    libc/pbsd.userland.libc.resolv.h_errno.cppm
    libc/pbsd.userland.libc.resolv.herror.cppm
    libc/pbsd.userland.libc.resolv.mtctxres.cppm
    libc/pbsd.userland.libc.resolv.res_comp.cppm
    libc/pbsd.userland.libc.resolv.res_data.cppm
    libc/pbsd.userland.libc.resolv.res_debug.cppm
    libc/pbsd.userland.libc.resolv.res_findzonecut.cppm
    libc/pbsd.userland.libc.resolv.res_init.cppm
    libc/pbsd.userland.libc.resolv.res_mkquery.cppm
    libc/pbsd.userland.libc.resolv.res_mkupdate.cppm
    libc/pbsd.userland.libc.resolv.res_query.cppm
    libc/pbsd.userland.libc.resolv.res_send.cppm
    libc/pbsd.userland.libc.resolv.res_state.cppm
    libc/pbsd.userland.libc.resolv.res_update.cppm
    libc/pbsd.userland.libc.secure.fgets_chk.cppm
    libc/pbsd.userland.libc.secure.libc_stack_protector.cppm
    libc/pbsd.userland.libc.secure.memcpy_chk.cppm
    libc/pbsd.userland.libc.secure.memmove_chk.cppm
    libc/pbsd.userland.libc.secure.mempcpy_chk.cppm
    libc/pbsd.userland.libc.secure.memset_chk.cppm
    libc/pbsd.userland.libc.secure.snprintf_chk.cppm
    libc/pbsd.userland.libc.secure.sprintf_chk.cppm
    libc/pbsd.userland.libc.secure.stack_protector_compat.cppm
    libc/pbsd.userland.libc.secure.stpcpy_chk.cppm
    libc/pbsd.userland.libc.secure.stpncpy_chk.cppm
    libc/pbsd.userland.libc.secure.strcat_chk.cppm
    libc/pbsd.userland.libc.secure.strcpy_chk.cppm
    libc/pbsd.userland.libc.secure.strlcat_chk.cppm
    libc/pbsd.userland.libc.secure.strlcpy_chk.cppm
    libc/pbsd.userland.libc.secure.strncat_chk.cppm
    libc/pbsd.userland.libc.secure.strncpy_chk.cppm
    libc/pbsd.userland.libc.secure.vsnprintf_chk.cppm
    libc/pbsd.userland.libc.secure.vsprintf_chk.cppm
    libc/pbsd.userland.libc.stdbit.stdc_bit_ceil.cppm
    libc/pbsd.userland.libc.stdbit.stdc_bit_floor.cppm
    libc/pbsd.userland.libc.stdbit.stdc_bit_width.cppm
    libc/pbsd.userland.libc.stdbit.stdc_count_ones.cppm
    libc/pbsd.userland.libc.stdbit.stdc_count_zeros.cppm
    libc/pbsd.userland.libc.stdbit.stdc_first_leading_one.cppm
    libc/pbsd.userland.libc.stdbit.stdc_first_leading_zero.cppm
    libc/pbsd.userland.libc.stdbit.stdc_first_trailing_one.cppm
    libc/pbsd.userland.libc.stdbit.stdc_first_trailing_zero.cppm
    libc/pbsd.userland.libc.stdbit.stdc_has_single_bit.cppm
    libc/pbsd.userland.libc.stdbit.stdc_leading_ones.cppm
    libc/pbsd.userland.libc.stdbit.stdc_leading_zeros.cppm
    libc/pbsd.userland.libc.stdbit.stdc_trailing_ones.cppm
    libc/pbsd.userland.libc.stdbit.stdc_trailing_zeros.cppm
    libc/pbsd.userland.libc.stdio.flock_stub_impl.cppm
    libc/pbsd.userland.libc.stdio.mktemp.cppm
    libc/pbsd.userland.libc.stdio.printf_pos.cppm
    libc/pbsd.userland.libc.stdio.refill.cppm
    libc/pbsd.userland.libc.stdio.rget.cppm
    libc/pbsd.userland.libc.stdio.swscanf.cppm
    libc/pbsd.userland.libc.stdio.tempnam.cppm
    libc/pbsd.userland.libc.stdio.vswprintf.cppm
    libc/pbsd.userland.libc.stdio.vswscanf.cppm
    libc/pbsd.userland.libc.stdio.wbuf.cppm
    libc/pbsd.userland.libc.stdio.wsetup.cppm
    libc/pbsd.userland.libc.stdio.xprintf.cppm
    libc/pbsd.userland.libc.stdio.xprintf_errno.cppm
    libc/pbsd.userland.libc.stdio.xprintf_float.cppm
    libc/pbsd.userland.libc.stdio.xprintf_hexdump.cppm
    libc/pbsd.userland.libc.stdio.xprintf_int.cppm
    libc/pbsd.userland.libc.stdio.xprintf_quote.cppm
    libc/pbsd.userland.libc.stdio.xprintf_str.cppm
    libc/pbsd.userland.libc.stdio.xprintf_time.cppm
    libc/pbsd.userland.libc.stdio.xprintf_vis.cppm
    libc/pbsd.userland.libc.stdtime.strftime.cppm
    libc/pbsd.userland.libc.stdtime.strptime.cppm
    libc/pbsd.userland.libc.stdtime.time32.cppm
    libc/pbsd.userland.libc.stdtime.timelocal.cppm
    libc/pbsd.userland.libc.string.wcscat.cppm
    libc/pbsd.userland.libc.string.wcschr.cppm
    libc/pbsd.userland.libc.string.wcscoll.cppm
    libc/pbsd.userland.libc.string.wcslcat.cppm
    libc/pbsd.userland.libc.string.wcslcpy.cppm
    libc/pbsd.userland.libc.string.wcsncat.cppm
    libc/pbsd.userland.libc.string.wcsncpy.cppm
    libc/pbsd.userland.libc.string.wcsnlen.cppm
    libc/pbsd.userland.libc.string.wcspbrk.cppm
    libc/pbsd.userland.libc.string.wcsstr.cppm
    libc/pbsd.userland.libc.string.wcstok.cppm
    libc/pbsd.userland.libc.string.wcswidth.cppm
    libc/pbsd.userland.libc.string.wcsxfrm.cppm
    libc/pbsd.userland.libc.string.wmemcmp.cppm
    libc/pbsd.userland.libc.string.wmempcpy.cppm
    libc/pbsd.userland.libc.xdr.xdr.cppm
    libc/pbsd.userland.libc.xdr.xdr_array.cppm
    libc/pbsd.userland.libc.xdr.xdr_float.cppm
    libc/pbsd.userland.libc.xdr.xdr_mem.cppm
    libc/pbsd.userland.libc.xdr.xdr_rec.cppm
    libc/pbsd.userland.libc.xdr.xdr_reference.cppm
    libc/pbsd.userland.libc.xdr.xdr_sizeof.cppm
    libc/pbsd.userland.libc.xdr.xdr_stdio.cppm
    libc/pbsd.userland.libc.yp.xdryp.cppm
    libc/pbsd.userland.libc.yp.yplib.cppm

# --- burst 16 wave 2: libc locale/rpc ---
    libc/pbsd.userland.libc.gdtoa.machdep_ldisd.cppm
    libc/pbsd.userland.libc.locale.ascii.cppm
    libc/pbsd.userland.libc.locale.big5.cppm
    libc/pbsd.userland.libc.locale.c16rtomb.cppm
    libc/pbsd.userland.libc.locale.c16rtomb_iconv.cppm
    libc/pbsd.userland.libc.locale.c32rtomb.cppm
    libc/pbsd.userland.libc.locale.c32rtomb_iconv.cppm
    libc/pbsd.userland.libc.locale.collate.cppm
    libc/pbsd.userland.libc.locale.collcmp.cppm
    libc/pbsd.userland.libc.locale.euc.cppm
    libc/pbsd.userland.libc.locale.fix_grouping.cppm
    libc/pbsd.userland.libc.locale.gb18030.cppm
    libc/pbsd.userland.libc.locale.gb2312.cppm
    libc/pbsd.userland.libc.locale.gbk.cppm
    libc/pbsd.userland.libc.locale.isctype.cppm
    libc/pbsd.userland.libc.locale.iswctype.cppm
    libc/pbsd.userland.libc.locale.ldpart.cppm
    libc/pbsd.userland.libc.locale.lmessages.cppm
    libc/pbsd.userland.libc.locale.lmonetary.cppm
    libc/pbsd.userland.libc.locale.lnumeric.cppm
    libc/pbsd.userland.libc.locale.mbrlen.cppm
    libc/pbsd.userland.libc.locale.mbrtoc16.cppm
    libc/pbsd.userland.libc.locale.mbrtoc16_iconv.cppm
    libc/pbsd.userland.libc.locale.mbrtoc32.cppm
    libc/pbsd.userland.libc.locale.mbrtoc32_iconv.cppm
    libc/pbsd.userland.libc.locale.mbrtowc.cppm
    libc/pbsd.userland.libc.locale.mbsinit.cppm
    libc/pbsd.userland.libc.locale.mbsnrtowcs.cppm
    libc/pbsd.userland.libc.locale.mbsrtowcs.cppm
    libc/pbsd.userland.libc.locale.mbtowc.cppm
    libc/pbsd.userland.libc.locale.mskanji.cppm
    libc/pbsd.userland.libc.locale.nextwctype.cppm
    libc/pbsd.userland.libc.locale.nomacros.cppm
    libc/pbsd.userland.libc.locale.none.cppm
    libc/pbsd.userland.libc.locale.rune.cppm
    libc/pbsd.userland.libc.locale.runetype.cppm
    libc/pbsd.userland.libc.locale.setrunelocale.cppm
    libc/pbsd.userland.libc.locale.table.cppm
    libc/pbsd.userland.libc.locale.utf8.cppm
    libc/pbsd.userland.libc.locale.wcrtomb.cppm
    libc/pbsd.userland.libc.locale.wcsftime.cppm
    libc/pbsd.userland.libc.locale.wcsnrtombs.cppm
    libc/pbsd.userland.libc.locale.wcsrtombs.cppm
    libc/pbsd.userland.libc.locale.wcstod.cppm
    libc/pbsd.userland.libc.locale.wcstof.cppm
    libc/pbsd.userland.libc.locale.wcstoimax.cppm
    libc/pbsd.userland.libc.locale.wcstol.cppm
    libc/pbsd.userland.libc.locale.wcstold.cppm
    libc/pbsd.userland.libc.locale.wcstoll.cppm
    libc/pbsd.userland.libc.locale.wcstoul.cppm
    libc/pbsd.userland.libc.locale.wcstoull.cppm
    libc/pbsd.userland.libc.locale.wcstoumax.cppm
    libc/pbsd.userland.libc.locale.wctomb.cppm
    libc/pbsd.userland.libc.locale.wctrans.cppm
    libc/pbsd.userland.libc.locale.wctype.cppm
    libc/pbsd.userland.libc.locale.xlocale.cppm
    libc/pbsd.userland.libc.rpc.auth_des.cppm
    libc/pbsd.userland.libc.rpc.auth_none.cppm
    libc/pbsd.userland.libc.rpc.auth_time.cppm
    libc/pbsd.userland.libc.rpc.auth_unix.cppm
    libc/pbsd.userland.libc.rpc.authdes_prot.cppm
    libc/pbsd.userland.libc.rpc.authunix_prot.cppm
    libc/pbsd.userland.libc.rpc.bindresvport.cppm
    libc/pbsd.userland.libc.rpc.clnt_bcast.cppm
    libc/pbsd.userland.libc.rpc.clnt_dg.cppm
    libc/pbsd.userland.libc.rpc.clnt_generic.cppm
    libc/pbsd.userland.libc.rpc.clnt_perror.cppm
    libc/pbsd.userland.libc.rpc.clnt_raw.cppm
    libc/pbsd.userland.libc.rpc.clnt_simple.cppm
    libc/pbsd.userland.libc.rpc.clnt_vc.cppm
    libc/pbsd.userland.libc.rpc.crypt_client.cppm
    libc/pbsd.userland.libc.rpc.des_crypt.cppm
    libc/pbsd.userland.libc.rpc.des_soft.cppm
    libc/pbsd.userland.libc.rpc.getnetconfig.cppm
    libc/pbsd.userland.libc.rpc.getnetpath.cppm
    libc/pbsd.userland.libc.rpc.getpublickey.cppm
    libc/pbsd.userland.libc.rpc.getrpcent.cppm
    libc/pbsd.userland.libc.rpc.getrpcport.cppm
    libc/pbsd.userland.libc.rpc.key_call.cppm
    libc/pbsd.userland.libc.rpc.key_prot_xdr.cppm
    libc/pbsd.userland.libc.rpc.mt_misc.cppm
    libc/pbsd.userland.libc.rpc.netname.cppm
    libc/pbsd.userland.libc.rpc.netnamer.cppm
    libc/pbsd.userland.libc.rpc.pmap_clnt.cppm
    libc/pbsd.userland.libc.rpc.pmap_getmaps.cppm
    libc/pbsd.userland.libc.rpc.pmap_getport.cppm
    libc/pbsd.userland.libc.rpc.pmap_prot.cppm
    libc/pbsd.userland.libc.rpc.pmap_prot2.cppm
    libc/pbsd.userland.libc.rpc.pmap_rmt.cppm
    libc/pbsd.userland.libc.rpc.rpc_callmsg.cppm
    libc/pbsd.userland.libc.rpc.rpc_commondata.cppm
    libc/pbsd.userland.libc.rpc.rpc_dtablesize.cppm
    libc/pbsd.userland.libc.rpc.rpc_generic.cppm
    libc/pbsd.userland.libc.rpc.rpc_prot.cppm
    libc/pbsd.userland.libc.rpc.rpc_soc.cppm
    libc/pbsd.userland.libc.rpc.rpcb_clnt.cppm
    libc/pbsd.userland.libc.rpc.rpcb_prot.cppm
    libc/pbsd.userland.libc.rpc.rpcb_st_xdr.cppm
    libc/pbsd.userland.libc.rpc.rpcdname.cppm
    libc/pbsd.userland.libc.rpc.rpcsec_gss_stub.cppm
    libc/pbsd.userland.libc.rpc.rtime.cppm
    libc/pbsd.userland.libc.rpc.svc.cppm
    libc/pbsd.userland.libc.rpc.svc_auth.cppm
    libc/pbsd.userland.libc.rpc.svc_auth_des.cppm
    libc/pbsd.userland.libc.rpc.svc_auth_unix.cppm
    libc/pbsd.userland.libc.rpc.svc_dg.cppm
    libc/pbsd.userland.libc.rpc.svc_generic.cppm
    libc/pbsd.userland.libc.rpc.svc_nl.cppm
    libc/pbsd.userland.libc.rpc.svc_raw.cppm
    libc/pbsd.userland.libc.rpc.svc_run.cppm
    libc/pbsd.userland.libc.rpc.svc_simple.cppm
    libc/pbsd.userland.libc.rpc.svc_vc.cppm
    libc/pbsd.userland.libc.stdlib.free_aligned_sized.cppm
    libc/pbsd.userland.libc.stdlib.free_sized.cppm
    libc/pbsd.userland.libc.stdio.stdio.cppm
    libc/pbsd.userland.libc.regex.regprefix.cppm
    libc/pbsd.userland.libc.net.link_proto.cppm
    libc/pbsd.userland.libc.gen.errlst.cppm
# --- burst 17: libc msun float bridges (delegates to pbsd.userland.msun) -----
    libc/msun/pbsd.userland.libc.msun.cppm
    libc/msun/pbsd.userland.libc.msun.ldexpf.cppm
    libc/msun/pbsd.userland.libc.msun.scalbnf.cppm
    libc/msun/pbsd.userland.libc.msun.nextafterf.cppm
    libc/msun/pbsd.userland.libc.msun.sinpif.cppm
    libc/msun/pbsd.userland.libc.msun.cospif.cppm
    libc/msun/pbsd.userland.libc.msun.exp2f.cppm
    libc/msun/pbsd.userland.libc.msun.truncf.cppm
    libc/msun/pbsd.userland.libc.msun.frexpf.cppm
    libc/msun/pbsd.userland.libc.msun.log2f.cppm
    libc/msun/pbsd.userland.libc.msun.fmodf.cppm
    libc/msun/pbsd.userland.libc.msun.acosf.cppm
    libc/msun/pbsd.userland.libc.msun.asinf.cppm
    libc/msun/pbsd.userland.libc.msun.atanf.cppm
    libc/msun/pbsd.userland.libc.msun.atan2f.cppm
    libc/msun/pbsd.userland.libc.msun.cosf.cppm
    libc/msun/pbsd.userland.libc.msun.sinf.cppm
    libc/msun/pbsd.userland.libc.msun.tanf.cppm
    libc/msun/pbsd.userland.libc.msun.sqrtf.cppm
    libc/msun/pbsd.userland.libc.msun.expf.cppm
    libc/msun/pbsd.userland.libc.msun.logf.cppm
    libc/msun/pbsd.userland.libc.msun.log10f.cppm
    libc/msun/pbsd.userland.libc.msun.fabsf.cppm
    libc/msun/pbsd.userland.libc.msun.ceilf.cppm
    libc/msun/pbsd.userland.libc.msun.floorf.cppm
    libc/msun/pbsd.userland.libc.msun.roundf.cppm
    libc/msun/pbsd.userland.libc.msun.rintf.cppm
    libc/msun/pbsd.userland.libc.msun.hypotf.cppm
    libc/msun/pbsd.userland.libc.msun.cbrtf.cppm
    libc/msun/pbsd.userland.libc.msun.sinhf.cppm
    libc/msun/pbsd.userland.libc.msun.coshf.cppm
    libc/msun/pbsd.userland.libc.msun.modff.cppm
    libc/msun/pbsd.userland.libc.msun.nearbyintf.cppm
    libc/msun/pbsd.userland.libc.msun.remainderf.cppm
    libc/pbsd.userland.libc.cppm)
pbsd_userland_module(pbsd_userland_libc)
target_link_libraries(pbsd_userland_libc PUBLIC pbsd_userland_hosted)

# --- libthr (pthread concept stubs) -------------------------------------------
pbsd_userland_add_library(pbsd_userland_libthr)
target_sources(pbsd_userland_libthr PUBLIC FILE_SET CXX_MODULES FILES
    libthr/pbsd.userland.libthr.mutex.cppm
    libthr/pbsd.userland.libthr.cond.cppm
    libthr/pbsd.userland.libthr.thread.cppm
    libthr/pbsd.userland.libthr.once.cppm
    libthr/pbsd.userland.libthr.key.cppm
    libthr/pbsd.userland.libthr.equal.cppm
    libthr/pbsd.userland.libthr.spinlock.cppm
    libthr/pbsd.userland.libthr.sem.cppm
    libthr/pbsd.userland.libthr.detach.cppm
    libthr/pbsd.userland.libthr.kill.cppm
    libthr/pbsd.userland.libthr.barrier.cppm
    libthr/pbsd.userland.libthr.mutexattr.cppm
    libthr/pbsd.userland.libthr.condattr.cppm
    libthr/pbsd.userland.libthr.rwlock.cppm
    libthr/pbsd.userland.libthr.yield.cppm
    libthr/pbsd.userland.libthr.affinity.cppm
    libthr/pbsd.userland.libthr.attr.cppm
    libthr/pbsd.userland.libthr.cancel.cppm
    libthr/pbsd.userland.libthr.join.cppm
    libthr/pbsd.userland.libthr.sched.cppm
    libthr/pbsd.userland.libthr.tls.cppm
    libthr/pbsd.userland.libthr.signals.cppm
    libthr/pbsd.userland.libthr.symbols.cppm
    libthr/pbsd.userland.libthr.concurrency.cppm
    libthr/pbsd.userland.libthr.cleanup.cppm
    libthr/pbsd.userland.libthr.setname.cppm
    libthr/pbsd.userland.libthr.getname_np.cppm
    libthr/pbsd.userland.libthr.setcancelstate.cppm
    libthr/pbsd.userland.libthr.testcancel.cppm
    libthr/pbsd.userland.libthr.exit.cppm
    libthr/pbsd.userland.libthr.init.cppm
    libthr/pbsd.userland.libthr.stack.cppm
    libthr/pbsd.userland.libthr.rwlock_try.cppm
    libthr/pbsd.userland.libthr.mutex.timed.cppm
    libthr/pbsd.userland.libthr.cond_timed.cppm
    libthr/pbsd.userland.libthr.rwlock_timed.cppm
    libthr/pbsd.userland.libthr.spinlock_try.cppm
    libthr/pbsd.userland.libthr.self.cppm
    libthr/pbsd.userland.libthr.create.cppm
    libthr/pbsd.userland.libthr.fork.cppm
    libthr/pbsd.userland.libthr.getthreadid_np.cppm
    libthr/pbsd.userland.libthr.getschedparam.cppm
    libthr/pbsd.userland.libthr.setschedparam.cppm
    libthr/pbsd.userland.libthr.getprio.cppm
    libthr/pbsd.userland.libthr.setprio.cppm
    libthr/pbsd.userland.libthr.getcpuclockid.cppm
    libthr/pbsd.userland.libthr.info.cppm
    libthr/pbsd.userland.libthr.barrierattr.cppm
    libthr/pbsd.userland.libthr.rwlockattr.cppm
    libthr/pbsd.userland.libthr.pshared.cppm
    libthr/pbsd.userland.libthr.multi_np.cppm
    libthr/pbsd.userland.libthr.single_np.cppm
    libthr/pbsd.userland.libthr.switch_np.cppm
    libthr/pbsd.userland.libthr.suspend_np.cppm
    libthr/pbsd.userland.libthr.resume_np.cppm
    libthr/pbsd.userland.libthr.main_np.cppm
    libthr/pbsd.userland.libthr.sig.cppm
    libthr/pbsd.userland.libthr.sigqueue.cppm
    libthr/pbsd.userland.libthr.autoinit.cppm
    libthr/pbsd.userland.libthr.clean.cppm
    libthr/pbsd.userland.libthr.event.cppm
    libthr/pbsd.userland.libthr.list.cppm
    libthr/pbsd.userland.libthr.kern.cppm
    libthr/pbsd.userland.libthr.malloc.cppm
    libthr/pbsd.userland.libthr.printf.cppm
    libthr/pbsd.userland.libthr.rtld.cppm
    libthr/pbsd.userland.libthr.sleepq.cppm
    libthr/pbsd.userland.libthr.spec.cppm
    libthr/pbsd.userland.libthr.syscalls.cppm
    libthr/pbsd.userland.libthr.umtx.cppm
    libthr/pbsd.userland.libthr.ctrdtr.cppm
    libthr/pbsd.userland.libthr.pspinlock.cppm
    libthr/pbsd.userland.libthr.error.cppm
    libthr/pbsd.userland.libthr.machdep.cppm
    libthr/pbsd.userland.libthr.cppm)
pbsd_userland_module(pbsd_userland_libthr)

# --- rtld (dynamic linker symbol table stubs) ---------------------------------
pbsd_userland_add_library(pbsd_userland_rtld)
target_sources(pbsd_userland_rtld PUBLIC FILE_SET CXX_MODULES FILES
    rtld/pbsd.userland.rtld.symbols.cppm
    rtld/pbsd.userland.rtld.dl.cppm
    rtld/pbsd.userland.rtld.linkmap.cppm
    rtld/pbsd.userland.rtld.paths.cppm
    rtld/pbsd.userland.rtld.version.cppm
    rtld/pbsd.userland.rtld.libmap.cppm
    rtld/pbsd.userland.rtld.lock.cppm
    rtld/pbsd.userland.rtld.debug.cppm
    rtld/pbsd.userland.rtld.malloc.cppm
    rtld/pbsd.userland.rtld.reloc.cppm
    rtld/pbsd.userland.rtld.cppm)
pbsd_userland_module(pbsd_userland_rtld)

# --- libm (msun subset) -------------------------------------------------------
pbsd_userland_add_library(pbsd_userland_msun)
target_sources(pbsd_userland_msun PUBLIC FILE_SET CXX_MODULES FILES
    msun/pbsd.userland.msun.fabs.cppm
    msun/pbsd.userland.msun.ceil.cppm
    msun/pbsd.userland.msun.floor.cppm
    msun/pbsd.userland.msun.sqrt.cppm
    msun/pbsd.userland.msun.ceilf.cppm
    msun/pbsd.userland.msun.floorf.cppm
    msun/pbsd.userland.msun.frexp.cppm
    msun/pbsd.userland.msun.scalbn.cppm
    msun/pbsd.userland.msun.truncf.cppm
    msun/pbsd.userland.msun.roundf.cppm
    msun/pbsd.userland.msun.rintf.cppm
    msun/pbsd.userland.msun.signbit.cppm
    msun/pbsd.userland.msun.isfinite.cppm
    msun/pbsd.userland.msun.fdim.cppm
    msun/pbsd.userland.msun.copysign.cppm
    msun/pbsd.userland.msun.finite.cppm
    msun/pbsd.userland.msun.lrint.cppm
    msun/pbsd.userland.msun.llround.cppm
    msun/pbsd.userland.msun.drem.cppm
    msun/pbsd.userland.msun.sin.cppm
    msun/pbsd.userland.msun.cos.cppm
    msun/pbsd.userland.msun.log.cppm
    msun/pbsd.userland.msun.exp.cppm
    msun/pbsd.userland.msun.pow.cppm
    msun/pbsd.userland.msun.hypot.cppm
    msun/pbsd.userland.msun.fma.cppm
    msun/pbsd.userland.msun.nextafter.cppm
    msun/pbsd.userland.msun.nan.cppm
    msun/pbsd.userland.msun.tan.cppm
    msun/pbsd.userland.msun.atan.cppm
    msun/pbsd.userland.msun.atan2.cppm
    msun/pbsd.userland.msun.fmod.cppm
    msun/pbsd.userland.msun.asin.cppm
    msun/pbsd.userland.msun.acos.cppm
    msun/pbsd.userland.msun.log10.cppm
    msun/pbsd.userland.msun.modf.cppm
    msun/pbsd.userland.msun.fmin.cppm
    msun/pbsd.userland.msun.fmax.cppm
    msun/pbsd.userland.msun.round.cppm
    msun/pbsd.userland.msun.trunc.cppm
    msun/pbsd.userland.msun.ilogb.cppm
    msun/pbsd.userland.msun.logb.cppm
    msun/pbsd.userland.msun.isnan.cppm
    msun/pbsd.userland.msun.exp2.cppm
    msun/pbsd.userland.msun.log1p.cppm
    msun/pbsd.userland.msun.sinh.cppm
    msun/pbsd.userland.msun.cosh.cppm
    msun/pbsd.userland.msun.tanh.cppm
    msun/pbsd.userland.msun.cbrt.cppm
    msun/pbsd.userland.msun.expm1.cppm
    msun/pbsd.userland.msun.remainder.cppm
    msun/pbsd.userland.msun.nearbyint.cppm
    msun/pbsd.userland.msun.erf.cppm
    msun/pbsd.userland.msun.erfc.cppm
    msun/pbsd.userland.msun.lgamma.cppm
    msun/pbsd.userland.msun.remquo.cppm
    msun/pbsd.userland.msun.j0.cppm
    msun/pbsd.userland.msun.acosh.cppm
    msun/pbsd.userland.msun.asinh.cppm
    msun/pbsd.userland.msun.atanh.cppm
    msun/pbsd.userland.msun.hypotf.cppm
    msun/pbsd.userland.msun.log2.cppm
    msun/pbsd.userland.msun.j1.cppm
    msun/pbsd.userland.msun.y1.cppm
    msun/pbsd.userland.msun.tgamma.cppm
    msun/pbsd.userland.msun.rint.cppm
    msun/pbsd.userland.msun.fmaxf.cppm
    msun/pbsd.userland.msun.fminf.cppm
    msun/pbsd.userland.msun.remainderf.cppm
    msun/pbsd.userland.msun.nearbyintf.cppm
    msun/pbsd.userland.msun.copysignf.cppm
    msun/pbsd.userland.msun.frexpf.cppm
    msun/pbsd.userland.msun.modff.cppm
    msun/pbsd.userland.msun.lgammaf.cppm
    msun/pbsd.userland.msun.scalbln.cppm
    msun/pbsd.userland.msun.scalbnf.cppm
    msun/pbsd.userland.msun.ldexpf.cppm
    msun/pbsd.userland.msun.ilogbf.cppm
    msun/pbsd.userland.msun.logbf.cppm
    msun/pbsd.userland.msun.log10f.cppm
    msun/pbsd.userland.msun.expf.cppm
    msun/pbsd.userland.msun.sinf.cppm
    msun/pbsd.userland.msun.cosf.cppm
    msun/pbsd.userland.msun.tanf.cppm
    msun/pbsd.userland.msun.sqrtf.cppm
    msun/pbsd.userland.msun.asinf.cppm
    msun/pbsd.userland.msun.acosf.cppm
    msun/pbsd.userland.msun.atanf.cppm
    msun/pbsd.userland.msun.atan2f.cppm
    msun/pbsd.userland.msun.sinhf.cppm
    msun/pbsd.userland.msun.coshf.cppm
    msun/pbsd.userland.msun.tanhf.cppm
    msun/pbsd.userland.msun.cbrtf.cppm
    msun/pbsd.userland.msun.expm1f.cppm
    msun/pbsd.userland.msun.log1pf.cppm
    msun/pbsd.userland.msun.fmodf.cppm
    msun/pbsd.userland.msun.j0f.cppm
    msun/pbsd.userland.msun.y0f.cppm
    msun/pbsd.userland.msun.lroundf.cppm
    msun/pbsd.userland.msun.llroundf.cppm
    msun/pbsd.userland.msun.lrintf.cppm
    msun/pbsd.userland.msun.fabsf.cppm
    msun/pbsd.userland.msun.ldexp.cppm
    msun/pbsd.userland.msun.acoshf.cppm
    msun/pbsd.userland.msun.asinhf.cppm
    msun/pbsd.userland.msun.atanhf.cppm
    msun/pbsd.userland.msun.log2f.cppm
    msun/pbsd.userland.msun.exp2f.cppm
    msun/pbsd.userland.msun.nextafterf.cppm
    msun/pbsd.userland.msun.cospif.cppm
    msun/pbsd.userland.msun.sinpif.cppm
    msun/pbsd.userland.msun.fminimum.cppm
    msun/pbsd.userland.msun.fminimumf.cppm
    msun/pbsd.userland.msun.fminl.cppm
    msun/pbsd.userland.msun.fmaxl.cppm
    msun/pbsd.userland.msun.frexpl.cppm
    msun/pbsd.userland.msun.ilogbl.cppm
    msun/pbsd.userland.msun.isnormal.cppm
    msun/pbsd.userland.msun.logbl.cppm
    msun/pbsd.userland.msun.y0.cppm
    msun/pbsd.userland.msun.acoshl.cppm
    msun/pbsd.userland.msun.acosl.cppm
    msun/pbsd.userland.msun.asinhl.cppm
    msun/pbsd.userland.msun.asinl.cppm
    msun/pbsd.userland.msun.atan2l.cppm
    msun/pbsd.userland.msun.atanhl.cppm
    msun/pbsd.userland.msun.atanl.cppm
    msun/pbsd.userland.msun.carg.cppm
    msun/pbsd.userland.msun.cargf.cppm
    msun/pbsd.userland.msun.cargl.cppm
    msun/pbsd.userland.msun.catrig.cppm
    msun/pbsd.userland.msun.catrigf.cppm
    msun/pbsd.userland.msun.catrigl.cppm
    msun/pbsd.userland.msun.cbrtl.cppm
    msun/pbsd.userland.msun.ccosh.cppm
    msun/pbsd.userland.msun.ccoshf.cppm
    msun/pbsd.userland.msun.ceill.cppm
    msun/pbsd.userland.msun.cexp.cppm
    msun/pbsd.userland.msun.cexpf.cppm
    msun/pbsd.userland.msun.cimag.cppm
    msun/pbsd.userland.msun.cimagf.cppm
    msun/pbsd.userland.msun.cimagl.cppm
    msun/pbsd.userland.msun.clog.cppm
    msun/pbsd.userland.msun.clogf.cppm
    msun/pbsd.userland.msun.clogl.cppm
    msun/pbsd.userland.msun.conj.cppm
    msun/pbsd.userland.msun.conjf.cppm
    msun/pbsd.userland.msun.conjl.cppm
    msun/pbsd.userland.msun.copysignl.cppm
    msun/pbsd.userland.msun.coshl.cppm
    msun/pbsd.userland.msun.cosl.cppm
    msun/pbsd.userland.msun.cospi.cppm
    msun/pbsd.userland.msun.cpow.cppm
    msun/pbsd.userland.msun.cpowf.cppm
    msun/pbsd.userland.msun.cpowl.cppm
    msun/pbsd.userland.msun.cproj.cppm
    msun/pbsd.userland.msun.cprojf.cppm
    msun/pbsd.userland.msun.cprojl.cppm
    msun/pbsd.userland.msun.creal.cppm
    msun/pbsd.userland.msun.crealf.cppm
    msun/pbsd.userland.msun.creall.cppm
    msun/pbsd.userland.msun.csinh.cppm
    msun/pbsd.userland.msun.csinhf.cppm
    msun/pbsd.userland.msun.csqrt.cppm
    msun/pbsd.userland.msun.csqrtf.cppm
    msun/pbsd.userland.msun.csqrtl.cppm
    msun/pbsd.userland.msun.ctanh.cppm
    msun/pbsd.userland.msun.ctanhf.cppm
    msun/pbsd.userland.msun.erff.cppm
    msun/pbsd.userland.msun.fabsl.cppm
    msun/pbsd.userland.msun.finitef.cppm
    msun/pbsd.userland.msun.floorl.cppm
    msun/pbsd.userland.msun.fmaf.cppm
    msun/pbsd.userland.msun.fmal.cppm
    msun/pbsd.userland.msun.fmaximum.cppm
    msun/pbsd.userland.msun.fmaximum_mag.cppm
    msun/pbsd.userland.msun.fmaximumf.cppm
    msun/pbsd.userland.msun.fmaximuml.cppm
    msun/pbsd.userland.msun.fmodl.cppm
    msun/pbsd.userland.msun.gamma.cppm
    msun/pbsd.userland.msun.gamma_r.cppm
    msun/pbsd.userland.msun.gammaf.cppm
    msun/pbsd.userland.msun.gammaf_r.cppm
    msun/pbsd.userland.msun.hypotl.cppm
    msun/pbsd.userland.msun.j1f.cppm
    msun/pbsd.userland.msun.jn.cppm
    msun/pbsd.userland.msun.jnf.cppm
    msun/pbsd.userland.msun.lgamma_r.cppm
    msun/pbsd.userland.msun.lgammaf_r.cppm
    msun/pbsd.userland.msun.lgammal.cppm
    msun/pbsd.userland.msun.logf.cppm
    msun/pbsd.userland.msun.powf.cppm
    msun/pbsd.userland.msun.rem_pio2.cppm
    msun/pbsd.userland.msun.rem_pio2f.cppm
    msun/pbsd.userland.msun.remainderl.cppm
    msun/pbsd.userland.msun.scalb.cppm
    msun/pbsd.userland.msun.scalbf.cppm
    msun/pbsd.userland.msun.sinhl.cppm
    msun/pbsd.userland.msun.sqrtl.cppm
    msun/pbsd.userland.msun.llrint.cppm
    msun/pbsd.userland.msun.llrintf.cppm
    msun/pbsd.userland.msun.lround.cppm
    msun/pbsd.userland.msun.nexttoward.cppm
    msun/pbsd.userland.msun.nexttowardf.cppm
    msun/pbsd.userland.msun.sincos.cppm
    msun/pbsd.userland.msun.sinpi.cppm
    msun/pbsd.userland.msun.tanpi.cppm
    msun/pbsd.userland.msun.rsqrt.cppm
    msun/pbsd.userland.msun.scalbnl.cppm
    msun/pbsd.userland.msun.significand.cppm
    msun/pbsd.userland.msun.cabs.cppm
    msun/pbsd.userland.msun.cabsf.cppm
    msun/pbsd.userland.msun.cabsl.cppm
    msun/pbsd.userland.msun.dremf.cppm
    msun/pbsd.userland.msun.fmaximum_mag_num.cppm
    msun/pbsd.userland.msun.fmaximum_mag_numf.cppm
    msun/pbsd.userland.msun.fmaximum_mag_numl.cppm
    msun/pbsd.userland.msun.fmaximum_magf.cppm
    msun/pbsd.userland.msun.fmaximum_magl.cppm
    msun/pbsd.userland.msun.fmaximum_num.cppm
    msun/pbsd.userland.msun.fmaximum_numf.cppm
    msun/pbsd.userland.msun.fmaximum_numl.cppm
    msun/pbsd.userland.msun.fminimum_mag.cppm
    msun/pbsd.userland.msun.fminimum_mag_num.cppm
    msun/pbsd.userland.msun.fminimum_mag_numf.cppm
    msun/pbsd.userland.msun.fminimum_mag_numl.cppm
    msun/pbsd.userland.msun.fminimum_magf.cppm
    msun/pbsd.userland.msun.fminimum_magl.cppm
    msun/pbsd.userland.msun.fminimum_num.cppm
    msun/pbsd.userland.msun.fminimum_numf.cppm
    msun/pbsd.userland.msun.fminimum_numl.cppm
    msun/pbsd.userland.msun.fminimuml.cppm
    msun/pbsd.userland.msun.llrintl.cppm
    msun/pbsd.userland.msun.llroundl.cppm
    msun/pbsd.userland.msun.lrintl.cppm
    msun/pbsd.userland.msun.lroundl.cppm
    msun/pbsd.userland.msun.modfl.cppm
    msun/pbsd.userland.msun.nextafterl.cppm
    msun/pbsd.userland.msun.remquof.cppm
    msun/pbsd.userland.msun.remquol.cppm
    msun/pbsd.userland.msun.rintl.cppm
    msun/pbsd.userland.msun.roundl.cppm
    msun/pbsd.userland.msun.rsqrtf.cppm
    msun/pbsd.userland.msun.rsqrtl.cppm
    msun/pbsd.userland.msun.signgam.cppm
    msun/pbsd.userland.msun.significandf.cppm
    msun/pbsd.userland.msun.sincosf.cppm
    msun/pbsd.userland.msun.sincosl.cppm
    msun/pbsd.userland.msun.sinl.cppm
    msun/pbsd.userland.msun.tanhl.cppm
    msun/pbsd.userland.msun.tanl.cppm
    msun/pbsd.userland.msun.tanpif.cppm
    msun/pbsd.userland.msun.tgammaf.cppm
    msun/pbsd.userland.msun.truncl.cppm
    msun/pbsd.userland.msun.cppm)
pbsd_userland_module(pbsd_userland_msun)
target_link_libraries(pbsd_userland_libc PUBLIC pbsd_userland_msun)

# --- libprocstat --------------------------------------------------------------
if(NOT TARGET pbsd_userland_libprocstat)
    pbsd_userland_add_library(pbsd_userland_libprocstat)
    target_sources(pbsd_userland_libprocstat PUBLIC FILE_SET CXX_MODULES FILES
        libprocstat/pbsd.userland.libprocstat.core.cppm
        libprocstat/pbsd.userland.libprocstat.common_kvm.cppm
        libprocstat/pbsd.userland.libprocstat.cd9660.cppm
        libprocstat/pbsd.userland.libprocstat.msdosfs.cppm
        libprocstat/pbsd.userland.libprocstat.smbfs.cppm
        libprocstat/pbsd.userland.libprocstat.udf.cppm
        libprocstat/pbsd.userland.libprocstat.zfs.cppm
        libprocstat/pbsd.userland.libprocstat.compat.cppm
        libprocstat/pbsd.userland.libprocstat.cppm)
    target_link_libraries(pbsd_userland_libprocstat PUBLIC pbsd_userland_hosted)
    pbsd_userland_module(pbsd_userland_libprocstat)
endif()

# --- libjail ------------------------------------------------------------------
pbsd_userland_add_library(pbsd_userland_jail)
target_sources(pbsd_userland_jail PUBLIC FILE_SET CXX_MODULES FILES
    jail/pbsd.userland.jail.cppm
    jail/pbsd.userland.jail.param.cppm)
target_link_libraries(pbsd_userland_jail PUBLIC pbsd_userland_libc)
pbsd_userland_module(pbsd_userland_jail)

# --- libutil ------------------------------------------------------------------
pbsd_userland_add_library(pbsd_userland_util)
target_sources(pbsd_userland_util PUBLIC FILE_SET CXX_MODULES FILES
    util/pbsd.userland.util.numbers.cppm
    util/pbsd.userland.util.format.cppm
    util/pbsd.userland.util.host.cppm
    util/pbsd.userland.util.property.cppm
    util/pbsd.userland.util.cpuset.cppm
    util/pbsd.userland.util.flopen.cppm
    util/pbsd.userland.util.pidfile.cppm
    util/pbsd.userland.util.filemode.cppm
    util/pbsd.userland.util.fparseln.cppm
    util/pbsd.userland.util.secure.cppm
    util/pbsd.userland.util.paths.cppm
    util/pbsd.userland.util.mntopts.cppm
    util/pbsd.userland.util.uucplock.cppm
    util/pbsd.userland.util.login_times.cppm
    util/pbsd.userland.util.expand_number.cppm
    util/pbsd.userland.util.ftime.cppm
    util/pbsd.userland.util.getlocalbase.cppm
    util/pbsd.userland.util.gr_util.cppm
    util/pbsd.userland.util.hexdump.cppm
    util/pbsd.userland.util.kinfo_getproc.cppm
    util/pbsd.userland.util.kld.cppm
    util/pbsd.userland.util.login_cap.cppm
    util/pbsd.userland.util.pty.cppm
    util/pbsd.userland.util.pw_util.cppm
    util/pbsd.userland.util.trimdomain.cppm
    util/pbsd.userland.util.secure_path.cppm
    util/pbsd.userland.util.cppm)
target_link_libraries(pbsd_userland_util PUBLIC pbsd_userland_libc)
pbsd_userland_module(pbsd_userland_util)

# --- bin/ utilities (hbsd/src/bin/*) ------------------------------------------
pbsd_userland_add_library(pbsd_userland_echo)
target_sources(pbsd_userland_echo PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.echo.cppm)
target_link_libraries(pbsd_userland_echo PUBLIC pbsd_userland_capsicum_helpers)
pbsd_userland_module(pbsd_userland_echo)

pbsd_userland_add_library(pbsd_userland_hostname)
target_sources(pbsd_userland_hostname PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.hostname.cppm)
target_link_libraries(pbsd_userland_hostname PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_hostname)

pbsd_userland_add_library(pbsd_userland_domainname)
target_sources(pbsd_userland_domainname PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.domainname.cppm)
target_link_libraries(pbsd_userland_domainname PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_domainname)

pbsd_userland_add_library(pbsd_userland_nproc)
target_sources(pbsd_userland_nproc PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.nproc.cppm)
target_link_libraries(pbsd_userland_nproc PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_nproc)

pbsd_userland_add_library(pbsd_userland_pwd)
target_sources(pbsd_userland_pwd PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.pwd.cppm)
target_link_libraries(pbsd_userland_pwd PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_pwd)

pbsd_userland_add_library(pbsd_userland_cat)
target_sources(pbsd_userland_cat PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.cat.cppm)
target_link_libraries(pbsd_userland_cat PUBLIC pbsd_userland_capsicum_helpers)
pbsd_userland_module(pbsd_userland_cat)

pbsd_userland_add_library(pbsd_userland_chmod)
target_sources(pbsd_userland_chmod PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.chmod.cppm)
target_link_libraries(pbsd_userland_chmod PUBLIC pbsd_userland_hosted pbsd_userland_util)
pbsd_userland_module(pbsd_userland_chmod)

pbsd_userland_add_library(pbsd_userland_mkdir)
target_sources(pbsd_userland_mkdir PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.mkdir.cppm)
target_link_libraries(pbsd_userland_mkdir PUBLIC pbsd_userland_hosted pbsd_userland_util)
pbsd_userland_module(pbsd_userland_mkdir)

pbsd_userland_add_library(pbsd_userland_rmdir)
target_sources(pbsd_userland_rmdir PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.rmdir.cppm)
target_link_libraries(pbsd_userland_rmdir PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_rmdir)

pbsd_userland_add_library(pbsd_userland_ln)
target_sources(pbsd_userland_ln PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.ln.cppm)
target_link_libraries(pbsd_userland_ln PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_ln)

pbsd_userland_add_library(pbsd_userland_mv)
target_sources(pbsd_userland_mv PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.mv.cppm)
target_link_libraries(pbsd_userland_mv PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_mv)

pbsd_userland_add_library(pbsd_userland_cp)
target_sources(pbsd_userland_cp PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.cp.cppm)
target_link_libraries(pbsd_userland_cp PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_cp)

pbsd_userland_add_library(pbsd_userland_kill)
target_sources(pbsd_userland_kill PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.kill.cppm)
target_link_libraries(pbsd_userland_kill PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_kill)

pbsd_userland_add_library(pbsd_userland_sleep)
target_sources(pbsd_userland_sleep PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.sleep.cppm)
target_link_libraries(pbsd_userland_sleep PUBLIC pbsd_userland_capsicum_helpers)
pbsd_userland_module(pbsd_userland_sleep)

pbsd_userland_add_library(pbsd_userland_true)
target_sources(pbsd_userland_true PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.true.cppm)
pbsd_userland_module(pbsd_userland_true)

pbsd_userland_add_library(pbsd_userland_false)
target_sources(pbsd_userland_false PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.false.cppm)
pbsd_userland_module(pbsd_userland_false)

pbsd_userland_add_library(pbsd_userland_test)
target_sources(pbsd_userland_test PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.test.cppm)
target_link_libraries(pbsd_userland_test PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_test)

pbsd_userland_add_library(pbsd_userland_date)
target_sources(pbsd_userland_date PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.date.cppm)
target_link_libraries(pbsd_userland_date PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_date)

pbsd_userland_add_library(pbsd_userland_chflags)
target_sources(pbsd_userland_chflags PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.chflags.cppm)
target_link_libraries(pbsd_userland_chflags PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_chflags)

pbsd_userland_add_library(pbsd_userland_dd)
target_sources(pbsd_userland_dd PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.dd.cppm)
target_link_libraries(pbsd_userland_dd PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_dd)

pbsd_userland_add_library(pbsd_userland_getfacl)
target_sources(pbsd_userland_getfacl PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.getfacl.cppm)
target_link_libraries(pbsd_userland_getfacl PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_getfacl)

pbsd_userland_add_library(pbsd_userland_setfacl)
target_sources(pbsd_userland_setfacl PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.setfacl.cppm)
target_link_libraries(pbsd_userland_setfacl PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_setfacl)

pbsd_userland_add_library(pbsd_userland_sync)
target_sources(pbsd_userland_sync PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.sync.cppm)
pbsd_userland_module(pbsd_userland_sync)

pbsd_userland_add_library(pbsd_userland_uuidgen)
target_sources(pbsd_userland_uuidgen PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.uuidgen.cppm)
target_link_libraries(pbsd_userland_uuidgen PUBLIC pbsd_userland_capsicum_helpers)
pbsd_userland_module(pbsd_userland_uuidgen)

pbsd_userland_add_library(pbsd_userland_realpath)
target_sources(pbsd_userland_realpath PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.realpath.cppm)
target_link_libraries(pbsd_userland_realpath PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_realpath)

pbsd_userland_add_library(pbsd_userland_kenv)
target_sources(pbsd_userland_kenv PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.kenv.cppm)
target_link_libraries(pbsd_userland_kenv PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_kenv)

pbsd_userland_add_library(pbsd_userland_ls_cmp)
target_sources(pbsd_userland_ls_cmp PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.ls.cmp.cppm)
target_link_libraries(pbsd_userland_ls_cmp PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_ls_cmp)

pbsd_userland_add_library(pbsd_userland_stty_util)
target_sources(pbsd_userland_stty_util PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.stty.util.cppm)
pbsd_userland_module(pbsd_userland_stty_util)

pbsd_userland_add_library(pbsd_userland_sysctl)
target_sources(pbsd_userland_sysctl PUBLIC FILE_SET CXX_MODULES FILES
    sbin/pbsd.userland.sysctl.cppm)
target_link_libraries(pbsd_userland_sysctl PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_sysctl)

pbsd_userland_add_library(pbsd_userland_chown)
target_sources(pbsd_userland_chown PUBLIC FILE_SET CXX_MODULES FILES
    sbin/pbsd.userland.chown.cppm)
target_link_libraries(pbsd_userland_chown PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_chown)

pbsd_userland_add_library(pbsd_userland_df)
target_sources(pbsd_userland_df PUBLIC FILE_SET CXX_MODULES FILES
    sbin/pbsd.userland.df.cppm)
target_link_libraries(pbsd_userland_df PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_df)

# --- usr.bin/ utilities (hbsd/src/usr.bin/*) -------------------------------
pbsd_userland_add_library(pbsd_userland_yes)
target_sources(pbsd_userland_yes PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.yes.cppm)
target_link_libraries(pbsd_userland_yes PUBLIC pbsd_userland_capsicum_helpers)
pbsd_userland_module(pbsd_userland_yes)

pbsd_userland_add_library(pbsd_userland_usr_bin_true)
target_sources(pbsd_userland_usr_bin_true PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.usr_bin.true_cmd.cppm)
target_link_libraries(pbsd_userland_usr_bin_true PUBLIC pbsd_userland_true)
pbsd_userland_module(pbsd_userland_usr_bin_true)

pbsd_userland_add_library(pbsd_userland_usr_bin_false)
target_sources(pbsd_userland_usr_bin_false PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.usr_bin.false_cmd.cppm)
target_link_libraries(pbsd_userland_usr_bin_false PUBLIC pbsd_userland_false)
pbsd_userland_module(pbsd_userland_usr_bin_false)

pbsd_userland_add_library(pbsd_userland_getopt)
target_sources(pbsd_userland_getopt PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.getopt.cppm)
target_link_libraries(pbsd_userland_getopt PUBLIC pbsd_userland_capsicum_helpers)
pbsd_userland_module(pbsd_userland_getopt)

pbsd_userland_add_library(pbsd_userland_basename)
target_sources(pbsd_userland_basename PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.basename.cppm)
target_link_libraries(pbsd_userland_basename PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_basename)

pbsd_userland_add_library(pbsd_userland_dirname)
target_sources(pbsd_userland_dirname PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.dirname.cppm)
target_link_libraries(pbsd_userland_dirname PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_dirname)

pbsd_userland_add_library(pbsd_userland_uname)
target_sources(pbsd_userland_uname PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.uname.cppm)
target_link_libraries(pbsd_userland_uname PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_uname)

pbsd_userland_add_library(pbsd_userland_printenv)
target_sources(pbsd_userland_printenv PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.printenv.cppm)
target_link_libraries(pbsd_userland_printenv PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_printenv)

pbsd_userland_add_library(pbsd_userland_env)
target_sources(pbsd_userland_env PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.env.cppm)
target_link_libraries(pbsd_userland_env PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_env)

pbsd_userland_add_library(pbsd_userland_which)
target_sources(pbsd_userland_which PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.which.cppm)
target_link_libraries(pbsd_userland_which PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_which)

pbsd_userland_add_library(pbsd_userland_time_cmd)
target_sources(pbsd_userland_time_cmd PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.time.cppm)
target_link_libraries(pbsd_userland_time_cmd PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_time_cmd)

pbsd_userland_add_library(pbsd_userland_touch)
target_sources(pbsd_userland_touch PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.touch.cppm)
target_link_libraries(pbsd_userland_touch PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_touch)

pbsd_userland_add_library(pbsd_userland_truncate)
target_sources(pbsd_userland_truncate PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.truncate.cppm)
target_link_libraries(pbsd_userland_truncate PUBLIC pbsd_userland_hosted pbsd_userland_util)
pbsd_userland_module(pbsd_userland_truncate)

pbsd_userland_add_library(pbsd_userland_id)
target_sources(pbsd_userland_id PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.id.cppm)
target_link_libraries(pbsd_userland_id PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_id)

pbsd_userland_add_library(pbsd_userland_head)
target_sources(pbsd_userland_head PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.head.cppm)
target_link_libraries(pbsd_userland_head PUBLIC pbsd_userland_hosted pbsd_userland_util)
pbsd_userland_module(pbsd_userland_head)

pbsd_userland_add_library(pbsd_userland_tail)
target_sources(pbsd_userland_tail PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.tail.cppm)
target_link_libraries(pbsd_userland_tail PUBLIC pbsd_userland_hosted pbsd_userland_util)
pbsd_userland_module(pbsd_userland_tail)

pbsd_userland_add_library(pbsd_userland_wc)
target_sources(pbsd_userland_wc PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.wc.cppm)
target_link_libraries(pbsd_userland_wc PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_wc)

pbsd_userland_add_library(pbsd_userland_tee)
target_sources(pbsd_userland_tee PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.tee.cppm)
target_link_libraries(pbsd_userland_tee PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_tee)

pbsd_userland_add_library(pbsd_userland_mktemp)
target_sources(pbsd_userland_mktemp PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.mktemp.cppm)
target_link_libraries(pbsd_userland_mktemp PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_mktemp)

pbsd_userland_add_library(pbsd_userland_printf)
target_sources(pbsd_userland_printf PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.printf.cppm)
target_link_libraries(pbsd_userland_printf PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_printf)

pbsd_userland_add_library(pbsd_userland_tr)
target_sources(pbsd_userland_tr PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.tr.cppm)
target_link_libraries(pbsd_userland_tr PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_tr)

pbsd_userland_add_library(pbsd_userland_cut)
target_sources(pbsd_userland_cut PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.cut.cppm)
target_link_libraries(pbsd_userland_cut PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_cut)

pbsd_userland_add_library(pbsd_userland_sort)
target_sources(pbsd_userland_sort PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.sort.cppm)
target_link_libraries(pbsd_userland_sort PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_sort)

pbsd_userland_add_library(pbsd_userland_uniq)
target_sources(pbsd_userland_uniq PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.uniq.cppm)
target_link_libraries(pbsd_userland_uniq PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_uniq)

pbsd_userland_add_library(pbsd_userland_comm)
target_sources(pbsd_userland_comm PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.comm.cppm)
target_link_libraries(pbsd_userland_comm PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_comm)

pbsd_userland_add_library(pbsd_userland_cmp)
target_sources(pbsd_userland_cmp PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.cmp.cppm)
target_link_libraries(pbsd_userland_cmp PUBLIC pbsd_userland_hosted pbsd_userland_util)
pbsd_userland_module(pbsd_userland_cmp)

pbsd_userland_add_library(pbsd_userland_diff)
target_sources(pbsd_userland_diff PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.diff.cppm)
target_link_libraries(pbsd_userland_diff PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_diff)

pbsd_userland_add_library(pbsd_userland_seq)
target_sources(pbsd_userland_seq PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.seq.cppm)
target_link_libraries(pbsd_userland_seq PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_seq)

pbsd_userland_add_library(pbsd_userland_jot)
target_sources(pbsd_userland_jot PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.jot.cppm)
target_link_libraries(pbsd_userland_jot PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_jot)

pbsd_userland_add_library(pbsd_userland_calendar)
target_sources(pbsd_userland_calendar PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.calendar.cppm)
target_link_libraries(pbsd_userland_calendar PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_calendar)

pbsd_userland_add_library(pbsd_userland_logger)
target_sources(pbsd_userland_logger PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.logger.cppm)
target_link_libraries(pbsd_userland_logger PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_logger)

pbsd_userland_add_library(pbsd_userland_script)
target_sources(pbsd_userland_script PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.script.cppm)
target_link_libraries(pbsd_userland_script PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_script)

pbsd_userland_add_library(pbsd_userland_join)
target_sources(pbsd_userland_join PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.join.cppm)
target_link_libraries(pbsd_userland_join PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_join)

pbsd_userland_add_library(pbsd_userland_paste)
target_sources(pbsd_userland_paste PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.paste.cppm)
target_link_libraries(pbsd_userland_paste PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_paste)

pbsd_userland_add_library(pbsd_userland_nl)
target_sources(pbsd_userland_nl PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.nl.cppm)
target_link_libraries(pbsd_userland_nl PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_nl)

pbsd_userland_add_library(pbsd_userland_fold)
target_sources(pbsd_userland_fold PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.fold.cppm)
target_link_libraries(pbsd_userland_fold PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_fold)

pbsd_userland_add_library(pbsd_userland_split)
target_sources(pbsd_userland_split PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.split.cppm)
target_link_libraries(pbsd_userland_split PUBLIC pbsd_userland_hosted pbsd_userland_util)
pbsd_userland_module(pbsd_userland_split)

pbsd_userland_add_library(pbsd_userland_expand)
target_sources(pbsd_userland_expand PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.expand.cppm)
target_link_libraries(pbsd_userland_expand PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_expand)

pbsd_userland_add_library(pbsd_userland_tsort)
target_sources(pbsd_userland_tsort PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.tsort.cppm)
target_link_libraries(pbsd_userland_tsort PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_tsort)

pbsd_userland_add_library(pbsd_userland_rev)
target_sources(pbsd_userland_rev PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.rev.cppm)
target_link_libraries(pbsd_userland_rev PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_rev)

foreach(_ul_tool IN ITEMS du find xargs grep sed awk patch gzip ssh fetch od bc)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_tool}
        usr.bin/pbsd.userland.${_ul_tool}.cppm)
endforeach()

foreach(_ul_tool2 IN ITEMS dc column m4)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_tool2}
        usr.bin/pbsd.userland.${_ul_tool2}.cppm)
endforeach()

pbsd_userland_add_library(pbsd_userland_tar)
target_sources(pbsd_userland_tar PUBLIC FILE_SET CXX_MODULES FILES
    bin/pbsd.userland.tar.cppm)
target_link_libraries(pbsd_userland_tar PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_tar)

pbsd_userland_add_library(pbsd_userland_ifconfig)
target_sources(pbsd_userland_ifconfig PUBLIC FILE_SET CXX_MODULES FILES
    sbin/pbsd.userland.ifconfig.cppm)
target_link_libraries(pbsd_userland_ifconfig PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_ifconfig)

pbsd_userland_add_library(pbsd_userland_mount)
target_sources(pbsd_userland_mount PUBLIC FILE_SET CXX_MODULES FILES
    sbin/pbsd.userland.mount.cppm)
target_link_libraries(pbsd_userland_mount PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_mount)

pbsd_userland_add_library(pbsd_userland_route)
target_sources(pbsd_userland_route PUBLIC FILE_SET CXX_MODULES FILES
    sbin/pbsd.userland.route.cppm)
target_link_libraries(pbsd_userland_route PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_route)

pbsd_userland_add_library(pbsd_userland_install)
target_sources(pbsd_userland_install PUBLIC FILE_SET CXX_MODULES FILES
    sbin/pbsd.userland.install.cppm)
target_link_libraries(pbsd_userland_install PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_install)

pbsd_userland_add_library(pbsd_userland_umount)
target_sources(pbsd_userland_umount PUBLIC FILE_SET CXX_MODULES FILES
    sbin/pbsd.userland.umount.cppm)
target_link_libraries(pbsd_userland_umount PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_umount)

pbsd_userland_add_library(pbsd_userland_ping)
target_sources(pbsd_userland_ping PUBLIC FILE_SET CXX_MODULES FILES
    sbin/pbsd.userland.ping.cppm)
target_link_libraries(pbsd_userland_ping PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_ping)

foreach(_ul_sbin IN ITEMS pkg_cmd newsyslog cron pw vipw)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_sbin}
        sbin/pbsd.userland.${_ul_sbin}.cppm)
endforeach()

foreach(_ul_usr_sbin IN ITEMS sysrc service services)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_usr_sbin}
        usr.sbin/pbsd.userland.${_ul_usr_sbin}.cppm)
endforeach()

foreach(_ul_sbin8 IN ITEMS gpart mdconfig dumpon savecore fsck_ffs fsck_msdosfs newfs_msdos tunefs growfs dmesg dumpfs fdisk bsdlabel swapon shutdown reboot kldload)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_sbin8}
        sbin/pbsd.userland.${_ul_sbin8}.cppm)
endforeach()

pbsd_userland_add_library(pbsd_userland_rsync)
target_sources(pbsd_userland_rsync PUBLIC FILE_SET CXX_MODULES FILES
    usr.bin/pbsd.userland.rsync.cppm)
target_link_libraries(pbsd_userland_rsync PUBLIC pbsd_userland_hosted)
pbsd_userland_module(pbsd_userland_rsync)

foreach(_ul_b9 IN ITEMS col rs units make yacc lex apply chpass passwd login su sudo)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b9}
        usr.bin/pbsd.userland.${_ul_b9}.cppm)
endforeach()

# --- burst 8: helpers + compression + network + sysadmin (38 modules) ---------

set(PBSD_UL_B8_TARGETS
    pbsd_userland_find_helpers
    pbsd_userland_grep_context
    pbsd_userland_sed_addr
    pbsd_userland_awk_fields
    pbsd_userland_xargs_quote
    pbsd_userland_bzip2
    pbsd_userland_compress
    pbsd_userland_cpio
    pbsd_userland_xz
    pbsd_userland_curl
    pbsd_userland_scp
    pbsd_userland_ftp
    pbsd_userland_file
    pbsd_userland_hexdump
    pbsd_userland_stat
    pbsd_userland_strings
    pbsd_userland_cksum
    pbsd_userland_netstat
    pbsd_userland_sockstat
    pbsd_userland_vmstat
    pbsd_userland_who
    pbsd_userland_w_cmd
    pbsd_userland_ldd
    pbsd_userland_iconv
    pbsd_userland_periodic
    pbsd_userland_syslogd
    pbsd_userland_rcorder
    pbsd_userland_bectl
    pbsd_userland_zfs
    pbsd_userland_zpool
    pbsd_userland_md5
    pbsd_userland_fsck
    pbsd_userland_geom
    pbsd_userland_newfs
    pbsd_userland_camcontrol)

set(PBSD_UL_B8_FILES
    usr.bin/pbsd.userland.find.helpers.cppm
    usr.bin/pbsd.userland.grep.context.cppm
    usr.bin/pbsd.userland.sed.addr.cppm
    usr.bin/pbsd.userland.awk.fields.cppm
    usr.bin/pbsd.userland.xargs.quote.cppm
    usr.bin/pbsd.userland.bzip2.cppm
    usr.bin/pbsd.userland.compress.cppm
    usr.bin/pbsd.userland.cpio.cppm
    usr.bin/pbsd.userland.xz.cppm
    usr.bin/pbsd.userland.curl.cppm
    usr.bin/pbsd.userland.scp.cppm
    usr.bin/pbsd.userland.ftp.cppm
    usr.bin/pbsd.userland.file.cppm
    usr.bin/pbsd.userland.hexdump.cppm
    usr.bin/pbsd.userland.stat.cppm
    usr.bin/pbsd.userland.strings.cppm
    usr.bin/pbsd.userland.cksum.cppm
    usr.bin/pbsd.userland.netstat.cppm
    usr.bin/pbsd.userland.sockstat.cppm
    usr.bin/pbsd.userland.vmstat.cppm
    usr.bin/pbsd.userland.who.cppm
    usr.bin/pbsd.userland.w_cmd.cppm
    usr.bin/pbsd.userland.ldd.cppm
    usr.bin/pbsd.userland.iconv.cppm
    usr.sbin/pbsd.userland.periodic.cppm
    usr.sbin/pbsd.userland.syslogd.cppm
    sbin/pbsd.userland.rcorder.cppm
    sbin/pbsd.userland.bectl.cppm
    sbin/pbsd.userland.zfs.cppm
    sbin/pbsd.userland.zpool.cppm
    sbin/pbsd.userland.md5.cppm
    sbin/pbsd.userland.fsck.cppm
    sbin/pbsd.userland.geom.cppm
    sbin/pbsd.userland.newfs.cppm
    sbin/pbsd.userland.camcontrol.cppm)

list(LENGTH PBSD_UL_B8_TARGETS _b8_len)
math(EXPR _b8_last "${_b8_len} - 1")
foreach(i RANGE ${_b8_last})
    list(GET PBSD_UL_B8_TARGETS ${i} _tgt)
    list(GET PBSD_UL_B8_FILES ${i} _file)
    pbsd_userland_hosted_tool(${_tgt} ${_file})
endforeach()

# --- burst 9b: libc/msun/libthr + bin/sbin/usr.bin tools (61 modules) ---------
foreach(_ul_b9b_bin IN ITEMS ls rm timeout ps pwait cpuset pkill chio ed stty)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b9b_bin} bin/pbsd.userland.${_ul_b9b_bin}.cppm)
endforeach()

foreach(_ul_b9b_sbin IN ITEMS mknod kldstat kldunload ldconfig nfsiod restore pfctl)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b9b_sbin} sbin/pbsd.userland.${_ul_b9b_sbin}.cppm)
endforeach()

foreach(_ul_b9b_usr IN ITEMS fmt banner ar size nm look uuencode uudecode what whereis wall talk colrm factor mesg pr tty ul)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b9b_usr} usr.bin/pbsd.userland.${_ul_b9b_usr}.cppm)
endforeach()

# --- burst 10: usr.bin gaps (unexpand, nohup, pathchk, checksum helpers, …) ---
foreach(_ul_b10 IN ITEMS unexpand nohup pathchk logname nice renice primes mkfifo stdbuf uncompress sha256 sum egrep caesar fsync)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b10} usr.bin/pbsd.userland.${_ul_b10}.cppm)
endforeach()

# --- burst 11: usr.bin/bin gaps (tabs, csplit, expr, pax, …) -----------------
foreach(_ul_b11_usr IN ITEMS tabs csplit sdiff diff3 getconf perror number leave unvis grdc write whois soelim users pagesize pom)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b11_usr} usr.bin/pbsd.userland.${_ul_b11_usr}.cppm)
endforeach()

foreach(_ul_b11_bin IN ITEMS expr pax freebsd_version)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b11_bin} bin/pbsd.userland.${_ul_b11_bin}.cppm)
endforeach()

# --- burst 11 extension: additional usr.bin/bin + gaps -----------------------
foreach(_ul_b11x_usr IN ITEMS ncal indent from base64 clear lorder fortune spell vis tset)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b11x_usr} usr.bin/pbsd.userland.${_ul_b11x_usr}.cppm)
endforeach()

foreach(_ul_b11x_bin IN ITEMS umask)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b11x_bin} bin/pbsd.userland.${_ul_b11x_bin}.cppm)
endforeach()


# --- burst 12: libc/msun/libthr + usr.bin gaps ------------------------------
foreach(_ul_b12 IN ITEMS biff beep morse killall enigma ipcrm ipcs getent showmount finger vacation unifdef at ministat asa lam lockf random lsvfs brandelf msgs lock)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b12} usr.bin/pbsd.userland.${_ul_b12}.cppm)
endforeach()

# --- burst 13: libc/libthr gaps + usr.bin/bin utilities ----------------------
foreach(_ul_b13_usr IN ITEMS host last locale revoke limits logins fstat ident mkdep rpcinfo rwall rwho rup ruptime rusers lastcomm runat quota rctl newgrp readelf gencat mkstr)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b13_usr} usr.bin/pbsd.userland.${_ul_b13_usr}.cppm)
endforeach()

foreach(_ul_b13_bin IN ITEMS rmail)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b13_bin} bin/pbsd.userland.${_ul_b13_bin}.cppm)
endforeach()

# --- burst 13b: more usr.bin utilities ---------------------------------------
foreach(_ul_b13b IN ITEMS backlight bintrans c89 c99 cap_mkdb chat ctags ctlstat dpv elfctl elfdump file2c gcore genl getaddrinfo gprof hesinfo kdump ktrace localedef locate mkimg mkuzip mdo mididump etdump)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b13b} usr.bin/pbsd.userland.${_ul_b13b}.cppm)
endforeach()

# --- burst 14: libc gen/sys/net + msun float + usr.bin -----------------------
foreach(_ul_b14 IN ITEMS bsdiff iscsictl ktrdump mt nfsstat posixmqcontrol posixshmcontrol proccontrol procstat protect resizewin sdiotool tftp tip top usbhidaction vtfontcvt xinstall xstr ypcat ypmatch ypwhich)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b14} usr.bin/pbsd.userland.${_ul_b14}.cppm)
endforeach()

# --- burst 15 wave 2: usr.bin utility gaps -----------------------------------
foreach(_ul_b15 IN ITEMS addr2line alias bluetooth bmake bsdcat bsddialog bzip2recover clang compile_et cxxfilt drill dtc ee elfcopy gh_bc kyua ldd32 less lessecho lesskey lint lzmainfo mail man mandoc mkcsmapper mkcsmapper_static mkesdb mkesdb_static nc ofed pamtest rpcgen smbutil ssh_copy_id systat tcopy telnet truss ts unzip usbhidctl vgrind vi wg xo xohtml xolint xopo xzdec zstd)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b15} usr.bin/pbsd.userland.${_ul_b15}.cppm)
endforeach()

# --- burst 15 wave 3: libc sys + msun + libthr gaps ---------------------------
# (wired via FILE_SET above)

# --- burst 15 bin wave 2: bin utility gaps -----------------------------------
foreach(_ul_b15_bin IN ITEMS csh sh)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b15_bin} bin/pbsd.userland.${_ul_b15_bin}.cppm)
endforeach()

# --- burst 16: usr.sbin + sbin utility gaps ----------------------------------
foreach(_ul_b16_usr_sbin IN ITEMS ac accton acpi adduser apm arp audit auditd authpf autofs bhyve chroot devctl devinfo diskinfo gstat iostat jexec jls ndp pciconf pstat tcpdump traceroute usbconfig zic zdump)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b16_usr_sbin}
        usr.sbin/pbsd.userland.${_ul_b16_usr_sbin}.cppm)
endforeach()

foreach(_ul_b16_sbin IN ITEMS adjkerntz dhclient dump ffsinfo fsdb ipfw kldconfig nvmecontrol quotacheck resolvconf routed setkey)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b16_sbin}
        sbin/pbsd.userland.${_ul_b16_sbin}.cppm)
endforeach()

# --- burst 16 wave 2: usr.bin utility sub-modules ---------------------------
foreach(_ul_b16 IN ITEMS ar.read ar.util ar.write at.panic at.parsetime at.perm bintrans.qp bintrans.uudecode bintrans.uuencode bluetooth.bthost.bthost bluetooth.btsockstat.btsockstat bluetooth.rfcomm_sppd.rfcomm_sdp bluetooth.rfcomm_sppd.rfcomm_sppd bsdiff.bsdiff.bsdiff bsdiff.bspatch.bspatch calendar.dates calendar.day calendar.events calendar.io calendar.locale calendar.ostern calendar.parsedata calendar.paskha calendar.pom calendar.sunpos chpass.edit chpass.field chpass.table chpass.util cksum.crc cksum.crc32 cksum.print cksum.sum1 cksum.sum2 clang.clang.clang_driver clang.clang_scan_deps.clang_scan_deps_driver clang.lld.lld_driver clang.llvm_ar.llvm_ar_driver clang.llvm_cxxfilt.llvm_cxxfilt_driver clang.llvm_dwp.llvm_dwp_driver clang.llvm_nm.llvm_nm_driver clang.llvm_objcopy.llvm_objcopy_driver clang.llvm_objdump.llvm_objdump_driver clang.llvm_profdata.llvm_profdata_driver clang.llvm_readobj.llvm_readobj_driver clang.llvm_size.llvm_size_driver clang.llvm_symbolizer.llvm_symbolizer_driver cmp.link cmp.misc cmp.regular cmp.special compress.zopen ctags.C ctags.fortran ctags.lisp ctags.print ctags.tree ctags.yacc diff.diffdir diff.diffreg diff.diffreg_new diff.pr diff.xmalloc dtc.checking dtc.dtb dtc.fdt dtc.input_buffer dtc.string env.envopts etdump.output_shell etdump.output_text find.function find.ls find.main find.misc find.operator_ find.option find.printf finger.lprint finger.net finger.sprint finger.util fortune.fortune.fortune fortune.strfile.strfile fortune.unstr.unstr fstat.fuser fstat.main gcore.elf32core gcore.elfcore genl.parser_rpc getconf.tests.arch_type gprof.arcs gprof.dfn gprof.elf gprof.hertz gprof.kernel gprof.lookup gprof.printgprof gprof.printlist grep.file grep.queue grep.util gzip.unbzip2 gzip.unlz gzip.unpack gzip.unxz gzip.unzstd gzip.zuncompress hexdump.conv hexdump.display hexdump.hexsyntax hexdump.odsyntax hexdump.parse indent.args indent.io indent.lexi indent.parse indent.pr_comment ipcs.ipc iscsictl.periphs kdump.linux ktrace.subr lastcomm.readrec lex.initparse lex.initscan lex.initskel localedef.charmap localedef.collate localedef.ctype localedef.messages localedef.monetary localedef.numeric localedef.scanner localedef.time localedef.wide locate.bigram.locate.bigram locate.code.locate.code locate.locate.fastfind locate.locate.locate locate.locate.util login.login_audit login.login_fbtab m4.eval m4.expr m4.gnum4 m4.look m4.main m4.misc m4.trace mail.cmd1 mail.cmd2 mail.cmd3 mail.cmdtab mail.collect mail.edit mail.fio mail.getname mail.head mail.lex mail.list mail.main mail.names mail.popen mail.quit mail.send mail.strings mail.temp mail.tests.mailx_signal_test mail.tty mail.util mail.v7.local mail.vars mail.version mkimg.apm mkimg.bsd mkimg.ebr mkimg.format mkimg.gpt mkimg.image mkimg.mbr mkimg.qcow mkimg.raw mkimg.scheme mkimg.uuid mkimg.vhd mkimg.vhdx mkimg.vmdk mkuzip.mkuz_blk mkuzip.mkuz_blockcache mkuzip.mkuz_conveyor mkuzip.mkuz_fqueue mkuzip.mkuz_insize mkuzip.mkuz_lzma mkuzip.mkuz_time mkuzip.mkuz_zlib mkuzip.mkuz_zstd netstat.bpf netstat.common netstat.if_ netstat.inet netstat.inet6 netstat.ipsec netstat.main netstat.mbuf netstat.mroute netstat.mroute6 netstat.netgraph netstat.netisr netstat.nhgrp netstat.nhops netstat.pfkey netstat.route netstat.route_netlink netstat.sctp netstat.unix patch.backupfile patch.inp patch.mkpath patch.pch patch.util pr.egetopt primes.pattern primes.pr_tbl primes.spsp procstat.procstat_advlock procstat.procstat_args procstat.procstat_auxv procstat.procstat_basic procstat.procstat_bin procstat.procstat_cred procstat.procstat_cs procstat.procstat_files procstat.procstat_kqueue procstat.procstat_kstack procstat.procstat_penv procstat.procstat_ptlwpinfo procstat.procstat_pwdx procstat.procstat_rlimit procstat.procstat_rlimitusage procstat.procstat_rusage procstat.procstat_sigs procstat.procstat_threads procstat.procstat_vm procstat.tests.while1 random.randomize_fd rpcgen.rpc_clntout rpcgen.rpc_cout rpcgen.rpc_hout rpcgen.rpc_main rpcgen.rpc_parse rpcgen.rpc_sample rpcgen.rpc_scan rpcgen.rpc_svcout rpcgen.rpc_tblout rpcgen.rpc_util sdiff.edit sdiotool.cam_sdio sdiotool.linux_sdio_compat sed.compile sed.main sed.misc sed.process sockstat.main sockstat.tests.sockstat_test sort.bwstring sort.coll sort.file sort.mem sort.radixsort sort.vsort systat.cmds systat.cmdtab systat.convtbl systat.devs systat.fetch systat.icmp systat.icmp6 systat.ifcmds systat.ifstat systat.iolat systat.iostat systat.ip systat.ip6 systat.keyboard systat.main systat.mode systat.netcmds systat.netstat systat.pigs systat.proc systat.sctp systat.swap systat.sysput systat.tcp systat.vmstat systat.zarc tail.forward tail.misc tail.read tail.reverse talk.ctl talk.ctl_transact talk.display talk.get_addrs talk.get_iface talk.get_names talk.init_disp talk.invite talk.io talk.look_up talk.msgs tftp.main tip.libacu.biz22 tip.libacu.biz31 tip.libacu.courier tip.libacu.df tip.libacu.dn11 tip.libacu.hayes tip.libacu.t3000 tip.libacu.v3451 tip.libacu.v831 tip.libacu.ventel tip.tip.acu tip.tip.acutab tip.tip.cmds tip.tip.cmdtab tip.tip.cu tip.tip.hunt tip.tip.log tip.tip.partab tip.tip.remote tip.tip.tip tip.tip.tipout tip.tip.uucplock tip.tip.value tip.tip.vars top.commands top.display top.machine top.screen top.username top.utils tr.cmap tr.cset tr.str truss.main truss.setup truss.syscalls usbhidctl.usbhid vgrind.regexp vgrind.vfontedpr w.pr_time w.proc_compare wall.ttymsg xargs.strnsubst)
    string(REPLACE "." "_" _ul_b16_tgt "${_ul_b16}")
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b16_tgt} usr.bin/pbsd.userland.${_ul_b16}.cppm)
endforeach()

# --- burst 16 wave 2: bin utility sub-modules ---------------------------------
foreach(_ul_b16_bin IN ITEMS cp.utils csh.iconv_stub date.vary dd.args dd.conv dd.conv_tab dd.gen dd.misc dd.position ed.buf ed.glbl ed.io ed.main ed.re ed.sub ed.undo ls.print ls.util pax.ar_io pax.ar_subs pax.buf_subs pax.cache pax.cpio pax.file_subs pax.ftree pax.gen_subs pax.getoldopt pax.options pax.pat_rep pax.sel_subs pax.tables pax.tar pax.tty_subs pkill.tests.spin_helper ps.fmt ps.keyword ps.nlist ps.print setfacl.file setfacl.mask setfacl.merge setfacl.remove setfacl.util sh.alias sh.arith_yacc sh.arith_yylex sh.bltin.echo sh.cd sh.error sh.eval sh.exec sh.expand sh.histedit sh.input sh.jobs sh.mail sh.main sh.memalloc sh.miscbltin sh.mknodes sh.mksyntax sh.mystring sh.options sh.output sh.parser sh.redir sh.show sh.trap sh.var stty.cchar stty.gfmt stty.key stty.modes stty.print)
    string(REPLACE "." "_" _ul_b16_bin_tgt "${_ul_b16_bin}")
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b16_bin_tgt} bin/pbsd.userland.${_ul_b16_bin}.cppm)
endforeach()

# --- burst 16 userland: usr.bin utility gaps ---------------------------------
foreach(_ul_b16_usr IN ITEMS ncurses usr_bin_tar)
    pbsd_userland_hosted_tool(pbsd_userland_${_ul_b16_usr} usr.bin/pbsd.userland.${_ul_b16_usr}.cppm)
endforeach()

if(NOT TARGET pbsd_userland_bin)
add_library(pbsd_userland_bin INTERFACE)
target_link_libraries(pbsd_userland_bin INTERFACE
    pbsd_userland_echo
    pbsd_userland_hostname
    pbsd_userland_domainname
    pbsd_userland_nproc
    pbsd_userland_pwd
    pbsd_userland_cat
    pbsd_userland_chmod
    pbsd_userland_mkdir
    pbsd_userland_rmdir
    pbsd_userland_ln
    pbsd_userland_mv
    pbsd_userland_cp
    pbsd_userland_kill
    pbsd_userland_sleep
    pbsd_userland_true
    pbsd_userland_false
    pbsd_userland_test
    pbsd_userland_date
    pbsd_userland_chflags
    pbsd_userland_dd
    pbsd_userland_getfacl
    pbsd_userland_setfacl
    pbsd_userland_sync
    pbsd_userland_uuidgen
    pbsd_userland_realpath
    pbsd_userland_kenv
    pbsd_userland_ls_cmp
    pbsd_userland_stty_util
    pbsd_userland_tar
    pbsd_userland_ls
    pbsd_userland_ps
    pbsd_userland_rm
    pbsd_userland_timeout
    pbsd_userland_stty
    pbsd_userland_pkill
    pbsd_userland_pwait
    pbsd_userland_cpuset
    pbsd_userland_chio
    pbsd_userland_ed
    pbsd_userland_expr
    pbsd_userland_pax
    pbsd_userland_freebsd_version
    pbsd_userland_umask
    pbsd_userland_rmail
    pbsd_userland_csh
    pbsd_userland_sh
    pbsd_userland_cp_utils
    pbsd_userland_csh_iconv_stub
    pbsd_userland_date_vary
    pbsd_userland_dd_args
    pbsd_userland_dd_conv
    pbsd_userland_dd_conv_tab
    pbsd_userland_dd_gen
    pbsd_userland_dd_misc
    pbsd_userland_dd_position
    pbsd_userland_ed_buf
    pbsd_userland_ed_glbl
    pbsd_userland_ed_io
    pbsd_userland_ed_main
    pbsd_userland_ed_re
    pbsd_userland_ed_sub
    pbsd_userland_ed_undo
    pbsd_userland_ls_print
    pbsd_userland_ls_util
    pbsd_userland_pax_ar_io
    pbsd_userland_pax_ar_subs
    pbsd_userland_pax_buf_subs
    pbsd_userland_pax_cache
    pbsd_userland_pax_cpio
    pbsd_userland_pax_file_subs
    pbsd_userland_pax_ftree
    pbsd_userland_pax_gen_subs
    pbsd_userland_pax_getoldopt
    pbsd_userland_pax_options
    pbsd_userland_pax_pat_rep
    pbsd_userland_pax_sel_subs
    pbsd_userland_pax_tables
    pbsd_userland_pax_tar
    pbsd_userland_pax_tty_subs
    pbsd_userland_pkill_tests_spin_helper
    pbsd_userland_ps_fmt
    pbsd_userland_ps_keyword
    pbsd_userland_ps_nlist
    pbsd_userland_ps_print
    pbsd_userland_setfacl_file
    pbsd_userland_setfacl_mask
    pbsd_userland_setfacl_merge
    pbsd_userland_setfacl_remove
    pbsd_userland_setfacl_util
    pbsd_userland_sh_alias
    pbsd_userland_sh_arith_yacc
    pbsd_userland_sh_arith_yylex
    pbsd_userland_sh_bltin_echo
    pbsd_userland_sh_cd
    pbsd_userland_sh_error
    pbsd_userland_sh_eval
    pbsd_userland_sh_exec
    pbsd_userland_sh_expand
    pbsd_userland_sh_histedit
    pbsd_userland_sh_input
    pbsd_userland_sh_jobs
    pbsd_userland_sh_mail
    pbsd_userland_sh_main
    pbsd_userland_sh_memalloc
    pbsd_userland_sh_miscbltin
    pbsd_userland_sh_mknodes
    pbsd_userland_sh_mksyntax
    pbsd_userland_sh_mystring
    pbsd_userland_sh_options
    pbsd_userland_sh_output
    pbsd_userland_sh_parser
    pbsd_userland_sh_redir
    pbsd_userland_sh_show
    pbsd_userland_sh_trap
    pbsd_userland_sh_var
    pbsd_userland_stty_cchar
    pbsd_userland_stty_gfmt
    pbsd_userland_stty_key
    pbsd_userland_stty_modes
    pbsd_userland_stty_print)
endif()

if(NOT TARGET pbsd_userland_usr_bin)
add_library(pbsd_userland_usr_bin INTERFACE)
target_link_libraries(pbsd_userland_usr_bin INTERFACE
    pbsd_userland_yes
    pbsd_userland_usr_bin_true
    pbsd_userland_usr_bin_false
    pbsd_userland_getopt
    pbsd_userland_basename
    pbsd_userland_dirname
    pbsd_userland_uname
    pbsd_userland_printenv
    pbsd_userland_env
    pbsd_userland_which
    pbsd_userland_time_cmd
    pbsd_userland_touch
    pbsd_userland_truncate
    pbsd_userland_id
    pbsd_userland_head
    pbsd_userland_tail
    pbsd_userland_wc
    pbsd_userland_tee
    pbsd_userland_mktemp
    pbsd_userland_printf
    pbsd_userland_tr
    pbsd_userland_cut
    pbsd_userland_sort
    pbsd_userland_uniq
    pbsd_userland_comm
    pbsd_userland_cmp
    pbsd_userland_diff
    pbsd_userland_seq
    pbsd_userland_jot
    pbsd_userland_calendar
    pbsd_userland_logger
    pbsd_userland_script
    pbsd_userland_join
    pbsd_userland_paste
    pbsd_userland_nl
    pbsd_userland_fold
    pbsd_userland_split
    pbsd_userland_expand
    pbsd_userland_tsort
    pbsd_userland_rev
    pbsd_userland_du
    pbsd_userland_find
    pbsd_userland_xargs
    pbsd_userland_grep
    pbsd_userland_sed
    pbsd_userland_awk
    pbsd_userland_patch
    pbsd_userland_gzip
    pbsd_userland_ssh
    pbsd_userland_fetch
    pbsd_userland_od
    pbsd_userland_bc
    pbsd_userland_find_helpers
    pbsd_userland_grep_context
    pbsd_userland_sed_addr
    pbsd_userland_awk_fields
    pbsd_userland_xargs_quote
    pbsd_userland_bzip2
    pbsd_userland_compress
    pbsd_userland_cpio
    pbsd_userland_xz
    pbsd_userland_curl
    pbsd_userland_scp
    pbsd_userland_ftp
    pbsd_userland_file
    pbsd_userland_hexdump
    pbsd_userland_stat
    pbsd_userland_strings
    pbsd_userland_cksum
    pbsd_userland_netstat
    pbsd_userland_sockstat
    pbsd_userland_vmstat
    pbsd_userland_who
    pbsd_userland_w_cmd
    pbsd_userland_ldd
    pbsd_userland_iconv
    pbsd_userland_rsync
    pbsd_userland_col
    pbsd_userland_column
    pbsd_userland_rs
    pbsd_userland_units
    pbsd_userland_dc
    pbsd_userland_make
    pbsd_userland_yacc
    pbsd_userland_lex
    pbsd_userland_m4
    pbsd_userland_apply
    pbsd_userland_chpass
    pbsd_userland_passwd
    pbsd_userland_login
    pbsd_userland_su
    pbsd_userland_sudo
    pbsd_userland_fmt
    pbsd_userland_look
    pbsd_userland_mesg
    pbsd_userland_pr
    pbsd_userland_tty
    pbsd_userland_ul
    pbsd_userland_wall
    pbsd_userland_banner
    pbsd_userland_ar
    pbsd_userland_size
    pbsd_userland_nm
    pbsd_userland_uuencode
    pbsd_userland_uudecode
    pbsd_userland_what
    pbsd_userland_whereis
    pbsd_userland_talk
    pbsd_userland_colrm
    pbsd_userland_factor
    pbsd_userland_unexpand
    pbsd_userland_nohup
    pbsd_userland_pathchk
    pbsd_userland_logname
    pbsd_userland_nice
    pbsd_userland_renice
    pbsd_userland_primes
    pbsd_userland_mkfifo
    pbsd_userland_stdbuf
    pbsd_userland_uncompress
    pbsd_userland_sha256
    pbsd_userland_sum
    pbsd_userland_egrep
    pbsd_userland_caesar
    pbsd_userland_fsync
    pbsd_userland_tabs
    pbsd_userland_csplit
    pbsd_userland_sdiff
    pbsd_userland_diff3
    pbsd_userland_getconf
    pbsd_userland_perror
    pbsd_userland_number
    pbsd_userland_leave
    pbsd_userland_unvis
    pbsd_userland_grdc
    pbsd_userland_write
    pbsd_userland_whois
    pbsd_userland_soelim
    pbsd_userland_users
    pbsd_userland_pagesize
    pbsd_userland_pom
    pbsd_userland_ncal
    pbsd_userland_indent
    pbsd_userland_from
    pbsd_userland_base64
    pbsd_userland_clear
    pbsd_userland_lorder
    pbsd_userland_fortune
    pbsd_userland_spell
    pbsd_userland_vis
    pbsd_userland_biff
    pbsd_userland_beep
    pbsd_userland_morse
    pbsd_userland_killall
    pbsd_userland_enigma
    pbsd_userland_ipcrm
    pbsd_userland_ipcs
    pbsd_userland_getent
    pbsd_userland_showmount
    pbsd_userland_finger
    pbsd_userland_vacation
    pbsd_userland_unifdef
    pbsd_userland_at
    pbsd_userland_ministat
    pbsd_userland_asa
    pbsd_userland_lam
    pbsd_userland_lockf
    pbsd_userland_random
    pbsd_userland_lsvfs
    pbsd_userland_brandelf
    pbsd_userland_msgs
    pbsd_userland_lock
    pbsd_userland_tset
    pbsd_userland_host
    pbsd_userland_last
    pbsd_userland_locale
    pbsd_userland_revoke
    pbsd_userland_limits
    pbsd_userland_logins
    pbsd_userland_fstat
    pbsd_userland_ident
    pbsd_userland_mkdep
    pbsd_userland_rpcinfo
    pbsd_userland_rwall
    pbsd_userland_rwho
    pbsd_userland_rup
    pbsd_userland_ruptime
    pbsd_userland_rusers
    pbsd_userland_lastcomm
    pbsd_userland_runat
    pbsd_userland_quota
    pbsd_userland_rctl
    pbsd_userland_newgrp
    pbsd_userland_readelf
    pbsd_userland_gencat
    pbsd_userland_mkstr
    pbsd_userland_backlight
    pbsd_userland_bintrans
    pbsd_userland_c89
    pbsd_userland_c99
    pbsd_userland_cap_mkdb
    pbsd_userland_chat
    pbsd_userland_ctags
    pbsd_userland_ctlstat
    pbsd_userland_dpv
    pbsd_userland_elfctl
    pbsd_userland_elfdump
    pbsd_userland_file2c
    pbsd_userland_gcore
    pbsd_userland_genl
    pbsd_userland_getaddrinfo
    pbsd_userland_gprof
    pbsd_userland_hesinfo
    pbsd_userland_kdump
    pbsd_userland_ktrace
    pbsd_userland_localedef
    pbsd_userland_locate
    pbsd_userland_mkimg
    pbsd_userland_mkuzip
    pbsd_userland_mdo
    pbsd_userland_mididump
    pbsd_userland_etdump
    pbsd_userland_bsdiff
    pbsd_userland_iscsictl
    pbsd_userland_ktrdump
    pbsd_userland_mt
    pbsd_userland_nfsstat
    pbsd_userland_posixmqcontrol
    pbsd_userland_posixshmcontrol
    pbsd_userland_proccontrol
    pbsd_userland_procstat
    pbsd_userland_protect
    pbsd_userland_resizewin
    pbsd_userland_sdiotool
    pbsd_userland_tftp
    pbsd_userland_tip
    pbsd_userland_top
    pbsd_userland_usbhidaction
    pbsd_userland_vtfontcvt
    pbsd_userland_xinstall
    pbsd_userland_xstr
    pbsd_userland_ypcat
    pbsd_userland_ypmatch
    pbsd_userland_ypwhich
    pbsd_userland_addr2line
    pbsd_userland_alias
    pbsd_userland_bluetooth
    pbsd_userland_bmake
    pbsd_userland_bsdcat
    pbsd_userland_bsddialog
    pbsd_userland_bzip2recover
    pbsd_userland_clang
    pbsd_userland_compile_et
    pbsd_userland_cxxfilt
    pbsd_userland_drill
    pbsd_userland_dtc
    pbsd_userland_ee
    pbsd_userland_elfcopy
    pbsd_userland_gh_bc
    pbsd_userland_kyua
    pbsd_userland_ldd32
    pbsd_userland_less
    pbsd_userland_lessecho
    pbsd_userland_lesskey
    pbsd_userland_lint
    pbsd_userland_lzmainfo
    pbsd_userland_mail
    pbsd_userland_man
    pbsd_userland_mandoc
    pbsd_userland_mkcsmapper
    pbsd_userland_mkcsmapper_static
    pbsd_userland_mkesdb
    pbsd_userland_mkesdb_static
    pbsd_userland_nc
    pbsd_userland_ofed
    pbsd_userland_pamtest
    pbsd_userland_rpcgen
    pbsd_userland_smbutil
    pbsd_userland_ssh_copy_id
    pbsd_userland_systat
    pbsd_userland_tcopy
    pbsd_userland_telnet
    pbsd_userland_truss
    pbsd_userland_ts
    pbsd_userland_unzip
    pbsd_userland_usbhidctl
    pbsd_userland_vgrind
    pbsd_userland_vi
    pbsd_userland_wg
    pbsd_userland_xo
    pbsd_userland_xohtml
    pbsd_userland_xolint
    pbsd_userland_xopo
    pbsd_userland_xzdec
    pbsd_userland_zstd
    pbsd_userland_ar_read
    pbsd_userland_ar_util
    pbsd_userland_ar_write
    pbsd_userland_at_panic
    pbsd_userland_at_parsetime
    pbsd_userland_at_perm
    pbsd_userland_bintrans_qp
    pbsd_userland_bintrans_uudecode
    pbsd_userland_bintrans_uuencode
    pbsd_userland_bluetooth_bthost_bthost
    pbsd_userland_bluetooth_btsockstat_btsockstat
    pbsd_userland_bluetooth_rfcomm_sppd_rfcomm_sdp
    pbsd_userland_bluetooth_rfcomm_sppd_rfcomm_sppd
    pbsd_userland_bsdiff_bsdiff_bsdiff
    pbsd_userland_bsdiff_bspatch_bspatch
    pbsd_userland_calendar_dates
    pbsd_userland_calendar_day
    pbsd_userland_calendar_events
    pbsd_userland_calendar_io
    pbsd_userland_calendar_locale
    pbsd_userland_calendar_ostern
    pbsd_userland_calendar_parsedata
    pbsd_userland_calendar_paskha
    pbsd_userland_calendar_pom
    pbsd_userland_calendar_sunpos
    pbsd_userland_chpass_edit
    pbsd_userland_chpass_field
    pbsd_userland_chpass_table
    pbsd_userland_chpass_util
    pbsd_userland_cksum_crc
    pbsd_userland_cksum_crc32
    pbsd_userland_cksum_print
    pbsd_userland_cksum_sum1
    pbsd_userland_cksum_sum2
    pbsd_userland_clang_clang_clang_driver
    pbsd_userland_clang_clang_scan_deps_clang_scan_deps_driver
    pbsd_userland_clang_lld_lld_driver
    pbsd_userland_clang_llvm_ar_llvm_ar_driver
    pbsd_userland_clang_llvm_cxxfilt_llvm_cxxfilt_driver
    pbsd_userland_clang_llvm_dwp_llvm_dwp_driver
    pbsd_userland_clang_llvm_nm_llvm_nm_driver
    pbsd_userland_clang_llvm_objcopy_llvm_objcopy_driver
    pbsd_userland_clang_llvm_objdump_llvm_objdump_driver
    pbsd_userland_clang_llvm_profdata_llvm_profdata_driver
    pbsd_userland_clang_llvm_readobj_llvm_readobj_driver
    pbsd_userland_clang_llvm_size_llvm_size_driver
    pbsd_userland_clang_llvm_symbolizer_llvm_symbolizer_driver
    pbsd_userland_cmp_link
    pbsd_userland_cmp_misc
    pbsd_userland_cmp_regular
    pbsd_userland_cmp_special
    pbsd_userland_compress_zopen
    pbsd_userland_ctags_C
    pbsd_userland_ctags_fortran
    pbsd_userland_ctags_lisp
    pbsd_userland_ctags_print
    pbsd_userland_ctags_tree
    pbsd_userland_ctags_yacc
    pbsd_userland_diff_diffdir
    pbsd_userland_diff_diffreg
    pbsd_userland_diff_diffreg_new
    pbsd_userland_diff_pr
    pbsd_userland_diff_xmalloc
    pbsd_userland_dtc_checking
    pbsd_userland_dtc_dtb
    pbsd_userland_dtc_fdt
    pbsd_userland_dtc_input_buffer
    pbsd_userland_dtc_string
    pbsd_userland_env_envopts
    pbsd_userland_etdump_output_shell
    pbsd_userland_etdump_output_text
    pbsd_userland_find_function
    pbsd_userland_find_ls
    pbsd_userland_find_main
    pbsd_userland_find_misc
    pbsd_userland_find_operator_
    pbsd_userland_find_option
    pbsd_userland_find_printf
    pbsd_userland_finger_lprint
    pbsd_userland_finger_net
    pbsd_userland_finger_sprint
    pbsd_userland_finger_util
    pbsd_userland_fortune_fortune_fortune
    pbsd_userland_fortune_strfile_strfile
    pbsd_userland_fortune_unstr_unstr
    pbsd_userland_fstat_fuser
    pbsd_userland_fstat_main
    pbsd_userland_gcore_elf32core
    pbsd_userland_gcore_elfcore
    pbsd_userland_genl_parser_rpc
    pbsd_userland_getconf_tests_arch_type
    pbsd_userland_gprof_arcs
    pbsd_userland_gprof_dfn
    pbsd_userland_gprof_elf
    pbsd_userland_gprof_hertz
    pbsd_userland_gprof_kernel
    pbsd_userland_gprof_lookup
    pbsd_userland_gprof_printgprof
    pbsd_userland_gprof_printlist
    pbsd_userland_grep_file
    pbsd_userland_grep_queue
    pbsd_userland_grep_util
    pbsd_userland_gzip_unbzip2
    pbsd_userland_gzip_unlz
    pbsd_userland_gzip_unpack
    pbsd_userland_gzip_unxz
    pbsd_userland_gzip_unzstd
    pbsd_userland_gzip_zuncompress
    pbsd_userland_hexdump_conv
    pbsd_userland_hexdump_display
    pbsd_userland_hexdump_hexsyntax
    pbsd_userland_hexdump_odsyntax
    pbsd_userland_hexdump_parse
    pbsd_userland_indent_args
    pbsd_userland_indent_io
    pbsd_userland_indent_lexi
    pbsd_userland_indent_parse
    pbsd_userland_indent_pr_comment
    pbsd_userland_ipcs_ipc
    pbsd_userland_iscsictl_periphs
    pbsd_userland_kdump_linux
    pbsd_userland_ktrace_subr
    pbsd_userland_lastcomm_readrec
    pbsd_userland_lex_initparse
    pbsd_userland_lex_initscan
    pbsd_userland_lex_initskel
    pbsd_userland_localedef_charmap
    pbsd_userland_localedef_collate
    pbsd_userland_localedef_ctype
    pbsd_userland_localedef_messages
    pbsd_userland_localedef_monetary
    pbsd_userland_localedef_numeric
    pbsd_userland_localedef_scanner
    pbsd_userland_localedef_time
    pbsd_userland_localedef_wide
    pbsd_userland_locate_bigram_locate_bigram
    pbsd_userland_locate_code_locate_code
    pbsd_userland_locate_locate_fastfind
    pbsd_userland_locate_locate_locate
    pbsd_userland_locate_locate_util
    pbsd_userland_login_login_audit
    pbsd_userland_login_login_fbtab
    pbsd_userland_m4_eval
    pbsd_userland_m4_expr
    pbsd_userland_m4_gnum4
    pbsd_userland_m4_look
    pbsd_userland_m4_main
    pbsd_userland_m4_misc
    pbsd_userland_m4_trace
    pbsd_userland_mail_cmd1
    pbsd_userland_mail_cmd2
    pbsd_userland_mail_cmd3
    pbsd_userland_mail_cmdtab
    pbsd_userland_mail_collect
    pbsd_userland_mail_edit
    pbsd_userland_mail_fio
    pbsd_userland_mail_getname
    pbsd_userland_mail_head
    pbsd_userland_mail_lex
    pbsd_userland_mail_list
    pbsd_userland_mail_main
    pbsd_userland_mail_names
    pbsd_userland_mail_popen
    pbsd_userland_mail_quit
    pbsd_userland_mail_send
    pbsd_userland_mail_strings
    pbsd_userland_mail_temp
    pbsd_userland_mail_tests_mailx_signal_test
    pbsd_userland_mail_tty
    pbsd_userland_mail_util
    pbsd_userland_mail_v7_local
    pbsd_userland_mail_vars
    pbsd_userland_mail_version
    pbsd_userland_mkimg_apm
    pbsd_userland_mkimg_bsd
    pbsd_userland_mkimg_ebr
    pbsd_userland_mkimg_format
    pbsd_userland_mkimg_gpt
    pbsd_userland_mkimg_image
    pbsd_userland_mkimg_mbr
    pbsd_userland_mkimg_qcow
    pbsd_userland_mkimg_raw
    pbsd_userland_mkimg_scheme
    pbsd_userland_mkimg_uuid
    pbsd_userland_mkimg_vhd
    pbsd_userland_mkimg_vhdx
    pbsd_userland_mkimg_vmdk
    pbsd_userland_mkuzip_mkuz_blk
    pbsd_userland_mkuzip_mkuz_blockcache
    pbsd_userland_mkuzip_mkuz_conveyor
    pbsd_userland_mkuzip_mkuz_fqueue
    pbsd_userland_mkuzip_mkuz_insize
    pbsd_userland_mkuzip_mkuz_lzma
    pbsd_userland_mkuzip_mkuz_time
    pbsd_userland_mkuzip_mkuz_zlib
    pbsd_userland_mkuzip_mkuz_zstd
    pbsd_userland_netstat_bpf
    pbsd_userland_netstat_common
    pbsd_userland_netstat_if_
    pbsd_userland_netstat_inet
    pbsd_userland_netstat_inet6
    pbsd_userland_netstat_ipsec
    pbsd_userland_netstat_main
    pbsd_userland_netstat_mbuf
    pbsd_userland_netstat_mroute
    pbsd_userland_netstat_mroute6
    pbsd_userland_netstat_netgraph
    pbsd_userland_netstat_netisr
    pbsd_userland_netstat_nhgrp
    pbsd_userland_netstat_nhops
    pbsd_userland_netstat_pfkey
    pbsd_userland_netstat_route
    pbsd_userland_netstat_route_netlink
    pbsd_userland_netstat_sctp
    pbsd_userland_netstat_unix
    pbsd_userland_patch_backupfile
    pbsd_userland_patch_inp
    pbsd_userland_patch_mkpath
    pbsd_userland_patch_pch
    pbsd_userland_patch_util
    pbsd_userland_pr_egetopt
    pbsd_userland_primes_pattern
    pbsd_userland_primes_pr_tbl
    pbsd_userland_primes_spsp
    pbsd_userland_procstat_procstat_advlock
    pbsd_userland_procstat_procstat_args
    pbsd_userland_procstat_procstat_auxv
    pbsd_userland_procstat_procstat_basic
    pbsd_userland_procstat_procstat_bin
    pbsd_userland_procstat_procstat_cred
    pbsd_userland_procstat_procstat_cs
    pbsd_userland_procstat_procstat_files
    pbsd_userland_procstat_procstat_kqueue
    pbsd_userland_procstat_procstat_kstack
    pbsd_userland_procstat_procstat_penv
    pbsd_userland_procstat_procstat_ptlwpinfo
    pbsd_userland_procstat_procstat_pwdx
    pbsd_userland_procstat_procstat_rlimit
    pbsd_userland_procstat_procstat_rlimitusage
    pbsd_userland_procstat_procstat_rusage
    pbsd_userland_procstat_procstat_sigs
    pbsd_userland_procstat_procstat_threads
    pbsd_userland_procstat_procstat_vm
    pbsd_userland_procstat_tests_while1
    pbsd_userland_random_randomize_fd
    pbsd_userland_rpcgen_rpc_clntout
    pbsd_userland_rpcgen_rpc_cout
    pbsd_userland_rpcgen_rpc_hout
    pbsd_userland_rpcgen_rpc_main
    pbsd_userland_rpcgen_rpc_parse
    pbsd_userland_rpcgen_rpc_sample
    pbsd_userland_rpcgen_rpc_scan
    pbsd_userland_rpcgen_rpc_svcout
    pbsd_userland_rpcgen_rpc_tblout
    pbsd_userland_rpcgen_rpc_util
    pbsd_userland_sdiff_edit
    pbsd_userland_sdiotool_cam_sdio
    pbsd_userland_sdiotool_linux_sdio_compat
    pbsd_userland_sed_compile
    pbsd_userland_sed_main
    pbsd_userland_sed_misc
    pbsd_userland_sed_process
    pbsd_userland_sockstat_main
    pbsd_userland_sockstat_tests_sockstat_test
    pbsd_userland_sort_bwstring
    pbsd_userland_sort_coll
    pbsd_userland_sort_file
    pbsd_userland_sort_mem
    pbsd_userland_sort_radixsort
    pbsd_userland_sort_vsort
    pbsd_userland_systat_cmds
    pbsd_userland_systat_cmdtab
    pbsd_userland_systat_convtbl
    pbsd_userland_systat_devs
    pbsd_userland_systat_fetch
    pbsd_userland_systat_icmp
    pbsd_userland_systat_icmp6
    pbsd_userland_systat_ifcmds
    pbsd_userland_systat_ifstat
    pbsd_userland_systat_iolat
    pbsd_userland_systat_iostat
    pbsd_userland_systat_ip
    pbsd_userland_systat_ip6
    pbsd_userland_systat_keyboard
    pbsd_userland_systat_main
    pbsd_userland_systat_mode
    pbsd_userland_systat_netcmds
    pbsd_userland_systat_netstat
    pbsd_userland_systat_pigs
    pbsd_userland_systat_proc
    pbsd_userland_systat_sctp
    pbsd_userland_systat_swap
    pbsd_userland_systat_sysput
    pbsd_userland_systat_tcp
    pbsd_userland_systat_vmstat
    pbsd_userland_systat_zarc
    pbsd_userland_tail_forward
    pbsd_userland_tail_misc
    pbsd_userland_tail_read
    pbsd_userland_tail_reverse
    pbsd_userland_talk_ctl
    pbsd_userland_talk_ctl_transact
    pbsd_userland_talk_display
    pbsd_userland_talk_get_addrs
    pbsd_userland_talk_get_iface
    pbsd_userland_talk_get_names
    pbsd_userland_talk_init_disp
    pbsd_userland_talk_invite
    pbsd_userland_talk_io
    pbsd_userland_talk_look_up
    pbsd_userland_talk_msgs
    pbsd_userland_tftp_main
    pbsd_userland_tip_libacu_biz22
    pbsd_userland_tip_libacu_biz31
    pbsd_userland_tip_libacu_courier
    pbsd_userland_tip_libacu_df
    pbsd_userland_tip_libacu_dn11
    pbsd_userland_tip_libacu_hayes
    pbsd_userland_tip_libacu_t3000
    pbsd_userland_tip_libacu_v3451
    pbsd_userland_tip_libacu_v831
    pbsd_userland_tip_libacu_ventel
    pbsd_userland_tip_tip_acu
    pbsd_userland_tip_tip_acutab
    pbsd_userland_tip_tip_cmds
    pbsd_userland_tip_tip_cmdtab
    pbsd_userland_tip_tip_cu
    pbsd_userland_tip_tip_hunt
    pbsd_userland_tip_tip_log
    pbsd_userland_tip_tip_partab
    pbsd_userland_tip_tip_remote
    pbsd_userland_tip_tip_tip
    pbsd_userland_tip_tip_tipout
    pbsd_userland_tip_tip_uucplock
    pbsd_userland_tip_tip_value
    pbsd_userland_tip_tip_vars
    pbsd_userland_top_commands
    pbsd_userland_top_display
    pbsd_userland_top_machine
    pbsd_userland_top_screen
    pbsd_userland_top_username
    pbsd_userland_top_utils
    pbsd_userland_tr_cmap
    pbsd_userland_tr_cset
    pbsd_userland_tr_str
    pbsd_userland_truss_main
    pbsd_userland_truss_setup
    pbsd_userland_truss_syscalls
    pbsd_userland_usbhidctl_usbhid
    pbsd_userland_vgrind_regexp
    pbsd_userland_vgrind_vfontedpr
    pbsd_userland_w_pr_time
    pbsd_userland_w_proc_compare
    pbsd_userland_wall_ttymsg
    pbsd_userland_xargs_strnsubst
    pbsd_userland_ncurses
    pbsd_userland_usr_bin_tar)
endif()

if(NOT TARGET pbsd_userland_usr_sbin)
add_library(pbsd_userland_usr_sbin INTERFACE)
target_link_libraries(pbsd_userland_usr_sbin INTERFACE
    pbsd_userland_sysrc
    pbsd_userland_service
    pbsd_userland_services
    pbsd_userland_periodic
    pbsd_userland_syslogd
    pbsd_userland_ac
    pbsd_userland_accton
    pbsd_userland_acpi
    pbsd_userland_adduser
    pbsd_userland_apm
    pbsd_userland_arp
    pbsd_userland_audit
    pbsd_userland_auditd
    pbsd_userland_authpf
    pbsd_userland_autofs
    pbsd_userland_bhyve
    pbsd_userland_chroot
    pbsd_userland_devctl
    pbsd_userland_devinfo
    pbsd_userland_diskinfo
    pbsd_userland_gstat
    pbsd_userland_iostat
    pbsd_userland_jexec
    pbsd_userland_jls
    pbsd_userland_ndp
    pbsd_userland_pciconf
    pbsd_userland_pstat
    pbsd_userland_tcpdump
    pbsd_userland_traceroute
    pbsd_userland_usbconfig
    pbsd_userland_zic
    pbsd_userland_zdump)
endif()

if(NOT TARGET pbsd_userland_sbin)
add_library(pbsd_userland_sbin INTERFACE)
target_link_libraries(pbsd_userland_sbin INTERFACE
    pbsd_userland_sysctl
    pbsd_userland_chown
    pbsd_userland_df
    pbsd_userland_ifconfig
    pbsd_userland_mount
    pbsd_userland_route
    pbsd_userland_install
    pbsd_userland_umount
    pbsd_userland_ping
    pbsd_userland_pkg_cmd
    pbsd_userland_newsyslog
    pbsd_userland_cron
    pbsd_userland_pw
    pbsd_userland_vipw
    pbsd_userland_service
    pbsd_userland_services
    pbsd_userland_sysrc
    pbsd_userland_periodic
    pbsd_userland_syslogd
    pbsd_userland_rcorder
    pbsd_userland_bectl
    pbsd_userland_zfs
    pbsd_userland_zpool
    pbsd_userland_md5
    pbsd_userland_fsck
    pbsd_userland_geom
    pbsd_userland_newfs
    pbsd_userland_camcontrol
    pbsd_userland_gpart
    pbsd_userland_mdconfig
    pbsd_userland_dumpon
    pbsd_userland_savecore
    pbsd_userland_fsck_ffs
    pbsd_userland_fsck_msdosfs
    pbsd_userland_newfs_msdos
    pbsd_userland_tunefs
    pbsd_userland_growfs
    pbsd_userland_dmesg
    pbsd_userland_dumpfs
    pbsd_userland_fdisk
    pbsd_userland_bsdlabel
    pbsd_userland_swapon
    pbsd_userland_shutdown
    pbsd_userland_reboot
    pbsd_userland_kldload
    pbsd_userland_mknod
    pbsd_userland_kldstat
    pbsd_userland_kldunload
    pbsd_userland_ldconfig
    pbsd_userland_nfsiod
    pbsd_userland_restore
    pbsd_userland_pfctl
    pbsd_userland_adjkerntz
    pbsd_userland_dhclient
    pbsd_userland_dump
    pbsd_userland_ffsinfo
    pbsd_userland_fsdb
    pbsd_userland_ipfw
    pbsd_userland_kldconfig
    pbsd_userland_nvmecontrol
    pbsd_userland_quotacheck
    pbsd_userland_resolvconf
    pbsd_userland_routed
    pbsd_userland_setkey)
endif()

if(NOT TARGET pbsd_userland)
add_library(pbsd_userland INTERFACE)
target_link_libraries(pbsd_userland INTERFACE
    pbsd_userland_hosted
    pbsd_userland_capsicum_os
    pbsd_userland_capsicum
    pbsd_userland_capsicum_helpers
    pbsd_userland_casper
    pbsd_userland_jail
    pbsd_userland_util
    pbsd_userland_libc
    pbsd_userland_msun
    pbsd_userland_libthr
    pbsd_userland_libprocstat
    pbsd_userland_rtld
    pbsd_userland_bin
    pbsd_userland_usr_bin
    pbsd_userland_usr_sbin
    pbsd_userland_sbin)
endif()
