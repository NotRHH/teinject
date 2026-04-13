export module eel.injection;

import std;
import eel.util;
import eel.debug;
import <windows.h>;

using namespace eel::util;

#define CALL_CONV __vectorcall

namespace eel::injection {
    
    template<typename T>
    concept passable_by_value = std::is_lvalue_reference_v<T> || sizeof(T) <= sizeof(uintptr_t);
    
    template<typename T>
    concept direct_return = one_of<T, void, double>;

    template<passable_by_value T>
    __forceinline uintptr_t convert(T& v) {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return std::bit_cast<uintptr_t>(&v);
        } else {
            if constexpr (sizeof(v) == sizeof(uintptr_t)) {
                return std::bit_cast<uintptr_t>(v);
            } else {
                auto storage = std::array<std::byte, sizeof(uintptr_t)>();
                new (storage.data()) T(v);
                return std::bit_cast<uintptr_t>(storage);
            }
        }
    }

    template<passable_by_value T>
    __forceinline T convert_back(uintptr_t v) {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return *reinterpret_cast<std::remove_reference_t<T>*>(v);
        } else {
            return reinterpret_cast<T&>(v);
        }
    }
    
    template<typename T>
    requires std::is_member_function_pointer_v<T>
    struct member_function_traits;

    template<typename Ret, typename Inst, typename... Args>
    struct member_function_traits<Ret(Inst::*)(Args...)> {
        template<std::uintptr_t FuncOffset>
        requires 
            (passable_by_value<Ret> || direct_return<Ret>)
            && (passable_by_value<Args> && ...)
        static Ret virtcall(Inst* self, Args... args) {
            using unconverted_ret_t = std::conditional_t<direct_return<Ret>, Ret, std::uintptr_t>;
            using func_t = unconverted_ret_t(CALL_CONV)(Inst*, fake_edx_t, decltype(void(args), std::uintptr_t())...);
            auto const v_table_ptr = reinterpret_cast<dword&>(*self);
            auto& func_ref = *ptr_to<func_t>(*(v_table_ptr + dword(FuncOffset)));
            static_assert(FuncOffset < 0x1000, "virtual function offset is unlikely to be that large");
            
            if constexpr (direct_return<Ret>) {
                return func_ref(self, fake_edx, (convert<Args>)(args)...);
            } else {
                return (convert_back<Ret>)(func_ref(self, fake_edx, (convert<Args>)(args)...));
            }
        }
        
        template<std::uintptr_t FuncAddress>
        requires
            (passable_by_value<Ret> || direct_return<Ret>)
            && (passable_by_value<Args> && ...)
        static Ret thiscall(Inst* self, Args... args) {
            using unconverted_ret_t = std::conditional_t<direct_return<Ret>, Ret, std::uintptr_t>;
            using func_t = unconverted_ret_t(CALL_CONV)(Inst*, fake_edx_t, decltype(void(args), std::uintptr_t())...);
            auto& func_ref = *ptr_to<func_t>(FuncAddress);
            static_assert(FuncAddress >= 0x400000, "function address is unlikely to be that small");
            
            if constexpr (direct_return<Ret>) {
                return func_ref(self, fake_edx, (convert<Args>)(args)...);
            } else {
                return (convert_back<Ret>)(func_ref(self, fake_edx, (convert<Args>)(args)...));
            }
        }
    };

    
    export
    template<
        typename Func,
        std::uintptr_t FuncOffset>
    constexpr auto& virtcall = member_function_traits<Func>::template virtcall<FuncOffset>;

    export
    template<
        typename Func,
        std::uintptr_t FuncAddress>
    constexpr auto& thiscall = member_function_traits<Func>::template thiscall<FuncAddress>;
    
    export
    struct constructible_wrapper {
    protected:
        template<typename T, std::uintptr_t Offset> 
        T& field() { return ref_to<T>(dword(this) + dword(Offset)); }
        
        template<std::uintptr_t Offset, typename Ret, typename... Args, typename Self>
        Ret self_virtcall(this Self& self, std::type_identity_t<Args>... args) {
            using self_object_t = std::remove_reference_t<Self>;
            static_assert(!(std::is_const_v<self_object_t> || std::is_volatile_v<self_object_t>), "cv-qualified self object types not supported");
            return virtcall<Ret(self_object_t::*)(Args...), Offset>(&self, args...);
        }
        
        template<std::uintptr_t Address, typename Ret, typename... Args, typename Self>
        Ret self_thiscall(this Self& self, std::type_identity_t<Args>... args) {
            using self_object_t = std::remove_reference_t<Self>;
            static_assert(!(std::is_const_v<self_object_t> || std::is_volatile_v<self_object_t>), "cv-qualified self object types not supported");
            return thiscall<Ret(self_object_t::*)(Args...), Address>(&self, args...);
        }
    };

    export
    struct wrapper : constructible_wrapper {
        wrapper() = delete;
        wrapper(const wrapper&) = delete;
        ~wrapper() = delete;
    };
    
    static_assert(std::is_empty_v<wrapper>, "no members allowed");



    
    export
    struct saved_registers {
        uintptr_t eax, ecx, edx, ebx, ebp, esi, edi, esp;
    };
    
    export
    union xmm_reg {
        double f64[2];
        float f32[4];
    };  
    
    export
    struct xmm_registers {
        xmm_reg xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    };

    export
    struct saved_data {
        saved_registers registers;
    };
    
    export
    struct saved_data_extended : saved_data {
        xmm_registers xmm_registers;
    };

    export
    struct call_saved_data : saved_data {
        uintptr_t ret_addr;
    };

    export
    struct bounds_t {
        uintptr_t addr_begin, addr_end;
        constexpr size_t size() const { return addr_end - addr_begin; }
    };
    
    export
    using byte_array_t = std::vector<std::uint8_t>;
    
    export
    template<typename T>
    constexpr auto repr(T&& value) {
        return std::bit_cast<std::array<std::uint8_t, sizeof(T)>>(value);
    }


    inline namespace new_injector {
        
        export
        enum class injection_mode {
            _placement_mask = 0x3,
            // Original code is erased.
            replace = 0,
            // Injection is executed before replaced code.
            // Relative jumps out of replaced block are prohibited.
            before = 1,
            // Injection is executed after replaced code.
            // Relative jumps out of replaced block are prohibited.
            after = 2,
            
            _transfer_op_mask = 0x4,
            // 'jmp' forward
            // 'jmp' back
            // Preserves esp-dependent code.
            jmp = 0,
            // 'call' forward
            // 'ret' back
            // Requires esp-independent code block (unless it is erased with 'replace' mode).
            // Allows replacing return address dynamically through call_saved_data::ret_addr.
            call = 4,
            
            _extended_info_mask = 0x8,
            // only basic registers saved
            normal = 0,
            // saves basic registers + xmm0..xmm7
            extended = 8,
        };
        export
        constexpr injection_mode operator |(injection_mode m1, injection_mode m2) {
            return static_cast<injection_mode>(static_cast<int>(m1) | static_cast<int>(m2));
        }
        export
        constexpr injection_mode operator &(injection_mode m1, injection_mode m2) {
            return static_cast<injection_mode>(static_cast<int>(m1) & static_cast<int>(m2));
        }
        export
        constexpr injection_mode inj_trasfer_op(injection_mode mode) {
            return mode & injection_mode::_transfer_op_mask;
        }
        export
        constexpr injection_mode inj_placement(injection_mode mode) {
            return mode & injection_mode::_placement_mask;
        }
        export
        constexpr injection_mode inj_extended_info(injection_mode mode) {
            return mode & injection_mode::_extended_info_mask;
        }
        

        enum class transfer_op : std::uint8_t {
            call = 0xE8,
            jmp = 0xE9,
        };

        constexpr size_t transfer_control_opcode_size = 5;
        constexpr bool address_bounds_valid_for_injection(uintptr_t addr_begin, uintptr_t addr_end) {
            return addr_begin <= addr_end - transfer_control_opcode_size;
        }

        struct scoped_unprotect {
            scoped_unprotect(dword addr, size_t size) :
                dest_(addr), num_bytes_(size)
            {
                if (!VirtualProtect(ptr_to<void>(addr), size, PAGE_EXECUTE_READWRITE, &old_prot_)) {
                    throw std::runtime_error{ "failed to unlock memory for write" };
                }
            }
            scoped_unprotect(scoped_unprotect const&) = delete;
            scoped_unprotect& operator=(scoped_unprotect const&) = delete;
            ~scoped_unprotect() {
                if (!VirtualProtect(ptr_to<void>(dest_), num_bytes_, old_prot_, &old_prot_)) {
                    //throw runtime_error{ "failed to revert memory protection" };
                }
            }
        private:
            dword dest_;
            size_t num_bytes_;
            DWORD old_prot_;
        };

        void write_into_protected_memory(dword dest, dword src, size_t num_bytes) {
            if (src == nullptr) return;
            scoped_unprotect unprot{dest, num_bytes};
            std::memcpy(ptr_to<void>(dest), ptr_to<void>(src), num_bytes);
        }

        void write_transfer_op(dword addr_at, dword addr_to, transfer_op op) {
            dword offset = addr_to - addr_at - dword{transfer_control_opcode_size};
            ref_to<std::uint8_t>(addr_at) = static_cast<std::uint8_t>(op);
            std::memcpy(
                ptr_to<void>(addr_at + 1_dw),
                &offset,
                sizeof(offset));
        }
        
        template<auto Func, typename ParamType, uintptr_t Addr>
        void __fastcall caller(ParamType& data) noexcept {
            try {
                Func(data);
            } catch (std::exception const& ex) {
                debug::handle_uncaught_exception(ex, Addr);
            } catch (...) {
                debug::handle_uncaught_exception(std::runtime_error("[non-exception type]"), Addr);
            }
        }
        
        struct injection_mode_data {
            void (&write_jmp_back)(std::span<std::uint8_t> initial_codecave, size_t& index, size_t ret_addr);
            transfer_op codecave_transfer_op;
            std::span<std::uint8_t const> initial_codecave_template;
            size_t func_call_index;
        };

        struct inj_features { bool is_call; bool is_extended; };
        
        template<inj_features>
        struct injection_mode_settings {
            static_assert(false, "requested set of features from injection_mode not supported");
        };
        
        consteval inj_features from_inj_mode_enum(injection_mode inj_mode) {
            return inj_features{
                .is_call = inj_trasfer_op(inj_mode) == injection_mode::call,
                .is_extended = inj_extended_info(inj_mode) == injection_mode::extended,
            };
        }
        
        template<injection_mode TransferOp>
        using injection_mode_settings_by_enum = injection_mode_settings<from_inj_mode_enum(TransferOp)>;
        
        template<>
        struct injection_mode_settings<{.is_call = false, .is_extended = false}> {
            static constexpr auto initial_codecave_template = std::to_array<std::uint8_t>({
                // 	push esp
                // 	push edi
                // 	push esi
                // 	push ebp
                // 	push ebx
                // 	push edx
                // 	push ecx
                // 	push eax
                0x54, 0x57, 0x56, 0x55, 0x53, 0x52, 0x51, 0x50,

                // mov ecx, esp
                // call ???
                0x89, 0xE1, 0xE8, 0xFF, 0xFF, 0xFF, 0xFF,
        
                // pop eax
                // pop ecx
                // pop edx
                // pop ebx
                // pop ebp
                // pop esi
                // pop edi
                // add esp, 0x4
                0x58, 0x59, 0x5A, 0x5B, 0x5D, 0x5E, 0x5F, 0x83, 0xC4, 0x04,
            });
            using param_type = saved_data;
            using cpp_patch_function_t = void(*)(param_type&);

            static constexpr size_t jmp_back_size = transfer_control_opcode_size;
            static void write_jmp_back(std::span<std::uint8_t> initial_codecave, size_t& index, size_t ret_addr) {
                write_transfer_op(
                    dword{initial_codecave.data() + index},
                    dword{ret_addr},
                    transfer_op::jmp);
                index += jmp_back_size;
            }
            
            static constexpr auto data = injection_mode_data{
                .write_jmp_back = write_jmp_back,
                .codecave_transfer_op = transfer_op::jmp,
                .initial_codecave_template = initial_codecave_template,
                .func_call_index = 10,
            };
        };
        
        template<>
        struct injection_mode_settings<{.is_call = false, .is_extended = true}> {
            static constexpr auto initial_codecave_template = 
                // save xmm0..xmm7
                "81EC800000000F1104240F114C24100F115424200F115C24300F116424400F116C24500F117424600F117C2470"
            
                "54 57 56 55 53 52 51 50"

                // mov ecx, esp
                // call ???
                "89 E1 E8 FF FF FF FF"
        
                // pop eax
                // pop ecx
                // pop edx
                // pop ebx
                // pop ebp
                // pop esi
                // pop edi
                // add esp, 0x4
                "58 59 5A 5B 5D 5E 5F 83 C4 04"
                // load xmm0..xmm7
                "0F1004240F104C24100F105424200F105C24300F106424400F106C24500F107424600F107C247081C480000000"
                ""_bytes;
            
            using param_type = saved_data_extended;
            using cpp_patch_function_t = void(*)(param_type&);

            static constexpr size_t jmp_back_size = transfer_control_opcode_size;
            static void write_jmp_back(std::span<std::uint8_t> initial_codecave, size_t& index, size_t ret_addr) {
                write_transfer_op(
                    dword{initial_codecave.data() + index},
                    dword{ret_addr},
                    transfer_op::jmp);
                index += jmp_back_size;
            }
            
            static constexpr auto data = injection_mode_data{
                .write_jmp_back = write_jmp_back,
                .codecave_transfer_op = transfer_op::jmp,
                .initial_codecave_template = initial_codecave_template,
                .func_call_index = 55,
            };
        };
        
        template<>
        struct injection_mode_settings<{.is_call = true, .is_extended = false}> {
            static constexpr auto initial_codecave_template = std::to_array<std::uint8_t>({
                // 	push esp
                //  add  DWORD PTR [esp],0x4
                // 	push edi
                // 	push esi
                // 	push ebp
                // 	push ebx
                // 	push edx
                // 	push ecx
                // 	push eax
                0x54, 0x83, 0x04, 0x24, 0x04, 0x57, 0x56, 0x55, 0x53, 0x52, 0x51, 0x50,

                // mov ecx, esp
                // call ???
                0x89, 0xE1, 0xE8, 0xFF, 0xFF, 0xFF, 0xFF,
        
                // pop eax
                // pop ecx
                // pop edx
                // pop ebx
                // pop ebp
                // pop esi
                // pop edi
                // add esp, 0x4
                0x58, 0x59, 0x5A, 0x5B, 0x5D, 0x5E, 0x5F, 0x83, 0xC4, 0x04,
            });
            using param_type = call_saved_data;
            using cpp_patch_function_t = void(*)(param_type&);
            
            static constexpr size_t jmp_back_size = 1;
            static void write_jmp_back(std::span<std::uint8_t> initial_codecave, size_t& index, size_t ret_addr) {
                initial_codecave[index] = 0xC3;	// ret
                index += jmp_back_size;
            }
            
            static constexpr auto data = injection_mode_data{
                .write_jmp_back = write_jmp_back,
                .codecave_transfer_op = transfer_op::call,
                .initial_codecave_template = initial_codecave_template,
                .func_call_index = 14,
            };
        };
        
        void write_codecave(
            bounds_t codecave_bounds, 
            std::span<std::uint8_t> initial_codecave, 
            transfer_op transfer_op)
        {
            scoped_unprotect unprot{dword{codecave_bounds.addr_begin}, codecave_bounds.size()};

            write_transfer_op(
                dword{codecave_bounds.addr_begin},
                dword{initial_codecave.data()},
                transfer_op);

            if (codecave_bounds.size() > transfer_control_opcode_size) {
                std::memset(
                    ptr_to<void>(codecave_bounds.addr_begin + transfer_control_opcode_size),
                    0x90,
                    codecave_bounds.size() - transfer_control_opcode_size);
            }
        }
            
        void write_caller(
            size_t& index,
            std::span<std::uint8_t> initial_codecave,
            injection_mode_data const& inj_mode_data,
            dword caller_ptr)
        {
            std::memcpy(
                initial_codecave.data() + index,
                inj_mode_data.initial_codecave_template.data(),
                inj_mode_data.initial_codecave_template.size());

            write_transfer_op(
                dword{initial_codecave.data() + index + inj_mode_data.func_call_index},
                dword{caller_ptr},
                transfer_op::call);
                
            index += inj_mode_data.initial_codecave_template.size();
        }

        void write_replaced_code(
            size_t& index,
            bounds_t codecave_bounds,
            std::span<std::uint8_t> initial_codecave)
        {
            std::memcpy(
                initial_codecave.data() + index,
                ptr_to<void>(codecave_bounds.addr_begin),
                codecave_bounds.size());

            index += codecave_bounds.size();
        }

        void write_jmp_back(
            size_t& index,
            bounds_t codecave_bounds,
            std::span<std::uint8_t> initial_codecave,
            void(&write_jmp_back_p)(std::span<std::uint8_t> initial_codecave, size_t& index, size_t ret_addr))
        {
            write_jmp_back_p(initial_codecave, index, codecave_bounds.addr_end);
        }

        void make_executable(std::span<std::uint8_t> initial_codecave) {
            DWORD old_prot;
            VirtualProtect(initial_codecave.data(), initial_codecave.size(), PAGE_EXECUTE_READWRITE, &old_prot);
        }
        
        void initialize_codecave(
            injection_mode inj_placement,
            injection_mode_data const& inj_mode_data,
            bounds_t codecave_bounds,
            std::span<std::uint8_t> initial_codecave,
            dword caller_ptr)
        {
            {
                size_t index = 0;
                if (inj_placement == injection_mode::replace) {
                    write_caller(index, initial_codecave, inj_mode_data, caller_ptr);
                } else if (inj_placement == injection_mode::after) {
                    write_replaced_code(index, codecave_bounds, initial_codecave);
                    write_caller(index, initial_codecave, inj_mode_data, caller_ptr);
                } else if (inj_placement == injection_mode::before) {
                    write_caller(index, initial_codecave, inj_mode_data, caller_ptr);
                    write_replaced_code(index, codecave_bounds, initial_codecave);
                } else {
                    throw std::runtime_error("unknown injection_mode placement");
                }
                write_jmp_back(index, codecave_bounds, initial_codecave, inj_mode_data.write_jmp_back);
                if (index > initial_codecave.size()) {
                    throw std::out_of_range("initial codecave overflown");
                }
            }
            make_executable(initial_codecave);

            write_codecave(codecave_bounds, initial_codecave, inj_mode_data.codecave_transfer_op);
        }
        
        template<auto Func, bounds_t CodecaveBounds, injection_mode Mode>
        requires
            (address_bounds_valid_for_injection(CodecaveBounds.addr_begin, CodecaveBounds.addr_end))
            && std::same_as<decltype(Func), typename injection_mode_settings_by_enum<Mode>::cpp_patch_function_t>
        struct injector {

            using settings = injection_mode_settings_by_enum<Mode>;
            
            static constexpr size_t caller_size =
                settings::jmp_back_size
                + (inj_placement(Mode) != injection_mode::replace ? CodecaveBounds.size() : 0)
                + settings::initial_codecave_template.size();

            static void initialize() {
                (initialize_codecave)(
                    inj_placement(Mode),
                    settings::data,
                    CodecaveBounds,
                    initial_codecave,
                    dword(&caller<Func, typename settings::param_type, CodecaveBounds.addr_begin>));
            }
        private:
            alignas(0x10) static inline std::array<std::uint8_t, caller_size> initial_codecave{};
        };
    }
    
    
    export
    struct mem_chunk {
        
        constexpr mem_chunk(uintptr_t addr_begin, uintptr_t addr_end, std::span<std::uint8_t const> data):
            addr_begin_(addr_begin),
            addr_end_(addr_end),
            data_(std::from_range, data)
        {}

        constexpr mem_chunk(uintptr_t addr_begin, uintptr_t addr_end, byte_array_t&& data): 
            addr_begin_(addr_begin),
	        addr_end_(addr_end),
	        data_(std::move(data))
        {}

        constexpr mem_chunk(uintptr_t addr_begin, uintptr_t addr_end, std::initializer_list<std::uint8_t> data):
            addr_begin_(addr_begin),
	        addr_end_(addr_end),
	        data_(data)
        {}
        
        constexpr uintptr_t size() const { return addr_end_ - addr_begin_; }
        constexpr uintptr_t addr_begin() const { return addr_begin_; }
        constexpr uintptr_t addr_end() const { return addr_end_; }
        
        void write_data() const {
            if (addr_begin_ > addr_end_) {
                throw std::out_of_range("addr_begin > addr_end");
            }
            if (data_.size() > size()) {
                throw std::out_of_range(std::format("not enough space ({} required, got {})", data_.size(), size()));
            }
            write_into_protected_memory(dword{addr_begin_}, dword{data_.data()}, size());
        }

    private:
        uintptr_t addr_begin_, addr_end_;
        byte_array_t data_;
    };

    
    export
    struct patch_base {
        
        virtual void write_data() = 0;
        virtual std::vector<bounds_t> occupied_sections() = 0;

        patch_base() = default;
        patch_base(const patch_base&) = delete;
        virtual ~patch_base() = default;
    };

    
    export
    struct raw_mem_patch : patch_base {
        explicit raw_mem_patch(mem_chunk&& chunk) :
            chunk_v_(std::move(chunk))
        {}

        template<std::ranges::range T>
        explicit raw_mem_patch(T&& rng) :
            chunk_v_(std::vector(std::from_range, std::forward<T>(rng)))
        {}

        void write_data() override {
            for (auto& chunk : chunks_span()) {
                chunk.write_data();
            }
        }

        std::vector<bounds_t> occupied_sections() override {
            return chunks_span()
                | std::views::transform([](mem_chunk const& chunk){ return bounds_t{chunk.addr_begin(), chunk.addr_end()}; })
                | std::ranges::to<std::vector>();
        }
        
    private:
        std::variant<mem_chunk, std::vector<mem_chunk>> chunk_v_;
        
        std::span<mem_chunk const> chunks_span() const {
            static constexpr auto selector = overload {
                [](mem_chunk const& chunk) { return std::span(&chunk, &chunk + 1); },
                [](std::vector<mem_chunk> const& vec) { return std::span(vec.begin(), vec.end()); },
            };
            return selector.visit(chunk_v_);
        }
    };

    
    export
    template<auto MainFunction, uintptr_t AddrBegin, uintptr_t AddrEnd, injection_mode Mode>
    requires (address_bounds_valid_for_injection(AddrBegin, AddrEnd))
    struct cpp_patch : patch_base {
        void write_data() override {
            injector<MainFunction, bounds_t{AddrBegin, AddrEnd}, Mode>::initialize();
        }
        
        std::vector<bounds_t> occupied_sections() override {
            return {{AddrBegin, AddrEnd}};
        }
    };


    export
    struct patch_item {
        patch_item(std::string_view key, std::unique_ptr<patch_base>&& patch) :
            key_(key), patch_(std::move(patch)) 
        {}

        void rename(std::string_view key) {
            key_ = key;
        }
        
        std::string_view key() const { return key_; }
        patch_base& patch() const { return *patch_; }
        patch_base* operator ->() const { return &patch(); }

    private:
        std::string key_;
        std::shared_ptr<patch_base> patch_;
    };

    export
    std::unique_ptr<raw_mem_patch> fill_nop(std::uintptr_t addr_begin, std::uintptr_t addr_end) {
        return std::make_unique<raw_mem_patch>(mem_chunk(addr_begin, addr_end, byte_array_t(addr_end - addr_begin, 0x90)));
    }


    

    
    export
    struct patch_collection {
        template<std::ranges::range... T>
        explicit patch_collection(T&&... collection) :
            collection_(std::from_range, ref_array{collection...} | std::views::join | std::views::as_rvalue) 
        {}
        
        struct entry_t {
            refw<patch_item const> patch_ref;
            bounds_t bounds;
        };

        using error_list_t = std::vector<std::pair<refw<patch_item const>, std::exception>>;

        struct diagnostic {
            bool success;
            std::vector<entry_t> intersections;
            error_list_t errors;
            
            friend std::ostream& operator<<(std::ostream& os, diagnostic const& diag) {
                if (diag.success) {
                    os << "SUCCESS!\nPatch collection was successfully loaded.\n";
                    return os << std::endl;
                }
                os << "FAIL!\n";
                if (!diag.intersections.empty()) {
                    os << "Patch collection has intersecting segments:\n";
                    for (auto iter = diag.intersections.begin(), end = diag.intersections.end(); iter < end; iter += 2) {
                        os << format("\t'{}' [{:X}; {:X}] intersects '{}' [{:X}; {:X}]\n",
                            iter[0].patch_ref->key(), iter[0].bounds.addr_begin, iter[0].bounds.addr_end,
                            iter[1].patch_ref->key(), iter[1].bounds.addr_begin, iter[1].bounds.addr_end);
                    }
                }
                if (!diag.errors.empty()) {
                    os << "Errors occurred:\n";
                    for (auto& [p, ex] : diag.errors) {
                        os << std::format("\t'{}': {}\n", p->key(), ex.what());
                    }
                }
                return os << std::endl;
            }
        };

        diagnostic apply() const {
            auto intersections = check_intersections();

            bool success = intersections.empty();
            error_list_t errors;
            if (success) {
                errors = write_patches();
                success = errors.empty();
            }

            return diagnostic{success, std::move(intersections), std::move(errors)};
        }
        
    private:
        std::vector<patch_item> const collection_;
    
        std::vector<entry_t> check_intersections() const {
            auto bounds = collection_
                | std::views::transform([](patch_item const& p) {
                    return p->occupied_sections()
                        | std::views::transform([&p](bounds_t const& b) {
                            return entry_t{p, b};
                        });
                })
                | std::views::join
                | std::ranges::to<std::vector>();

            std::ranges::sort(bounds, [](entry_t const& x, entry_t const& y) {
                return x.bounds.addr_begin < y.bounds.addr_begin;
            });

            using tpl = std::tuple<entry_t, entry_t>;
            auto intersections = bounds
                | std::views::slide(2)
                | std::views::filter([](auto const& t) {
                    return t[0].bounds.addr_end > t[1].bounds.addr_begin;
                })
                | std::views::join
                | std::ranges::to<std::vector>();

            return intersections;
        }
    
        error_list_t write_patches() const {
            error_list_t errors{};
            for (auto& patch : collection_) {
                try {
                    patch->write_data();
                } catch (const std::exception& e) {
                    errors.push_back({patch, e});
                }
            }
            return errors;
        }
        
    };

}
