/*
 * solver.c - Cache Lab Bonus.
 * solve() is called once per mystery cache. See solver.h and README.md.
 */
#include "solver.h"

/* Return whether all count distinct blocks survived the initial fill. */
static int all_resident(uint32_t stride, int count)
{
    cache_flush();
    for (int i = 0; i < count; ++i)
        cache_load((uint32_t)i * stride);

    for (int i = 0; i < count; ++i)
        if (!cache_load((uint32_t)i * stride))
            return 0;
    return 1;
}

static int classify_policy(uint32_t stride, int E)
{
    int lru = 1;
    int fifo = 1;

    if (E == 1)
        return POLICY_LRU;

    /*
     * Touch block 0 before overflowing the set: LRU evicts block 1,
     * FIFO evicts block 0. Test each candidate in a fresh trial because
     * a miss on the first probe could otherwise evict the second block.
     */
    for (int trial = 0; trial < 96 && (lru || fifo); ++trial) {
        int probe = trial % 2;

        cache_flush();
        for (int i = 0; i < E; ++i)
            cache_load((uint32_t)i * stride);
        cache_load(0);
        cache_load((uint32_t)E * stride);

        int hit = cache_load((uint32_t)probe * stride);
        lru = lru && (hit == (probe == 0));
        fifo = fifo && (hit == (probe == 1));
    }

    if (lru)
        return POLICY_LRU;
    if (fifo)
        return POLICY_FIFO;
    return POLICY_RANDOM;
}

void solve(void)
{
    int b = 0;
    int s = 1;
    int E = 1;

    /* Load block 0. The first power-of-two offset that misses is its size. */
    cache_flush();
    cache_load(0);
    while (b < 7 && cache_load(1u << b))
        b++;

    /* s <= 9: this stride always changes the tag without changing the set. */
    uint32_t same_set = 1u << (b + 9);
    while (E < 16 && all_resident(same_set, E + 1))
        ++E;

    /* Below s, the stride distributes E+1 blocks over at least two sets. */
    while (s < 9 && all_resident(1u << (b + s), E + 1))
        ++s;

    cache_flush();
    cache_store(0);
    int write_allocate = cache_load(0);
    int policy = classify_policy(same_set, E);
    report(b, s, E, policy, write_allocate);
}
