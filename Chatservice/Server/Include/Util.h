#pragma once
#include <type_traits>
#include <array>
#include <string>

typedef uint8_t         byte;
typedef int8_t			sbyte;

// Helper used for determining the total size of template args pack
template<typename... TArgs> struct ArgsSize;
template<> struct ArgsSize<>
{
	static constexpr std::size_t value = 0;
};
template<typename T, typename... TArgs> struct ArgsSize<T, TArgs...>
{
	static constexpr std::size_t value = sizeof(T) + ArgsSize<TArgs...>::value;
};
//--------------------------------------------------------------------------------------------------
// Obtains type at position N in TArgs
template <std::size_t N, class... TArgs>
struct TypeAt;

template <std::size_t N, class T, class... TArgs>
struct TypeAt<N, T, TArgs...>
{
    typedef typename TypeAt<N - 1, TArgs...>::Result_t Result_t;
};
template<class T, class... TArgs>
struct TypeAt<0, T, TArgs...>
{
    typedef T Result_t;
};
//--------------------------------------------------------------------------------------------------
// Struct generator (generates class with one member of each type in TArgs and a setter and getter methods)
template<class... TArgs>
class StructGen;

//--------------------------------------------------------------------------------------------------
// Get substruct by index of its first template argument
template<class TStruct, std::size_t INDEX>
struct IndSubstruct;

template<class TFirst, class... TRest, std::size_t INDEX>
struct IndSubstruct<StructGen<TFirst, TRest...>, INDEX>
{
    typedef typename IndSubstruct<StructGen<TRest...>, INDEX - 1>::Result_t Result_t;
};
template<class TFirst, class... TRest>
struct IndSubstruct<StructGen<TFirst, TRest...>, 0U>
{
    typedef StructGen<TFirst, TRest...> Result_t;
};
//--------------------------------------------------------------------------------------------------
// Get substruct by type of its first template argument
template<class TStruct, class T>
struct TypeSubstruct;

template<class T, class TFirst, class... TRest>
struct TypeSubstruct<StructGen<TFirst, TRest...>, T>
{
    typedef typename TypeSubstruct<StructGen<TRest...>, T>::Result_t Result_t;
};
template<class T, class... TRest>
struct TypeSubstruct<StructGen<T, TRest...>, T>
{
    typedef StructGen<T, TRest...> Result_t;
};
//--------------------------------------------------------------------------------------------------
// Struct generator definitions
template<class TFirst, class... TRest>
class StructGen<TFirst, TRest...> : protected StructGen<TRest...>
{
public:
    typedef StructGen<TFirst, TRest...> My_t;
    typedef StructGen<TRest...>         Base_t;

    template<
        class TFirstArg,
        class... TRestArgs,
        class = std::enable_if_t<
        !std::is_base_of<My_t, std::decay_t<TFirstArg>>::value>
    >
    StructGen(TFirstArg&& arg, TRestArgs&&... args)
        : Base_t(std::forward<TRestArgs>(args)...), m_var(std::forward<TFirstArg>(arg))
    { }
    StructGen()                     = default;
    StructGen(const My_t&)          = default;
    StructGen(My_t&&)               = default;
    My_t& operator=(const My_t&)    = default;
    My_t& operator=(My_t&&)         = default;

    // Get-by-index methods
    template<std::size_t INDEX,
        class = std::enable_if_t<!std::is_fundamental<
            typename IndSubstruct<My_t, INDEX>::Result_t::Var_t
        >::value>>
    const auto& Get() const noexcept
    {
        using Parent_t = typename IndSubstruct<My_t, INDEX>::Result_t;
        return this->Parent_t::m_var;
    }
    template<std::size_t INDEX,
        class = std::enable_if_t<std::is_fundamental<
            typename IndSubstruct<My_t, INDEX>::Result_t::Var_t
        >::value>>
    auto Get() const noexcept
    {
        using Parent_t = typename IndSubstruct<My_t, INDEX>::Result_t;
        return this->Parent_t::m_var;
    }
    // Get-by-type methods
    template<class TMember,
        class = std::enable_if_t<!std::is_fundamental<TMember>::value>>
    const TMember& Get() const noexcept
    {
        using Parent_t = typename TypeSubstruct<My_t, TMember>::Result_t;
        return this->Parent_t::m_var;
    }
    template<class TMember,
        class = std::enable_if_t<std::is_fundamental<TMember>::value>>
    TMember Get() const noexcept
    {
        using Parent_t = typename TypeSubstruct<My_t, TMember>::Result_t;
        return this->Parent_t::m_var;
    }

