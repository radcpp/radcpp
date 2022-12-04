#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>

/// Returns element count of the array (size_t).
template <typename T, size_t N>
constexpr size_t ArraySize(const T(&array)[N])
{
    return N;
}

/// Returns element count of the array (uint32).
template <typename T, uint32_t N>
constexpr uint32_t ArraySize32(const T(&array)[N])
{
    return N;
}

/// Returns the high 32 bits of a 64-bit integer.
constexpr uint32_t HighPart(uint64_t value)
{
    return (value & 0xFFFFFFFF00000000) >> 32;
}

/// Returns the low 32 bits of a 64-bit integer.
constexpr uint32_t LowPart(uint64_t value)
{
    return (value & 0x00000000FFFFFFFF);
}

/// Returns a bitfield from within some value.
template <typename T>
constexpr T BitExtract(T value, uint32_t firstBit, uint32_t lastBit)
{
    return (value >> firstBit) & ((1 << (lastBit - firstBit + 1)) - 1);
}

/// Determines if any of the bits set in flag.
constexpr bool HasAny(uint32_t flag, uint32_t bits)
{
    return ((flag & bits) != 0);
}

/// Determines if all of the bits set in flag.
constexpr bool HasAll(uint32_t flag, uint32_t bits)
{
    return ((flag & bits) == bits);
}

/// Count the number of bits set in an unsigned integer.
constexpr uint32_t CountBits32(uint32_t x)
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (((x + (x >> 4)) & 0x0F0F0F0F) * 0x01010101) >> ((sizeof(uint32_t) - 1) << 3);
    return x;
}

/// Search the mask data from most significant bit (MSB) to least significant bit (LSB) for a set bit (1).
/// The mask must be nonzero or the index returned is undefined.
uint32_t BitScanReverse32(uint32_t mask);

/// Search the mask data from most significant bit (MSB) to least significant bit (LSB) for a set bit (1).
/// The mask must be nonzero or the index returned is undefined.
uint32_t BitScanReverse64(uint64_t mask);

constexpr bool IsPow2(uint64_t x)
{
    return (x == 0) ? false : ((x & (x - 1)) == 0);
}

template <typename T>
T Pow2AlignUp(T value, uint64_t alignment)
{
    assert(IsPow2(alignment));
    return ((value + static_cast<T>(alignment) - 1) & ~(static_cast<T>(alignment) - 1));
}

template <typename T>
T Pow2AlignDown(T value, uint64_t alignment)
{
    assert(IsPow2(alignment));
    return (value & ~(alignment - 1));
}

uint32_t RoundUpToNextPow2(uint32_t x);
uint64_t RoundUpToNextPow2(uint64_t x);

uint32_t RoundUpToPow2(uint32_t x);
uint64_t RoundUpToPow2(uint64_t x);

template<typename T>
constexpr T RoundUpToMultiple(T value, T alignment)
{
    return (((value + (alignment - 1)) / alignment) * alignment);
}

template <typename T>
constexpr T RoundDownToMultiple(T value, T alignment)
{
    return ((value / alignment) * alignment);
}

inline
uint32_t FloatBitsToUint(float f32)
{
    union Variable
    {
        float f32;
        uint32_t u32;
    } var;

    var.f32 = f32;
    return var.u32;
}

inline
float UintBitsToFloat(uint32_t u32)
{
    union Variable
    {
        uint32_t u32;
        float f32;
    } var;

    var.u32 = u32;
    return var.f32;
}
