#pragma once
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace toker {
inline void graphics_failure(HWND window,const std::filesystem::path& directory,
    const char* operation,HRESULT result,HRESULT device_reason){
    SYSTEMTIME time{};GetLocalTime(&time);
    std::ostringstream text;
    text<<operation<<": HRESULT 0x"<<std::hex<<std::uppercase<<static_cast<unsigned long>(result)
        <<"; device status 0x"<<static_cast<unsigned long>(device_reason);
    const auto path=directory/"graphics-error.log";
    std::ofstream log(path,std::ios::app);
    const bool opened=bool(log);
    if(opened)log<<std::dec<<time.wYear<<'-'<<time.wMonth<<'-'<<time.wDay<<' '
        <<time.wHour<<':'<<time.wMinute<<':'<<time.wSecond<<" "<<text.str()<<'\n';
    log.flush();
    const auto message=text.str()+(opened&&log?"\nDetails: "+path.string():"\nCould not write the diagnostic log.");
    OutputDebugStringA(message.c_str());
    MessageBoxA(window,message.c_str(),"TOKER graphics failure",MB_OK|MB_ICONERROR);
}
}
