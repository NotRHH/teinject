export module teinject.mod_loader;

import std;
import eel.util;
import eel.debug;
import teinject.constants;
import teinject.globals;
import <windows.h>;
import teinject.patching;
import external.sha256;

using namespace std::literals;
using namespace eel::util;

namespace teinject::mod_loader {
    
    constexpr auto LOG_FILE = "teinject.log"sv;
    constexpr auto EXE_FILE_NAME = "TheEnd.exe"sv;
    constexpr auto EXE_HASH = "6aaf25bbd9970275654e6aabd98346b9da37540044fc24b9df4368544dce3709"_bytes;
    
    void CheckExe() {
        auto file_content = fs::read_all_bytes(EXE_FILE_NAME);
        if (!file_content) {
            throw std::runtime_error(std::format("Could not read {}", EXE_FILE_NAME));
        }
        auto sha256 = external::sha256();
        sha256.update(*file_content);
        auto hash = sha256.digest();
        if (!std::ranges::equal(hash, EXE_HASH, {}, [](std::uint8_t b) { return std::byte{b}; })) {
            throw std::runtime_error(std::format("{} hash mismatch", EXE_FILE_NAME));
        }
    }
    
    void LoadPatches() {
        auto collection = patching::GetPatchCollection();
        auto diagnostic = collection.apply();
        
        if (!diagnostic.success) {
            auto diagnostic_description = (std::stringstream() << diagnostic).str();
            eel::debug::log("{}", diagnostic_description);
            throw std::runtime_error(diagnostic_description);
        }
        
        bool flushed = FlushInstructionCache(GetCurrentProcess(), nullptr, -1);
        if (!flushed) {
            throw std::runtime_error("Processor instruction cache flush failed");
        }
    }
    
    void SetupLogging() {
        eel::debug::set_logging_parameters({
            .log_dir = "./",
            .log_file = std::string(LOG_FILE),
            .log_max_file_size = 1 << 20,
        });
    }
    
    void LoadMods() {
        globals::mod_registry.AllowModRegistration(true);
        
        eel::debug::log("Loading mod DLLs from {} started", MOD_DLL_DIRECTORY);
        
        if (auto mods_path = std::filesystem::path(MOD_DLL_DIRECTORY); std::filesystem::exists(mods_path)) {
            auto dll_ext = std::filesystem::path(DLL_EXTENSION);
            for (auto& fs_entry : std::filesystem::directory_iterator(mods_path)) {
                auto path = std::filesystem::absolute(fs_entry.path());
                eel::debug::log("File: {}", path.string());
                if (!fs_entry.is_regular_file()) continue;
                if (path.extension() != dll_ext) continue;
                eel::debug::log("File has appropriate extension. Attempt to load it");
                auto lib_handle = LoadLibraryW(path.c_str());
                if (!lib_handle) {
                    auto code = GetLastError();
                    throw std::runtime_error(std::format("Error loading mod '{}': error code {}", path.string(), code));
                }
            }
        }
        
        eel::debug::log("Loading mod DLLs finished");
        
        globals::mod_registry.AllowModRegistration(false);
    }
    
    export
    void Initialize() {
        SetupLogging();
        eel::debug::log("==================================================");
        eel::debug::log("{} v{} started", LIBRARY_NAME, VERSION_STR);
        
        CheckExe();
        LoadMods();
        LoadPatches();
    }
    
}
