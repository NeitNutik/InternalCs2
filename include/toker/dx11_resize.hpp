#pragma once
#include <d3d11.h>
#include <wrl/client.h>

namespace toker {
inline HRESULT resize_target(IDXGISwapChain* chain,ID3D11Device* device,
    ID3D11DeviceContext* context,Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& target,
    UINT width,UINT height){
    if(!width||!height)return E_INVALIDARG;
    // OMSetRenderTargets retains a COM reference even after target.Reset().
    context->OMSetRenderTargets(0,nullptr,nullptr);
    target.Reset();
    context->Flush();
    HRESULT result=chain->ResizeBuffers(0,width,height,DXGI_FORMAT_UNKNOWN,0);
    if(FAILED(result))return result;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer;
    result=chain->GetBuffer(0,IID_PPV_ARGS(&buffer));
    if(FAILED(result))return result;
    return device->CreateRenderTargetView(buffer.Get(),nullptr,&target);
}
}
