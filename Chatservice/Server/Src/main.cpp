#include "WebClient.h"
#include <iterator>
#include <algorithm>
#include <string>
#include "Util.h"
#include "ThreadPool.h"

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

enum class Square : byte
{
    EMPTY = 0,
    WUMPUS = 1 << 0,
    GOLD = 1 << 1,
    PIT = 1 << 2,
    STENCH = 1 << 3,
    BREEZE = 1 << 4
};

#include <future>
#include <vector>
#include <chrono>

void foo(int id)
{
    std::cout << "Foo " << id << " started\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Foo " << id << " finished\n";
}

int main()
{
    ThreadPool<6> tp(60);
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


    std::vector<std::future<void>> fvec;
    Square s = (Square)((byte)Square::WUMPUS | (byte)Square::GOLD);

    std::cout << "SQUARE IS " << (byte)Square::WUMPUS << std::endl;

    for (int i = 0; i < 10; ++i)
        fvec.emplace_back(std::async(std::launch::async, [i](){ foo(i); }));
    std::cout << "Main thread\n";

    //for (auto& f : fvec)
    //    f.wait();

    //for (auto it = fvec.begin(); it != fvec.end(); ++it) {
    //    if (it->wait_for(std::chrono::seconds::zero()) == std::future_status::ready)
    //        std::cout << "Future is ready\n";
    //}

    while (fvec.size()) {
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        auto it = std::remove_if(fvec.begin(), fvec.end(), [](auto& f) { return f.wait_for(std::chrono::seconds(1)) == std::future_status::ready; });
        fvec.erase(it, fvec.end());
        std::cout << "fvec length is now: " << fvec.size() << std::endl;
    }

    //// Alternatively:
    //for (auto it = fvec.begin(); it != fvec.end();) {
    //    if (it->wait_for(std::chrono::seconds::zero()) == std::future_status::ready)
    //        it = fvec.erase(it);
    //}

}