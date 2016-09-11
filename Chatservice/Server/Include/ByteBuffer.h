#pragma once
#include<cstring>
#include<type_traits>
#include<array>
#include"Util.h"

// TObj - type to serialize
// TAttrs - dynamically allocated attributes of U
// Doesn't work with classes containing nested dynamically allocated members
template<class TObj, class... TAttrs> class ByteBuffer
{
    // Workaround because of the lack of partial template specialization for functions
    typedef std::integral_constant<byte, 0> ValArg_t;
    typedef std::integral_constant<byte, 1> PtrArg_t;
    typedef std::integral_constant<byte, 2> PtrptrArg_t;

public:
    typedef ByteBuffer<TObj, TAttrs...> My_t;
    enum : std::size_t
    {
        SIZE = Size<TObj, TAttrs...>::value
    };
    ByteBuffer() noexcept 
        :m_ppos(m_data), m_ptail(m_data + SIZE - 1)
    { }
    ByteBuffer(byte *parray, std::size_t length) :ByteBuffer()
    {
        SetBuffer(parray, length);
    }
    ByteBuffer(const std::array<byte, SIZE>& arr) :ByteBuffer()
    {
        std::copy(arr.cbegin(), arr.cend(), m_data);
    }
    // Deep copy
    ByteBuffer(const My_t& rhs) noexcept
        :ByteBuffer()
    {
        std::memcpy(m_data, rhs.m_data, SIZE);
        m_ppos = m_data + (rhs.m_ppos - rhs.m_data);
    }
    // Deep copy
    My_t& operator=(const My_t& rhs) noexcept
    {
        std::memcpy(m_data, rhs.m_data, SIZE);
        m_ppos = m_data + (rhs.m_ppos - rhs.m_data);
        return *this;
    }
    // If T is value type: copies 'arg' to internal buffer
    // If T is a pointer type: Copies to internal buffer the object 'arg' points to
    template<typename T> 
    bool Insert(const T& arg)
    {
        typedef typename std::conditional<std::is_pointer<T>::value, PtrArg_t, ValArg_t>::type TArg;
        return Insert(arg, TArg());
    }
    // If T is a pointer to object: copies array of statically allocated objects
    // If T is a pointer to pointer: copies objects pointed to by each pointer in a pointer array
    template<typename T,
        typename = std::enable_if_t<std::is_pointer<T>::value || std::is_array<T>::value>
    > 
    bool InsertArray(T arr, std::size_t length)
    {
        typedef typename std::conditional<
            (std::is_pointer<std::remove_pointer_t<T>>::value), PtrptrArg_t, PtrArg_t
        >::type TArg;
        return InsertArray(arr, length, TArg());
    }
    // If T is value type: retrieves value type with a well-behaved copy constructor
    // If T is a pointer type: creates a new dynamically allocated object
    template<typename T> T Extract()
    {
        typedef typename std::conditional<std::is_pointer<T>::value, PtrArg_t, ValArg_t>::type TArg;
        return Extract<T>(TArg());
    }
    // If T is a pointer type: creates a new dynamically allocated array of T
    // If T is a pointer to pointer: Creates a new dynamically allocated array of pointers, 
    // pointing to dynamically created objects
    // The array's length will be written to the variable 'plength' points to
    template<typename T,
        typename = std::enable_if_t<std::is_pointer<T>::value || std::is_array<T>::value>
    > 
    T ExtractArray(std::size_t *plength)
    {
        typedef typename std::conditional<
            (std::is_pointer<std::remove_pointer_t<T>>::value), PtrptrArg_t, PtrArg_t
        >::type TArg;
        return ExtractArray<T>(plength, TArg());
    }

    // Copies internal buffer
    std::array<byte, SIZE> CopyData() const
    {
        std::array<byte, SIZE> cpy;
        std::copy(m_data, m_data + SIZE, cpy.begin());
        return cpy;
    }
    void Reset() noexcept
    {
        m_ppos = m_data;
    }
    void Clear() noexcept
    {
        m_ppos = m_data;
    }
    // Copies content of 'parray' to internal buffer
    bool SetBuffer(byte *parray, std::size_t length)
    {
        if (m_ppos != m_data || length > SIZE || !parray)
            return false;
        Reset();
        std::memcpy(m_ppos, parray, SIZE);
        return true;
    }
    std::size_t GetCurrentLength() const noexcept
    {
        return m_ppos - m_data;
    }
    operator byte *() noexcept
    {
        return m_data;
    }

protected:
    byte m_data[SIZE];
    byte *m_ppos;
    const byte *m_ptail;

private:
    template<typename U,
        typename = std::enable_if_t<
            std::is_copy_constructible<U>::value && std::is_copy_assignable<U>::value &&
            !std::is_pointer<U>::value
        >
    > 
    bool Insert(const U& item, ValArg_t&&)
    {   // U is a value type
        typedef typename std::remove_pointer<U>::type T;
        if ((m_ptail - m_ppos) >= sizeof(T)) {
            *reinterpret_cast<T *>(m_ppos) = item;
            m_ppos += sizeof(T);
            return true;
        }
        return false;
    }
    template<typename U,
        typename = std::enable_if_t<std::is_pointer<U>::value>
    > 
    bool Insert(const U& pitem, PtrArg_t&&)
    {   // U is a pointer type
        typedef typename std::remove_pointer<U>::type T;
        if ((m_ptail - m_ppos) >= sizeof(T) && pitem) {
            std::memcpy(m_ppos, pitem, sizeof(T));
            m_ppos += sizeof(T);
            return true;
        }
        return false;
    }
    template<typename U,
        typename = std::enable_if_t<
            std::is_pointer<U>::value || std::is_array<U>::value
        >
    > 
    bool InsertArray(U parray, std::size_t length, PtrArg_t&&)
    {   // U is a pointer type or an array
        Insert(length);
        typedef typename std::remove_pointer<U>::type T;
        if ((m_ptail - m_ppos) >= length * sizeof(T) && parray) {
            std::memcpy(m_ppos, parray, length * sizeof(T));
            m_ppos += length * sizeof(T);
            return true;
        }
        return false;
    }
    template<typename U,
        typename = std::enable_if_t<
            (std::is_pointer<U>::value || std::is_array<U>::value) &&
            (std::is_pointer<std::remove_pointer_t<U>>::value ||
             std::is_array<std::remove_pointer_t<U>>::value)
        >
    > 
    bool InsertArray(U pparray, std::size_t length, PtrptrArg_t&&)
    {   // U is a pointer to pointer
        Insert(length);
        typedef typename std::remove_pointer<typename std::remove_pointer<U>::type>::type T;
        if ((m_ptail - m_ppos) >= length * sizeof(T) && pparray) {
            for (std::size_t i = 0; i < length; ++i) {
                std::memcpy(m_ppos, *(pparray + i), sizeof(T));
                m_ppos += sizeof(T);
            }
            return true;
        }
        return false;
    }
    template<typename U,
        typename = std::enable_if_t<
            std::is_copy_constructible<U>::value && std::is_default_constructible<U>::value
            && !std::is_pointer<U>::value
        >
    > 
    U Extract(ValArg_t&&)
    {   // U is a value type
        typedef typename std::remove_pointer<U>::type T;
        if ((m_ptail - m_ppos) >= sizeof(T)) {
            T temp = *reinterpret_cast<T *>(m_ppos);
            m_ppos += sizeof(T);
            return temp;
        }
        return T();
    }
    template<typename U,
        typename = std::enable_if_t<std::is_pointer<U>::value>
    > 
    U Extract(PtrArg_t&&)
    {   // U is a pointer type
        typedef typename std::remove_pointer<U>::type T;
        if ((m_ptail - m_ppos) >= sizeof(T)) {
            byte *ptemp = new byte[sizeof(T)];
            std::memcpy(ptemp, m_ppos, sizeof(T));
            m_ppos += sizeof(T);
            return reinterpret_cast<T *>(ptemp);
        }
        return nullptr;
    }
    template<typename U,
        typename = std::enable_if_t<
            (std::is_pointer<U>::value || std::is_array<U>::value) &&
            std::is_default_constructible<std::remove_pointer_t<U>>::value
        >
    > 
    U ExtractArray(std::size_t *plength, PtrArg_t&&)
    {   // U is a pointer type
        *plength = Extract<std::size_t>();
        typedef typename std::remove_pointer<U>::type T;

        if ((m_ptail - m_ppos) >= (*plength) * sizeof(T)) {
            T *ptemp = new T[(*plength)];
            std::memcpy(ptemp, m_ppos, (*plength) * sizeof(T));
            m_ppos += (*plength) * sizeof(T);
            return ptemp;
        }
        return nullptr;
    }
    template<typename U,
        typename = std::enable_if_t<
            (std::is_pointer<U>::value || std::is_array<U>::value) &&
            (std::is_pointer<std::remove_pointer_t<U>>::value ||
             std::is_array<std::remove_pointer_t<U>>::value)
        >
    > 
    U ExtractArray(std::size_t *plength, PtrptrArg_t&&)
    {   // U is a pointer to pointer
        *plength = Extract<std::size_t>();
        typedef typename std::remove_pointer<typename std::remove_pointer<U>::type>::type T;

        if ((m_ptail - m_ppos) >= (*plength) * sizeof(T)) {
            T **pptemp = new T*[(*plength)];
            for (std::size_t i = 0; i < (*plength); ++i) {
                byte *ptemp = new byte[sizeof(T)];
                std::memcpy(ptemp, m_ppos, sizeof(T));
                *(pptemp + i) = reinterpret_cast<T *>(ptemp);
                m_ppos += sizeof(T);
            }
            return pptemp;
        }
        return nullptr;
    }
};

// Myt: this type implementng the ISerializable interface
// TAttrs: (optional) types of dynamically allocated objects contained in Myt
template<class Myt, typename... TAttrs> class ISerializable
{
public:
    typedef ISerializable<Myt, TAttrs...> MySerType;
    typedef ByteBuffer<Myt, TAttrs...> Buffer_t;

    // Should put into the buffer all data members to be serialized
    // Must insert in the same order as they are retrieved in OnDeserialize()
    virtual void OnSerialize(Buffer_t *pbuffer) = 0;

    // Should assign all data members to values extracted from the buffer
    // Must extract in the same order as they are inserted in OnSerialize()
    virtual void OnDeserialize(Buffer_t *pbuffer) = 0;
};