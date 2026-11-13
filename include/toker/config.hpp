#pragma once
#include "core.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <map>

namespace toker {
struct Settings {
    bool esp{true},enemies_only{true},snapline{false},spline{false},distance{true};
    bool health{true},armour{true},weapon{true},ammo{true},names{true};
    int box_style{1}; // 0 off, 1 full, 2 corner
    float max_distance_m{150};
    bool radar{true},speed{false},fov_circle{true};
    float radar_range_m{60},radar_size{220};
    std::array<float,4> enemy_color{1.0f,0.38f,0.34f,1.0f};
    std::array<float,4> team_color{0.32f,0.73f,1.0f,1.0f};
    AimOptions aim;
};
bool load_settings(const std::filesystem::path&,Settings&,std::string&);
bool save_settings(const std::filesystem::path&,const Settings&,std::string&);
nlohmann::json settings_json(const Settings&);
}
