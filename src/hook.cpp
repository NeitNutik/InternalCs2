#include "toker/hook.hpp"
#include "toker/game.hpp"
#include "toker/ui.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <MinHook.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <chrono>
#include <mutex>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND,UINT,WPARAM,LPARAM);

namespace toker {
namespace {
using Microsoft::WRL::ComPtr;
using Present=HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*,UINT,UINT);
using Resize=HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*,UINT,UINT,UINT,DXGI_FORMAT,UINT);
// IDXGISwapChain interface slots are fixed by the DXGI ABI, not game offsets.
constexpr unsigned present_slot=8,resize_slot=13;
Present original_present{};Resize original_resize{};
struct Runtime {
    explicit Runtime(std::filesystem::path root):ui(root/"config"),game(root/"config"/"offsets.json"),ini((root/"config"/"imgui.ini").string()){}
    std::recursive_mutex mutex;
    Ui ui;Game game;std::string ini;
    HWND window{};WNDPROC wndproc{};IDXGISwapChain* chain{};
    ComPtr<ID3D11Device> device;ComPtr<ID3D11DeviceContext> context;ComPtr<ID3D11RenderTargetView> target;
    bool initialized{},insert_down{};
    std::chrono::steady_clock::time_point last=std::chrono::steady_clock::now();
};
Runtime* runtime{};
bool input_message(UINT m){
    return (m>=WM_MOUSEFIRST&&m<=WM_MOUSELAST)||(m>=WM_KEYFIRST&&m<=WM_KEYLAST)||m==WM_INPUT||m==WM_SETCURSOR;
}
LRESULT CALLBACK wndproc(HWND w,UINT message,WPARAM wp,LPARAM lp){
    std::lock_guard lock(runtime->mutex);
    if(runtime->initialized){
        ImGui_ImplWin32_WndProcHandler(w,message,wp,lp);
        if(runtime->ui.menu_open&&input_message(message)){
            if(message==WM_INPUT)return DefWindowProcW(w,message,wp,lp);
            return 1;
        }
    }
    return CallWindowProcW(runtime->wndproc,w,message,wp,lp);
}
bool create_target(IDXGISwapChain* chain){
    ComPtr<ID3D11Texture2D> backbuffer;
    return SUCCEEDED(chain->GetBuffer(0,IID_PPV_ARGS(&backbuffer)))&&
        SUCCEEDED(runtime->device->CreateRenderTargetView(backbuffer.Get(),nullptr,&runtime->target));
}
bool initialize(IDXGISwapChain* chain){
    auto& r=*runtime;DXGI_SWAP_CHAIN_DESC desc{};
    if(FAILED(chain->GetDesc(&desc))||!desc.OutputWindow)return false;
    DWORD pid{};GetWindowThreadProcessId(desc.OutputWindow,&pid);
    if(pid!=GetCurrentProcessId()||!IsWindowVisible(desc.OutputWindow))return false;
    if(FAILED(chain->GetDevice(IID_PPV_ARGS(&r.device))))return false;
    r.device->GetImmediateContext(&r.context);r.window=desc.OutputWindow;
    if(!create_target(chain))return false;
    IMGUI_CHECKVERSION();ImGui::CreateContext();ImGui::GetIO().IniFilename=r.ini.c_str();
    if(!ImGui_ImplWin32_Init(r.window)){ImGui::DestroyContext();return false;}
    if(!ImGui_ImplDX11_Init(r.device.Get(),r.context.Get())){ImGui_ImplWin32_Shutdown();ImGui::DestroyContext();return false;}
    SetLastError(0);r.wndproc=reinterpret_cast<WNDPROC>(SetWindowLongPtrW(r.window,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(wndproc)));
    if(!r.wndproc){ImGui_ImplDX11_Shutdown();ImGui_ImplWin32_Shutdown();ImGui::DestroyContext();return false;}
    r.chain=chain;r.initialized=true;r.ui.initialize();r.game.reload();return true;
}
HRESULT STDMETHODCALLTYPE present(IDXGISwapChain* chain,UINT interval,UINT flags){
    {
        std::lock_guard lock(runtime->mutex);auto& r=*runtime;
        if(!(flags&DXGI_PRESENT_TEST)){
            try{
                if(!r.initialized)initialize(chain);
                if(r.initialized&&r.chain==chain&&(r.target||create_target(chain))){
                    DWORD foreground_pid{};GetWindowThreadProcessId(GetForegroundWindow(),&foreground_pid);
                    const bool foreground=foreground_pid==GetCurrentProcessId();
                    const bool key=foreground&&(GetAsyncKeyState(VK_INSERT)&0x8000);
                    if(key&&!r.insert_down)r.ui.menu_open=!r.ui.menu_open;r.insert_down=key;
                    const auto now=std::chrono::steady_clock::now();const float dt=std::chrono::duration<float>(now-r.last).count();r.last=now;
                    if(r.ui.reload_offsets){r.game.reload();r.ui.reload_offsets=false;}
                    const auto frame=r.game.sample();
                    r.game.aim(frame,r.ui.settings.aim,dt,r.ui.menu_open);
                    ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::GetIO().MouseDrawCursor=r.ui.menu_open&&foreground;
                    if(r.ui.menu_open&&foreground)ClipCursor(nullptr);
                    ImGui::NewFrame();r.ui.render(frame);ImGui::Render();
                    std::array<ID3D11RenderTargetView*,D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> old_targets{};ID3D11DepthStencilView* old_depth{};
                    r.context->OMGetRenderTargets(UINT(old_targets.size()),old_targets.data(),&old_depth);
                    auto* target=r.target.Get();r.context->OMSetRenderTargets(1,&target,nullptr);
                    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
                    r.context->OMSetRenderTargets(UINT(old_targets.size()),old_targets.data(),old_depth);
                    for(auto* t:old_targets)if(t)t->Release();if(old_depth)old_depth->Release();
                }
            }catch(const std::exception& e){OutputDebugStringA(e.what());}
        }
    }
    return original_present(chain,interval,flags);
}
HRESULT STDMETHODCALLTYPE resize(IDXGISwapChain* chain,UINT count,UINT width,UINT height,DXGI_FORMAT format,UINT flags){
    std::lock_guard lock(runtime->mutex);
    if(runtime->chain==chain)runtime->target.Reset();
    return original_resize(chain,count,width,height,format,flags);
}
}
bool install_hooks(HMODULE module){
    wchar_t module_path[32768]{};
    const auto path_len=GetModuleFileNameW(module,module_path,DWORD(std::size(module_path)));
    if(!path_len||path_len>=std::size(module_path))return false;
    runtime=new Runtime(std::filesystem::path(module_path).parent_path());
    const wchar_t* class_name=L"TokerDxgiBootstrap";
    WNDCLASSW wc{};wc.lpfnWndProc=DefWindowProcW;wc.hInstance=module;wc.lpszClassName=class_name;
    if(!RegisterClassW(&wc))return false;
    HWND window=CreateWindowExW(0,class_name,L"",WS_OVERLAPPEDWINDOW,0,0,64,64,nullptr,nullptr,module,nullptr);
    if(!window){UnregisterClassW(class_name,module);return false;}
    DXGI_SWAP_CHAIN_DESC desc{};desc.BufferCount=1;desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;desc.OutputWindow=window;desc.SampleDesc.Count=1;desc.Windowed=TRUE;
    desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
    ComPtr<IDXGISwapChain> chain;ComPtr<ID3D11Device> device;ComPtr<ID3D11DeviceContext> context;
    auto hr=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&desc,&chain,&device,nullptr,&context);
    if(FAILED(hr))hr=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&desc,&chain,&device,nullptr,&context);
    bool installed=false;
    if(SUCCEEDED(hr)){
        auto** vtable=*reinterpret_cast<void***>(chain.Get());
        void* present_address=vtable[present_slot];void* resize_address=vtable[resize_slot];
        if(MH_Initialize()==MH_OK){
            const bool created=MH_CreateHook(present_address,reinterpret_cast<void*>(present),reinterpret_cast<void**>(&original_present))==MH_OK&&
                MH_CreateHook(resize_address,reinterpret_cast<void*>(resize),reinterpret_cast<void**>(&original_resize))==MH_OK;
            if(created&&MH_QueueEnableHook(present_address)==MH_OK&&MH_QueueEnableHook(resize_address)==MH_OK&&MH_ApplyQueued()==MH_OK)installed=true;
            if(!installed){MH_DisableHook(present_address);MH_DisableHook(resize_address);MH_RemoveHook(present_address);MH_RemoveHook(resize_address);MH_Uninitialize();}
        }
    }
    chain.Reset();context.Reset();device.Reset();DestroyWindow(window);UnregisterClassW(class_name,module);
    return installed;
}
}
