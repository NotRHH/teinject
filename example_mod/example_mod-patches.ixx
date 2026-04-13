export module example_mod:patches;

import std;
import eel.util;
import eel.injection;
import example_mod.globals;

using namespace eel::util;
using namespace eel::injection;

namespace example_mod {
    
    constexpr double CRUMBLE_TILE_TIME = 0.2;
    
    void example_random_direction_change(saved_data& data) {
        auto& continue_same_direction = reinterpret_as<bool>(data.registers.eax);
        
        if (std::uniform_real_distribution()(globals::random) < 0.005) {
            continue_same_direction = false;
        }
    }
    
    std::vector<patch_item> GetAllModPatches() {
        using enum injection_mode;
        return init_range<std::vector>({
            patch_item {
                // Ash can't die from poison
                "example_patch_fill_nop",
                fill_nop(0x4eb116, 0x4eb15b),
            },
            patch_item {
                // crumbling tiles break 0.2 seconds after being touched (instead of 0.55)
                "example_patch_raw_mem",
                std::make_unique<raw_mem_patch>(mem_chunk(0x48bb55, 0x48bb59, repr(&CRUMBLE_TILE_TIME))),
            },
            patch_item {
                // hoasts randomly change direction
                "example_patch_cpp_1",
                std::make_unique<cpp_patch<example_random_direction_change, 0x4f53c9, 0x4f53d1, after>>(),
            },
            patch_item {
                // hoasts randomly change direction
                "example_patch_cpp_2",
                std::make_unique<cpp_patch<example_random_direction_change, 0x4f5440, 0x4f5455, before>>(),
            },
        });
    }
    
}
