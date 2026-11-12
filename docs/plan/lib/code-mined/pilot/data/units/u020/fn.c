template<typename T, uptr kSize>
uptr AddrHashMap<T, kSize>::calcHash(uptr addr) {
  addr += addr << 10;
  addr ^= addr >> 6;
  return addr % kSize;
}
