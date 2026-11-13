#pragma once
#include "core.hpp"
#include <cstring>
#include <functional>
#include <limits>
#include <set>
#include <stdexcept>

namespace toker {
using Address=std::uint64_t;
inline Address add(Address a,Address b){if(b>UINT64_MAX-a)throw std::runtime_error("Address overflow");return a+b;}
struct Region { Address guest{},host{},size{}; };
using HostRead=std::function<bool(Address,void*,std::size_t)>;
class GuestMemory {
public:
    GuestMemory(std::vector<Region> regions,unsigned pointer_bytes,HostRead reader)
        :regions_(std::move(regions)),pointer_bytes_(pointer_bytes),reader_(std::move(reader)){
        if(pointer_bytes!=4&&pointer_bytes!=8)throw std::runtime_error("Guest pointer size must be 4 or 8");
        std::sort(regions_.begin(),regions_.end(),[](auto a,auto b){return a.guest<b.guest;});
        Address end{};
        for(auto r:regions_){
            if(!r.size||!r.host||r.guest<end)throw std::runtime_error("Empty or overlapping guest mappings");
            end=add(r.guest,r.size);add(r.host,r.size);
        }
    }
    bool bytes(Address at,void* destination,std::size_t size)const{
        if(size>UINT64_MAX-at)return false;
        auto* out=static_cast<std::byte*>(destination);
        while(size){
            auto it=std::upper_bound(regions_.begin(),regions_.end(),at,[](Address a,const Region& r){return a<r.guest;});
            if(it==regions_.begin())return false;
            const auto& r=*--it;if(at-r.guest>=r.size)return false;
            const auto n=std::min<std::uint64_t>(size,r.size-(at-r.guest));
            if(!reader_(r.host+(at-r.guest),out,static_cast<std::size_t>(n)))return false;
            at+=n;out+=n;size-=static_cast<std::size_t>(n);
        }
        return true;
    }
    template<class T>T read(Address at)const{T out{};if(!bytes(at,&out,sizeof(out)))throw std::runtime_error("Unmapped or unreadable guest address");return out;}
    Address pointer(Address at)const{return pointer_bytes_==4?read<std::uint32_t>(at):read<std::uint64_t>(at);}
    Address path(Address base,std::span<const Address> offsets)const{
        if(offsets.empty())throw std::runtime_error("Empty field path");
        for(std::size_t i=0;i<offsets.size();++i){base=add(base,offsets[i]);if(i+1<offsets.size()){base=pointer(base);if(!base)throw std::runtime_error("Null guest pointer");}}
        return base;
    }
private:
    std::vector<Region> regions_;unsigned pointer_bytes_;HostRead reader_;
};
struct Quaternion { float x{},y{},z{},w{1}; };
inline Vec3 cross(Vec3 a,Vec3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
inline Vec3 rotate(Quaternion q,Vec3 v){const Vec3 xyz{q.x,q.y,q.z};const auto t=cross(xyz,v)*2;return v+t*q.w+cross(xyz,t);}
struct TransformLayout {
    std::vector<Address> data_path,index_path,matrices_path,parents_path;
    Address stride{},translation{},rotation{},scale{},parent_stride{};
    std::uint32_t max_transforms{65536},max_depth{128};
};
inline Vec3 transform_position(const GuestMemory& m,Address transform,const TransformLayout& l){
    if(!transform)throw std::runtime_error("Null transform");
    const auto data=m.pointer(m.path(transform,l.data_path));
    if(!data)throw std::runtime_error("Null transform data");
    auto index=m.read<std::int32_t>(m.path(transform,l.index_path));
    const auto matrices=m.pointer(m.path(data,l.matrices_path)),parents=m.pointer(m.path(data,l.parents_path));
    if(!data||!matrices||!parents||index<0||std::uint32_t(index)>=l.max_transforms)throw std::runtime_error("Invalid transform access");
    auto pos=m.read<Vec3>(add(add(matrices,Address(index)*l.stride),l.translation));
    std::set<std::int32_t> seen{index};
    for(std::uint32_t depth=0;depth<l.max_depth;++depth){
        index=m.read<std::int32_t>(add(parents,Address(index)*l.parent_stride));
        if(index==-1){if(!finite(pos))throw std::runtime_error("Nonfinite transform");return pos;}
        if(index<0||std::uint32_t(index)>=l.max_transforms||!seen.insert(index).second)throw std::runtime_error("Invalid transform hierarchy");
        const auto entry=add(matrices,Address(index)*l.stride);
        const auto translation=m.read<Vec3>(add(entry,l.translation)),scale=m.read<Vec3>(add(entry,l.scale));
        const auto rotation=m.read<Quaternion>(add(entry,l.rotation));
        const float norm=rotation.x*rotation.x+rotation.y*rotation.y+rotation.z*rotation.z+rotation.w*rotation.w;
        if(!finite(translation)||!finite(scale)||!std::isfinite(norm)||std::abs(norm-1)>0.05f)throw std::runtime_error("Invalid transform components");
        pos=rotate(rotation,{pos.x*scale.x,pos.y*scale.y,pos.z*scale.z})+translation;
    }
    throw std::runtime_error("Transform depth limit");
}
}
