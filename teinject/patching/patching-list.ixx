export module teinject.patching:list;

import std;
import eel.util;
import eel.injection;
import eel.debug;
import teinject.tein;
import :hooks;

using namespace eel::util;
using namespace eel::injection;

namespace teinject::patching {
    
    std::vector<patch_item> GetImplementationPatchList() {
        return init_range<std::vector>({
patch_item {
    "load_level",
    std::make_unique<cpp_patch<load_level, 0x4b1df1, 0x4b1dfa, injection_mode::before>>(),
},
patch_item {
    "check_area_gon",
    std::make_unique<cpp_patch<check_area_gon, 0x4b32f7, 0x4b32ff, injection_mode::before>>(),
},
patch_item {
    "check_level_gon",
    std::make_unique<cpp_patch<check_level_gon, 0x4b3e6f, 0x4b3e75, injection_mode::before>>(),
},
patch_item {
    "frame_update",
    std::make_unique<cpp_patch<frame_update, 0x40f519, 0x40f51e, injection_mode::before>>(),
},      
            
patch_item {
    "check_hollows_water_flag",
    std::make_unique<cpp_patch<check_hollows_water_flag, 0x510104, 0x510109, injection_mode::before>>(),
},
patch_item {
    "check_trail_flag",
    std::make_unique<cpp_patch<check_trail_flag, 0x4b6442, 0x4b6449, injection_mode::after>>(),
},
patch_item {
    "check_trail_flag",
    std::make_unique<cpp_patch<check_trail_flag, 0x4b63ce, 0x4b63d5, injection_mode::after>>(),
},
patch_item {
    "check_sprite_rotation_flag",
    std::make_unique<cpp_patch<check_sprite_rotation_flag, 0x4b646f, 0x4b6476, injection_mode::after>>(),
},
patch_item {
    "check_sprite_rotation_flag",
    std::make_unique<cpp_patch<check_sprite_rotation_flag, 0x4b63fb, 0x4b6402, injection_mode::after>>(),
},
patch_item {
    "check_particles_flag",
    std::make_unique<cpp_patch<check_particles_flag, 0x4b649e, 0x4b64a5, injection_mode::after>>(),
},
patch_item {
    "check_particles_flag",
    std::make_unique<cpp_patch<check_particles_flag, 0x4b642e, 0x4b6435, injection_mode::after>>(),
},
patch_item {
    "retinara_timer",
    std::make_unique<cpp_patch<retinara_timer, 0x4f42f0, 0x4f42f8, injection_mode::after>>(),
},
patch_item {
    "retinara_timer",
    std::make_unique<cpp_patch<retinara_timer, 0x4f476d, 0x4f4775, injection_mode::after>>(),
},
patch_item {
    "static_turret_pause",
    std::make_unique<cpp_patch<static_turret_pause, 0x4f64e5, 0x4f64ed, injection_mode::after>>(),
},
patch_item {
    "static_turret_bullet_velocity",
    std::make_unique<cpp_patch<static_turret_bullet_velocity, 0x4f6524, 0x4f652a, injection_mode::before | injection_mode::extended>>(),
},
patch_item {
    "aim_turret_pause",
    std::make_unique<cpp_patch<aim_turret_pause, 0x4f68ed, 0x4f68f4, injection_mode::before>>(),
},
patch_item {
    "aim_turret_bullet_velocity",
    std::make_unique<cpp_patch<aim_turret_bullet_velocity, 0x4f6aa9, 0x4f6aaf, injection_mode::before | injection_mode::extended>>(),
},
patch_item {
    "index_component",
    std::make_unique<cpp_patch<index_component, 0x40e610, 0x40e615, injection_mode::before>>(),
},
patch_item {
    "mine_explosion",
    std::make_unique<cpp_patch<mine_explosion, 0x4df10a, 0x4df10f, injection_mode::after | injection_mode::call>>(),
},
patch_item {
    "xfloast_jump",
    std::make_unique<cpp_patch<xfloast_jump, 0x4f5086, 0x4f508b, injection_mode::after | injection_mode::call>>(),
},
patch_item {
    "crumble_tile_update",
    std::make_unique<cpp_patch<crumble_tile_update, 0x48bb30, 0x48bb36, injection_mode::after>>(),
},
patch_item {
    "default_level",
    std::make_unique<cpp_patch<default_level, 0x4f0722, 0x4f0729, injection_mode::after>>(),
},


            
        });
    }

}

