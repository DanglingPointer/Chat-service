#include "WebClient.h"
#include <iterator>
#include <algorithm>
#include <string>
#include "Util.h"

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

struct Astruct
{
    int mem[24];
};

int main()
{
    Datagram temp(99, "kkk", 23.45);

    std::cout << temp.Get<INTMEMBER>() << ' ' << temp.Get<STRMEMBER>() <<
        ' ' << temp.Get<DOUBLEMEMBER>() << std::endl;

    temp.Set<INTMEMBER>(1);
    temp.Set<STRMEMBER>("aaa");

    std::cout << temp.Get<INTMEMBER>() << ' ' << temp.Get<STRMEMBER>() <<
        ' ' << temp.Get<DOUBLEMEMBER>() << std::endl;

    Serializer<Datagram, sizeof("kkk")> srz;
    srz.Serialize(temp);

    auto temp2 = srz.Deserialize();

    std::cout << temp2.Get<INTMEMBER>() << ' ' << temp2.Get<STRMEMBER>() <<
        ' ' << temp2.Get<DOUBLEMEMBER>() << std::endl;

    auto *p = temp2.Get<STRMEMBER>();
    delete p;

	system("pause");
}