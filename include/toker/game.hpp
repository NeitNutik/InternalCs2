#pragma once
#include "layout.hpp"
#include <windows.h>
#include <memory>

namespace toker {
class Game {
public:
    explicit Game(std::filesystem::path path):path_(std::move(path)){}
    ~Game(){detach();}
    void reload();void poll_reload();Frame sample();
    std::uint32_t pid()const{return layout_.enabled?layout_.pid:0;}
    const Layout& layout()const{return layout_;}
    const std::string& status()const{return status_;}
private:
    bool attach();void detach();bool identity_matches()const;
    Address root(const char* name)const;
    Vec3 position(Address object,const char* name)const;
    int field(Address object,const char* name)const;
    std::filesystem::path path_;std::filesystem::file_time_type modified_{};
    Layout layout_;std::string status_;HANDLE process_{};std::unique_ptr<GuestMemory> memory_;
    ULONGLONG last_poll_{},last_sample_{};Address previous_local_{};Vec3 previous_origin_{};
};
}
