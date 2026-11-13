#include "toker/game.hpp"

namespace toker {
void Game::detach(){memory_.reset();if(process_)CloseHandle(process_);process_=nullptr;last_sample_=0;previous_local_=0;}
void Game::reload(){detach();load_layout(path_,layout_,status_);std::error_code ec;modified_=std::filesystem::last_write_time(path_,ec);}
void Game::poll_reload(){
    const auto now=GetTickCount64();if(now-last_poll_<1000)return;last_poll_=now;
    std::error_code ec;const auto timestamp=std::filesystem::last_write_time(path_,ec);
    if(ec){detach();layout_.enabled=false;status_="Cannot read layout.json";return;}
    if(timestamp!=modified_)reload();
}
bool Game::identity_matches()const{
    if(!process_)return false;DWORD exit{};if(!GetExitCodeProcess(process_,&exit)||exit!=STILL_ACTIVE)return false;
    FILETIME created{},ended{},kernel{},user{};if(!GetProcessTimes(process_,&created,&ended,&kernel,&user))return false;
    const Address timestamp=(Address(created.dwHighDateTime)<<32)|created.dwLowDateTime;
    return timestamp==layout_.created;
}
bool Game::attach(){
    if(process_&&identity_matches())return true;detach();
    process_=OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,layout_.pid);
    if(!process_){status_="Cannot open configured emulator PID: "+std::to_string(GetLastError());return false;}
    wchar_t executable[32768]{};DWORD size=32768;
    if(!QueryFullProcessImageNameW(process_,0,executable,&size)||_wcsicmp(std::filesystem::path(executable).filename().c_str(),L"HD-Player.exe")!=0||!identity_matches()){
        status_="Emulator identity mismatch; recapture session mapping";detach();return false;
    }
    memory_=std::make_unique<GuestMemory>(layout_.regions,layout_.pointer_bytes,[this](Address address,void* data,std::size_t count){
        SIZE_T bytes{};return ReadProcessMemory(process_,reinterpret_cast<const void*>(address),data,count,&bytes)&&bytes==count;
    });return true;
}
Address Game::root(const char* name)const{const auto& r=layout_.roots.at(name);return memory_->path(r.base,r.offsets);}
int Game::field(Address object,const char* name)const{
    const auto& spec=layout_.fields.at(name);return read_scalar(*memory_,memory_->path(object,spec.offsets),spec.type);
}
Vec3 Game::position(Address object,const char* name)const{
    const auto& p=layout_.positions.at(name);const auto address=memory_->path(object,p.offsets);
    const Vec3 v=p.transform?transform_position(*memory_,memory_->pointer(address),layout_.transform):memory_->read<Vec3>(address);
    if(!finite(v)||length(v)>1e7f)throw std::runtime_error("Invalid world position");return canonical(v,layout_.world_unit_meters);
}
Frame Game::sample(){
    poll_reload();Frame f;f.status=status_;
    if(!layout_.enabled)return f;
    if(!attach()){f.status=status_;return f;}
    try{
        for(const auto& marker:layout_.markers){std::vector<std::uint8_t> bytes(marker.bytes.size());if(!memory_->bytes(marker.address,bytes.data(),bytes.size())||bytes!=marker.bytes)throw std::runtime_error("Session marker mismatch; recapture mappings");}
        const auto local=memory_->pointer(root("local_player")),array=memory_->pointer(root("entities"));
        const int count=memory_->read<std::int32_t>(root("count"));
        if(!local||!array||count<0||std::uint32_t(count)>layout_.max_players)throw std::runtime_error("Invalid player collection");
        const int hp=field(local,"health");f.team=field(local,"team");
        if(hp<=0||hp>10000||f.team<0||f.team>32)throw std::runtime_error("Local player unavailable");
        f.origin=position(local,"feet");f.eye=position(local,"eye");
        const auto matrix=memory_->read<Matrix>(root("matrix"));
        for(const auto& row:matrix.m)for(float value:row)if(!std::isfinite(value))throw std::runtime_error("Invalid camera matrix");
        f.matrix=canonical_matrix(matrix,layout_.column_major,layout_.world_unit_meters,layout_.clip_sign);
        const Vec3 forward{f.matrix.m[3][0],f.matrix.m[3][1],f.matrix.m[3][2]};
        if(length(forward)<1e-7f)throw std::runtime_error("Degenerate camera matrix");
        f.angles=angle_to({},forward);
        for(int i=0;i<count;++i){
            const auto player=memory_->pointer(add(array,layout_.array_data+Address(i)*layout_.array_stride));if(!player||player==local)continue;
            Player p;p.handle=player;p.health=field(player,"health");p.max_health=field(player,"max_health");p.team=field(player,"team");
            if(p.health<=0)continue;
            if(p.health>10000||p.max_health<=0||p.max_health>10000||p.team<0||p.team>32)throw std::runtime_error("Player fields failed validation");
            p.feet=position(player,"feet");p.head=position(player,"head");p.chest=position(player,"chest");p.spine=position(player,"spine");
            if(length(p.head-p.feet)*meters_per_unit>5)throw std::runtime_error("Bone positions failed validation");
            p.armour=std::clamp(field(player,"armour"),0,100);p.ammo=std::clamp(field(player,"ammo"),-1,1000);
            if(layout_.fields.contains("visible"))p.spotted_by_local=field(player,"visible")!=0;
            const auto id=field(player,"weapon_id");const auto weapon=layout_.weapons.find(id);p.weapon=weapon==layout_.weapons.end()?"Weapon #"+std::to_string(id):weapon->second;
            p.name="Player "+std::to_string(i+1);f.players.push_back(std::move(p));
        }
        // Reject a frame if the collection changed while it was sampled.
        if(memory_->pointer(root("local_player"))!=local||memory_->pointer(root("entities"))!=array||memory_->read<std::int32_t>(root("count"))!=count)throw std::runtime_error("Player collection changed during sample");
        const auto now=GetTickCount64();
        if(last_sample_&&previous_local_==local&&now>last_sample_&&now-last_sample_<250)f.velocity=(f.origin-previous_origin_)*(1000.0f/float(now-last_sample_));
        last_sample_=now;previous_local_=local;previous_origin_=f.origin;
        f.ready=true;f.status="Live | "+std::to_string(f.players.size())+" players";return f;
    }catch(const std::exception& e){f=Frame{};f.status=e.what();return f;}
}
}
