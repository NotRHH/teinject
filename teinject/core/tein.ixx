export module teinject.tein;

import std;
import eel.util;
import eel.injection;
import "export/dllexport-macros.h";

using namespace eel::util;
using namespace eel::injection;

namespace teinject::tein {
    
    export struct GlobalGfxState;
    export struct String;
    export enum struct TextureIdentity;
    
    auto& global_gfx_state = ref_to<GlobalGfxState>(0x6d8a84);
    auto& GetTexture = ref_to<TextureIdentity(__vectorcall)(GlobalGfxState&, String)>(0x4164a0);
    
    
    export
    struct DLLEXPORT String : constructible_wrapper {
        String() = default;
        explicit String(char const* s) { self_thiscall<0x402d70, void, char const*>(s); }
        explicit String(char const* s, std::size_t size) { self_thiscall<0x402ec0, void, char const*, std::size_t>(s, size); }
        explicit String(std::string_view sv) : String(sv.data(), sv.size()) {}
        auto& operator=(String const&) = delete;
        String(String const&) = delete;
        ~String() { self_thiscall<0x402ce0, void>(); }
        
        auto begin() const { return capacity_ <= 0xf ? in_place_storage_ : storage_; }
        auto end() const { return begin() + length_; }
        auto size() const { return length_; }
        auto capacity() const { return capacity_; }
        auto c_str() const { return begin(); }
        auto data() const { return begin(); }
        auto operator[](std::size_t index) const { return begin()[index]; }
        
        operator std::string_view() const { return std::string_view(begin(), size()); }
    private:
        union {
            char* storage_{};
            char in_place_storage_[0x10];
        };
        std::size_t length_ = 0;
        std::size_t capacity_ = 0xf;
    };
    
    export
    DLLEXPORT String operator ""_S(char const* ch, std::size_t size) {
        return String(ch, size);
    }
    
    export
    enum struct DLLEXPORT GonObjectType {
        NULLGON,
        STRING,
        NUMBER,
        OBJECT,
        ARRAY,
        BOOL,
    };
    
    export
    struct DLLEXPORT GonObject : wrapper {
        bool HasField(String const& key) { return self_thiscall<0x46f320, bool, String const&>(key); }
        GonObject& ExtractField(String const& key) { return self_thiscall<0x46f3a0, GonObject&, String const&>(key); }
        int ExtractInt() { return self_thiscall<0x46e9a0, int>(); }
        double ExtractDouble() { return self_thiscall<0x46ecb0, double>(); }
        bool ExtractBool() { return self_thiscall<0x46efc0, int>(); }
        GonObjectType Type() { return field<GonObjectType, 0x6c>(); }
        
        std::size_t size() { return self_thiscall<0x46f460, int>(); }
        GonObject* begin() {
            auto type = Type();
            if (type != GonObjectType::OBJECT && type != GonObjectType::ARRAY) {
                return this;
            }
            return field<GonObject*, 0x20>();
        }
        GonObject* end() {
            auto type = Type();
            if (type != GonObjectType::OBJECT && type != GonObjectType::ARRAY) {
                return this + 1;
            }
            return field<GonObject*, 0x24>();
        }
        GonObject& operator[](std::size_t index) { return self_thiscall<0x46f440, GonObject&, std::size_t>(index); }
    private:
        alignas(double) std::byte _[0x70];
    };
    
    struct AccessXY {
        auto& X(this auto&& self) { return self[0]; }
        auto& Y(this auto&& self) { return self[1]; }
    };
    
    export
    struct DLLEXPORT Vec2 : vec<double, 2>, AccessXY {};
    
    export
    struct DLLEXPORT Vec2Int : vec<int, 2>, AccessXY {};
    
    export enum struct DLLEXPORT ComponentType : int {};
    export enum struct DLLEXPORT TileState : int {
        Free = 0,
        Blocked = 1,
        CrumblingBlock = 0xA,
    };
    
    export
    struct DLLEXPORT Tilemap : wrapper {
        Vec2Int Size() { return field<Vec2Int, 0x1c>(); }
        TileState& GetTileState(Vec2Int pos) {
            auto ptr = field<TileState*, 0x30>();
            auto width = Size().X();
            return ptr[pos.Y() * width + pos.X()];
        }
    };
    
    export
    struct DLLEXPORT Component : wrapper {
        ComponentType GetType() { return self_virtcall<0x4, ComponentType>(); }
        
        bool& FlagUpd1() { return field<bool, 0xe>(); }
        bool& FlagUpd2() { return field<bool, 0xd>(); }
        
        Vec2Int& TilemapPosition() { return field<Vec2Int, 0x18>(); }
        Tilemap& GetTilemap() { return *field<Tilemap*, 0x24>(); }
        Vec2& AbsolutePosition() { return field<Vec2, 0x28>(); }
        Vec2& AttachedPosition() { return field<Vec2, 0x88>(); }
        
        double& Time() { return field<double, 0x108>(); }
        
        double& UpdateTime() { return field<double, 0x10>(); }
        
        // ???
        bool& Dead() { return field<bool, 0x11a>(); }
        short& Smth() { return field<short, 0x42>(); }
        bool& Smth2() { return field<bool, 0x43>(); }
    };
    
    export
    struct DLLEXPORT CrumbleTile : Component {
        bool& Crumbled() { return field<bool, 0x30>(); }
        bool& Touched() { return field<bool, 0x31>(); }
        double& TimeSinceTouched() { return field<double, 0x28>(); }
    };
    
}
