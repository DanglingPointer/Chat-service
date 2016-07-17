#pragma once
#include<thread>
#include<forward_list>
#include<stdexcept>
#include<mutex>
#include<iostream>

template<
    class Arg,          // event argument type
    class Caller,       // type firing the event
    class Ret = void>   // return type
    class Event
{
public:
    typedef Event<Arg, Caller, Ret> Myt;
    typedef Arg Arg;
    typedef Caller Caller;
    typedef Ret Ret;
    typedef std::function<Ret(const Arg&, Caller*)> Func;

    Event() :m_handlers()
    { }
    Event(const Myt&) = delete;
    Myt operator=(const Myt&) = delete;

    template<typename T> void AddHandler(const T& foo)
    {
        m_handlers.push_front(Func(foo));
    }
    template<typename T> void RemoveHandler(const T& foo)
    {
        Func temp(foo);
        m_handlers.remove_if([&temp](const Func& f) {
            return f.target<Ret(const Arg&, Caller*)>() == temp.target<Ret(const Arg&, Caller*)>();
        });
    }
    void RemoveAll()
    {
        m_handlers.clear();
    }

protected:
    Ret Invoke(const Arg& arg, Caller *pobj) const
    {
        if (m_handlers.begin() != m_handlers.end())
            return (m_handlers.begin())->operator()(arg, pobj);
        throw std::out_of_range("");
    }
    void Fire(const Arg& arg, Caller *pobj) const
    {
        for (const auto& handler : m_handlers)
            if (handler)
                handler(arg, pobj);
    }
    void FireAsync(const Arg& arg, Caller *pobj) const
    {
        for (const auto& handler : m_handlers)
            if (handler)
                std::thread(handler, arg, pobj).detach();
    }

private:
    std::forward_list<Func> m_handlers;
};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \\
// +                                                            + \\
// +               Example use of Event class                   + \\
// +                                                            + \\
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \\

std::mutex cout_mutex;

class Worker : public Event<int, Worker>,
    public Event<char, Worker>
{
public:
    typedef Event<int, Worker> IntEvent;
    typedef Event<char, Worker> CharEvent;

    void Run()
    {
        IntEvent::FireAsync(123, this);
        CharEvent::FireAsync('a', this);
    }
    void foo1(int i, Worker* p)
    {
        using namespace std::chrono_literals;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        {
            std::lock_guard<std::mutex> guard(cout_mutex);
            std::cout << "Foo1: " << i << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));

        {
            std::lock_guard<std::mutex> guard(cout_mutex);
            std::cout << "Foo1: " << i << std::endl;
        }

    }
};

void foo2(char i, Worker* p)
{
    using namespace std::chrono_literals;

    std::this_thread::sleep_for(1s);
    {
        std::lock_guard<std::mutex> guard(cout_mutex);
        std::cout << "Foo2: " << i << std::endl;
    }

    std::this_thread::sleep_for(1s);
    {
        std::lock_guard<std::mutex> guard(cout_mutex);
        std::cout << "Foo2: " << i << std::endl;
    }
}
char *strfoo()
{
    char *str = "This is my string";
    return str;
}