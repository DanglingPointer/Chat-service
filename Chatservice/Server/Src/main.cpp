#include "WebClient.h"
#include <iterator>
#include <algorithm>
#include <string>
#include <chrono>
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

int foo(int id)
{
    int temp = id * 200;
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return ++temp;
}

int main()
{
    std::vector<long long> times;

    for (int j = 0; j < 100; ++j) {
        auto started = std::chrono::high_resolution_clock::now();

        {
            ThreadPool<10> tp(0);

            for (int i = 0; i < 50; ++i)
                tp.AddTask(foo, i);

        }
        auto done = std::chrono::high_resolution_clock::now();

        times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(done - started).count());
    }

    double sum = 0;
    for (long long val : times)
        sum += val;

    std::cout << "\n\nAverage time: " << sum / 100;

    ////Datagram temp(99, "kkk", 23.45);

    ////std::cout << temp.Get<INTMEMBER>() << ' ' << temp.Get<STRMEMBER>() <<
    ////    ' ' << temp.Get<DOUBLEMEMBER>() << std::endl;

    ////temp.Set<INTMEMBER>(1);
    ////temp.Set<STRMEMBER>("aaa");

    ////std::cout << temp.Get<INTMEMBER>() << ' ' << temp.Get<STRMEMBER>() <<
    ////    ' ' << temp.Get<DOUBLEMEMBER>() << std::endl;

    ////Serializer<Datagram, sizeof("kkk")> srz;
    ////srz.Serialize(temp);

    ////auto temp2 = srz.Deserialize();

    ////std::cout << temp2.Get<INTMEMBER>() << ' ' << temp2.Get<STRMEMBER>() <<
    ////    ' ' << temp2.Get<DOUBLEMEMBER>() << std::endl;

    ////auto *p = temp2.Get<STRMEMBER>();
    ////delete[] p;


    //std::vector<std::future<void>> fvec;
    //Square s = (Square)((byte)Square::WUMPUS | (byte)Square::GOLD);

    //std::cout << "SQUARE IS " << (byte)Square::WUMPUS << std::endl;

    //for (int i = 0; i < 10; ++i)
    //    fvec.emplace_back(std::async(std::launch::async, [i](){ foo(i); }));
    //std::cout << "Main thread\n";

    ////for (auto& f : fvec)
    ////    f.wait();

    ////for (auto it = fvec.begin(); it != fvec.end(); ++it) {
    ////    if (it->wait_for(std::chrono::seconds::zero()) == std::future_status::ready)
    ////        std::cout << "Future is ready\n";
    ////}

    //while (fvec.size()) {
    //    //std::this_thread::sleep_for(std::chrono::seconds(1));
    //    auto it = std::remove_if(fvec.begin(), fvec.end(), [](auto& f) { return f.wait_for(std::chrono::seconds(1)) == std::future_status::ready; });
    //    fvec.erase(it, fvec.end());
    //    std::cout << "fvec length is now: " << fvec.size() << std::endl;
    //}

    ////// Alternatively:
    ////for (auto it = fvec.begin(); it != fvec.end();) {
    ////    if (it->wait_for(std::chrono::seconds::zero()) == std::future_status::ready)
    ////        it = fvec.erase(it);
    ////}

}