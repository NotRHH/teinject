
import <windows.h>;

import std;
import eel.debug;
import teinject.mod_loader;

//-----------------------------------------------------------------------------

extern "C" __declspec(dllexport) void __cdecl Initialize()
{
    try {
        teinject::mod_loader::Initialize();
    } catch (std::exception const& ex) {
        eel::debug::exit_with_error(std::format("Initialization error: {}", ex.what()));
    } catch (...) {
        eel::debug::exit_with_error("Initialization error: unknown exception type");
    }
}

//-----------------------------------------------------------------------------

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);
    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        break;

    case DLL_THREAD_ATTACH:
        eel::debug::set_signal_handler();
        break;

    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
