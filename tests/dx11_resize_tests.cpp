#include "toker/dx11_resize.hpp"
#include <iostream>

int main(){
    using Microsoft::WRL::ComPtr;
    const HINSTANCE instance=GetModuleHandleW(nullptr);
    WNDCLASSW cls{};cls.hInstance=instance;cls.lpfnWndProc=DefWindowProcW;cls.lpszClassName=L"TokerResizeTest";
    if(!RegisterClassW(&cls))return 1;
    HWND window=CreateWindowW(cls.lpszClassName,L"Resize test",WS_OVERLAPPEDWINDOW,0,0,320,240,nullptr,nullptr,instance,nullptr);
    if(!window)return 2;
    ComPtr<ID3D11Device> device;ComPtr<ID3D11DeviceContext> context;ComPtr<IDXGISwapChain> chain;
    DXGI_SWAP_CHAIN_DESC desc{};desc.BufferCount=2;desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;desc.OutputWindow=window;desc.SampleDesc.Count=1;desc.Windowed=TRUE;
    auto result=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&desc,&chain,&device,nullptr,&context);
    if(FAILED(result)){std::cerr<<"WARP initialization failed: "<<std::hex<<result<<'\n';return 3;}
    ComPtr<ID3D11RenderTargetView> target;
    result=toker::resize_target(chain.Get(),device.Get(),context.Get(),target,320,240);
    if(FAILED(result))return 4;
    for(UINT i=0;i<12;++i){
        // Reproduce the real application's retained context reference.
        auto* view=target.Get();context->OMSetRenderTargets(1,&view,nullptr);
        const UINT width=320+i*17,height=240+i*11;
        result=toker::resize_target(chain.Get(),device.Get(),context.Get(),target,width,height);
        if(FAILED(result)){std::cerr<<"Resize "<<i<<" failed: "<<std::hex<<result<<'\n';return 5;}
        ComPtr<ID3D11Texture2D> buffer;result=chain->GetBuffer(0,IID_PPV_ARGS(&buffer));
        if(FAILED(result)||!target)return 6;
        D3D11_TEXTURE2D_DESC actual{};buffer->GetDesc(&actual);
        if(actual.Width!=width||actual.Height!=height)return 7;
    }
    context->OMSetRenderTargets(0,nullptr,nullptr);target.Reset();chain.Reset();context.Reset();device.Reset();
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,instance);
    std::cout<<"12 bound-target resizes passed\n";return 0;
}
