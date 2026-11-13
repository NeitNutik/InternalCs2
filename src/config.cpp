#include "toker/config.hpp"
#include <fstream>
#include <stdexcept>

namespace toker {
using nlohmann::json;
namespace {
void require(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
void range(float v,float lo,float hi){require(std::isfinite(v)&&v>=lo&&v<=hi,"Setting outside allowed range");}
json read_json(const std::filesystem::path& p){std::ifstream f(p);if(!f)throw std::runtime_error("Cannot open "+p.string());return json::parse(f);}
template<class T>void get(const json& j,const char* k,T& v){if(j.contains(k))j.at(k).get_to(v);}
}
json settings_json(const Settings& s){
    return {{"schema_version",1},{"esp",s.esp},{"enemies_only",s.enemies_only},{"snapline",s.snapline},{"spline",s.spline},
    {"distance",s.distance},{"health",s.health},{"armour",s.armour},{"weapon",s.weapon},{"ammo",s.ammo},{"names",s.names},
    {"box_style",s.box_style},{"max_distance_m",s.max_distance_m},{"radar",s.radar},{"speed",s.speed},{"fov_circle",s.fov_circle},
    {"radar_range_m",s.radar_range_m},{"radar_size",s.radar_size},{"enemy_color",s.enemy_color},{"team_color",s.team_color},
    {"aim",{{"enabled",s.aim.enabled},{"enemies_only",s.aim.enemies_only},{"spotted_only",s.aim.spotted_only},
    {"point",int(s.aim.point)},{"key",s.aim.key},{"fov",s.aim.fov},{"smoothing_seconds",s.aim.smoothing_seconds},
    {"max_distance_m",s.aim.max_distance_m},{"mouse_gain",s.aim.mouse_gain},{"max_step",s.aim.max_step}}}};
}
bool load_settings(const std::filesystem::path& p,Settings& out,std::string& error){
    try{
        const auto j=read_json(p); require(j.at("schema_version")==1,"Unsupported settings schema"); Settings s;
#define READ(name) get(j,#name,s.name)
        READ(esp);READ(enemies_only);READ(snapline);READ(spline);READ(distance);READ(health);READ(armour);READ(weapon);READ(ammo);READ(names);
        READ(box_style);READ(max_distance_m);READ(radar);READ(speed);READ(fov_circle);READ(radar_range_m);READ(radar_size);READ(enemy_color);READ(team_color);
#undef READ
        if(j.contains("aim")){
            const auto& a=j.at("aim");int point=int(s.aim.point);
#define READ(name) get(a,#name,s.aim.name)
            READ(enabled);READ(enemies_only);READ(spotted_only);READ(key);READ(fov);READ(smoothing_seconds);READ(max_distance_m);READ(mouse_gain);READ(max_step);
#undef READ
            get(a,"point",point);require(point>=0&&point<=2,"Invalid hit point");s.aim.point=HitPoint(point);
        }
        require(s.box_style>=0&&s.box_style<=2,"Invalid box style");require(s.aim.key>=1&&s.aim.key<=254&&s.aim.key!=0x2D&&s.aim.key!=0x23,"Invalid aim key (Insert/End are reserved)");
        range(s.max_distance_m,1,1000);range(s.radar_range_m,5,500);range(s.radar_size,150,500);
        range(s.aim.fov,1,1000);range(s.aim.mouse_gain,0.01f,10);range(s.aim.max_step,1,200);range(s.aim.smoothing_seconds,0,1);range(s.aim.max_distance_m,1,1000);
        for(auto c:s.enemy_color)range(c,0,1);
        for(auto c:s.team_color)range(c,0,1);
        out=s;error.clear();return true;
    }catch(const std::exception& e){error=e.what();return false;}
}
bool save_settings(const std::filesystem::path& p,const Settings& s,std::string& error){
    try{
        std::filesystem::create_directories(p.parent_path());auto tmp=p;tmp+=".tmp";
        {std::ofstream f(tmp);if(!f)throw std::runtime_error("Cannot create config temporary file");f<<settings_json(s).dump(2)<<'\n';f.flush();if(!f)throw std::runtime_error("Config write failed");}
        // Keep a backup on Windows, where rename does not replace an existing file.
        auto backup=p;backup+=".bak";std::error_code ec;std::filesystem::remove(backup,ec);
        const bool existed=std::filesystem::exists(p);if(existed)std::filesystem::rename(p,backup);
        try{std::filesystem::rename(tmp,p);}catch(...){if(existed)std::filesystem::rename(backup,p);throw;}
        error.clear();return true;
    }catch(const std::exception& e){error=e.what();return false;}
}
}