    // Set methods
    template<std::size_t INDEX, class T>
    void Set(T&& value)
    {
        using Parent_t = typename IndSubstruct<My_t, INDEX>::Result_t;
        this->Parent_t::m_var = std::forward<T>(value);
    }
    template<class TMember, class T>
    void Set(T&& value)
    {
        using Parent_t = typename TypeSubstruct<My_t, TMember>::Result_t;
        this->Parent_t::m_var = std::forward<T>(value);
    }

protected:
    typedef TFirst       Var_t;
    Var_t m_var;
};
template<>
class StructGen<> 
{ };
//--------------------------------------------------------------------------------------------------
//// Checks whether all TArgs are copy assignable and copy constructible
//template <class... TArgs>
//struct AllCopyable;
//
//template<class TFirst, class...TRest>
//struct AllCopyable<TFirst, TRest...>
//{
//    static constexpr bool value = (std::is_copy_assignable<TFirst>::value && std::is_copy_constructible<TFirst>::value)
//        ? AllCopyable<TRest...>::value : false;
//};
//template<>
//struct AllCopyable<>
//{
//    static constexpr bool value = true;
//};
//--------------------------------------------------------------------------------------------------

template <class TStruct, std::size_t N = 0> class Serializer;

// All TArgs must be trivially copy assignable and trivially copy constructible (= all arrays must be std::array)
// N = extra size in bytes, accounts for dynamically allocated members
template<class... TArgs, std::size_t N>
class Serializer<StructGen<TArgs...>, N>
{
    typedef std::integral_constant<byte, 0> Zero_t;
    typedef std::integral_constant<byte, 1> NZero_t;

    static constexpr std::size_t NARGS = sizeof...(TArgs);
    static constexpr std::size_t SIZE = ArgsSize<TArgs...>::value + N;

public:
    typedef Serializer<StructGen<TArgs...>>     My_t;
    typedef StructGen<TArgs...>                 Data_t;

    Serializer() noexcept
        :m_buffer(), m_piter(m_buffer)
    { }
    void Serialize(const Data_t& data)
    {
        Reset();
        InternalSerialize<NARGS - 1>(data, NZero_t());
    }
    Data_t Deserialize() const // dynamically allocates c-strings
    {
        Reset();
        Data_t res;
        InternalDeserialize<NARGS - 1>(&res, NZero_t());
        return res;
    }
    void Reset() const noexcept
    {
        m_piter = (byte *)m_buffer;
    }
    byte *get_Buffer() noexcept
    {
        return (byte *)m_buffer;
    }
    const byte *get_Buffer() const noexcept
    {
        return (byte *)m_buffer;
    }

private:
    // Serialization:
    template <std::size_t IND> 
    void InternalSerialize(const Data_t& data, NZero_t&&)
    {
        Insert(data.Get<IND>());

        using Arg_t = std::conditional_t<(IND > 0), NZero_t, Zero_t>;
        InternalSerialize<IND - 1>(data, Arg_t());
    }
    template <std::size_t IND> 
    void InternalSerialize(const Data_t& data, Zero_t&&)
    { }
    template <class T> void Insert(const T& arg)
    {
        *reinterpret_cast<T *>(m_piter) = arg;
        m_piter += sizeof(T);
    }
    void Insert(const char *str)
    {
        std::size_t length = std::strlen(str) + 1;
        std::memcpy(m_piter, str, sizeof(char) * length);
        m_piter += sizeof(char) * length;
    }
    void Insert(const std::string& str)
    {
        Insert(str.c_str());
    }

    // Deserialization:
    template <std::size_t IND>
    void InternalDeserialize(Data_t *pobj, NZero_t&&) const
    {
        using T = typename TypeAt<IND, TArgs...>::Result_t;
        T temp;
        Extract(temp);
        pobj->Set<T>(std::move(temp));

        using Arg_t = std::conditional_t<(IND > 0), NZero_t, Zero_t>;
        InternalDeserialize<IND - 1>(pobj, Arg_t());
    }
    template <std::size_t IND>
    void InternalDeserialize(Data_t *pobj, Zero_t&&) const
    { }
    template<class T> void Extract(T& membr) const
    {
        membr = *reinterpret_cast<T *>(m_piter);
        m_piter += sizeof(T);
    }
    void Extract(const char *& str) const
    {
        const char *psrc = reinterpret_cast<const char *>(m_piter);
        std::size_t length = std::strlen(psrc) + 1;
        auto *ptemp = new char[length];
        std::memcpy(ptemp, psrc, length * sizeof(char));
        str = ptemp;
        m_piter += length * sizeof(char);
    }
    void Extract(const std::string& str) const
    {
        const char *cstr;
        Extract(cstr);
        str = std::string(cstr, std::strlen(cstr) + 1);
    }

    byte            m_buffer[SIZE];
    mutable byte    *m_piter;
};
