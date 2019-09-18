#pragma once

#include <functional>
#include <any>

template<typename Ret>
struct callable
{
    callable() {}
    template<typename F>
    callable(F&& fun) : callable(std::function(fun)) {}

    template<typename ... Args>
    callable(std::function<Ret(Args...)> fun) : func_(fun) {}
    
    template<typename ... Args>
    Ret operator()(Args&& ... args) 
    { 
        return std::invoke(std::any_cast<std::function<Ret(Args...)>>(func_), std::forward<Args>(args)...); 
    }
    std::any func_;
};

template<>
struct callable<void>
{
    callable() {}
    
    template<typename F>
    callable(F&& fun) : callable(std::function(fun)) {}

    template<typename ... Args>
    callable(std::function<void(Args...)> fun) : func_(fun) {}
    
    template<typename ... Args>
    void operator()(Args&& ... args) 
    { 
        std::invoke(std::any_cast<std::function<void(Args...)>>(func_), std::forward<Args>(args)...); 
    }
    std::any func_;
};
