static U32 HUF_decodeLastSymbolsX6(void* op, const U32 maxL, BIT_DStream_t* DStream,
                                  const HUF_DDescX6* dd, const HUF_DSeqX6* ds, const U32 dtLog)
{
    const size_t val = BIT_lookBitsFast(DStream, dtLog);   /* note : dtLog >= 1 */
    U32 length = dd[val].nbBytes;
    if (length <= maxL)
    {
        memcpy(op, ds+val, length);
        BIT_skipBits(DStream, dd[val].nbBits);
        return length;
    }
    memcpy(op, ds+val, maxL);
    if (DStream->bitsConsumed < (sizeof(DStream->bitContainer)*8))
    {
        BIT_skipBits(DStream, dd[val].nbBits);
        if (DStream->bitsConsumed > (sizeof(DStream->bitContainer)*8))
            DStream->bitsConsumed = (sizeof(DStream->bitContainer)*8);   /* ugly hack; works only because it's the last symbol. Note : can't easily extract nbBits from just this symbol */
    }
    return maxL;
}
