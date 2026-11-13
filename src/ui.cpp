#include "toker/ui.hpp"
#include <windows.h>
#include <cstdio>

namespace toker {
namespace {
ImVec2 iv(Vec2 v){return {v.x,v.y};}
ImU32 color(const std::array<float,4>& c){return ImGui::ColorConvertFloat4ToU32({c[0],c[1],c[2],c[3]});}
void label(ImDrawList* d,ImVec2 p,ImU32 c,const std::string& s,bool centered=true){
    if(centered)p.x-=ImGui::CalcTextSize(s.c_str()).x*0.5f;
    d->AddText({p.x+1,p.y+1},IM_COL32(0,0,0,230),s.c_str());d->AddText(p,c,s.c_str());
}
std::string key_name(int key){
    if(key==VK_LBUTTON)return "Left mouse";if(key==VK_RBUTTON)return "Right mouse";
    if(key==VK_MBUTTON)return "Middle mouse";if(key==VK_XBUTTON1)return "Mouse 4";if(key==VK_XBUTTON2)return "Mouse 5";
    if(key==VK_MENU)return "Alt";if(key==VK_SHIFT)return "Shift";if(key==VK_CONTROL)return "Ctrl";
    char name[64]{};const auto scan=MapVirtualKeyA(UINT(key),MAPVK_VK_TO_VSC);
    if(GetKeyNameTextA(LONG(scan<<16),name,int(sizeof(name))))return name;
    return "VK "+std::to_string(key);
}
void corner_box(ImDrawList* d,ImVec2 lo,ImVec2 hi,ImU32 c,float thickness){
    const float w=(hi.x-lo.x)*0.27f,h=(hi.y-lo.y)*0.20f;
    for(int ix=0;ix<2;++ix)for(int iy=0;iy<2;++iy){
        const float x=ix?hi.x:lo.x,y=iy?hi.y:lo.y;
        d->AddLine({x,y},{x+(ix?-w:w),y},c,thickness);d->AddLine({x,y},{x,y+(iy?-h:h)},c,thickness);
    }
}
void meter(ImDrawList* d,float x,float top,float bottom,float fraction,ImU32 c){
    d->AddRectFilled({x-1,top-1},{x+4,bottom+1},IM_COL32(5,9,15,220));
    const float y=bottom-(bottom-top)*std::clamp(fraction,0.0f,1.0f);
    d->AddRectFilled({x,y},{x+3,bottom},c);
}
}
void Ui::initialize(){
    auto& s=ImGui::GetStyle();ImGui::StyleColorsDark();
    s.WindowRounding=12;s.FrameRounding=5;s.GrabRounding=5;s.TabRounding=5;s.WindowPadding={20,18};s.ItemSpacing={10,10};
    s.Colors[ImGuiCol_WindowBg]={0.045f,0.06f,0.085f,0.98f};s.Colors[ImGuiCol_Border]={0.16f,0.22f,0.29f,1};
    s.Colors[ImGuiCol_FrameBg]={0.10f,0.14f,0.19f,1};s.Colors[ImGuiCol_FrameBgHovered]={0.13f,0.21f,0.27f,1};
    s.Colors[ImGuiCol_CheckMark]={0.28f,0.88f,0.68f,1};s.Colors[ImGuiCol_SliderGrab]={0.28f,0.88f,0.68f,1};
    s.Colors[ImGuiCol_Button]={0.09f,0.28f,0.25f,1};s.Colors[ImGuiCol_ButtonHovered]={0.12f,0.41f,0.34f,1};
    s.Colors[ImGuiCol_TabSelected]={0.09f,0.30f,0.25f,1};s.Colors[ImGuiCol_TabHovered]={0.15f,0.44f,0.36f,1};
    const auto path=directory_/"settings.json";
    if(std::filesystem::exists(path))load_settings(path,settings,feedback_);
}
void Ui::capture_key(){
    if(!capture_)return;
    if(!capture_ready_){bool down=false;for(int k=1;k<=254;++k)down|=(GetAsyncKeyState(k)&0x8000)!=0;if(!down)capture_ready_=true;return;}
    for(int k=1;k<=254;++k)if(GetAsyncKeyState(k)&0x8000){
        if(k==VK_ESCAPE){capture_=false;return;}
        if(k==VK_INSERT||k==VK_END)continue;
        settings.aim.key=k;capture_=false;return;
    }
}
void Ui::menu(const Frame& f){
    if(!menu_open){capture_=false;return;}capture_key();
    ImGui::SetNextWindowSize({580,480},ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos({40,70},ImGuiCond_FirstUseEver);
    if(ImGui::Begin("TOKER / kindmops",&menu_open,ImGuiWindowFlags_NoCollapse)){
        ImGui::TextColored({0.35f,0.95f,0.75f,1},"TOKER");ImGui::SameLine();ImGui::TextDisabled("  /  %s",f.preview?"PREVIEW":"STANDOFF 2 / EXTERNAL");
        ImGui::TextWrapped("%s",f.status.c_str());ImGui::Separator();
        if(ImGui::BeginTabBar("main")){
            if(ImGui::BeginTabItem("VISUALS")){
                ImGui::Checkbox("ESP",&settings.esp);ImGui::SameLine();ImGui::Checkbox("Enemies only##esp",&settings.enemies_only);
                ImGui::Combo("Box",&settings.box_style,"Off\0Full box\0Corner box\0");
                ImGui::Checkbox("Straight snapline",&settings.snapline);ImGui::SameLine();ImGui::Checkbox("Spline snapline",&settings.spline);
                ImGui::Checkbox("Health bar",&settings.health);ImGui::SameLine();ImGui::Checkbox("Armour bar",&settings.armour);
                ImGui::Checkbox("Weapon",&settings.weapon);ImGui::SameLine();ImGui::Checkbox("Magazine rounds",&settings.ammo);
                ImGui::Checkbox("Distance",&settings.distance);ImGui::SameLine();ImGui::Checkbox("Names",&settings.names);
                ImGui::SliderFloat("Range (m)",&settings.max_distance_m,5,300,"%.0f");
                ImGui::ColorEdit4("Enemy",settings.enemy_color.data(),ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();ImGui::ColorEdit4("Team",settings.team_color.data(),ImGuiColorEditFlags_NoInputs);
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("AIM")){
                ImGui::Checkbox("Aim enabled",&settings.aim.enabled);ImGui::SameLine();ImGui::Checkbox("Show FOV",&settings.fov_circle);
                ImGui::Text("Hold: %s",key_name(settings.aim.key).c_str());ImGui::SameLine();
                if(ImGui::Button(capture_?"Press key (Esc cancels)":"Bind key")){capture_=true;capture_ready_=false;}
                if(ImGui::Button("Alt"))settings.aim.key=VK_MENU;ImGui::SameLine();
                if(ImGui::Button("Left mouse"))settings.aim.key=VK_LBUTTON;ImGui::SameLine();
                if(ImGui::Button("Right mouse"))settings.aim.key=VK_RBUTTON;
                int point=int(settings.aim.point);if(ImGui::Combo("Target",&point,"Head\0Chest\0Spine\0"))settings.aim.point=HitPoint(point);
                ImGui::SliderFloat("FOV radius (pixels)",&settings.aim.fov,10,500,"%.1f");
                ImGui::SliderFloat("Smoothing (seconds)",&settings.aim.smoothing_seconds,0,0.5f,"%.3f");
                ImGui::SliderFloat("Mouse gain",&settings.aim.mouse_gain,0.01f,2.0f,"%.2f");
                ImGui::SliderFloat("Maximum mouse step",&settings.aim.max_step,1,100,"%.0f");
                ImGui::SliderFloat("Aim range (m)",&settings.aim.max_distance_m,5,300,"%.0f");
                ImGui::Checkbox("Enemies only##aim",&settings.aim.enemies_only);
                ImGui::Checkbox("Visibility flag",&settings.aim.spotted_only);
                ImGui::TextDisabled("Visibility requires a verified game field.");
                ImGui::TextDisabled("Aim pauses while this menu is open.");
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("MOVEMENT")){
                ImGui::Checkbox("Show speed",&settings.speed);
                ImGui::Text("Horizontal: %.1f units/s",std::hypot(f.velocity.x,f.velocity.y));
                ImGui::Text("Vertical: %.1f units/s",f.velocity.z);
                ImGui::Text("Position: %.1f, %.1f, %.1f",f.origin.x,f.origin.y,f.origin.z);
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("MISC")){
                ImGui::Checkbox("Radar overlay",&settings.radar);
                ImGui::SliderFloat("Radar range (m)",&settings.radar_range_m,5,200,"%.0f");
                ImGui::SliderFloat("Radar size (px)",&settings.radar_size,150,400,"%.0f");
                ImGui::TextWrapped("Drag the radar by its title. It rotates with your view.");
                ImGui::Text("Frame rate: %.0f",ImGui::GetIO().Framerate);
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("CONFIG")){
                if(ImGui::Button("Save JSON")){if(save_settings(directory_/"settings.json",settings,feedback_))feedback_="Settings saved";}
                ImGui::SameLine();if(ImGui::Button("Load JSON")){if(load_settings(directory_/"settings.json",settings,feedback_))feedback_="Settings loaded";}
                ImGui::SameLine();if(ImGui::Button("Defaults")){settings=Settings{};feedback_="Defaults restored (not saved)";}
                if(!f.preview&&ImGui::Button("Reload layout"))reload_offsets=true;
                ImGui::TextWrapped("%s",directory_.string().c_str());
                if(!feedback_.empty())ImGui::TextWrapped("%s",feedback_.c_str());
                ImGui::TextDisabled("Insert: menu | End: exit live overlay");
                ImGui::TextDisabled("layout.json reloads automatically; session mismatch blocks reads.");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }ImGui::End();
}
void Ui::visuals(const Frame& f){
    auto* d=ImGui::GetBackgroundDrawList();const auto display=ImGui::GetIO().DisplaySize;const Vec2 size{display.x,display.y};
    const ImVec2 center{size.x/2,size.y/2};
    if(f.preview){
        // The demo scene and ESP share the same camera matrix.
        for(int i=-8;i<=8;++i){
            auto a=project({100,float(i*100),0},f.matrix,size),b=project({2000,float(i*100),0},f.matrix,size);
            if(a&&b)d->AddLine(iv(*a),iv(*b),IM_COL32(39,50,66,180));
        }
        for(int i=2;i<=20;++i){auto a=project({float(i*100),-1000,0},f.matrix,size),b=project({float(i*100),1000,0},f.matrix,size);if(a&&b)d->AddLine(iv(*a),iv(*b),IM_COL32(39,50,66,180));}
        d->AddLine({center.x-5,center.y},{center.x+5,center.y},IM_COL32_WHITE);d->AddLine({center.x,center.y-5},{center.x,center.y+5},IM_COL32_WHITE);
    }
    if(settings.fov_circle&&f.ready){
        d->AddCircle(center,settings.aim.fov,IM_COL32(110,225,190,110),80,1);
    }
    for(const auto& p:f.players){
        const auto head=project(p.head+Vec3{0,0,6},f.matrix,size),feet=project(p.feet-Vec3{0,0,2},f.matrix,size);
        if(!head||!feet)continue;
        const float h=feet->y-head->y,w=h*0.43f;
        if(h<3||h>size.y*3)continue;
        const ImVec2 lo{head->x-w/2,head->y},hi{head->x+w/2,feet->y};
        if(hi.x<0||lo.x>size.x||hi.y<0||lo.y>size.y)continue;
        if(f.preview){
            const auto c=p.team==f.team?IM_COL32(47,83,112,220):IM_COL32(90,56,60,220);
            d->AddRectFilled({lo.x+w*0.15f,lo.y+h*0.20f},{hi.x-w*0.15f,lo.y+h*0.69f},c,4);
            d->AddCircleFilled({head->x,lo.y+h*0.11f},w*0.18f,c);
            d->AddLine({head->x-w*0.16f,lo.y+h*0.65f},{head->x-w*0.19f,hi.y},c,w*0.22f);
            d->AddLine({head->x+w*0.16f,lo.y+h*0.65f},{head->x+w*0.19f,hi.y},c,w*0.22f);
        }
        const float distance=length(p.feet-f.origin)*meters_per_unit;
        if(!settings.esp||(settings.enemies_only&&p.team==f.team)||distance>settings.max_distance_m)continue;
        const auto c=color(p.team==f.team?settings.team_color:settings.enemy_color);
        if(settings.box_style==1){d->AddRect(lo,hi,IM_COL32(0,0,0,200),0,0,3);d->AddRect(lo,hi,c,0,0,1);}
        if(settings.box_style==2){corner_box(d,lo,hi,IM_COL32(0,0,0,200),3);corner_box(d,lo,hi,c,1);}
        if(settings.snapline)d->AddLine({center.x,size.y},iv(*feet),c,1);
        if(settings.spline)d->AddBezierCubic({center.x,size.y},{center.x,size.y*0.65f},{feet->x,size.y*0.65f},iv(*feet),c,1.2f);
        if(settings.health)meter(d,lo.x-7,lo.y,hi.y,float(p.health)/p.max_health,IM_COL32(97,224,151,255));
        if(settings.armour)meter(d,hi.x+4,lo.y,hi.y,p.armour/100.0f,IM_COL32(95,170,245,255));
        if(settings.names)label(d,{head->x,lo.y-17},c,p.name);
        std::string detail;if(settings.weapon)detail=p.weapon;
        if(settings.ammo&&p.ammo>=0){if(!detail.empty())detail+="  ";detail+='['+std::to_string(p.ammo)+']';}
        float text_y=hi.y+5;if(!detail.empty()){label(d,{head->x,text_y},IM_COL32(230,235,240,255),detail);text_y+=16;}
        if(settings.distance){char text[24]{};std::snprintf(text,sizeof(text),"%.0f m",distance);label(d,{head->x,text_y},IM_COL32(170,190,210,255),text);}
    }
    if(settings.speed){char text[64]{};std::snprintf(text,sizeof(text),"%.0f units/s",std::hypot(f.velocity.x,f.velocity.y));label(d,{center.x,size.y-55},IM_COL32(200,240,230,255),text);}
    label(d,{18,16},IM_COL32(121,231,197,255),"TOKER / kindmops",false);
    if(!f.ready||f.preview)label(d,{18,36},IM_COL32(180,195,210,255),f.status,false);
}
void Ui::radar(const Frame& f){
    if(!settings.radar||!f.ready)return;
    ImGui::SetNextWindowSize({settings.radar_size,settings.radar_size+28});
    ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x-settings.radar_size-24,56},ImGuiCond_FirstUseEver);
    const auto flags=ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoScrollbar|(menu_open?0:ImGuiWindowFlags_NoInputs);
    if(ImGui::Begin("RADAR",nullptr,flags)){
        auto* d=ImGui::GetWindowDrawList();const auto pos=ImGui::GetCursorScreenPos(),avail=ImGui::GetContentRegionAvail();
        const ImVec2 center{pos.x+avail.x/2,pos.y+avail.y/2};const float r=std::min(avail.x,avail.y)/2-4;
        d->AddCircleFilled(center,r,IM_COL32(14,23,32,255));d->AddCircle(center,r,IM_COL32(64,94,110,255),64);
        d->AddCircle(center,r/2,IM_COL32(42,62,74,255),64);
        d->AddLine({center.x-r,center.y},{center.x+r,center.y},IM_COL32(42,62,74,255));d->AddLine({center.x,center.y-r},{center.x,center.y+r},IM_COL32(42,62,74,255));
        for(const auto& p:f.players){
            if(settings.enemies_only&&p.team==f.team)continue;
            const auto v=radar_position(p.feet-f.origin,f.angles.y,settings.radar_range_m/meters_per_unit,r-5);
            d->AddCircleFilled({center.x+v.x,center.y+v.y},3.5f,color(p.team==f.team?settings.team_color:settings.enemy_color));
        }
        d->AddTriangleFilled({center.x,center.y-6},{center.x-5,center.y+4},{center.x+5,center.y+4},IM_COL32(120,240,200,255));
        ImGui::Dummy(avail);
    }ImGui::End();
}
void Ui::render(const Frame& f){visuals(f);radar(f);menu(f);}
}
