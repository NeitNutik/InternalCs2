#include "toker/ui.hpp"
#include <windows.h>
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
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
}
int WINAPI WinMain(HINSTANCE instance,HINSTANCE,LPSTR,int){
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc{sizeof(WNDCLASSEXW)};wc.style=CS_CLASSDC;wc.lpfnWndProc=wndproc;wc.hInstance=instance;wc.lpszClassName=L"TokerPreview";
    if(!RegisterClassExW(&wc))return 1;
    HWND hwnd=CreateWindowW(wc.lpszClassName,L"TOKER / Standoff 2 / preview",WS_OVERLAPPEDWINDOW,100,100,1440,900,nullptr,nullptr,instance,nullptr);
    if(!hwnd)return 2;
    DXGI_SWAP_CHAIN_DESC desc{};desc.BufferCount=2;desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow=hwnd;desc.SampleDesc.Count=1;desc.Windowed=TRUE;desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
    auto result=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&desc,&chain,&device,nullptr,&context);
    if(FAILED(result))result=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&desc,&chain,&device,nullptr,&context);
    if(FAILED(result)||!create_target()){MessageBoxW(hwnd,L"Direct3D 11 device creation failed",L"TOKER",MB_ICONERROR);DestroyWindow(hwnd);return 3;}
    IMGUI_CHECKVERSION();ImGui::CreateContext();
    wchar_t module_path[32768]{};GetModuleFileNameW(nullptr,module_path,DWORD(std::size(module_path)));
    const auto root=std::filesystem::path(module_path).parent_path()/"config";std::filesystem::create_directories(root);
    const std::string ini=(root/"preview.ini").string();ImGui::GetIO().IniFilename=ini.c_str();
    if(!ImGui_ImplWin32_Init(hwnd)||!ImGui_ImplDX11_Init(device.Get(),context.Get()))return 4;
    toker::Ui ui(root);ui.initialize();ShowWindow(hwnd,SW_SHOWDEFAULT);UpdateWindow(hwnd);
    bool done=false,insert_down=false;toker::Vec3 angles{};
    const auto started=std::chrono::steady_clock::now();auto last=started;
    while(!done){
        MSG msg{};while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);if(msg.message==WM_QUIT)done=true;}
        if(done)break;
        if(IsIconic(hwnd)){WaitMessage();continue;}
        if(next_width&&next_height){target.Reset();if(FAILED(chain->ResizeBuffers(0,next_width,next_height,DXGI_FORMAT_UNKNOWN,0))||!create_target())break;next_width=next_height=0;}
        const bool focused=GetForegroundWindow()==hwnd;
        const bool insert=focused&&(GetAsyncKeyState(VK_INSERT)&0x8000);if(insert&&!insert_down)ui.menu_open=!ui.menu_open;insert_down=insert;
        const auto now=std::chrono::steady_clock::now();const float dt=std::chrono::duration<float>(now-last).count();last=now;
        const float elapsed=std::chrono::duration<float>(now-started).count();
        ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();
        const auto size=ImGui::GetIO().DisplaySize;
        auto frame=toker::demo_frame(elapsed,{size.x,size.y},angles);
        if(focused&&!ui.menu_open){
            if(GetAsyncKeyState(VK_LEFT)&0x8000)angles.y+=dt*30;
            if(GetAsyncKeyState(VK_RIGHT)&0x8000)angles.y-=dt*30;
            if(GetAsyncKeyState(VK_UP)&0x8000)angles.x-=dt*30;
            if(GetAsyncKeyState(VK_DOWN)&0x8000)angles.x+=dt*30;
            if(GetAsyncKeyState('R')&0x8000)angles={};
            frame.angles=angles;
            if(ui.settings.aim.enabled&&(GetAsyncKeyState(ui.settings.aim.key)&0x8000))if(auto selection=toker::select_target(frame,ui.settings.aim,{size.x,size.y})){
                const auto step=toker::mouse_step({selection->screen.x-size.x/2,selection->screen.y-size.y/2},ui.settings.aim,dt);
                angles.y-=std::atan2(step.x,size.x/2)*toker::degrees;
                angles.x+=std::atan2(step.y,size.x/2)*toker::degrees;
            }
            angles=toker::clamp_angles(angles);frame=toker::demo_frame(elapsed,{size.x,size.y},angles);
        }
        ui.render(frame);ImGui::Render();
        const float clear[4]{0.035f,0.045f,0.065f,1};auto* view=target.Get();context->OMSetRenderTargets(1,&view,nullptr);context->ClearRenderTargetView(view,clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        if(FAILED(chain->Present(1,0)))break;
    }
    ImGui_ImplDX11_Shutdown();ImGui_ImplWin32_Shutdown();ImGui::DestroyContext();
    target.Reset();chain.Reset();context.Reset();device.Reset();DestroyWindow(hwnd);UnregisterClassW(wc.lpszClassName,instance);return 0;
}
