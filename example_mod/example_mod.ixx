export module example_mod;

import std;
import teinject;
export import :patches;
export import :handlers;

namespace example_mod {
    
    export
    void RegisterMod() {
        teinject::RegisterMod(teinject::ModRegistryEntry{
            .required_teinject_version = {0, 1, 0},
            .name = "example_mod",
            .version = "0.1",
            .handlers = GetAllModHandlers(),
            .patch_items = GetAllModPatches(),
        });
    }
    
}
