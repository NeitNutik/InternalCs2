#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace toker {
inline constexpr float pi = 3.14159265358979323846f;
inline constexpr float degrees = 180.0f / pi;
inline constexpr float meters_per_unit = 0.0254f;
struct Vec2 { float x{}, y{}; };
struct Vec3 {
    float x{}, y{}, z{};
    Vec3 operator+(Vec3 b) const { return {x+b.x,y+b.y,z+b.z}; }
    Vec3 operator-(Vec3 b) const { return {x-b.x,y-b.y,z-b.z}; }
    Vec3 operator*(float v) const { return {x*v,y*v,z*v}; }
};
inline bool finite(Vec3 v) { return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z); }
inline float length(Vec3 v) { return std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z); }
inline float yaw_wrap(float v) { return std::remainder(v,360.0f); }
inline Vec3 clamp_angles(Vec3 a) { return {std::clamp(a.x,-89.0f,89.0f),yaw_wrap(a.y),0}; }
inline Vec3 angle_to(Vec3 from, Vec3 to) {
    const auto d=to-from;
    return {-std::atan2(d.z,std::hypot(d.x,d.y))*degrees,std::atan2(d.y,d.x)*degrees,0};
}
inline float angular_distance(Vec3 a,Vec3 b) {
    return std::hypot(a.x-b.x,yaw_wrap(a.y-b.y));
}
inline Vec3 smooth_angles(Vec3 from,Vec3 to,float seconds,float dt) {
    const float alpha=seconds<=0?1.0f:1.0f-std::exp(-std::clamp(dt,0.0f,0.1f)/seconds);
    return clamp_angles({from.x+(to.x-from.x)*alpha,from.y+yaw_wrap(to.y-from.y)*alpha,0});
}
struct Matrix { float m[4][4]{}; };
inline std::optional<Vec2> project(Vec3 p,const Matrix& a,Vec2 size) {
    if(!finite(p)||size.x<=0||size.y<=0) return {};
    const float w=a.m[3][0]*p.x+a.m[3][1]*p.y+a.m[3][2]*p.z+a.m[3][3];
    if(!std::isfinite(w)||w<0.01f) return {};
    const float x=(a.m[0][0]*p.x+a.m[0][1]*p.y+a.m[0][2]*p.z+a.m[0][3])/w;
    const float y=(a.m[1][0]*p.x+a.m[1][1]*p.y+a.m[1][2]*p.z+a.m[1][3])/w;
    if(!std::isfinite(x)||!std::isfinite(y)) return {};
    return Vec2{size.x*(0.5f+x*0.5f),size.y*(0.5f-y*0.5f)};
}
// Radar up is the local view's forward direction; right is camera right.
inline Vec2 radar_position(Vec3 delta,float yaw,float range,float radius) {
    const float a=yaw/degrees;
    Vec2 p{(delta.x*std::sin(a)-delta.y*std::cos(a))/range*radius,
           -(delta.x*std::cos(a)+delta.y*std::sin(a))/range*radius};
    const float r=std::hypot(p.x,p.y);
    if(r>radius) { p.x*=radius/r; p.y*=radius/r; }
    return p;
}
enum class HitPoint : int { Head, Chest, Spine };
struct Player {
    std::uint32_t handle{};
    int team{},health{},max_health{100},armour{},ammo{-1};
    bool immune{},spotted_by_local{};
    Vec3 feet{},head{},chest{},spine{};
    std::string name,weapon;
    Vec3 point(HitPoint h) const { return h==HitPoint::Head?head:h==HitPoint::Chest?chest:spine; }
};
struct Frame {
    bool ready{},preview{};
    int team{},build{};
    Vec3 origin{},eye{},angles{},velocity{};
    Matrix matrix{};
    std::vector<Player> players;
    std::string status{"Waiting for a match"};
};
struct AimOptions {
    bool enabled{false},enemies_only{true},spotted_only{true};
    HitPoint point{HitPoint::Head};
    int key{0x12}; // VK_MENU / Alt
    float fov{5.0f},smoothing_seconds{0.08f},max_distance_m{150.0f};
};
struct Selection { std::uint32_t handle{}; Vec3 point{},angles{}; float error{}; };
inline std::optional<Selection> select_target(const Frame& f,const AimOptions& opt) {
    if(!f.ready||!finite(f.eye)||!finite(f.angles)) return {};
    std::optional<Selection> best;
    for(const auto& p:f.players) {
        if(p.health<=0||p.immune||(opt.enemies_only&&p.team==f.team)||(opt.spotted_only&&!p.spotted_by_local)) continue;
        const auto pos=p.point(opt.point);
        if(!finite(pos)||length(pos-f.eye)<0.01f||length(pos-f.eye)*meters_per_unit>opt.max_distance_m) continue;
        const auto angles=angle_to(f.eye,pos);
        const float error=angular_distance(angles,f.angles);
        if(error<=opt.fov&&(!best||error<best->error)) best=Selection{p.handle,pos,angles,error};
    }
    return best;
}
Frame demo_frame(float time,Vec2 viewport,Vec3 angles);
}
