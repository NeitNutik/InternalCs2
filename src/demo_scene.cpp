#include "toker/core.hpp"

namespace toker {
Frame demo_frame(float time,Vec2 viewport,Vec3 angles){
    Frame f;f.preview=true;f.ready=true;f.team=2;f.eye={0,0,64};f.angles=angles;
    f.velocity={180.0f*std::cos(time),80.0f*std::sin(time),0};
    const float y=angles.y/degrees,p=angles.x/degrees;
    const Vec3 forward{std::cos(p)*std::cos(y),std::cos(p)*std::sin(y),-std::sin(p)};
    const Vec3 right{std::sin(y),-std::cos(y),0};
    const Vec3 up{std::sin(p)*std::cos(y),std::sin(p)*std::sin(y),std::cos(p)};
    const float aspect=viewport.y>0?viewport.x/viewport.y:1;
    const auto row=[&](int n,Vec3 axis,float scale){
        f.matrix.m[n][0]=axis.x*scale;f.matrix.m[n][1]=axis.y*scale;f.matrix.m[n][2]=axis.z*scale;
        f.matrix.m[n][3]=-(axis.x*f.eye.x+axis.y*f.eye.y+axis.z*f.eye.z)*scale;
    };
    row(0,right,1);row(1,up,aspect);row(3,forward,1);
    const std::array<const char*,6> names{"NOVA","CIPHER","ROOK","ECHO","GHOST","ALLY"};
    const std::array<const char*,6> weapons{"AK-47","M4A1-S","AWP","MP9","Desert Eagle","M4A4"};
    for(int i=0;i<6;++i){
        Player q;q.handle=std::uint32_t(i+1);q.team=i==5?2:3;q.health=100-i*13;q.armour=100-i*16;q.ammo=i==2?7:30-i*3;
        q.name=names[i];q.weapon=weapons[i];q.spotted_by_local=i!=4;
        q.feet={450.0f+i*85.0f,(i-2.5f)*135.0f+std::sin(time*0.4f+i)*40,0};
        q.head=q.feet+Vec3{0,0,68};q.chest=q.feet+Vec3{0,0,50};q.spine=q.feet+Vec3{0,0,36};
        f.players.push_back(q);
    }
    f.status="Preview | synthetic scene | Insert: menu";return f;
}
}
