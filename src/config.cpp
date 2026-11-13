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
    {"max_distance_m",s.aim.max_distance_m}}}};
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
            READ(enabled);READ(enemies_only);READ(spotted_only);READ(key);READ(fov);READ(smoothing_seconds);READ(max_distance_m);
#undef READ
            get(a,"point",point);require(point>=0&&point<=2,"Invalid hit point");s.aim.point=HitPoint(point);
        }
        require(s.box_style>=0&&s.box_style<=2,"Invalid box style");require(s.aim.key>=1&&s.aim.key<=254&&s.aim.key!=0x2D,"Invalid aim key (Insert is reserved)");
        range(s.max_distance_m,1,1000);range(s.radar_range_m,5,500);range(s.radar_size,150,500);
        range(s.aim.fov,0.1f,45);range(s.aim.smoothing_seconds,0,1);range(s.aim.max_distance_m,1,1000);
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
bool load_offsets(const std::filesystem::path& p,Offsets& out,std::string& error){
    try{
        const auto j=read_json(p);require(j.at("schema_version")==1,"Unsupported offsets schema");Offsets o;
        o.build=j.at("build_number").get<int>();require(o.build>0,"Invalid build number");o.commit=j.at("source_commit").get<std::string>();
        const auto table=[&](const char* group,std::map<std::string,std::uintptr_t>& dest,std::initializer_list<const char*> keys,std::uint64_t max){
            for(auto k:keys){const auto& v=j.at(group).at(k);require(v.is_number_integer(),"Offset must be an integer");const auto n=v.get<std::int64_t>();require(n>=0&&std::uint64_t(n)<=max,"Offset outside range");dest[k]=static_cast<std::uintptr_t>(n);}
        };
        table("modules",o.modules,{"entity_list","local_pawn","local_controller","view_matrix","view_angles","build_number"},0x40000000);
        table("fields",o.fields,{"health","max_health","team","life_state","scene_node","origin","dormant","pawn_handle","player_name","armour","immunity","view_offset","weapon_services","active_weapon","clip","attribute_manager","item","item_definition","spotted_state","spotted","spotted_mask","model_state","velocity"},0x100000);
        table("layout",o.layout,{"entity_chunks","entity_stride","entity_chunk_shift","entity_chunk_mask","entity_index_mask","max_players","bones_in_model_state","bone_stride","head_bone","chest_bone","spine_bone"},0x100000);
        require(o.l("entity_chunk_shift")>=1&&o.l("entity_chunk_shift")<=16,"Invalid entity chunk shift");
        require(o.l("entity_chunk_mask")==((std::uintptr_t(1)<<o.l("entity_chunk_shift"))-1),"Invalid entity chunk mask");
        require(o.l("entity_stride")>=sizeof(void*)&&o.l("entity_stride")<=512,"Invalid entity stride");
        require(o.l("bone_stride")>=sizeof(Vec3)&&o.l("bone_stride")<=256,"Invalid bone stride");
        require(o.l("max_players")>=1&&o.l("max_players")<=64,"Invalid player count");
        require(o.l("head_bone")<256&&o.l("chest_bone")<256&&o.l("spine_bone")<256,"Invalid bone index");
        out=std::move(o);error.clear();return true;
    }catch(const std::exception& e){error=e.what();return false;}
}
}
