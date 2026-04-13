export module teinject;

export import teinject.tein;
export import teinject.handlers;

import std;
import eel.util;
import eel.debug;
import teinject.constants;
import teinject.globals;
import "export/dllexport-macros.h";
import eel.injection;

using namespace eel::util;

namespace teinject {
    
    export
    struct DLLEXPORT TeinjectVersion {
        unsigned a, b, c;
    };
    
    export
    struct DLLEXPORT ModRegistryEntry {
        TeinjectVersion required_teinject_version;
        std::string name;
        std::string version;
        std::vector<std::shared_ptr<IHandler>> handlers;
        std::vector<eel::injection::patch_item> patch_items;
    };
    
    export 
    DLLEXPORT void RegisterMod(ModRegistryEntry&& mod) 
    try {
        eel::debug::log("Register mod '{}'", mod.name);
        
        auto [a, b, c] = mod.required_teinject_version;
        globals::mod_registry.RegisterMod(mod.name, mod.name, {a, b, c});
    
        for (auto& patch : mod.patch_items) {
            patch.rename(std::format("{}::{}", mod.name, patch.key()));
        }
    
        globals::handler_registry.RegisterHandlers(mod.handlers);
        globals::user_defined_patches.RegisterPatches(std::move(mod.patch_items));
        
        eel::debug::log("Mod loaded successfully");
    } catch (std::exception const& ex) {
        eel::debug::exit_with_error(std::format("Error when loading mod '{}': {}", mod.name, ex.what()));
    } catch (...) {
        eel::debug::exit_with_error(std::format("Error when loading mod '{}': [non-exception error type]", mod.name));
    }
    
}