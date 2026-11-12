static int b64_write(BIO *b, const char *in, int inl)
{
    int ret = 0;
    int n;
    int i;
    BIO_B64_CTX *ctx;
    BIO *next;

    ctx = (BIO_B64_CTX *)BIO_get_data(b);
    next = BIO_next(b);
    if (ctx == NULL || next == NULL)
        return 0;

    BIO_clear_retry_flags(b);

    if (ctx->encode != B64_ENCODE) {
        ctx->encode = B64_ENCODE;
        ctx->buf_len = 0;
        ctx->buf_off = 0;
        ctx->tmp_len = 0;
        EVP_EncodeInit(ctx->base64);
    }

    OPENSSL_assert(ctx->buf_off < (int)sizeof(ctx->buf));
    OPENSSL_assert(ctx->buf_len <= (int)sizeof(ctx->buf));
    OPENSSL_assert(ctx->buf_len >= ctx->buf_off);
    n = ctx->buf_len - ctx->buf_off;
    while (n > 0) {
        i = BIO_write(next, &(ctx->buf[ctx->buf_off]), n);
        if (i <= 0) {
            BIO_copy_next_retry(b);
            return i;
        }
        OPENSSL_assert(i <= n);
        ctx->buf_off += i;
        OPENSSL_assert(ctx->buf_off <= (int)sizeof(ctx->buf));
        OPENSSL_assert(ctx->buf_len >= ctx->buf_off);
        n -= i;
    }
    /* at this point all pending data has been written */
    ctx->buf_off = 0;
    ctx->buf_len = 0;

    if (in == NULL || inl <= 0)
        return 0;

    while (inl > 0) {
        n = inl > B64_BLOCK_SIZE ? B64_BLOCK_SIZE : inl;

        if ((BIO_get_flags(b) & BIO_FLAGS_BASE64_NO_NL) != 0) {
            if (ctx->tmp_len > 0) {
                OPENSSL_assert(ctx->tmp_len <= 3);
                n = 3 - ctx->tmp_len;
                /*
                 * There's a theoretical possibility for this
                 */
                if (n > inl)
                    n = inl;
                memcpy(&(ctx->tmp[ctx->tmp_len]), in, n);
                ctx->tmp_len += n;
                ret += n;
                if (ctx->tmp_len < 3)
                    break;
                ctx->buf_len = EVP_EncodeBlock(ctx->buf, ctx->tmp, ctx->tmp_len);
                OPENSSL_assert(ctx->buf_len <= (int)sizeof(ctx->buf));
                OPENSSL_assert(ctx->buf_len >= ctx->buf_off);
                /*
                 * Since we're now done using the temporary buffer, the
                 * length should be 0'd
                 */
                ctx->tmp_len = 0;
            } else {
                if (n < 3) {
                    memcpy(ctx->tmp, in, n);
                    ctx->tmp_len = n;
                    ret += n;
                    break;
                }
                n -= n % 3;
                ctx->buf_len = EVP_EncodeBlock(ctx->buf, (unsigned char *)in, n);
                OPENSSL_assert(ctx->buf_len <= (int)sizeof(ctx->buf));
                OPENSSL_assert(ctx->buf_len >= ctx->buf_off);
                ret += n;
            }
        } else {
            if (!EVP_EncodeUpdate(ctx->base64, ctx->buf, &ctx->buf_len,
                    (unsigned char *)in, n))
                return ret == 0 ? -1 : ret;
            OPENSSL_assert(ctx->buf_len <= (int)sizeof(ctx->buf));
            OPENSSL_assert(ctx->buf_len >= ctx->buf_off);
            ret += n;
        }
        inl -= n;
        in += n;

        ctx->buf_off = 0;
        n = ctx->buf_len;
        while (n > 0) {
            i = BIO_write(next, &(ctx->buf[ctx->buf_off]), n);
            if (i <= 0) {
                BIO_copy_next_retry(b);
                return ret == 0 ? i : ret;
            }
            OPENSSL_assert(i <= n);
            n -= i;
            ctx->buf_off += i;
            OPENSSL_assert(ctx->buf_off <= (int)sizeof(ctx->buf));
            OPENSSL_assert(ctx->buf_len >= ctx->buf_off);
        }
        ctx->buf_len = 0;
        ctx->buf_off = 0;
    }
    return ret;
}
