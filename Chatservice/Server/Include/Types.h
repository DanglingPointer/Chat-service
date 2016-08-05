#pragma once

typedef unsigned char	byte;
typedef char			sbyte;

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