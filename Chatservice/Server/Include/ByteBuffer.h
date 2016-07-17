#pragma once
#include<cstring>
#include<type_traits>

typedef unsigned char byte;

// TObj - type to serialize
// TAttrs - dynamically allocated attributes of U
// Doesn't work with classes containing nested dynamically allocated members
template<class TObj, class... TAttrs> class ByteBuffer
{
    // Helper used for determining the total size of template args
    template<typename... Args> struct Size;
    template<> struct Size<>
    {
        static constexpr size_t value = 0;
    };
    template<typename T, typename... Args> struct Size<T, Args...>
    {
        static constexpr size_t value = sizeof(T) + Size<Args...>::value;
    };
    // Workaround because of the lack of partial template specialization for functions
    typedef std::integral_constant<byte, 0> ValArg;
    typedef std::integral_constant<byte, 1> PtrArg;
    typedef std::integral_constant<byte, 2> PtrptrArg;

public:
    typedef ByteBuffer<TObj, TAttrs...> Myt;
    enum : size_t
    {
        SIZE = Size<TObj, TAttrs...>::value
    };
    ByteBuffer() :m_ppos(m_data), m_ptail(m_data + SIZE - 1)
    { }
    ByteBuffer(byte *parray, size_t length) :ByteBuffer()
    {
        SetBuffer(parray, length);
    }
    // Deep copy
    ByteBuffer(const Myt& rhs) :ByteBuffer()
    {
        std::memcpy(m_data, rhs.m_data, SIZE);
        m_ppos = m_data + (rhs.m_ppos - rhs.m_data);
    }
    // Deep copy
    Myt& operator=(const Myt& rhs)
    {
        std::memcpy(m_data, rhs.m_data, SIZE);
        m_ppos = m_data + (rhs.m_ppos - rhs.m_data);
        return *this;
    }
    // If T is value type: copies 'arg' to internal buffer
    // If T is a pointer type: Copies to internal buffer the object 'arg' points to
    template<typename T> bool Insert(T arg)
    {
        typedef typename std::conditional<std::is_pointer<T>::value, PtrArg, ValArg>::type TArg;
        return Insert(arg, TArg());
    }
    // If T is a pointer to object: copies array of statically allocated objects
    // If T is a pointer to pointer: copies objects pointed to by each pointer in a pointer array
    template<typename T> bool InsertArray(T arr, size_t length)
    {
        typedef typename std::enable_if<std::is_pointer<T>::value || std::is_array<T>::value>::type Typecheck;
        typedef typename std::conditional<
            (std::is_pointer<typename std::remove_pointer<T>::type>::value), PtrptrArg, PtrArg
        >::type TArg;
        return InsertArray(arr, length, TArg());
    }
    // If T is value type: retrieves value type with a well-behaved copy constructor
    // If T is a pointer type: creates a new dynamically allocated object
    template<typename T> T Extract()
    {
        typedef typename std::conditional<std::is_pointer<T>::value, PtrArg, ValArg>::type TArg;
        return Extract<T>(TArg());
    }
    // If T is a pointer type: creates a new dynamically allocated array of T
    // If T is a pointer to pointer: Creates a new dynamically allocated array of pointers, 
    // pointing to dynamically created objects
    // The array's length will be written to the variable 'plength' points to
    template<typename T> T ExtractArray(size_t *plength)
    {
        typedef typename std::enable_if<std::is_pointer<T>::value || std::is_array<T>::value>::type Typecheck;
        typedef typename std::conditional<
            (std::is_pointer<typename std::remove_pointer<T>::type>::value), PtrptrArg, PtrArg
        >::type TArg;
        return ExtractArray<T>(plength, TArg());
    }
    // Copies internal buffer to 'pbuffer'
    bool CopyBufferTo(byte *pbuffer, size_t length) const
    {
        if (length < SIZE || !pbuffer)
            return false;
        std::memcpy(pbuffer, m_data, SIZE);
        return true;
    }
    void Reset()
    {
        m_ppos = m_data;
    }
    void Clear()
    {
        m_ppos = m_data;
    }
    // Copies content of 'parray' to internal buffer
    bool SetBuffer(byte *parray, size_t length)
    {
        if (m_ppos != m_data || length > SIZE || !parray)
            return false;
        Reset();
        std::memcpy(m_ppos, parray, SIZE);
        return true;
    }
    size_t GetCurrentLength() const
    {
        return m_ppos - m_data;
    }

protected:
    byte m_data[SIZE];
    byte *m_ppos;
    const byte *m_ptail;

private:
    template<typename U> bool Insert(U item, ValArg)
    {   // U is a value type
        typedef typename std::enable_if<
            std::is_copy_constructible<U>::value && std::is_copy_assignable<U>::value &&
            !std::is_pointer<U>::value> ::type
            Typecheck;

        typedef typename std::remove_pointer<U>::type T;
        if ((m_ptail - m_ppos) >= sizeof(T)) {
            *reinterpret_cast<T *>(m_ppos) = item;
            m_ppos += sizeof(T);
            return true;
        }
        return false;
    }
    template<typename U> bool Insert(U pitem, PtrArg)
    {   // U is a pointer type
        typedef typename std::enable_if<std::is_pointer<U>::value>::type Typecheck;

        typedef typename std::remove_pointer<U>::type T;
        if ((m_ptail - m_ppos) >= sizeof(T) && pitem) {
            std::memcpy(m_ppos, pitem, sizeof(T));
            m_ppos += sizeof(T);
            return true;
        }
        return false;
    }
    template<typename U> bool InsertArray(U parray, size_t length, PtrArg)
    {   // U is a pointer type or an array
        typedef typename std::enable_if<
            std::is_pointer<U>::value || std::is_array<U>::value>::type Typecheck;

        Insert(length);
        typedef typename std::remove_pointer<U>::type T;
        if ((m_ptail - m_ppos) >= length * sizeof(T) && parray) {
            std::memcpy(m_ppos, parray, length * sizeof(T));
            m_ppos += length * sizeof(T);
            return true;
        }
        return false;
    }
    template<typename U> bool InsertArray(U pparray, size_t length, PtrptrArg)
    {   // U is a pointer to pointer
        typedef typename std::enable_if<
            (std::is_pointer<U>::value || std::is_array<U>::value) &&
            (std::is_pointer<typename std::remove_pointer<U>::type>::value ||
             std::is_array<typename std::remove_pointer<U>::type>::value)>::type Typecheck;

        Insert(length);
        typedef typename std::remove_pointer<typename std::remove_pointer<U>::type>::type T;
        if ((m_ptail - m_ppos) >= length * sizeof(T) && pparray) {
            for (size_t i = 0; i < length; ++i) {
                std::memcpy(m_ppos, *(pparray + i), sizeof(T));
                m_ppos += sizeof(T);
            }
            return true;
        }
        return false;
    }
    template<typename U> U Extract(ValArg)
    {   // U is a value type
        typedef typename std::enable_if<
            std::is_copy_constructible<U>::value && std::is_default_constructible<U>::value
            && !std::is_pointer<U>::value>::type Typecheck;

        typedef typename std::remove_pointer<U>::type T;
        if ((m_ptail - m_ppos) >= sizeof(T)) {
            T temp = *reinterpret_cast<T *>(m_ppos);
            m_ppos += sizeof(T);
            return temp;
        }
        return T();
    }
    template<typename U> U Extract(PtrArg)
    {   // U is a pointer type
        typedef typename std::enable_if<std::is_pointer<U>::value>::type Typecheck;

        typedef typename std::remove_pointer<U>::type T;
        if ((m_ptail - m_ppos) >= sizeof(T)) {
            byte *ptemp = new byte[sizeof(T)];
            std::memcpy(ptemp, m_ppos, sizeof(T));
            m_ppos += sizeof(T);
            return reinterpret_cast<T *>(ptemp);
        }
        return nullptr;
    }
    template<typename U> U ExtractArray(size_t *plength, PtrArg)
    {   // U is a pointer type
        typedef typename std::enable_if<
            (std::is_pointer<U>::value || std::is_array<U>::value) &&
            std::is_default_constructible<typename std::remove_pointer<U>::type>::value
        >::type Typecheck;

        *plength = Extract<size_t>();
        typedef typename std::remove_pointer<U>::type T;

        if ((m_ptail - m_ppos) >= (*plength) * sizeof(T)) {
            T *ptemp = new T[(*plength)];
            std::memcpy(ptemp, m_ppos, (*plength) * sizeof(T));
            m_ppos += (*plength) * sizeof(T);
            return ptemp;
        }
        return nullptr;
    }
    template<typename U> U ExtractArray(size_t *plength, PtrptrArg)
    {   // U is a pointer to pointer
        typedef typename std::enable_if<
            (std::is_pointer<U>::value || std::is_array<U>::value) &&
            (std::is_pointer<typename std::remove_pointer<U>::type>::value ||
             std::is_array<typename std::remove_pointer<U>::type>::value)>::type Typecheck;

        *plength = Extract<size_t>();
        typedef typename std::remove_pointer<typename std::remove_pointer<U>::type>::type T;

        if ((m_ptail - m_ppos) >= (*plength) * sizeof(T)) {
            T **pptemp = new T*[(*plength)];
            for (size_t i = 0; i < (*plength); ++i) {
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
    typedef ByteBuffer<Myt, TAttrs...> BufferType;

    // Should put into the buffer all data members to be serialized
    // Must insert in the same order as they are retrieved in OnDeserialize()
    virtual void OnSerialize(BufferType *pbuffer) = 0;

    // Should assign all data members to values extracted from the buffer
    // Must extract in the same order as they are inserted in OnSerialize()
    virtual void OnDeserialize(BufferType *pbuffer) = 0;
};