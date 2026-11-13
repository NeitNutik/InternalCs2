#include "toker/hook.hpp"

namespace {
DWORD WINAPI start(void* module){
    if(!toker::install_hooks(static_cast<HMODULE>(module)))OutputDebugStringA("TOKER: DX11 hook installation failed\n");
    return 0;
}
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID){
    if(reason==DLL_PROCESS_ATTACH){
        DisableThreadLibraryCalls(module);
        if(const HANDLE thread=CreateThread(nullptr,0,start,module,0,nullptr))CloseHandle(thread);
    }
    return TRUE;
}
