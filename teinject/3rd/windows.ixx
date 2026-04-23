module;
#include <time.h>
export module external.windows;

import <windows.h>;
import std;
import eel.util;

using namespace eel::util;

namespace external::windows {

    export
    bool LaunchProcess(zstring_view path, zstring_view args) {
        auto process_information = PROCESS_INFORMATION();
        auto startup_info = STARTUPINFO(sizeof(STARTUPINFO));
        bool result = CreateProcessA(
            path.c_str(),
            std::string(args).data(),
            nullptr,
            nullptr,
            true,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &startup_info,
            &process_information);
        if (result) {
            CloseHandle(process_information.hProcess);
            CloseHandle(process_information.hThread);
        }
        return result;
    }

    export
    bool IsAltPressed() {
        return GetAsyncKeyState(VK_MENU) & 0x8000;
    }

    export
    bool IsShiftPressed() {
        return GetAsyncKeyState(VK_SHIFT) & 0x8000;
    }
    
    export 
    bool IsCtrlPressed() {
        return GetAsyncKeyState(VK_CONTROL) & 0x8000;
    } 
    
    export
    bool IsKeyPressed(int key) {
        return GetAsyncKeyState(key) & 0x8000;
    }
    
    export
    bool IsApplicationWindowFocused() {
        auto foreground_hwnd = GetForegroundWindow();
        DWORD foreground_pid;
        GetWindowThreadProcessId(foreground_hwnd, &foreground_pid);

        return foreground_pid == GetCurrentProcessId();
    }
    
    export
    std::tm CurrentTime() {
        auto tm = std::tm();
        auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        localtime_s(&tm, &time);
        return tm;
    }

    export
    std::string FormatDate(std::tm const& tm) {
        return std::format(
            "{:04}.{:02}.{:02} {:02}:{:02}:{:02}",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

    export
    bool InvokeQuestionBox(
        zstring_view title,
        zstring_view message)
    {
        return MessageBoxA(nullptr, message.c_str(), title.c_str(), MB_YESNO) == IDYES;
    }

    export
    void InvokeMessageBox(
        zstring_view title,
        zstring_view message)
    {
        MessageBoxA(nullptr, message.c_str(), title.c_str(), MB_OK);
    }
    
}
