export module teinject.patching;

import std;
import eel.injection;
import eel.util;
import teinject.globals;
export import :list;
export import :hooks;

using namespace eel::injection;

namespace teinject::patching {

    export
    patch_collection GetPatchCollection() {
        return patch_collection{
            GetImplementationPatchList(),
            globals::user_defined_patches.TakePatches(),
        };
    }
    
}
