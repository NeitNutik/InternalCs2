#pragma once
#include "memory.hpp"
#include <filesystem>
#include <map>
#include <nlohmann/json.hpp>

namespace toker {
struct Root { Address base{};std::vector<Address> offsets; };
struct PositionField { bool transform{};std::vector<Address> offsets; };
struct Marker { Address address{};std::vector<std::uint8_t> bytes; };
struct Layout {
    bool enabled{},column_major{true};
    std::uint32_t pid{},max_players{64};
    Address created{},array_data{},array_stride{};
    unsigned pointer_bytes{};
    float world_unit_meters{1},clip_sign{1};
    int viewport_left{},viewport_top{},viewport_right{},viewport_bottom{};
    std::string game_version,game_build_id;
    std::vector<Region> regions;std::vector<Marker> markers;
    std::map<std::string,Root> roots;
    std::map<std::string,ScalarField> fields;
    std::map<std::string,PositionField> positions;
    std::map<int,std::string> weapons;
    TransformLayout transform;
};
bool load_layout(const std::filesystem::path&,Layout&,std::string&);
inline Vec3 canonical(Vec3 unity,float meters){return Vec3{unity.z,-unity.x,unity.y}*(meters/meters_per_unit);}
inline Matrix canonical_matrix(Matrix m,bool column_major,float unit_meters,float sign){
    if(column_major)for(int row=0;row<4;++row)for(int col=row+1;col<4;++col)std::swap(m.m[row][col],m.m[col][row]);
    Matrix out;const float s=meters_per_unit/unit_meters;
    for(int i=0;i<4;++i){out.m[i][0]=m.m[i][2]*s*sign;out.m[i][1]=-m.m[i][0]*s*sign;out.m[i][2]=m.m[i][1]*s*sign;out.m[i][3]=m.m[i][3]*sign;}
    return out;
}
}
