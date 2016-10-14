// https://msdn.microsoft.com/en-us/library/windows/desktop/bb530742(v=vs.85).aspx

#pragma once
#include <mutex>
#include <winsock2.h>
#include <WS2tcpip.h>
#include "Event.h"

#pragma comment(lib, "Ws2_32.lib")

// TData - type to be sent over TCP, must derive from ISerializable
template<class TRequest, class TResponse> 
class Listener
{
public:
    Listener() :m_listenSock(INVALID_SOCKET)
    {
        WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 2);
        if (WSAStartup(wVersionRequested, &wsaData)) {
            std::cerr << "WSAStartup error" << std::endl;
            WSACleanup();
            exit(1);
        }
    }
    void Start(int portNum)
    {
        m_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_listenSock) {
            std::cerr << "Socket error: " << WSAGetLastError() << std::endl;
            WSACleanup();
            exit(1);
        }

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons((USHORT)portNum);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(m_listenSock, (sockaddr *)&addr, sizeof(addr))) {
            std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
            closesocket(m_listenSock);
            WSACleanup();
            exit(1);
        }

        if (listen(m_listenSock, SOMAXCONN_HINT(200))) {
            std::cerr << "Socket listening fail: " << WSAGetLastError() << std::endl;
            closesocket(m_listenSock);
            WSACleanup();
            exit(1);
        }

        // accepting:
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms737525(v=vs.85).aspx
        // http://www.codeproject.com/Articles/412511/Simple-client-server-network-using-Cplusplus-and-W
    }

private:
    SOCKET m_listenSock;
    // ... sockets etc ...
};