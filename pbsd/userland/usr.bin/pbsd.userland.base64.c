/* Reference logic from hbsd/src/usr.bin/base64/base64.c (dual-world). */
#include <stddef.h>

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int
b64_value(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c - 'A';
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 26;
	if (c >= '0' && c <= '9')
		return c - '0' + 52;
	if (c == '+')
		return 62;
	if (c == '/')
		return 63;
	return -1;
}

size_t
base64_encode_block(const unsigned char *in, size_t inlen, char *out, size_t outlen)
{
	size_t o = 0;
	for (size_t i = 0; i + 2 < inlen; i += 3) {
		if (o + 4 >= outlen)
			return (size_t)-1;
		out[o++] = b64_table[(in[i] >> 2) & 0x3F];
		out[o++] = b64_table[((in[i] & 0x3) << 4) | ((in[i + 1] >> 4) & 0xF)];
		out[o++] = b64_table[((in[i + 1] & 0xF) << 2) | ((in[i + 2] >> 6) & 0x3)];
		out[o++] = b64_table[in[i + 2] & 0x3F];
	}
	size_t rem = inlen % 3;
	if (rem == 0)
		goto done;
	if (o + 4 >= outlen)
		return (size_t)-1;
	unsigned char a = in[inlen - rem];
	unsigned char b = rem == 2 ? in[inlen - 1] : 0;
	out[o++] = b64_table[(a >> 2) & 0x3F];
	out[o++] = b64_table[((a & 0x3) << 4) | ((b >> 4) & 0xF)];
	out[o++] = rem == 2 ? b64_table[((b & 0xF) << 2)] : '=';
	out[o++] = '=';
done:
	if (o < outlen)
		out[o] = '\0';
	return o;
}

int
base64_decode_char(char c)
{
	return b64_value(c);
}
