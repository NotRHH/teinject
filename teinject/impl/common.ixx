export module teinject.common;

import std;
import eel.util;
import eel.debug;
import zpp.bits;
import teinject.tein;
import teinject.constants;
import "3rd/rfl.h";
import external.windows;

using namespace std::literals;
using namespace eel::util;
using namespace teinject::tein;

namespace teinject::impl {
    
    export
    opt<GonObject&> ExtractField(GonObject& gon, String const& name) {
        if (gon.HasField(name)) {
            return gon.ExtractField(name);
        }
        return {};
    }
    
    export
    template<typename T>
    struct EntityProperty {
        opt<T> common;
        std::vector<T> per_instance;
        
        opt<T const&> GetValueForIndex(std::size_t index) const {
            if (index < per_instance.size()) {
                return per_instance[index];
            }
            if (common) {
                return *common;
            }
            return {};
        }
    };
    
    template<typename T>
    auto GetPropertyValue(GonObject& config) {
        if constexpr (std::same_as<T, bool>) {
            return config.ExtractBool();
        } else if constexpr (std::same_as<T, double>) {
            return config.ExtractDouble();
        } else if constexpr (std::same_as<T, int>) {
            return config.ExtractInt();
        } else if constexpr (specialization_of<T, std::vector>) {
            return config 
                | std::views::transform(GetPropertyValue<typename T::value_type>)
                | std::ranges::to<T>();
        } else {
            static_assert(false, "unsupported property type");
        }
    }
    
    template<typename T>
    void FillProperty(opt<T>& property, GonObject& field) {
        property.emplace(GetPropertyValue<T>(field));
    }
    
    template<typename T>
    void FillProperty(EntityProperty<T>& property, GonObject& field) {
        if (auto common_value = ExtractField(field, "common"_S)) {
            property.common.emplace(GetPropertyValue<T>(*common_value));
        }
        if (auto per_instance_values = ExtractField(field, "per_instance"_S)) {
            property.per_instance.assign_range(*per_instance_values | std::views::transform(GetPropertyValue<T>));
        }
    }
    
    template<typename T>
    void FillPropertyFromConfig(T& property, GonObject& config, String const& name) {
        if (!config.HasField(name)) return;
        auto& field = config.ExtractField(name);
        (FillProperty)(property, field);
    }
    
    struct CustomTilesetFlagsData {
        EntityProperty<double> retinara_timer{};
        EntityProperty<std::vector<double>> static_turret_intervals{};
        EntityProperty<std::vector<double>> aim_turret_intervals{};
        EntityProperty<std::vector<double>> static_turret_bullet_velocity{};
        EntityProperty<std::vector<double>> aim_turret_bullet_velocity{};
        EntityProperty<int> mine_extra_jumps{};
        EntityProperty<int> xfloast_extra_jumps{};
        opt<bool> enable_hollows_water{};
        opt<bool> enable_particles{};
        opt<bool> enable_ash_trail{};
        opt<bool> enable_retro_gfx_tweaks{};
    };
    
    export
    struct CustomTilesetFlagsManager {
        CustomTilesetFlagsManager() = default;
        void Reset() { *this = {}; }
        
        void FillProperties(GonObject& config) {
            auto view = rfl::to_view(data_);
            view.apply([&](auto const& field) {
                (FillPropertyFromConfig)(*field.value(), config, String(std::string("teinject."sv).append(field.name())));
            });
        }
        
        CustomTilesetFlagsData const& Data() const { return data_; }
        
    private:
        CustomTilesetFlagsData data_{};
    };
    
    export
    struct FastRestarter {
        FastRestarter() = default;
        
        void Enable(bool enable) {
            enabled_ = enable;
        }
        
        void SaveCurrentLevel(std::string_view level_file_name) {
            if (!enabled_) return;
            
            static constexpr auto lvl_extension = ".lvl"sv;
            if (level_file_name.ends_with(lvl_extension)) {
                level_file_name.remove_suffix(lvl_extension.size());
            } else {
                throw std::runtime_error("no .lvl extension found");
            }
            current_level_name_ = level_file_name;
        }
        
        bool IsRestartRequested() const {
            if (!enabled_) return false;
            
            if (!external::windows::IsApplicationWindowFocused()) return false;
            for (auto& key : FAST_RESTART_KEYS) {
                if (!external::windows::IsKeyPressed(key)) return false;
            }
            return true;
        }
        
        [[noreturn]] void RestartGame() {
            if (!enabled_) return;
            
            fs::write_all_bytes(FAST_RESTART_TEMP_PATH, current_level_name_);
            external::windows::LaunchProcess(LOADER_PATH, "");
            std::quick_exit(0);
        }
        
        opt<std::string_view> GetLoadedLevelName() {
            if (!enabled_) return {};
            
            if (!file_read_) {
                file_read_ = true;
                if (auto name = fs::read_all_bytes<std::string>(FAST_RESTART_TEMP_PATH)) {
                    fast_restart_level_name_ = *std::move(name);
                }
            };
            if (!fast_restart_level_name_.empty()) {
                return fast_restart_level_name_;
            }
            return {};
        }
        
        void CleanUp() {
            if (std::filesystem::exists(FAST_RESTART_TEMP_PATH)) {
                std::filesystem::remove(FAST_RESTART_TEMP_PATH);
            }
        }
        
    private:
        std::string current_level_name_{};
        std::string fast_restart_level_name_{};
        bool file_read_ = false;
        bool enabled_ = false;
    };
    
    export
    struct GlobalState {
        GlobalState() = default;
        void Reset() { *this = {}; }
        
        std::unordered_map<Component*, int> turret_counter{};
        std::unordered_map<Component*, int> extra_jumps{};
        
        std::unordered_map<ComponentType, int> component_type_next_index{};
        std::unordered_map<Component*, int> component_index{};
    };
    
    
}
