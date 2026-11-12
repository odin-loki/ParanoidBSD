#undef IIC_NOERR
#define IIC_NOERR	0x0
#undef IIC_EBUSERR
#define IIC_EBUSERR	0x1
#undef IIC_ERRNO
#define IIC_ERRNO	__INT_MIN
#undef __INT_MIN
#define __INT_MIN	(-0x7fffffff - 1)
