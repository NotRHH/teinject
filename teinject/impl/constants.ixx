module;
#include "resource.h"
export module teinject.constants;

import std;
import eel.util;
import <windows.h>;

using namespace std::literals;
using namespace eel::util;

namespace teinject::inline constants {
    
    export constexpr auto LIBRARY_NAME = "TEINJECT"_zsv;
    export constexpr auto VERSION_NUM = std::tuple(DLL_VERSION_A, DLL_VERSION_B, DLL_VERSION_C);
    export constexpr auto VERSION_STR = DLL_VERSION_STRING_SHORT ""_zsv;
    
    export constexpr auto LOADER_PATH = "./teinject.loader.exe"_zsv;
    export constexpr auto DLL_EXTENSION = ".dll"sv;
    export constexpr auto MOD_DLL_DIRECTORY = "./mods"sv;
    
    export constexpr auto FAST_RESTART_TEMP_PATH = "./teinject.fast_restart.txt"sv;
    export constexpr auto FAST_RESTART_KEYS = std::array{VK_CONTROL, VK_SHIFT, VK_MENU, VK_ESCAPE};
    
}
