#pragma once

#include "rad/Core/Common.h"
#include <atomic>

// https://github.com/mmp/pbrt-v3/blob/master/src/core/parallel.h
template<typename Float = float,
    typename = std::enable_if_t<std::is_floating_point_v<Float>>
>
class AtomicFloat
{
public:
    template<typename Integer>
    struct IntegerSizeEqual : std::bool_constant<sizeof(Integer) == sizeof(Float)>
    {
        using type = Integer;
    };
    struct TypeNotSupported : std::true_type
    {
        using type = void;
    };

    using UnderlyingType = typename std::disjunction<
        IntegerSizeEqual<uint32_t>,
        IntegerSizeEqual<uint64_t>,
        TypeNotSupported
    >::type;

    explicit AtomicFloat(Float f = 0) { m_bits = FloatBitsToUint(f); }
    operator Float() const { return UintBitsToFloat(m_bits); }

    Float operator=(Float f)
    {
        m_bits = FloatToBits(f);
        return f;
    }

    void Add(Float f)
    {
        UnderlyingType oldBits = m_bits, newBits;
        do {
            newBits = FloatBitsToUint(UintBitsToFloat(oldBits) + f);
        } while (!m_bits.compare_exchange_weak(oldBits, newBits));
    }

private:
    std::atomic<UnderlyingType> m_bits;

}; // AtomicFloat
