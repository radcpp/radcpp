#pragma once

#include "rad/Core/Common.h"

template<typename T, size_t DefaultCapacity>
class SmallVector
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using difference_type = int32_t;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    SmallVector() noexcept :
        m_data(reinterpret_cast<T*>(m_stackStorage)),
        m_elementCount(0),
        m_capacity(DefaultCapacity)
    {
    }

    SmallVector(size_type n)
    {
        Resize(n);
    }

    SmallVector(size_type n, const T& value)
    {
        Resize(n, value);
    }

    SmallVector(const_iterator first, const_iterator last)
    {
        for (; first != last; ++first)
        {
            PushBack(*first);
        }
    }

    SmallVector(const_reverse_iterator first, const_reverse_iterator last)
    {
        for (; first != last; ++first)
        {
            PushBack(*first);
        }
    }

    SmallVector(SmallVector&& other) noexcept;

    SmallVector(std::initializer_list<T> list) :
        SmallVector()
    {
        for (auto iter = list.begin(); iter != list.end(); ++iter)
        {
            PushBack(*iter);
        }
    }

    virtual ~SmallVector()
    {
        Reset();
    }

    iterator                begin()  noexcept { return iterator(m_data); }
    iterator                end()    noexcept { return iterator(m_data + m_elementCount); }
    const_iterator          begin()  const noexcept { return const_iterator(m_data); }
    const_iterator          end()    const noexcept { return const_iterator(m_data + m_elementCount); }
    reverse_iterator        rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator  rbegin() const noexcept { return const_reverse_iterator(end()); }
    reverse_iterator        rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator  rend() const noexcept { return const_reverse_iterator(begin()); }
    const_iterator          cbegin() const noexcept { return const_iterator(m_data); }
    const_iterator          cend()   const noexcept { return const_iterator(m_data + m_elementCount); }
    const_reverse_iterator  crbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator  crend() const noexcept { return const_reverse_iterator(begin()); }

    [[nodiscard]] bool empty()  const noexcept { return IsEmpty(); }
    size_type size()   const noexcept { return m_elementCount; }
    size_type count()   const noexcept { return m_elementCount; }
    size_type max_size() const noexcept { return UINT32_MAX; }
    size_type capacity() const noexcept { return m_capacity; }

    T* data() { return m_data; }
    const T* data() const { return m_data; }

    constexpr void Reserve(size_type newCapacity);
    constexpr void Resize(size_type newSize, const T& value = T());
    constexpr void PushBack(const T& value);
    void PopBack();
    void Clear();

    reference at(size_type index)
    {
        assert(index < m_elementCount);
        return *(m_data + index);
    }

    const_reference at(size_type index) const
    {
        assert(index < m_elementCount);
        return *(m_data + index);
    }

    reference operator[](size_type index) noexcept { return at(index); }
    const_reference operator[](size_type index) const noexcept { return at(index); }

    T& Front() const
    {
        assert(!IsEmpty());
        return *m_data;
    }
    T& Back() const
    {
        assert(!IsEmpty());
        return *(m_data + (m_elementCount - 1));
    }

    iterator Begin() const { return m_data; }
    iterator End() const { return (m_data + m_elementCount); }

    size_type Count() const { return m_elementCount; }
    bool IsEmpty() const { return (m_elementCount == 0); }

    void Reset();

private:
    using StackStorage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    StackStorage m_stackStorage[DefaultCapacity] = {};
    T* m_data;
    size_type m_elementCount;
    size_type m_capacity;

}; // class SmallVector

template<typename T, size_t DefaultCapacity>
SmallVector<T, DefaultCapacity>::SmallVector(SmallVector&& other) noexcept :
    m_elementCount(other.m_elementCount)
{
    if (other.m_data == reinterpret_cast<T*>(other.m_data))
    {
        m_data = reinterpret_cast<T*>(m_data);

        if (std::is_trivial_v<T>)
        {
            std::memcpy(m_data, other.m_data, sizeof(T) * m_elementCount);
        }
        else
        {
            for (size_type i = 0; i < m_elementCount; ++i)
            {
                new(m_data + i) T(std::move(other.m_data[i]));
            }
        }
    }
    else
    {
        m_data = other.m_data;
        other.m_data = nullptr;
        other.m_elementCount = 0;
    }
}

template<typename T, size_t DefaultCapacity>
void SmallVector<T, DefaultCapacity>::Reset()
{
    if (!std::is_trivial_v<T>)
    {
        for (size_type i = 0; i < m_elementCount; ++i)
        {
            m_data[i].~T();
        }
    }

    if (m_data != reinterpret_cast<T*>(m_data))
    {
        std::free(m_data);
    }

    m_elementCount = 0;
    m_capacity = DefaultCapacity;
}

template<typename T, size_t DefaultCapacity>
constexpr void SmallVector<T, DefaultCapacity>::Reserve(size_type newCapacity)
{
    if (m_capacity < newCapacity)
    {
        void* const pNewMemory = std::malloc(sizeof(T) * newCapacity);

        if (pNewMemory != nullptr)
        {
            T* const pNewData = static_cast<T*>(pNewMemory);

            if (std::is_trivial_v<T>)
            {
                std::memcpy(pNewData, m_data, sizeof(T) * m_elementCount);
            }
            else
            {
                for (size_type i = 0; i < m_elementCount; ++i)
                {
                    new(pNewData + i) T(std::move(m_data[i]));
                    m_data[i].~T();
                }
            }

            if (m_data != reinterpret_cast<T*>(m_data))
            {
                std::free(m_data);
            }

            m_data = pNewData;
            m_capacity = newCapacity;
        }
    }
}

template<typename T, size_t DefaultCapacity>
constexpr void SmallVector<T, DefaultCapacity>::Resize(size_type newSize, const T& value)
{
    if (m_elementCount > newSize)
    {
        if (std::is_trivial_v<T>)
        {
            m_elementCount = newSize;
        }
        else
        {
            while (m_elementCount > newSize)
            {
                m_data[--m_elementCount].~T();
            }
        }
    }
    else if (m_elementCount < newSize)
    {
        Reserve(newSize);

        while (m_elementCount < newSize)
        {
            new(m_data + m_elementCount) T(value);
            ++(m_elementCount);
        }
    }
}

template<typename T, size_t DefaultCapacity>
constexpr void SmallVector<T, DefaultCapacity>::PushBack(const T& value)
{
    if (m_elementCount == m_capacity)
    {
        Reserve(m_elementCount * 2);
    }

    new(m_data + m_elementCount) T(value);
    ++m_elementCount;
}

template<typename T, size_t DefaultCapacity>
void SmallVector<T, DefaultCapacity>::PopBack()
{
    assert(IsEmpty() == false);
    --m_elementCount;

    if (!std::is_trivial_v<T>)
    {
        m_data[m_elementCount].~T();
    }
}

template<typename T, size_t DefaultCapacity>
void SmallVector<T, DefaultCapacity>::Clear()
{
    if (!std::is_trivial_v<T>)
    {
        for (size_type i = 0; i < m_elementCount; ++i)
        {
            m_data[i].~T();
        }
    }

    m_elementCount = 0;
}
