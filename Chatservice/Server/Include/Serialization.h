#pragma once
#include <cstring>
#include <type_traits>
#include <memory>
#include "Util.h"


class Buffer // obsolete?
{
public:
    typedef std::unique_ptr<byte[]> UPtr_t;
    typedef byte *Iter_t;
    typedef std::size_t Len_t;

	explicit Buffer(Len_t length)
		:m_pdata(std::make_unique<byte[]>(length)), m_length(length), m_piter(m_pdata.get())
	{ }
    Buffer(byte *pdata, Len_t length) noexcept 
        : m_pdata(pdata), m_length(length), m_piter(m_pdata.get())
    { }
	Buffer(UPtr_t&& data, Len_t length) noexcept
		: m_pdata(std::move(data)), m_length(length), m_piter(m_pdata.get())
    { }
	Buffer(const Buffer& rhs)
		:m_pdata(std::make_unique<byte[]>(rhs.m_length)), m_length(rhs.m_length), m_piter(nullptr)
	{
        auto pstart = rhs.m_pdata.get();
        auto pend = pstart + rhs.m_length;
		std::copy(pstart, pend, m_pdata.get());

        m_piter = m_pdata.get() + (rhs.m_piter - rhs.m_pdata.get());
	}
	Buffer(Buffer&& rhs) noexcept
		:m_pdata(std::move(rhs.m_pdata)), m_length(rhs.m_length), m_piter(rhs.m_piter)
	{ }
	Buffer& operator=(Buffer&& rhs) noexcept
	{
		m_pdata = std::move(rhs.m_pdata);
        m_length = rhs.m_length;
        m_piter = rhs.m_piter;
		return *this;
	}


protected:
	void IncreaseBy(std::size_t amount)
	{
        auto pos = m_piter - m_pdata.get();

		UPtr_t temp = std::make_unique<byte[]>(m_length + amount);

        auto pstart = m_pdata.get();
        auto pend = pstart + m_length;
		std::copy(pstart, pend, temp.get());

		m_pdata.swap(temp);
        m_length += amount;
        m_piter = m_pdata.get() + pos;
	}
private:
	UPtr_t m_pdata;
    Len_t m_length;
    Iter_t m_piter;
};
