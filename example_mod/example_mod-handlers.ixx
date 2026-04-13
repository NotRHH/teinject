export module example_mod:handlers;

import std;
import eel.util;
import teinject;
import example_mod.globals;

using namespace eel::util;

namespace example_mod {
    
    struct ExampleLevelLoadHandler : teinject::ILevelLoadHandler {
        void OnLevelLoad() override {
            globals::random = {};
        }
    };
    
    export
    std::vector<std::shared_ptr<teinject::IHandler>> GetAllModHandlers() {
        return {
            std::make_shared<ExampleLevelLoadHandler>(),
        };
    }
    
}