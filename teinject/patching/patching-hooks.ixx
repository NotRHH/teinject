export module teinject.patching:hooks;

import std;
import eel.injection;
import eel.util;
import teinject.tein;
import teinject.globals;
import teinject.constants;
import teinject.game;
import teinject.handlers;

using namespace std::literals;
using namespace eel::injection;
using namespace eel::util;
using namespace teinject::tein;
using namespace teinject::impl;

namespace teinject::patching {
    
    void check_area_gon(saved_data& data) {
        auto& gon = ref_to<GonObject>(data.registers.eax);
    
        globals::handler_registry.Apply<ILevelLoadHandler>(&ILevelLoadHandler::OnLevelLoad);
        
        globals::state.Reset();
        globals::custom_flags.Reset();
        globals::custom_flags.FillProperties(gon);
    }
    
    void check_level_gon(saved_data& data) {
        auto& gon = ref_to<GonObject>(data.registers.eax);
        
        globals::custom_flags.FillProperties(gon);
    }
    
    void frame_update(saved_data& data) {
        globals::handler_registry.Apply<IFrameUpdateHandler>(&IFrameUpdateHandler::OnFrameUpdate);
    }

    void check_hollows_water_flag(saved_data& data) {
        auto& result = reinterpret_as<bool>(data.registers.eax);

        if (auto& prop = globals::custom_flags.Data().enable_hollows_water) {
            result = *prop;
        }
    }
    
    void check_trail_flag(saved_data& data) {
        auto& result = ref_to<bool>(data.registers.edi + 0x494);
        
        if (auto& prop = globals::custom_flags.Data().enable_ash_trail) {
            result = !*prop;
        }
    }
    
    void check_sprite_rotation_flag(saved_data& data) {
        auto& result = ref_to<bool>(data.registers.eax + 0x30);
        
        if (auto& prop = globals::custom_flags.Data().enable_retro_gfx_tweaks) {
            result = *prop;
        }
    }
    
    void check_particles_flag(saved_data& data) {
        auto& result = ref_to<bool>(data.registers.eax + 0x125);
        
        if (auto& prop = globals::custom_flags.Data().enable_particles) {
            result = *prop;
        }
    }
    
    void retinara_timer(saved_data& data) {
        auto& component = ref_to<Component>(data.registers.edi);
        auto& timer = ref_to<double>(data.registers.edi + 0x108);
        
        if (auto prop = GetValueForComponent(globals::custom_flags.Data().retinara_timer, component)) {
            timer = *prop;
        }
    }
    
    void static_turret_pause(saved_data& data) {
        auto& component = ref_to<Component>(data.registers.edi);
        auto& result = ref_to<double>(data.registers.edi + 0x108);
        
        TurretShot(result, globals::custom_flags.Data().static_turret_intervals, component, true);
    }
    
    void static_turret_bullet_velocity(saved_data_extended& data) {
        auto& component = ref_to<Component>(data.registers.edi);
        auto& result = data.xmm_registers.xmm0.f64[0];
        
        TurretShot(result, globals::custom_flags.Data().static_turret_bullet_velocity, component, false);
    }
    
    void aim_turret_pause(saved_data& data) {
        auto& component = ref_to<Component>(data.registers.edi);
        auto& result = ref_to<double>(data.registers.edi + 0x108);
        
        TurretShot(result, globals::custom_flags.Data().aim_turret_intervals, component, true);
    }
    
    void aim_turret_bullet_velocity(saved_data_extended& data) {
        auto& component = ref_to<Component>(data.registers.edi);
        auto& result = data.xmm_registers.xmm3.f64[0];
        
        TurretShot(result, globals::custom_flags.Data().aim_turret_bullet_velocity, component, false);
    }
    
    void index_component(saved_data& data) {
        auto& component = *ref_to<Component*>(data.registers.esp + 0x4);
        
        auto component_type = component.GetType();
        auto index = globals::state.component_type_next_index[component_type]++;
        globals::state.component_index[&component] = index;
    }
    
    void mine_explosion(call_saved_data& data) {
        auto& component = ref_to<Component>(data.registers.edi);
        auto dont_destroy = [&] {
            component.Smth() = 0;
            data.registers.esi = ref_to<std::uintptr_t>(data.registers.eax + 0x18);
            data.ret_addr = 0x4df14f;
        };
        
        if (auto prop = GetValueForComponent(globals::custom_flags.Data().mine_extra_jumps, component)) {
            if (++globals::state.extra_jumps[&component] <= *prop) {
                dont_destroy();
            }
        }
    }
    
    void xfloast_jump(call_saved_data& data) {
        auto& component = ref_to<Component>(data.registers.edi);
        auto dont_destroy = [&] {
            component.Smth2() = false;
            data.ret_addr = 0x4f511d;
        };
        
        if (auto prop = GetValueForComponent(globals::custom_flags.Data().xfloast_extra_jumps, component)) {
            if (component.Smth2() && ++globals::state.extra_jumps[&component] <= *prop) {
                dont_destroy();
            }
        }
    }
    
    void crumble_tile_update(saved_data& data) {
        auto& crumble_tile = ref_to<CrumbleTile>(data.registers.ecx);
        
        globals::handler_registry.Apply<ICrumbleTileUpdateHandler>(&ICrumbleTileUpdateHandler::OnUpdate, crumble_tile);
    }
    
}
