export module teinject.handlers;

import std;
import eel.util;
import teinject.tein;
import teinject.constants;
import "export/dllexport-macros.h";
import eel.injection;

using namespace eel::util;
using namespace teinject::tein;

namespace teinject::inline handlers {
    
    export
    struct DLLEXPORT IHandler {
        virtual ~IHandler() = default;
    };
    
    export
    struct DLLEXPORT IFrameUpdateHandler : IHandler {
        virtual void OnFrameUpdate() = 0;
    };
    
    export
    struct LevelLoadData {
        DLLEXPORT std::string_view FullLevelName() const { return m_.full_level_name; }
        
        struct Members {
            std::string_view full_level_name;
        };
        LevelLoadData(Members&& members) : m_(std::move(members)) {}
    private:
        Members m_;
    };
    
    export
    struct LevelConfigLoadData {
        DLLEXPORT std::string_view AreaName() const { return m_.area_name; }
        DLLEXPORT std::string_view LevelName() const { return m_.level_name; }
        DLLEXPORT std::string_view FullLevelName() const { return m_.full_level_name; }
        DLLEXPORT GonObject& MainConfig() const { return m_.main_config; }
        DLLEXPORT GonObject& AreaConfig() const { return m_.area_config; }
        DLLEXPORT opt<GonObject&> LevelConfig() const { return m_.level_config; }

        struct Members {
            std::string_view area_name;
            std::string_view level_name;
            std::string_view full_level_name;
            refw<GonObject> main_config;
            refw<GonObject> area_config;
            opt<GonObject&> level_config;
        };
        LevelConfigLoadData(Members&& members) : m_(std::move(members)) {}
    private:
        Members m_;
    };
    
    export
    struct DLLEXPORT ILevelLoadHandler : IHandler {
        virtual void OnLevelConfigLoad(LevelConfigLoadData const& data) = 0;
        virtual void OnLevelLoad(LevelLoadData const& data) = 0;
    };
    
    export
    struct DLLEXPORT ICrumbleTileUpdateHandler : IHandler {
        virtual void OnUpdate(CrumbleTile& tile) = 0;
    };
    
    template<typename T>
    struct HandlerList {
        std::vector<std::shared_ptr<T>> handlers;
    };
    
    template<typename... T>
    struct Handlers : HandlerList<T>... {
        template<typename Handler>
        auto& Get(this auto&& self) {
            return self.template HandlerList<Handler>::handlers;
        }
    };
    
    template<typename... HandlerInterfaces>
    requires (std::is_base_of_v<IHandler, HandlerInterfaces> && ...) && (std::is_abstract_v<HandlerInterfaces> && ...)
    struct HandlerRegistry {
        void RegisterHandlers(std::vector<std::shared_ptr<IHandler>> const& handlers) {
            for (auto& handler : handlers) {
                (RegisterHandler(std::dynamic_pointer_cast<HandlerInterfaces>(handler)), ...);
            }
        }
        
        template<one_of<HandlerInterfaces...> Handler>
        std::vector<std::shared_ptr<Handler>> const& GetHandlers() const {
            return GetHandlerList<Handler>();
        }
        
        template<one_of<HandlerInterfaces...> Handler, typename Func, typename... Args>
        requires std::invocable<Func, Handler&, Args&...>
        void Apply(Func&& function, Args&&... args) {
            for (auto& handler : GetHandlers<Handler>()) {
                std::invoke(function, handler, args...);  // no forwarding
            }
        }
        
    private:
        Handlers<HandlerInterfaces...> handlers_;
        
        template<one_of<HandlerInterfaces...> Handler>
        auto& GetHandlerList(this auto&& self) {
            return self.handlers_.template Get<Handler>();
        }
        
        template<one_of<HandlerInterfaces...> Handler>
        void RegisterHandler(std::shared_ptr<Handler> handler) {
            if (handler) {
                GetHandlerList<Handler>().push_back(handler);
            }
        }
    };
    
    export
    struct TeinjectHandlerRegistry : 
        HandlerRegistry<
            IFrameUpdateHandler,
            ILevelLoadHandler,
            ICrumbleTileUpdateHandler>
    {
    };
    
    export
    struct UserDefinedPatchRegistry {
        void RegisterPatches(std::vector<eel::injection::patch_item>&& patches) {
            patches_.append_range(patches | std::views::as_rvalue);
        }
        std::vector<eel::injection::patch_item>&& TakePatches() {
            return std::move(patches_);
        }
    private:
        std::vector<eel::injection::patch_item> patches_;
    };
    
    export
    struct ModRegistry {
        void RegisterMod(
            std::string_view mod_name,
            std::string_view mod_version,
            std::tuple<unsigned, unsigned, unsigned> required_teinject_version) 
        {
            if (!IsModRegistrationAllowed()) {
                throw std::runtime_error("Mods should be registered from DllMain upon attachment to process");
            }
            
            auto& [a, b, c] = required_teinject_version;
            auto& [ra, rb, rc] = VERSION_NUM;
            if (std::tie(a, b) != std::tie(ra, rb)) {
                throw std::runtime_error(std::format("Mod requires teinject v{}.{}. Current teinject is v{}.{}", a, b, ra, rb));
            }
            
            if (mods_.contains(mod_name)) {
                throw std::runtime_error("Mod with such name is already registered");
            }
            
            mods_.emplace(mod_name, mod_version);
        }
        
        void AllowModRegistration(bool allow) {
            *allowed_thread_id_.acquire() = allow ? std::this_thread::get_id() : std::thread::id();
        }
        
        bool IsModRegistrationAllowed() const {
            return *allowed_thread_id_.acquire() == std::this_thread::get_id();
        }
        
    private:
        std::unordered_map<std::string, std::string, string_hash, std::equal_to<>> mods_{};
        thread_safe<std::thread::id> allowed_thread_id_{};
    };
    
}
