#pragma once

#include "rad/Core/Common.h"
#include <memory>

namespace adl
{
    template<typename T>
    class RefCounted;

    template<typename T>
    void IncRef(const RefCounted<T>* p);
    template<typename T>
    void DecRef(const RefCounted<T>* p);

    template<class T>
    class RefCounted
    {
    protected:
        RefCounted() noexcept
        {
        }

        ~RefCounted()
        {
        }

        size_t GetRefCount() const noexcept
        {
            return m_count;
        }

    private:
        mutable std::atomic<size_t> m_count{ 0 };

        friend void IncRef<T>(const RefCounted<T>* p);
        friend void DecRef<T>(const RefCounted<T>* p);
    };

    template<class T>
    inline void IncRef(const RefCounted<T>* p)
    {
        p->m_count.fetch_add(1, std::memory_order_relaxed);
    }

    template<class T>
    inline void DecRef(const RefCounted<T>* p)
    {
        if (p->m_count.fetch_sub(1, std::memory_order_release) == 1)
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            delete static_cast<const T*>(p);
        }
    }

} // namespace adl

using adl::RefCounted;

template<class T>
class Ref
{
public:
    constexpr Ref() noexcept :
        m_ptr(nullptr)
    {}

    Ref(T* p, bool addRefCount = true) :
        m_ptr(p)
    {
        if (m_ptr && addRefCount)
        {
            IncRef(m_ptr);
        }
    }

    Ref(Ref const& rhs) :
        m_ptr(rhs.get())
    {
        if (m_ptr)
        {
            IncRef(m_ptr);
        }
    }

    template<class U>
    Ref(Ref<U> const& rhs) :
        m_ptr(rhs.get())
    {
        if (m_ptr)
        {
            IncRef(m_ptr);
        }
    }

    ~Ref()
    {
        if (m_ptr)
        {
            DecRef(m_ptr);
        }
    }

    template<class U>
    Ref& operator=(Ref<U> const& rhs)
    {
        Ref(rhs).swap(*this);
        return *this;
    }

    Ref& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    Ref(Ref&& rhs) noexcept :
        m_ptr(rhs.m_ptr)
    {
        rhs.m_ptr = nullptr;
    }

    Ref& operator=(Ref&& rhs) noexcept
    {
        Ref(static_cast<Ref&&>(rhs)).swap(*this);
        return *this;
    }

    template<class U> friend class Ref;

    template<class U>
    Ref(Ref<U>&& rhs) :
        m_ptr(rhs.m_ptr)
    {
        rhs.m_ptr = nullptr;
    }

    template<class U>
    Ref& operator=(Ref<U>&& rhs) noexcept
    {
        Ref(static_cast<Ref<U>&&>(rhs)).swap(*this);
        return *this;
    }

    Ref& operator=(Ref const& rhs)
    {
        Ref(rhs).swap(*this);
        return *this;
    }

    Ref& operator=(T* rhs)
    {
        Ref(rhs).swap(*this);
        return *this;
    }

    void reset() noexcept
    {
        Ref().swap(*this);
    }

    void reset(T* rhs) noexcept
    {
        Ref(rhs).swap(*this);
    }

    void reset(T* rhs, bool addRefCount) noexcept
    {
        Ref(rhs, addRefCount).swap(*this);
    }

    T* get() const noexcept
    {
        return m_ptr;
    }

    T* detach() noexcept
    {
        T* p = m_ptr;
        m_ptr = 0;
        return p;
    }

    T& operator*() const noexcept
    {
        assert(m_ptr != 0);
        return *m_ptr;
    }

    T* operator->() const noexcept
    {
        assert(m_ptr != 0);
        return m_ptr;
    }

    void swap(Ref& rhs) noexcept
    {
        std::swap(m_ptr, rhs.m_ptr);
    }

    explicit operator bool() const noexcept
    {
        return get() != nullptr;
    }

private:
    T* m_ptr = nullptr;

};

template<class T, class U> inline bool operator==(Ref<T> const& a, Ref<U> const& b) noexcept
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(Ref<T> const& a, Ref<U> const& b) noexcept
{
    return a.get() != b.get();
}

template<class T, class U> inline bool operator==(Ref<T> const& a, U* b) noexcept
{
    return a.get() == b;
}

template<class T, class U> inline bool operator!=(Ref<T> const& a, U* b) noexcept
{
    return a.get() != b;
}

template<class T, class U> inline bool operator==(T* a, Ref<U> const& b) noexcept
{
    return a == b.get();
}

template<class T, class U> inline bool operator!=(T* a, Ref<U> const& b) noexcept
{
    return a != b.get();
}

template<class T> inline bool operator==(Ref<T> const& p, std::nullptr_t) noexcept
{
    return p.get() == 0;
}

template<class T> inline bool operator==(std::nullptr_t, Ref<T> const& p) noexcept
{
    return p.get() == 0;
}

template<class T> inline bool operator!=(Ref<T> const& p, std::nullptr_t) noexcept
{
    return p.get() != 0;
}

template<class T> inline bool operator!=(std::nullptr_t, Ref<T> const& p) noexcept
{
    return p.get() != 0;
}

template<class T> inline bool operator<(Ref<T> const& a, Ref<T> const& b) noexcept
{
    return std::less<T*>()(a.get(), b.get());
}

template<class T> void swap(Ref<T>& lhs, Ref<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

// pointer casts

template<class T, class U> Ref<T> static_pointer_cast(Ref<U> const& p)
{
    return static_cast<T*>(p.get());
}

template<class T, class U> Ref<T> const_pointer_cast(Ref<U> const& p)
{
    return const_cast<T*>(p.get());
}

template<class T, class U> Ref<T> dynamic_pointer_cast(Ref<U> const& p)
{
    return dynamic_cast<T*>(p.get());
}


template<class T, class U> Ref<T> static_pointer_cast(Ref<U>&& p) noexcept
{
    return Ref<T>(static_cast<T*>(p.detach()), false);
}

template<class T, class U> Ref<T> const_pointer_cast(Ref<U>&& p) noexcept
{
    return Ref<T>(const_cast<T*>(p.detach()), false);
}

template<class T, class U> Ref<T> dynamic_pointer_cast(Ref<U>&& p) noexcept
{
    T* p2 = dynamic_cast<T*>(p.get());

    Ref<T> r(p2, false);

    if (p2)
    {
        p.detach();
    }

    return r;
}

template<class T> struct std::hash<Ref<T>>
{
    std::size_t operator()(Ref<T> const& p) const
    {
        return std::hash<T*>()(p.get());
    }
};
