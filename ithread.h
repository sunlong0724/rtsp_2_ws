#pragma once

#include <chrono>
#include <thread>
class IThread{
public:
    void start(){
        m_running_flag = true;
        m_thread_impl = std::thread([&](){
                                    this->run();
                                    });
    }
    void stop(){
        m_running_flag = false;
        if (m_thread_impl.joinable())
            m_thread_impl.join();
    }
    virtual void run(){
        while(m_running_flag){
            std::this_thread::sleep_for( std::chrono::milliseconds(10) );;
        }
    }

    bool        m_running_flag = false;
private:
    std::thread m_thread_impl;
};



