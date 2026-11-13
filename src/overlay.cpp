#include "toker/dx11_resize.hpp"
#include "toker/diagnostics.hpp"
#include "toker/game.hpp"
#include "toker/ui.hpp"
#include <d3d11.h>
#include <wrl/client.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <chrono>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND,UINT,WPARAM,LPARAM);
namespace {
using Microsoft::WRL::ComPtr;
ComPtr<ID3D11Device> device;ComPtr<ID3D11DeviceContext> context;ComPtr<IDXGISwapChain> chain;ComPtr<ID3D11RenderTargetView> target;
UINT next_width{},next_height{};
bool create_target(){ComPtr<ID3D11Texture2D> buffer;return SUCCEEDED(chain->GetBuffer(0,IID_PPV_ARGS(&buffer)))&&SUCCEEDED(device->CreateRenderTargetView(buffer.Get(),nullptr,&target));}
LRESULT CALLBACK wndproc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    if(ImGui_ImplWin32_WndProcHandler(hwnd,msg,wp,lp))return true;
    switch(msg){
    case WM_SIZE:if(wp!=SIZE_MINIMIZED){next_width=LOWORD(lp);next_height=HIWORD(lp);}return 0;
    case WM_SYSCOMMAND:if((wp&0xfff0)==SC_KEYMENU)return 0;break;
    case WM_DESTROY:PostQuitMessage(0);return 0;
    }return DefWindowProcW(hwnd,msg,wp,lp);
}
struct FindWindow {DWORD pid;HWND found{};LONG area{};};
BOOL CALLBACK enumerate(HWND window,LPARAM parameter){
    auto& query=*reinterpret_cast<FindWindow*>(parameter);DWORD pid{};GetWindowThreadProcessId(window,&pid);
    if(pid!=query.pid||!IsWindowVisible(window)||GetWindow(window,GW_OWNER))return TRUE;
    RECT r{};GetClientRect(window,&r);const auto area=(r.right-r.left)*(r.bottom-r.top);
    if(area>query.area){query.found=window;query.area=area;}return TRUE;
}
HWND emulator_window(DWORD pid){if(!pid)return nullptr;FindWindow query{pid};EnumWindows(enumerate,reinterpret_cast<LPARAM>(&query));return query.found;}
bool viewport(HWND window,const toker::Layout& l,RECT& out){
    RECT r{};POINT p{};if(!GetClientRect(window,&r)||!ClientToScreen(window,&p))return false;
    out={p.x+l.viewport_left,p.y+l.viewport_top,p.x+r.right-l.viewport_right,p.y+r.bottom-l.viewport_bottom};
    return out.right-out.left>=64&&out.bottom-out.top>=64;
}
class MouseAim {
public:
    void reset(){remainder_={};handle_=0;}
    bool run(const toker::Frame& frame,const toker::AimOptions& options,toker::Vec2 size,float dt,bool active){
        if(!active||!frame.ready||!options.enabled||!(GetAsyncKeyState(options.key)&0x8000)){reset();return true;}
        const auto selected=toker::select_target(frame,options,size);if(!selected){reset();return true;}
        if(handle_!=selected->handle){reset();handle_=selected->handle;}
        const auto step=toker::mouse_step({selected->screen.x-size.x/2,selected->screen.y-size.y/2},options,dt);
        remainder_.x+=step.x;remainder_.y+=step.y;
        const auto dx=LONG(remainder_.x),dy=LONG(remainder_.y);if(!dx&&!dy)return true;
        INPUT input{};input.type=INPUT_MOUSE;input.mi.dwFlags=MOUSEEVENTF_MOVE;input.mi.dx=dx;input.mi.dy=dy;
        if(SendInput(1,&input,sizeof(input))!=1){reset();return false;}
        remainder_.x-=float(dx);remainder_.y-=float(dy);return true;
    }
private:toker::Vec2 remainder_{};std::uint64_t handle_{};
};
}
int WINAPI WinMain(HINSTANCE instance,HINSTANCE,LPSTR,int){
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc{sizeof(WNDCLASSEXW)};wc.style=CS_CLASSDC;wc.lpfnWndProc=wndproc;wc.hInstance=instance;wc.lpszClassName=L"TokerStandoffOverlay";
    if(!RegisterClassExW(&wc))return 1;
    HWND hwnd=CreateWindowExW(WS_EX_LAYERED|WS_EX_TOPMOST|WS_EX_TOOLWINDOW,wc.lpszClassName,L"TOKER / Standoff 2",WS_POPUP,50,50,1280,720,nullptr,nullptr,instance,nullptr);
    if(!hwnd)return 2;SetLayeredWindowAttributes(hwnd,RGB(0,0,0),255,LWA_COLORKEY);
    DXGI_SWAP_CHAIN_DESC desc{};desc.BufferCount=2;desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow=hwnd;desc.SampleDesc.Count=1;desc.Windowed=TRUE;desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
    auto hr=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&desc,&chain,&device,nullptr,&context);
    if(FAILED(hr))hr=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&desc,&chain,&device,nullptr,&context);
    if(FAILED(hr)||!create_target()){MessageBoxW(hwnd,L"Direct3D 11 initialization failed",L"TOKER",MB_ICONERROR);DestroyWindow(hwnd);return 3;}
    wchar_t module[32768]{};GetModuleFileNameW(nullptr,module,DWORD(std::size(module)));
    const auto root=std::filesystem::path(module).parent_path()/"config";
    std::error_code ec;std::filesystem::create_directories(root,ec);
    const auto ini=(root/"overlay.ini").string();
    IMGUI_CHECKVERSION();ImGui::CreateContext();ImGui::GetIO().IniFilename=ini.c_str();
    if(!ImGui_ImplWin32_Init(hwnd)||!ImGui_ImplDX11_Init(device.Get(),context.Get()))return 4;
    toker::Ui ui(root);ui.initialize();toker::Game game(root/"layout.json");game.reload();MouseAim aim;
    ShowWindow(hwnd,SW_SHOWDEFAULT);UpdateWindow(hwnd);
    bool done=false,insert_down=false,previous_menu=true;HWND emulator{};DWORD attached_pid{};
    auto last=std::chrono::steady_clock::now();
    while(!done){
        MSG message{};while(PeekMessageW(&message,nullptr,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);if(message.message==WM_QUIT)done=true;}
        if(done)break;
        if(ui.reload_offsets){game.reload();ui.reload_offsets=false;aim.reset();}game.poll_reload();
        if(attached_pid!=game.pid()||!IsWindow(emulator)){attached_pid=game.pid();emulator=emulator_window(attached_pid);}
        const HWND foreground=GetAncestor(GetForegroundWindow(),GA_ROOT);
        const bool focused=foreground==hwnd||(emulator&&foreground==emulator);
        if(focused&&(GetAsyncKeyState(VK_END)&0x8000))break;
        const bool insert=focused&&(GetAsyncKeyState(VK_INSERT)&0x8000);
        if(insert&&!insert_down)ui.menu_open=!ui.menu_open;insert_down=insert;
        if(ui.menu_open!=previous_menu){
            auto style=GetWindowLongPtrW(hwnd,GWL_EXSTYLE);
            if(ui.menu_open){style&=~(WS_EX_TRANSPARENT|WS_EX_NOACTIVATE);}else{style|=WS_EX_TRANSPARENT|WS_EX_NOACTIVATE;}
            SetWindowLongPtrW(hwnd,GWL_EXSTYLE,style);previous_menu=ui.menu_open;aim.reset();
            if(ui.menu_open)SetForegroundWindow(hwnd);else if(emulator)SetForegroundWindow(emulator);
        }
        if(emulator){
            RECT r{};
            if(IsIconic(emulator)||!focused||!viewport(emulator,game.layout(),r)){ShowWindow(hwnd,SW_HIDE);aim.reset();last=std::chrono::steady_clock::now();Sleep(30);continue;}
            RECT current{};GetWindowRect(hwnd,&current);
            UINT flags=SWP_NOACTIVATE|SWP_SHOWWINDOW;
            if(current.left==r.left&&current.top==r.top)flags|=SWP_NOMOVE;
            if(current.right-current.left==r.right-r.left&&current.bottom-current.top==r.bottom-r.top)flags|=SWP_NOSIZE;
            SetWindowPos(hwnd,HWND_TOPMOST,r.left,r.top,r.right-r.left,r.bottom-r.top,flags);
        }else if(attached_pid){ShowWindow(hwnd,SW_SHOWNA);}
        if(next_width&&next_height){
            const auto resized=toker::resize_target(chain.Get(),device.Get(),context.Get(),target,next_width,next_height);
            next_width=next_height=0;
            if(FAILED(resized)){toker::graphics_failure(hwnd,root,"ResizeBuffers / render target",resized,device->GetDeviceRemovedReason());break;}
        }
        const auto now=std::chrono::steady_clock::now();const float dt=std::chrono::duration<float>(now-last).count();last=now;
        ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();
        auto frame=game.sample();const auto display=ImGui::GetIO().DisplaySize;
        // Only send input when the emulator itself owns foreground focus.
        if(!aim.run(frame,ui.settings.aim,{display.x,display.y},dt,emulator&&foreground==emulator&&!ui.menu_open))frame.status="Mouse input rejected by Windows";
        ui.render(frame);ImGui::Render();
        const float clear[4]{0,0,0,0};auto* view=target.Get();context->OMSetRenderTargets(1,&view,nullptr);context->ClearRenderTargetView(view,clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());const auto presented=chain->Present(1,0);
        if(FAILED(presented)){toker::graphics_failure(hwnd,root,"Present",presented,device->GetDeviceRemovedReason());break;}
    }
    ImGui_ImplDX11_Shutdown();ImGui_ImplWin32_Shutdown();ImGui::DestroyContext();target.Reset();chain.Reset();context.Reset();device.Reset();
    DestroyWindow(hwnd);UnregisterClassW(wc.lpszClassName,instance);return 0;
}
