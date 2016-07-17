#pragma once
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Event.h"
#include "ByteBuffer.h"

#pragma comment(lib, "Ws2_32.lib")

// TData - type to be sent over TCP, must derive from ISerializable
template<class TData> class WebClient
{
public:
    typedef typename TData::BufferType TBuffer;
    typedef typename TData::MySerType TDgram;


    WebClient() :m_buffer(), m_writeLock()
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD(2, 2);
        if (WSAStartup(wVersionRequested, &wsaData)) {
            std::cout << "WSAStratup error" << std::endl;
            exit(1);
        }
    }

    void SendDgram(TDgram& dgram)
    {
        std::lock_guard<std::mutex> lock(m_writeLock);

        dgram.OnSerialize(&m_buffer);

        byte bufferCopy[TBuffer::SIZE];

        m_buffer.CopyBufferTo(bufferCopy, m_buffer.SIZE);

        // ... send bufferCopy to TCP stream ...
    }

    void RecvDgram(TDgram *poutmsg)
    {
        // ... retrieve a byte array from the tcp stream...
        
        int length = TBuffer::SIZE;
        byte *pdata = new byte[length];

        m_buffer.SetBuffer(pdata, length);

        poutmsg->OnDeserialize(&m_buffer);
    }

private:
    TBuffer m_buffer;
    std::mutex m_writeLock;

    // ... sockets etc ...
};