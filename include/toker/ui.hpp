#pragma once
#include "config.hpp"
#include <imgui.h>

namespace toker {
class Ui {
public:
    explicit Ui(std::filesystem::path directory):directory_(std::move(directory)){}
    void initialize();
    void render(const Frame&);
    Settings settings;
    bool menu_open{true};
    bool reload_offsets{};
private:
    void menu(const Frame&);
    void visuals(const Frame&);
    void radar(const Frame&);
    void capture_key();
    std::filesystem::path directory_;
    std::string feedback_;
    bool capture_{},capture_ready_{};
};
}
