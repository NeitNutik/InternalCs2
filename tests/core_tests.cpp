#include "toker/config.hpp"
#include "toker/layout.hpp"
#include <chrono>
#include <fstream>
#include <iostream>

namespace {
int checks{};
void check(bool value,const char* name){++checks;if(!value)throw std::runtime_error(name);}
bool near(float a,float b){return std::abs(a-b)<0.001f;}
template<class F>bool throws(F f){try{f();return false;}catch(const std::exception&){return true;}}
}
int main(){
    using namespace toker;
    const auto temp=std::filesystem::temp_directory_path()/("toker-standoff-tests-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(temp);
    try{
        std::vector<std::byte> host(0x5000);constexpr Address host_base=0x100000,guest_base=0x1000;
        const HostRead reader=[&](Address at,void* dst,std::size_t n){if(at<host_base||at-host_base>host.size()||n>host.size()-(at-host_base))return false;std::memcpy(dst,host.data()+at-host_base,n);return true;};
        const auto put=[&]<class T>(Address at,const T& value){std::memcpy(host.data()+at-guest_base,&value,sizeof(value));};
        GuestMemory mem({{guest_base,host_base,host.size()}},8,reader);
        put(0x1000,std::uint64_t(0x1100));put(0x1108,std::int32_t(73));
        const std::array<Address,2> path{0,8};check(mem.read<int>(mem.path(0x1000,path))==73,"pointer chain");
        check(throws([&]{mem.read<int>(0x99999);}),"unmapped read fails");
        check(throws([&]{add(UINT64_MAX,1);}),"address overflow fails");
        check(throws([&]{GuestMemory m({{0,100,20},{10,200,20}},8,reader);}),"overlapping regions fail");
        check(throws([&]{GuestMemory m({},3,reader);}),"invalid guest ABI fails");
        put(0x1200,std::uint32_t(0x1337));GuestMemory m32({{guest_base,host_base,host.size()}},4,reader);
        check(m32.pointer(0x1200)==0x1337,"32-bit guest pointers");
        // Packed dump fields: uint16 HP immediately followed by uint16 max HP.
        put(0x1400,std::uint16_t(75));put(0x1402,std::uint16_t(100));
        check(read_scalar(mem,0x1400,ScalarType::U16)==75,"packed health read does not include adjacent field");
        check(mem.read<std::int32_t>(0x1400)!=75,"old int32 read would merge packed fields");
        put(0x1404,std::int16_t(-1));check(read_scalar(mem,0x1404,ScalarType::I16)==-1,"signed magazine sentinel");
        put(0x1406,std::uint8_t(3));put(0x1407,std::uint8_t(255));
        check(read_scalar(mem,0x1406,ScalarType::U8)==3,"byte team read ignores next byte");
        check(read_scalar(mem,0x1407,ScalarType::I8)==-1,"signed byte read");
        GuestMemory narrow({{0x1400,host_base+0x400,2}},8,reader);
        check(read_scalar(narrow,0x1400,ScalarType::U16)==75,"uint16 read at end of mapped page");
        check(throws([&]{read_scalar(narrow,0x1400,ScalarType::I32);}),"wide read at page boundary rejected");
        put(0x1408,std::uint32_t(UINT32_MAX));check(throws([&]{read_scalar(mem,0x1408,ScalarType::U32);}),"uint32 overflow rejected");
        put(0x140c,std::uint8_t(2));check(throws([&]{read_scalar(mem,0x140c,ScalarType::Bool);}),"invalid boolean rejected");
        put(0x140c,std::uint8_t(1));check(read_scalar(mem,0x140c,ScalarType::Bool)==1,"boolean read");
        put(0x1200,std::uint64_t(0x100001337));check(mem.pointer(0x1200)==0x100001337,"64-bit guest pointers");
        GuestMemory split({{0x1000,host_base,8},{0x1008,host_base+8,8}},8,reader);
        const std::uint64_t crossing=0x0123456789abcdef;put(0x1004,crossing);check(split.read<std::uint64_t>(0x1004)==crossing,"read spanning mapped pages");
        GuestMemory gap({{0x1000,host_base,8},{0x1009,host_base+9,8}},8,reader);
        check(throws([&]{gap.read<std::uint64_t>(0x1004);}),"read spanning mapping gap fails");
        TransformLayout t{{0},{8},{0},{8},48,0,16,32,4,100,16};
        put(0x1100,std::uint64_t(0x1200));put(0x1108,std::int32_t(0));put(0x1200,std::uint64_t(0x2000));put(0x1208,std::uint64_t(0x3000));
        put(0x2000,Vec3{1,2,3});put(0x2030,Vec3{10,0,0});put(0x2040,Quaternion{0,0,0.70710678f,0.70710678f});put(0x2050,Vec3{2,2,2});
        put(0x3000,std::int32_t(1));put(0x3004,std::int32_t(-1));
        auto v=transform_position(mem,0x1100,t);check(near(v.x,6)&&near(v.y,2)&&near(v.z,6),"transform parent rotation/scale/translation");
        put(0x3004,std::int32_t(0));check(throws([&]{transform_position(mem,0x1100,t);}),"cyclic transform fails");
        put(0x3004,std::int32_t(-1));put(0x1108,std::int32_t(101));check(throws([&]{transform_position(mem,0x1100,t);}),"bad transform index fails");
        v=canonical({1,2,3},1);check(near(v.x*meters_per_unit,3)&&near(v.y*meters_per_unit,-1)&&near(v.z*meters_per_unit,2),"Unity coordinate normalization");
        Matrix unity{};unity.m[0][0]=1;unity.m[1][1]=1;unity.m[3][2]=1;
        const auto matrix=canonical_matrix(unity,false,1,1);
        auto screen=project(canonical({0,0,10},1),matrix,{1000,1000});check(screen&&near(screen->x,500)&&near(screen->y,500),"normalized projection center");
        check(!project(canonical({0,0,-10},1),matrix,{1000,1000}),"behind camera rejected");
        Frame frame;frame.ready=true;frame.team=2;frame.matrix=matrix;
        Player p;p.handle=1;p.team=3;p.health=100;p.spotted_by_local=true;p.head=canonical({0.5f,0,10},1);p.chest=canonical({0.25f,0,10},1);p.spine=canonical({0.1f,0,10},1);frame.players={p};
        AimOptions opt;opt.fov=100;auto chosen=select_target(frame,opt,{1000,1000});check(chosen&&near(chosen->error,25),"pixel FOV selection");
        opt.fov=20;check(!select_target(frame,opt,{1000,1000}),"outside FOV rejected");opt.fov=100;
        frame.players[0].team=2;check(!select_target(frame,opt,{1000,1000}),"team filter");frame.players[0].team=3;
        frame.players[0].spotted_by_local=false;check(!select_target(frame,opt,{1000,1000}),"visibility filter");frame.players[0].spotted_by_local=true;
        frame.players[0].health=0;check(!select_target(frame,opt,{1000,1000}),"dead player filter");frame.players[0].health=100;
        frame.players[0].immune=true;check(!select_target(frame,opt,{1000,1000}),"immunity filter");frame.players[0].immune=false;
        opt.max_distance_m=5;check(!select_target(frame,opt,{1000,1000}),"metric range filter");opt.max_distance_m=150;
        opt.point=HitPoint::Spine;check(near(select_target(frame,opt,{1000,1000})->error,5),"bone selection");
        frame.ready=false;check(!select_target(frame,opt,{1000,1000}),"inactive frame cannot aim");
        opt.smoothing_seconds=0;opt.mouse_gain=1;opt.max_step=10;
        const auto step=mouse_step({100,100},opt,0.016f);check(near(std::hypot(step.x,step.y),10),"mouse delta magnitude cap");
        check(near(mouse_step({1,1},opt,0).x,0),"zero time input rejected");
        const auto radar=radar_position({100,0,0},0,200,100);check(near(radar.x,0)&&near(radar.y,-50),"radar direction");
        Settings settings;settings.aim.mouse_gain=0.31f;settings.aim.fov=250;std::string error;
        check(save_settings(temp/"settings.json",settings,error),"save settings");Settings loaded;
        check(load_settings(temp/"settings.json",loaded,error)&&near(loaded.aim.mouse_gain,0.31f)&&near(loaded.aim.fov,250),"settings round trip");
        auto js=settings_json(settings);js["aim"]["key"]=35;std::ofstream(temp/"settings.json")<<js;
        check(!load_settings(temp/"settings.json",loaded,error)&&near(loaded.aim.fov,250),"reserved End binding rejected transactionally");
        Layout layout;check(load_layout("config/layout.json",layout,error)&&!layout.enabled,"unconfigured layout remains inactive");
        std::ifstream source("config/layout.json");source>>js;js["enabled"]=true;std::ofstream(temp/"layout.json")<<js;
        check(!load_layout(temp/"layout.json",layout,error)&&!layout.enabled,"enabled incomplete layout rejected");
        js["process_id"]=1;js["process_created_filetime"]="0x100";js["game_version"]="fixture";js["libil2cpp_build_id"]="fixture";js["guest_pointer_bytes"]=8;
        js["regions"]={{{"guest",0x1000},{"host",0x100000},{"size",0x5000}}};js["markers"]={{{"address",0x1000},{"bytes",{1,2,3,4,5,6,7,8}}}};
        for(auto key:{"matrix","local_player","entities","count"})js["roots"][key]={{"base",0x1000},{"offsets",{0}}};
        for(auto key:{"health","max_health","team","armour","ammo","weapon_id"})js["fields"][key]={0};
        for(auto key:{"feet","head","chest","spine","eye"})js["positions"][key]={{"kind","vec3"},{"offsets",{0}}};
        js["array_data_offset"]=0;js["array_stride"]=8;std::ofstream(temp/"layout.json")<<js;
        check(load_layout(temp/"layout.json",layout,error)&&layout.enabled,"complete fixture accepted");
        js["fields"]["health"]={{"offsets",{16}},{"type","u16"}};
        js["fields"]["team"]={{"offsets",{32}},{"type","u8"}};
        js["fields"]["ammo"]={{"offsets",{64,4}},{"type","i16"}};
        js["fields"]["visible"]={{"offsets",{1}},{"type","bool"}};
        std::ofstream(temp/"layout.json")<<js;
        check(load_layout(temp/"layout.json",layout,error)&&layout.fields.at("health").type==ScalarType::U16&&layout.fields.at("ammo").type==ScalarType::I16,"typed field configuration");
        js["fields"]["ammo"]["type"]="guess";std::ofstream(temp/"layout.json")<<js;
        check(!load_layout(temp/"layout.json",layout,error)&&!layout.enabled,"unknown scalar type disables live mode");
        js["fields"]["ammo"]["type"]="i16";
        js["regions"][0]["size"]=-1;std::ofstream(temp/"layout.json")<<js;
        check(!load_layout(temp/"layout.json",layout,error)&&!layout.enabled,"negative mapping rejected and live disabled");
        std::filesystem::remove_all(temp);std::cout<<checks<<" checks passed\n";return 0;
    }catch(const std::exception& e){std::cerr<<"FAILED after "<<checks<<" checks: "<<e.what()<<'\n';std::filesystem::remove_all(temp);return 1;}
}
