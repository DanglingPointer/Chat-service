#include "WebClient.h"
#include <iterator>
#include <algorithm>
#include <string>
#include <chrono>


template<class T> struct  Check;

#include "Util.h"
#include "ThreadPool.h"
#include "DadTP.h"

template <typename T, std::size_t N>
std::ostream& operator << (std::ostream& out, const std::array<T, N>& arr)
{
    for (const auto& c : arr)
        out << arr;
    return out;
}


typedef StructGen<int, const char*, double> Datagram;
enum MemberNames
{
    INTMEMBER = 0, STRMEMBER = 1, DOUBLEMEMBER = 2
};

#include <future>
#include <vector>
#include <chrono>

int foo(int id)
{
    int temp = id * 200;
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return ++temp;
}

void foo2(char arr[])
{
    char *pstr = "blablabla";
    std::strcpy(arr, pstr);
}

int main()
{
    Request req;
    req.Set<TYPE>(23);

    req.Set<ID>("mikhail_vasilyev");

    req.Set<CONTENT>("wazzup?");

    std::cout << "Type: " << req.Get<TYPE>() << " ID: " << req.Get<ID>()
        << " Content: " << req.Get<CONTENT>() << std::endl;

    ReqSer ser;

    ser.Serialize(req);


    Request req2;

    req2 = ser.Deserialize();

    std::cout << "Type: " << req2.Get<TYPE>() << " ID: " << req2.Get<ID>()
        << " Content: " << req2.Get<CONTENT>() << std::endl;

    system("pause");


    //Datagram temp(99, "kkk", 23.45);

    //std::cout << temp.Get<INTMEMBER>() << ' ' << temp.Get<STRMEMBER>() <<
    //    ' ' << temp.Get<DOUBLEMEMBER>() << std::endl;

    //temp.Set<INTMEMBER>(1);
    //temp.Set<STRMEMBER>("aaa");

    //std::cout << temp.Get<INTMEMBER>() << ' ' << temp.Get<STRMEMBER>() <<
    //    ' ' << temp.Get<DOUBLEMEMBER>() << std::endl;

    //Serializer<Datagram, sizeof("kkk")> srz;
    //srz.Serialize(temp);

    //auto temp2 = srz.Deserialize();

    //std::cout << temp2.Get<INTMEMBER>() << ' ' << temp2.Get<STRMEMBER>() <<
    //    ' ' << temp2.Get<DOUBLEMEMBER>() << std::endl;

    //auto *p = temp2.Get<STRMEMBER>();
    //delete[] p;
}