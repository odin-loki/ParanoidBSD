#undef CDIOREADTOCHEADER
#define CDIOREADTOCHEADER _IOR('c',4,struct ioc_toc_header)
#undef n
#define n(flags) (~(flags) & (flags))
