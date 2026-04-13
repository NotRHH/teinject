module;
#include "resource.h"
export module teinject.constants;

import std;
import eel.util;

using namespace eel::util;

namespace teinject::inline constants {
    
    export constexpr auto LIBRARY_NAME = "TEINJECT"_zsv;
    export constexpr auto VERSION_NUM = std::tuple(DLL_VERSION_A, DLL_VERSION_B, DLL_VERSION_C);
    export constexpr auto VERSION_STR = DLL_VERSION_STRING_SHORT ""_zsv;
    
    export constexpr auto DLL_EXTENSION = ".dll"_zsv;
    export constexpr auto MOD_DLL_DIRECTORY = "./mods"_zsv;
    
}
