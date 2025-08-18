#pragma once

template<typename T>
class Singleton 
{

public: 
    static T& instance() 
    {
        static T _instance; //thread safe from C++11 that guarantee static instance is initialized only once in case of thread concurrency
        return _instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;
protected:
    Singleton() = default;
    virtual ~Singleton() = default; //allow derived class to run destructor, otherwise only base destructor is run
};

