__owur int CRYPTO_THREAD_read_lock(CRYPTO_RWLOCK *lock)
{
    if (!ossl_assert(*(unsigned int *)lock == 1))
        return 0;
    return 1;
}
