#pragma once
#include<type_traits>

typedef unsigned char	byte;
typedef char			sbyte;

#if 0
// Helper used for determining the total size of template args pack
template<typename... Args> struct Size;
template<> struct Size<>
{
	static constexpr std::size_t value = 0;
};
template<typename T, typename... Args> struct Size<T, Args...>
{
	static constexpr std::size_t value = sizeof(T) + Size<Args...>::value;
};
//----------------------------------------------------------------------------------------------

template<class... TArgs> struct Typelist
{ };
//----------------------------------------------------------------------------------------------

template<class TList, int INDEX> struct GetType;

template<class TFirst, class... TArgs, int INDEX>
struct GetType<Typelist<TFirst, TArgs...>, INDEX>
{
    typedef typename GetType<Typelist<TArgs...>, INDEX - 1>::type type;
};

template<class TFirst, class... TArgs>
struct GetType<Typelist<TFirst, TArgs...>, 0>
{
    typedef TFirst type;
};
//----------------------------------------------------------------------------------------------
// Get sub-typelist starting from INDEX
template<class TList, int INDEX> struct GetSublist;

template<class TFirst, class... TArgs, int INDEX>
struct GetSublist<Typelist<TFirst, TArgs...>, INDEX>
{
    typedef typename GetSublist<Typelist<TArgs...>, INDEX - 1>::type type;
};
template<class TFirst, class... TArgs>
struct GetSublist<Typelist<TFirst, TArgs...>, 0>
{
    typedef Typelist<TFirst, TArgs...> type;
};
//----------------------------------------------------------------------------------------------

template<class T, class TList> struct GetIndex;

template<class T, class TFirst, class... TArgs>
struct GetIndex<T, Typelist<TFirst, TArgs...>>
{
    static constexpr std::size_t INDEX = GetIndex<T, Typelist<TArgs...>>::INDEX + 1;
};
template<class T, class... TArgs>
struct GetIndex<T, Typelist<T, TArgs...>>
{
    static constexpr std::size_t INDEX = 0;
};
//----------------------------------------------------------------------------------------------

template<class TList> class DataContainer;

template<class TFirst, class... TArgs>
class DataContainer<Typelist<TFirst, TArgs...>> : protected DataContainer<Typelist<TArgs...>>
{
    using List_t = Typelist<TFirst, TArgs...>;
    using My_t = DataContainer<Typelist<TFirst, TArgs...>>;
    using Base_t = DataContainer<Typelist<TArgs...>>;

public:
    DataContainer()                 = default;
    DataContainer(const My_t&)      = default;
    DataContainer(My_t&&)           = default;
    My_t& operator=(const My_t&)    = default;
    My_t& operator=(My_t&&)         = default;

    template<class TFirstArg,
        class... TRestArgs,
        class = std::enable_if_t<
        !std::is_base_of<My_t, std::decay_t<TFirstArg>>::value
        >
    >
    DataContainer(TFirstArg&& mem, TRestArgs&&... mems) 
        : Base_t(std::forward<TRestArgs>(mems)...), member(std::forward<TFirstArg>(mem))
    { }
    template<int INDEX> 
    const auto& Get() const noexcept
    {
        using Sublist_t = typename GetSublist<List_t, INDEX>::type;
        using Parent_t = DataContainer<Sublist_t>;
        return this->Parent_t::member;
    }
    template<int INDEX, class T> 
    void Set(T&& value)
    {
        using Sublist_t = typename GetSublist<List_t, INDEX>::type;
        using Parent_t = DataContainer<Sublist_t>;
        this->Parent_t::member = std::forward<T>(value);
    }

protected:
    TFirst member;
};
template<>
class DataContainer<Typelist<>>
{ 
    using My_t = DataContainer<Typelist<>>;
public:
    DataContainer()                 = default;
    DataContainer(const My_t&)      = default;
    DataContainer(My_t&&)           = default;
    My_t& operator=(const My_t&)    = default;
    My_t& operator=(My_t&&)         = default;
};
#endif
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
// Struct generator (generates class with one member of each type in TArgs and a setter and getter methods)
template<class... TArgs>
class StructGen;

//--------------------------------------------------------------------------------------------------
// Get substruct by index of its first template argument
template<class TStruct, int INDEX>
struct IndSubstruct;

template<class TFirst, class... TRest, int INDEX>
struct IndSubstruct<StructGen<TFirst, TRest...>, INDEX>
{
    typedef typename IndSubstruct<StructGen<TRest...>, INDEX - 1>::Result_t Result_t;
};
template<class TFirst, class... TRest>
struct IndSubstruct<StructGen<TFirst, TRest...>, 0>
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

    template<int INDEX>
    const auto& Get() const noexcept
    {
        using Parent_t = typename IndSubstruct<My_t, INDEX>::Result_t;
        return this->Parent_t::m_var;
    }
    template<class TMember>
    const TMember& Get() const noexcept
    {
        using Parent_t = typename TypeSubstruct<My_t, TMember>::Result_t;
        return this->Parent_t::m_var;
    }

    template<int INDEX, class T>
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
    TFirst m_var;
};

template<>
class StructGen<> { };
