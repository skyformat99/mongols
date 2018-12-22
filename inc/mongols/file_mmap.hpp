#ifndef FILE_MMAP_HPP
#define FILE_MMAP_HPP

#include <string>
#include <utility>
#include <unordered_map>
#include <sys/stat.h>

namespace mongols{
    class file_mmap {
    public:
        file_mmap();
        virtual~file_mmap();
        bool get(const std::string&,std::pair<char*, struct stat>&);
        std::pair<char*, struct stat> get(const std::string&);
    private:
        std::unordered_map<std::string,std::pair<char*, struct stat>> data;
    };
}

#endif /* FILE_MMAP_HPP */

