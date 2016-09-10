#pragma once
#include<thread>
#include<forward_list>
#include<stdexcept>
#include<mutex>
#include<iostream>

template<
    class TArg,          // event argument type
    class Caller,       // type firing the event
    class Ret = void>   // return type
    class Event
{
public:
    typedef Event<TArg, Caller, Ret> My_t;
    typedef TArg Arg_t;
    typedef Caller Caller_t;
    typedef Ret Ret_t;
    typedef std::function<Ret_t(const Arg_t&, Caller_t*)> Func_t;

    Event() :m_handlers()
    { }
    Event(const My_t&) = delete;
    My_t operator=(const My_t&) = delete;

    template<typename T> void AddHandler(const T& foo)
    {
        m_handlers.push_front(Func_t(foo));
    }
    template<typename T> void RemoveHandler(const T& foo)
    {
        Func_t temp(foo);
        m_handlers.remove_if([&temp](const Func_t& f) {
            return f.target<Ret_t(const Arg_t&, Caller_t*)>() == temp.target<Ret_t(const Arg_t&, Caller_t*)>();
        });
    }
    void RemoveAll()
    {
        m_handlers.clear();
    }

protected:
    Ret_t Invoke(const Arg_t& arg, Caller_t *pobj) const
    {
        if (m_handlers.begin() != m_handlers.end())
            return (m_handlers.begin())->operator()(arg, pobj);
        throw std::out_of_range("");
    }
    void Fire(const Arg_t& arg, Caller_t *pobj) const
    {
        for (const auto& handler : m_handlers)
            if (handler)
                handler(arg, pobj);
    }
    void FireAsync(const Arg_t& arg, Caller_t *pobj) const
    {
        for (const auto& handler : m_handlers)
            if (handler)
                std::thread(handler, arg, pobj).detach();
    }

private:
    std::forward_list<Func_t> m_handlers;
};

//// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \\
//// +                                                            + \\
//// +               Example use of Event class                   + \\
//// +                                                            + \\
//// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \\
//
//std::mutex cout_mutex;
//
//class Worker : public Event<int, Worker>,
//    public Event<char, Worker>
//{
//public:
//    typedef Event<int, Worker> IntEvent;
//    typedef Event<char, Worker> CharEvent;
//
//    void Run()
//    {
//        IntEvent::FireAsync(123, this);
//        CharEvent::FireAsync('a', this);
//    }
//    void foo1(int i, Worker* p)
//    {
//        using namespace std::chrono_literals;
//
//        std::this_thread::sleep_for(std::chrono::seconds(2));
//
//        {
//            std::lock_guard<std::mutex> guard(cout_mutex);
//            std::cout << "Foo1: " << i << std::endl;
//        }
//
//        std::this_thread::sleep_for(std::chrono::seconds(2));
//
//        {
//            std::lock_guard<std::mutex> guard(cout_mutex);
//            std::cout << "Foo1: " << i << std::endl;
//        }
//
//    }
//};