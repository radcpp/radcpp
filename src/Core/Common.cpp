#include "rad/Core/Common.h"

#include <intrin.h>

uint32_t BitScanReverse32(uint32_t mask)
{
    assert(mask != 0);
#if defined(_WIN32)
    unsigned long index = 0;
    ::_BitScanReverse(&index, static_cast<uint32_t>(mask));
    return static_cast<uint32_t>(index);
#elif defined(__GNUC__)
    static_assert(sizeof(uint32_t) == sizeof(unsigned int));
    return (31u - __builtin_clz(static_cast<uint32_t>(mask)));
#else
    uint32_t index = 31u;
    mask |= 0x1; // index=0 if mask=0
    while (((mask >> index) & 0x1) == 0)
    {
        --index;
    }
    return index;
#endif
}

uint32_t BitScanReverse64(uint64_t mask)
{
    assert(mask != 0);
#if defined(_WIN64)
    unsigned long index = 0;
    ::_BitScanReverse64(&index, mask);
    return static_cast<uint32_t>(index);
#elif defined(_WIN32)
    unsigned long index = 0;
    const uint32_t highPart = HighPart(mask);
    ::_BitScanReverse(&index, (highPart != 0) ? highPart : LowPart(mask));
    return (highPart != 0) ? (index + 32u) : index;
#elif defined(__GNUC__)
    static_assert(sizeof(uint64_t) == sizeof(unsigned long long));
    return (63u - __builtin_clzll(mask));
#else
    uint32_t index = 63u;
    mask |= 0x1; // index=0 if mask=0
    while (((mask >> index) & 0x1) == 0)
    {
        --index;
    }
    return index;
#endif
}