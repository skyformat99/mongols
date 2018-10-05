#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <thread>
#include <functional>
#include <vector>
#include <atomic>
#include <iostream>

#include "safe_queue.hpp"


namespace mongols {

    class join_thread {
    private:
        std::vector<std::thread>& th;
    public:

        join_thread(std::vector<std::thread>& th) : th(th) {
        }

        virtual~join_thread() {
            for (auto& i : this->th) {
                if (i.joinable()) {
                    i.join();
                }
            }
        }

    };

    class thread_pool {
    private:
        safe_queue<std::function<bool() > > q;
        std::vector<std::thread> th;
        join_thread joiner;

        void work() {
            std::function<bool() > task;
            while (true) {
                this->q.wait_and_pop(task);
                if (task()) {
                    break;
                };
                std::this_thread::yield();
            }
        }

        void shutdown() {
            for (size_t i = 0; i < this->th.size(); ++i) {
                this->submit([]() {
                    return true;
                });
            }
        }
    public:

        thread_pool(size_t th_size = std::thread::hardware_concurrency()) : q(), th(), joiner(th) {
            try {
                for (size_t i = 0; i < th_size; ++i) {
                    this->th.push_back(std::move(std::thread(std::bind(&thread_pool::work, this))));
                }
            } catch (...) {
                this->shutdown();
            }
        }

        virtual~thread_pool() {
            this->shutdown();
        }

        template<typename F>
        void submit(F f) {
            if (!this->th.empty()) {
                this->q.push(f);
            }
        }

        size_t size()const {
            return this->th.size();
        }

        bool empty()const {
            return this->th.empty();
        }

    };
}

#endif /* THREAD_POOL_HPP */

