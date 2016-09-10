#pragma once
#include <cstring>
#include <type_traits>
#include <memory>
#include "Types.h"

class Buffer
{
public:
	explicit Buffer(std::size_t length)
		:m_pData(std::make_unique<byte[]>(length)), m_pTail(m_pData.get() + length)
	{ }
	Buffer(std::unique_ptr<byte[]>&& data, std::size_t length) noexcept
		: m_pData(std::move(data)), m_pTail(m_pData.get() + length)
	{ }
	Buffer(const Buffer& rhs)
		:m_pData(std::make_unique<byte[]>(rhs.get_Length())), m_pTail(m_pData.get() + rhs.get_Length())
	{
		std::copy(rhs.m_pData.get(), rhs.m_pTail, m_pData.get());
	}
	Buffer& operator=(const Buffer& rhs)
	{
		m_pData.reset(rhs.m_pData.get());
		m_pTail = m_pData.get() + rhs.get_Length();
		return *this;
	}
	Buffer(Buffer&& rhs) noexcept  
		:m_pData(std::move(rhs.m_pData)), m_pTail(rhs.m_pTail)
	{ }
	Buffer& operator=(Buffer&& rhs) noexcept
	{
		m_pData = std::move(rhs.m_pData);
		m_pTail = rhs.m_pTail;
		return *this;
	}
	std::size_t get_Length() const noexcept { return m_pTail - m_pData.get(); }
	byte *get_Tail() const noexcept { return m_pTail; }
	byte *get_Data() const noexcept { return m_pData.get(); }
	byte *get_Data(std::size_t pos) const noexcept { return &m_pData[pos]; }
	void IncreaseBy(std::size_t amount)
	{
		std::size_t prevLen = get_Length();
		std::unique_ptr<byte[]> temp = std::make_unique<byte[]>(prevLen + amount);
		std::copy(m_pData.get(), m_pTail, temp.get());
		m_pData.swap(temp);
		m_pTail = m_pData.get() + prevLen + amount;
	}
private:
	std::unique_ptr<byte[]> m_pData;
	byte *m_pTail;
};
