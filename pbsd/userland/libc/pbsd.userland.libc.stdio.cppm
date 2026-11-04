export module pbsd.userland.libc.stdio;

export import pbsd.userland.libc.stdio.file;
export import pbsd.userland.libc.stdio.flags;
export import pbsd.userland.libc.stdio.fileno;
export import pbsd.userland.libc.stdio.putc;
export import pbsd.userland.libc.stdio.getc;
export import pbsd.userland.libc.stdio.putchar;
export import pbsd.userland.libc.stdio.getchar;
export import pbsd.userland.libc.stdio.puts;
export import pbsd.userland.libc.stdio.fputs;
export import pbsd.userland.libc.stdio.fputc;
export import pbsd.userland.libc.stdio.fgetc;
export import pbsd.userland.libc.stdio.sprintf;
export import pbsd.userland.libc.stdio.vsnprintf;
export import pbsd.userland.libc.stdio.perror;
export import pbsd.userland.libc.stdio.fflush;
export import pbsd.userland.libc.stdio.fopen;
export import pbsd.userland.libc.stdio.fread;
export import pbsd.userland.libc.stdio.fwrite;
export import pbsd.userland.libc.stdio.fseek;
export import pbsd.userland.libc.stdio.getwchar;
export import pbsd.userland.libc.stdio.putwchar;
export import pbsd.userland.libc.stdio.remove;
export import pbsd.userland.libc.stdio.swprintf;
export import pbsd.userland.libc.stdio.tmpnam;
export import pbsd.userland.libc.stdio.vasprintf;
export import pbsd.userland.libc.stdio.makebuf;
export import pbsd.userland.libc.stdio.setbuffer;
export import pbsd.userland.libc.stdio.stdio;

/// stdio helper umbrella (concept FILE; hosted I/O deferred).
export namespace pbsd::userland::libc::stdio {} // namespace
