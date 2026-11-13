#include "toker/layout.hpp"
#include <fstream>

namespace toker {
namespace {
using nlohmann::json;
void require(bool v,const char* text){if(!v)throw std::runtime_error(text);}
Address number(const json& j){
    if(j.is_string()){const auto s=j.get<std::string>();require(!s.empty()&&s[0]!='-',"Invalid address");std::size_t n{};const auto v=std::stoull(s,&n,0);require(n==s.size(),"Invalid address suffix");return v;}
    require(j.is_number_integer(),"Missing address: complete the FILL entries");
    if(j.is_number_unsigned())return j.get<Address>();
    require(j.get<std::int64_t>()>=0,"Negative address");return j.get<Address>();
}
std::vector<Address> path(const json& j){require(j.is_array()&&!j.empty()&&j.size()<=16,"Field path requires 1..16 offsets");std::vector<Address> p;for(const auto& v:j){auto x=number(v);require(x<=0x1000000,"Field offset too large");p.push_back(x);}return p;}
ScalarField scalar_field(const json& j,ScalarType legacy){
    if(j.is_array())return {path(j),legacy};
    require(j.is_object(),"Scalar field requires offsets and type");
    const std::map<std::string,ScalarType> types{{"i8",ScalarType::I8},{"u8",ScalarType::U8},
        {"i16",ScalarType::I16},{"u16",ScalarType::U16},{"i32",ScalarType::I32},{"u32",ScalarType::U32},{"bool",ScalarType::Bool}};
    require(j.contains("type")&&j.at("type").is_string(),"Missing scalar type: use the declaring type in the dump");
    const auto it=types.find(j.at("type").get<std::string>());require(it!=types.end(),"Unsupported scalar type");
    return {path(j.at("offsets")),it->second};
}
}
bool load_layout(const std::filesystem::path& file,Layout& out,std::string& error){
    try{
        std::ifstream stream(file);if(!stream)throw std::runtime_error("Cannot open config/layout.json");const auto j=json::parse(stream);
        require(j.at("schema_version")==1,"Unsupported layout schema");Layout l;l.enabled=j.at("enabled").get<bool>();
        if(!l.enabled){out=std::move(l);error="Live mode inactive: configure process identity, memory mappings and roots in layout.json";return true;}
        l.pid=j.at("process_id").get<std::uint32_t>();l.created=number(j.at("process_created_filetime"));require(l.pid>0&&l.created>0,"Missing emulator process identity");
        l.game_version=j.at("game_version").get<std::string>();l.game_build_id=j.at("libil2cpp_build_id").get<std::string>();
        require(!l.game_version.empty()&&!l.game_build_id.empty(),"Missing game version/build ID");
        l.pointer_bytes=j.at("guest_pointer_bytes").get<unsigned>();require(l.pointer_bytes==4||l.pointer_bytes==8,"Unsupported guest pointer width");
        for(const auto& r:j.at("regions"))l.regions.push_back({number(r.at("guest")),number(r.at("host")),number(r.at("size"))});
        require(!l.regions.empty()&&l.regions.size()<=65536,"Missing guest mappings");
        GuestMemory validate(l.regions,l.pointer_bytes,[](Address,void*,std::size_t){return false;});
        for(const auto& r:j.at("markers")){
            Marker m;m.address=number(r.at("address"));const auto bytes=r.at("bytes");
            require(bytes.is_array()&&bytes.size()>=8&&bytes.size()<=256,"Markers need 8..256 bytes");
            for(const auto& b:bytes){auto n=number(b);require(n<=255,"Invalid marker byte");m.bytes.push_back(std::uint8_t(n));}
            l.markers.push_back(std::move(m));
        }
        require(!l.markers.empty()&&l.markers.size()<=32,"Missing session markers");
        for(auto key:{"matrix","local_player","entities","count"}){const auto& r=j.at("roots").at(key);l.roots[key]={number(r.at("base")),path(r.at("offsets"))};require(l.roots[key].base>0,"Null root address");}
        for(auto key:{"health","max_health","team","armour","ammo","weapon_id"})l.fields[key]=scalar_field(j.at("fields").at(key),ScalarType::I32);
        if(j.at("fields").contains("visible")&&!j.at("fields").at("visible").is_null())l.fields["visible"]=scalar_field(j.at("fields").at("visible"),ScalarType::U8);
        bool transforms=false;
        for(auto key:{"feet","head","chest","spine","eye"}){
            const auto& p=j.at("positions").at(key);const auto kind=p.at("kind").get<std::string>();require(kind=="vec3"||kind=="transform","Unknown position type");
            l.positions[key]={kind=="transform",path(p.at("offsets"))};transforms|=kind=="transform";
        }
        l.array_data=number(j.at("array_data_offset"));l.array_stride=number(j.at("array_stride"));
        require(l.array_data<=0x100000&&l.array_stride>=l.pointer_bytes&&l.array_stride<=256,"Invalid entity array layout");
        l.max_players=j.at("max_players").get<std::uint32_t>();require(l.max_players>0&&l.max_players<=128,"Invalid player limit");
        l.world_unit_meters=j.at("world_unit_meters").get<float>();require(std::isfinite(l.world_unit_meters)&&l.world_unit_meters>0&&l.world_unit_meters<=1000,"Invalid world unit scale");
        l.column_major=j.at("matrix_column_major").get<bool>();l.clip_sign=j.at("clip_w_sign").get<float>();require(l.clip_sign==1||l.clip_sign==-1,"Clip sign must be 1 or -1");
        const auto& v=j.at("viewport_insets_px");l.viewport_left=v.at(0).get<int>();l.viewport_top=v.at(1).get<int>();l.viewport_right=v.at(2).get<int>();l.viewport_bottom=v.at(3).get<int>();
        for(auto n:{l.viewport_left,l.viewport_top,l.viewport_right,l.viewport_bottom})require(n>=0&&n<=4096,"Invalid viewport inset");
        for(auto it=j.at("weapon_names").begin();it!=j.at("weapon_names").end();++it)l.weapons[std::stoi(it.key())]=it.value().get<std::string>();
        if(transforms){
            auto& t=l.transform;const auto& x=j.at("transform_layout");
            t.data_path=path(x.at("data_path"));t.index_path=path(x.at("index_path"));t.matrices_path=path(x.at("matrices_path"));t.parents_path=path(x.at("parents_path"));
            t.stride=number(x.at("stride"));t.translation=number(x.at("translation"));t.rotation=number(x.at("rotation"));t.scale=number(x.at("scale"));t.parent_stride=number(x.at("parent_stride"));
            require(t.stride>=40&&t.stride<=256&&t.parent_stride>=4&&t.parent_stride<=64,"Invalid transform stride");
            require(t.translation<=t.stride-12&&t.rotation<=t.stride-16&&t.scale<=t.stride-12,"Transform field outside record");
        }
        out=std::move(l);error.clear();return true;
    }catch(const std::exception& e){out.enabled=false;error=e.what();return false;}
}
}
