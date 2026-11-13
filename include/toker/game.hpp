#pragma once
#include "config.hpp"
#include <windows.h>

namespace toker {
class Game {
public:
    explicit Game(std::filesystem::path offsets_path):path_(std::move(offsets_path)){}
    bool reload();
    void poll_reload();
    Frame sample();
    bool aim(const Frame&,const AimOptions&,float dt,bool menu_open);
    const std::string& error()const{return error_;}
private:
    std::uintptr_t entity(std::uintptr_t list,std::uint32_t handle)const;
    bool validate_modules();
    bool bones(std::uintptr_t node,Player&)const;
    std::filesystem::path path_;
    std::filesystem::file_time_type modified_{};
    Offsets offsets_;
    bool loaded_{},valid_{};
    std::uintptr_t client_{},engine_{};
    std::size_t client_size_{},engine_size_{};
    std::string error_;
    ULONGLONG last_poll_{};
};
}
