export module eel.debug;

import <windows.h>;
import <eh.h>;
import <intrin.h>;
import std;
import eel.util;

using namespace std::literals;
using namespace eel::util;

namespace eel::debug {
    
    export
    struct logging_parameters {
        std::string log_dir;
        std::string log_file;
        std::size_t log_max_file_size;
    };
    
    auto errh_params = opt<logging_parameters>();
    auto debug_log_mtx = std::shared_mutex();
    
    export
    void set_logging_parameters(logging_parameters&& parameters) {
        errh_params = std::move(parameters);
    }
    
    void write_debug_log(std::string_view msg) {
        if (!errh_params) return;
        
        auto lk = std::lock_guard(debug_log_mtx);
        
        static auto _ = [] {
            std::filesystem::create_directories(errh_params->log_dir);
            if (std::filesystem::exists(errh_params->log_file) && std::filesystem::file_size(errh_params->log_file) > errh_params->log_max_file_size) {
                std::filesystem::resize_file(errh_params->log_file, 0);
            } 
            return 0;
        }();
        
        std::ofstream(errh_params->log_file, std::ofstream::app)
            << '['
            << std::chrono::system_clock::now()
            << "]: "sv
            << msg
            << '\n';
    }
    
    export
    template<typename... Types>
    void log(std::format_string<Types&...> message, Types&&... params) {
        write_debug_log(std::format(message, params...));
    }

    export
    [[noreturn]] void handle_uncaught_exception(std::exception const& ex, uintptr_t addr) {
        auto message = std::format("Exception: {}\nat 0x{:X}", ex.what(), addr);
        log("{}", message);
        MessageBoxA(nullptr, message.c_str(), "Uncaught exception", MB_ICONERROR);
        std::terminate();
    }

    export
    [[noreturn]] void exit_with_error(zstring_view message) {
        log("{}", message);
        MessageBoxA(nullptr, message.c_str(), "Error", MB_ICONERROR);
        std::exit(EXIT_FAILURE);
    }

    export
    void set_signal_handler() {
        // requires <ExceptionHandling>Async</ExceptionHandling>
        _set_se_translator([](unsigned sig, EXCEPTION_POINTERS* ep) {
            auto message = std::format(
                "System error {:X}:\n"
                "EAX={:X}, EDX={:X}, ECX={:X}, EBX={:X}, EDI={:X}, ESI={:X}, EBP={:X}, ESP={:X}, EIP={:X}\n",
                sig,
                ep->ContextRecord->Eax,
                ep->ContextRecord->Edx,
                ep->ContextRecord->Ecx,
                ep->ContextRecord->Ebx,
                ep->ContextRecord->Edi,
                ep->ContextRecord->Esi,
                ep->ContextRecord->Ebp,
                ep->ContextRecord->Esp,
                ep->ContextRecord->Eip);
            throw std::system_error(std::error_code(sig, std::system_category()), message);
        });
    }

    export
    void debug_message(zstring_view message) {
        MessageBoxA(nullptr, message.c_str(), "Debug", MB_ICONINFORMATION);
    }

    export
    [[noreturn]] void pure_virtual_function_call_exit(uintptr_t ret_addr) noexcept {
        exit_with_error(std::format("Pure virtual function call at 0x{:X}", ret_addr));
    }

    export
    void __fastcall pure_virtual_function_call_raw() noexcept {
        pure_virtual_function_call_exit(reinterpret_cast<uintptr_t>(_ReturnAddress()));
    }
    
}