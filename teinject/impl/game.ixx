export module teinject.game;

import std;
import eel.util;
import teinject.common;
import teinject.tein;
import teinject.globals;

using namespace eel::util;
using namespace teinject::tein;

namespace teinject::impl {
    
    export
    template<typename T>
    opt<T const&> GetValueForComponent(EntityProperty<T> const& prop, Component& component) {
        return prop.GetValueForIndex(globals::state.component_index[&component]);
    }
    
    export
    void TurretShot(double& result, EntityProperty<std::vector<double>> const& cfg, Component& turret, bool increment) {
        if (auto prop = GetValueForComponent(cfg, turret); prop && !prop->empty()) {
            auto& counter = globals::state.turret_counter[&turret];
            auto& vec = *prop;
            result = vec[counter % vec.size()];
            if (increment) {
                ++counter;
            }
        }
    }
    
}
