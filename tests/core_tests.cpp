#include "toker/config.hpp"
#include <fstream>
#include <iostream>
#include <limits>
#include <chrono>

namespace {
int checks{};
void check(bool ok,const char* name){++checks;if(!ok)throw std::runtime_error(name);}
bool near(float a,float b,float epsilon=0.001f){return std::abs(a-b)<epsilon;}
void write(const std::filesystem::path& p,const nlohmann::json& j){std::ofstream(p)<<j.dump(2);}
}
int main(){
    using namespace toker;
    const auto stamp=std::chrono::steady_clock::now().time_since_epoch().count();
    const auto temp=std::filesystem::temp_directory_path()/("toker-tests-"+std::to_string(stamp));
    std::filesystem::create_directories(temp);
    try{
        Matrix m{};m.m[0][0]=m.m[1][1]=m.m[3][3]=1;
        auto p=project({0,0,0},m,{1920,1080});check(p&&near(p->x,960)&&near(p->y,540),"projection center");
        p=project({1,1,0},m,{1920,1080});check(p&&near(p->x,1920)&&near(p->y,0),"projection corners");
        m.m[3][3]=-1;check(!project({0,0,0},m,{1920,1080}),"behind camera rejected");
        m.m[3][3]=1;check(!project({std::numeric_limits<float>::quiet_NaN(),0,0},m,{1920,1080}),"NaN projection rejected");
        check(near(angular_distance({0,179,0},{0,-179,0}),2),"yaw wrap chooses short arc");
        auto a=angle_to({0,0,0},{100,100,100});check(near(a.y,45)&&near(a.x,-35.26439f),"3D look angle");
        a=smooth_angles({0,179,0},{0,-179,0},0.1f,0.01f);check(a.y>179&&a.y<180,"smoothing crosses yaw boundary correctly");
        auto fast=smooth_angles({0,0,0},{20,50,0},0.2f,0.02f);
        auto slow=smooth_angles({0,0,0},{20,50,0},0.2f,0.01f);slow=smooth_angles(slow,{20,50,0},0.2f,0.01f);
        check(near(fast.x,slow.x)&&near(fast.y,slow.y),"smoothing independent of frame subdivision");
        auto r=radar_position({100,0,0},0,200,100);check(near(r.x,0)&&near(r.y,-50),"radar forward is up");
        r=radar_position({0,100,0},0,200,100);check(near(r.x,-50)&&near(r.y,0),"radar world left is screen left");
        r=radar_position({0,100,0},90,200,100);check(near(r.x,0)&&near(r.y,-50),"radar follows yaw");
        r=radar_position({1000,1000,0},0,100,100);check(near(std::hypot(r.x,r.y),100),"radar clamps range");
        Frame f;f.ready=true;f.team=2;Player enemy;enemy.handle=10;enemy.health=100;enemy.team=3;enemy.spotted_by_local=true;
        enemy.head={100,0,0};enemy.chest={100,5,0};enemy.spine={100,10,0};f.players={enemy};AimOptions opt;
        auto selected=select_target(f,opt);check(selected&&selected->handle==10,"select target in FOV");
        opt.point=HitPoint::Chest;selected=select_target(f,opt);check(selected&&near(selected->point.y,5),"select requested bone");opt.point=HitPoint::Head;
        f.players[0].team=2;check(!select_target(f,opt),"ignore teammates");f.players[0].team=3;
        f.players[0].spotted_by_local=false;check(!select_target(f,opt),"spotted filter");f.players[0].spotted_by_local=true;
        f.players[0].immune=true;check(!select_target(f,opt),"ignore immunity");f.players[0].immune=false;
        f.players[0].health=0;check(!select_target(f,opt),"ignore dead players");f.players[0].health=100;
        f.angles.y=15;check(!select_target(f,opt),"FOV rejects distant angle");f.angles.y=0;
        opt.max_distance_m=1;check(!select_target(f,opt),"metric range limit");opt.max_distance_m=150;
        f.ready=false;check(!select_target(f,opt),"no selection before ready");f.ready=true;
        Player closer=enemy;closer.handle=11;f.players[0].head.y=2;f.players.push_back(closer);
        check(select_target(f,opt)->handle==11,"smallest angular error wins");
        Settings s;s.aim.key=2;s.spline=true;s.box_style=2;std::string error;const auto config=temp/"settings.json";
        check(save_settings(config,s,error),"save config");Settings loaded;
        check(load_settings(config,loaded,error)&&loaded.spline&&loaded.box_style==2&&loaded.aim.key==2,"JSON round trip");
        s.aim.key=1;check(save_settings(config,s,error)&&std::filesystem::exists(config.string()+".bak"),"replacement keeps backup");
        auto j=settings_json(s);j["aim"]["fov"]=1000;write(config,j);
        const auto before=settings_json(loaded);check(!load_settings(config,loaded,error)&&settings_json(loaded)==before,"invalid config leaves active settings unchanged");
        j=settings_json(s);j["aim"]["key"]=45;write(config,j);check(!load_settings(config,loaded,error),"reserved menu key rejected");
        j=settings_json(s);j["enemy_color"]={1,2,3,4};write(config,j);check(!load_settings(config,loaded,error),"invalid colors rejected");
        Offsets offsets;check(load_offsets("config/offsets.json",offsets,error),error.c_str());
        std::ifstream source("config/offsets.json");source>>j;j["layout"]["entity_chunk_shift"]=64;write(temp/"offsets.json",j);
        check(!load_offsets(temp/"offsets.json",offsets,error),"bad shift rejected before address calculation");
        j["layout"]["entity_chunk_shift"]=9;j["modules"]["entity_list"]=-1;write(temp/"offsets.json",j);
        check(!load_offsets(temp/"offsets.json",offsets,error),"negative address rejected");
        std::ofstream(config)<<"{broken";check(!load_settings(config,loaded,error),"malformed JSON rejected");
        std::filesystem::remove_all(temp);std::cout<<checks<<" checks passed\n";return 0;
    }catch(const std::exception& e){std::cerr<<"FAILED after "<<checks<<" checks: "<<e.what()<<'\n';std::filesystem::remove_all(temp);return 1;}
}
