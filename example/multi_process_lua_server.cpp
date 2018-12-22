#include <unistd.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <mongols/util.hpp>
#include <mongols/lib/hash/sha1.hpp>
#include <mongols/lib/hash/md5.hpp>
#include <mongols/lua_server.hpp>

#include <iostream>
#include <algorithm>

class person {
public:

    person() : name("Tom"), age(0) {
    }
    virtual~person() = default;

    void set_name(const std::string& name) {
        this->name = name;
    }

    void set_age(unsigned int age) {
        this->age = age;
    }

    const std::string& get_name() const {
        return this->name;
    }

    unsigned int get_age() {
        return this->age;
    }
private:
    std::string name;
    unsigned int age;
};

class studest : public person {
public:

    studest() : person() {
    }
    virtual~studest() = default;

    double get_score() {
        return this->score;
    }

    void set_score(double score) {
        this->score = score;
    }
private:
    double score;
};

int main(int, char**) {
    //    daemon(1, 0);

    int port = 9090;
    const char* host = "127.0.0.1";
    mongols::lua_server
    server(host, port, 5000, 8096, 0/*2*/);
    server.set_root_path("html/lua");
    server.set_enable_bootstrap(true);
    server.set_enable_lru_cache(true);
    server.set_lru_cache_expires(1);

    server.set_function(&mongols::sha1, "sha1");
    server.set_function(&mongols::md5, "md5");

    server.set_class(
            kaguya::UserdataMetatable<person>()
            .setConstructors < person()>()
            .addFunction("get_age", &person::get_age)
            .addFunction("get_name", &person::get_name)
            .addFunction("set_age", &person::set_age)
            .addFunction("set_name", &person::set_name)
            , "person");

    server.set_class(
            kaguya::UserdataMetatable<studest, person>()
            .setConstructors < studest()>()
            .addFunction("get_score", &studest::get_score)
            .addFunction("set_score", &studest::set_score)
            , "studest");


    std::function<void(pthread_mutex_t*, size_t*) > f = [&](pthread_mutex_t* mtx, size_t * data) {
        prctl(PR_SET_NAME, "mongols: worker");
        std::string i;
        pthread_mutex_lock(mtx);
        if (*data > std::thread::hardware_concurrency() - 1) {
            *data = 0;
        }
        i = std::move(std::to_string(*data));
        *data = (*data) + 1;
        pthread_mutex_unlock(mtx);
        server.set_db_path("html/leveldb/" + i);
        server.run("html/lua/package/?.lua;", "html/lua/package/?.so;");
    };

    std::function<bool(int) > g = [&](int status) {
        std::cout << strsignal(WTERMSIG(status)) << std::endl;
        return false;
    };

    mongols::multi_process main_precess;
    main_precess.run(f, g);


}


