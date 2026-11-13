#include "toker/game.hpp"
#include <array>
#include <cstring>

namespace toker {
namespace {
template<class T> bool read(std::uintptr_t at,T& value){
    if(at<0x10000||at>0x00007FFFFFFEFFFFull||sizeof(T)>0x00007FFFFFFEFFFFull-at)return false;
    SIZE_T done{};T temp{};
    if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(at),&temp,sizeof(T),&done)||done!=sizeof(T))return false;
    value=temp;return true;
}
template<class T> T value(std::uintptr_t at,T fallback={}){T v{};return read(at,v)?v:fallback;}
bool module_size(std::uintptr_t base,std::size_t& size){
    IMAGE_DOS_HEADER dos{};if(!read(base,dos)||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<=0||dos.e_lfanew>0x100000)return false;
    IMAGE_NT_HEADERS64 nt{};if(!read(base+dos.e_lfanew,nt)||nt.Signature!=IMAGE_NT_SIGNATURE||nt.OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR64_MAGIC)return false;
    size=nt.OptionalHeader.SizeOfImage;return size>0;
}
std::string weapon_name(std::uint16_t id){
    switch(id){
    case 1:return "Desert Eagle";case 2:return "Dual Berettas";case 3:return "Five-SeveN";case 4:return "Glock-18";
    case 7:return "AK-47";case 8:return "AUG";case 9:return "AWP";case 10:return "FAMAS";case 11:return "G3SG1";
    case 13:return "Galil AR";case 14:return "M249";case 16:return "M4A4";case 17:return "MAC-10";case 19:return "P90";
    case 23:return "MP5-SD";case 24:return "UMP-45";case 25:return "XM1014";case 26:return "PP-Bizon";case 27:return "MAG-7";
    case 28:return "Negev";case 29:return "Sawed-Off";case 30:return "Tec-9";case 31:return "Zeus x27";case 32:return "P2000";
    case 33:return "MP7";case 34:return "MP9";case 35:return "Nova";case 36:return "P250";case 38:return "SCAR-20";
    case 39:return "SG 553";case 40:return "SSG 08";case 42:case 59:return "Knife";case 43:return "Flashbang";
    case 44:return "HE Grenade";case 45:return "Smoke";case 46:return "Molotov";case 47:return "Decoy";case 48:return "Incendiary";
    case 49:return "C4";case 60:return "M4A1-S";case 61:return "USP-S";case 63:return "CZ75-Auto";case 64:return "R8 Revolver";
    default:return id>=500?"Knife":"Item "+std::to_string(id);
    }
}
}
bool Game::reload(){
    loaded_=load_offsets(path_,offsets_,error_);valid_=false;
    std::error_code ec;modified_=std::filesystem::last_write_time(path_,ec);
    return loaded_;
}
void Game::poll_reload(){
    const auto now=GetTickCount64();if(now-last_poll_<1000)return;last_poll_=now;
    std::error_code ec;const auto t=std::filesystem::last_write_time(path_,ec);
    if(ec){loaded_=false;valid_=false;error_="Cannot read config/offsets.json";return;}
    if(!loaded_||t!=modified_)reload();
}
bool Game::validate_modules(){
    client_=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(L"client.dll"));
    engine_=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(L"engine2.dll"));
    if(!loaded_){if(error_.empty())error_="Offsets not loaded";return false;}
    if(!client_||!engine_){error_="Waiting for client.dll and engine2.dll";return false;}
    if(!module_size(client_,client_size_)||!module_size(engine_,engine_size_)){error_="Cannot read module headers";return false;}
    for(const auto& [key,offset]:offsets_.modules){
        const auto size=key=="build_number"?engine_size_:client_size_;
        if(offset==0||offset>=size||size-offset<sizeof(Matrix)){error_="Module offset outside image";return false;}
    }
    int build{};if(!read(engine_+offsets_.m("build_number"),build)||build!=offsets_.build){
        error_="Build mismatch: game "+std::to_string(build)+", offsets "+std::to_string(offsets_.build);return false;
    }
    error_.clear();return true;
}
std::uintptr_t Game::entity(std::uintptr_t list,std::uint32_t handle)const{
    if(!list||handle==0xFFFFFFFFu)return 0;
    const auto index=handle&offsets_.l("entity_index_mask");
    const auto chunk=value<std::uintptr_t>(list+offsets_.l("entity_chunks")+sizeof(std::uintptr_t)*(index>>offsets_.l("entity_chunk_shift")));
    return chunk?value<std::uintptr_t>(chunk+offsets_.l("entity_stride")*(index&offsets_.l("entity_chunk_mask"))):0;
}
bool Game::bones(std::uintptr_t node,Player& p)const{
    const auto array=value<std::uintptr_t>(node+offsets_.f("model_state")+offsets_.l("bones_in_model_state"));
    if(!array)return false;
    Vec3 head{},chest{},spine{};
    if(!read(array+offsets_.l("head_bone")*offsets_.l("bone_stride"),head)||
       !read(array+offsets_.l("chest_bone")*offsets_.l("bone_stride"),chest)||
       !read(array+offsets_.l("spine_bone")*offsets_.l("bone_stride"),spine))return false;
    const auto sensible=[&](Vec3 v){return finite(v)&&length(v-p.feet)<120.0f&&length(v-p.feet)>1.0f;};
    if(!sensible(head)||!sensible(chest)||!sensible(spine))return false;
    p.head=head;p.chest=chest;p.spine=spine;return true;
}
Frame Game::sample(){
    poll_reload();Frame frame;valid_=validate_modules();
    if(!valid_){frame.status=error_;return frame;}
    frame.build=offsets_.build;
    const auto list=value<std::uintptr_t>(client_+offsets_.m("entity_list"));
    const auto local=value<std::uintptr_t>(client_+offsets_.m("local_pawn"));
    const auto controller=value<std::uintptr_t>(client_+offsets_.m("local_controller"));
    if(!list||!local||!controller){frame.status="Waiting for a live local player";return frame;}
    if(entity(list,value<std::uint32_t>(controller+offsets_.f("pawn_handle"),0xFFFFFFFFu))!=local){frame.status="Entity layout validation failed";return frame;}
    const int hp=value<int>(local+offsets_.f("health"));
    if(hp<=0||hp>1000||value<std::uint8_t>(local+offsets_.f("life_state"),255)!=0){frame.status="Local player is not alive";return frame;}
    frame.team=value<std::uint8_t>(local+offsets_.f("team"));
    const auto node=value<std::uintptr_t>(local+offsets_.f("scene_node"));Vec3 view{};
    if((frame.team!=2&&frame.team!=3)||!node||!read(node+offsets_.f("origin"),frame.origin)||
       !read(local+offsets_.f("view_offset"),view)||!read(client_+offsets_.m("view_angles"),frame.angles)||
       !read(client_+offsets_.m("view_matrix"),frame.matrix)||!finite(frame.origin)||!finite(view)||!finite(frame.angles)){
        frame.status="Local player data failed validation";return frame;
    }
    frame.eye=frame.origin+view;read(local+offsets_.f("velocity"),frame.velocity);
    int local_index=-1;
    for(std::uint32_t i=1;i<=offsets_.l("max_players");++i)if(entity(list,i)==controller){local_index=int(i)-1;break;}
    for(std::uint32_t i=1;i<=offsets_.l("max_players");++i){
        const auto ctrl=entity(list,i);if(!ctrl||ctrl==controller)continue;
        const auto handle=value<std::uint32_t>(ctrl+offsets_.f("pawn_handle"),0xFFFFFFFFu);
        const auto pawn=entity(list,handle);if(!pawn||pawn==local)continue;
        Player p;p.handle=handle;p.health=value<int>(pawn+offsets_.f("health"));p.team=value<std::uint8_t>(pawn+offsets_.f("team"));
        if(p.health<=0||p.health>1000||(p.team!=2&&p.team!=3)||value<std::uint8_t>(pawn+offsets_.f("life_state"),255)!=0)continue;
        const auto scene=value<std::uintptr_t>(pawn+offsets_.f("scene_node"));
        if(!scene||value<std::uint8_t>(scene+offsets_.f("dormant"),1)||!read(scene+offsets_.f("origin"),p.feet)||!finite(p.feet))continue;
        if(!bones(scene,p))continue; // Never aim using guessed head coordinates.
        p.max_health=std::clamp(value<int>(pawn+offsets_.f("max_health"),100),1,1000);
        p.armour=std::clamp(value<int>(pawn+offsets_.f("armour")),0,100);
        p.immune=value<std::uint8_t>(pawn+offsets_.f("immunity"))!=0;
        const auto mask=value<std::uint64_t>(pawn+offsets_.f("spotted_state")+offsets_.f("spotted_mask"));
        p.spotted_by_local=local_index>=0&&(mask&(std::uint64_t(1)<<local_index));
        std::array<char,128> name{};
        if(read(ctrl+offsets_.f("player_name"),name)){name.back()='\0';p.name=name.data();}else p.name="Player";
        const auto services=value<std::uintptr_t>(pawn+offsets_.f("weapon_services"));
        if(services){
            const auto active=entity(list,value<std::uint32_t>(services+offsets_.f("active_weapon"),0xFFFFFFFFu));
            if(active){
                p.ammo=std::clamp(value<int>(active+offsets_.f("clip"),-1),-1,1000);
                const auto id=value<std::uint16_t>(active+offsets_.f("attribute_manager")+offsets_.f("item")+offsets_.f("item_definition"));
                p.weapon=weapon_name(id);
            }
        }
        frame.players.push_back(std::move(p));
    }
    frame.ready=true;frame.status="Live | "+std::to_string(frame.players.size())+" players";return frame;
}
bool Game::aim(const Frame& f,const AimOptions& opt,float dt,bool menu_open){
    if(!valid_||!f.ready||!opt.enabled||menu_open||!(GetAsyncKeyState(opt.key)&0x8000))return false;
    DWORD foreground_pid{};GetWindowThreadProcessId(GetForegroundWindow(),&foreground_pid);if(foreground_pid!=GetCurrentProcessId())return false;
    const auto target=select_target(f,opt);if(!target)return false;
    const auto angles=smooth_angles(f.angles,target->angles,opt.smoothing_seconds,dt);
    if(!finite(angles))return false;
    SIZE_T written{};
    return WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(client_+offsets_.m("view_angles")),&angles,sizeof(angles),&written)&&written==sizeof(angles);
}
}
