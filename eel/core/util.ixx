export module eel.util;

import std;
import zpp.bits;
export import external.zstring_view;

using namespace std::literals;



namespace eel::util {

    export
    [[nodiscard]] int ifloor(double x) {
        return static_cast<int>(std::floor(x));
    }

    export
    [[nodiscard]] int iceil(double x) {
        return static_cast<int>(std::ceil(x));
    }

    export
    [[nodiscard]] int iround(double x) {
        return static_cast<int>(std::round(x));
    }

    export
    template<typename T>
    [[nodiscard]] int sgn(T x) {
        if (x > 0) return 1;
        if (x < 0) return -1;
        return 0;
    }
    
    export
    decltype(auto) random_element(auto&& range, auto&& rng) {
        if (std::ranges::empty(range)) {
            throw std::out_of_range("range is empty");
        }
        return range[std::uniform_int_distribution(0, std::ranges::ssize(range) - 1)(rng)];
    }

    template<typename T>
    decltype(auto) vector_get_value_helper(T&& v, std::size_t index) {
        if constexpr (requires { vector_get_value(v, index); }) {
            return vector_get_value(v, index);
        } else {
            return v[index];
        }
    } 

    export
    template<typename T>
    concept vec_like = requires (T v, T* ptr) {
        requires allow_vector_operations(decltype(ptr)());
        [] {
            constexpr std::size_t size = vector_size(decltype(ptr)());
        };
        (vector_get_value_helper)(v, 0);
    };

    template<vec_like T>
    consteval std::size_t get_vector_size() {
        std::remove_cvref_t<T>* p{};
        return vector_size(p);
    }
    
    template<auto Op, typename Vec, typename X>
    Vec& vec_arithmetic(Vec& lhs, X&& rhs) {
        for (std::size_t i = 0; i < get_vector_size<Vec>(); i++) {
            if constexpr (std::same_as<std::remove_cvref_t<X>, std::remove_cvref_t<Vec>>) {
                (vector_get_value_helper)(lhs, i) = Op((vector_get_value_helper)(lhs, i), (vector_get_value_helper)(rhs, i));
            } else {
                (vector_get_value_helper)(lhs, i) = Op((vector_get_value_helper)(lhs, i), rhs);
            }
        }
        return lhs;
    }

    export
    template<vec_like T>
    T& operator +=(T& lhs, T const& rhs) {
        return vec_arithmetic<[](auto&& a, auto&& b){ return a + b; }>(lhs, rhs);
    }
    
    export
    template<vec_like T>
    T& operator -=(T& lhs, T const& rhs) {
        return vec_arithmetic<[](auto&& a, auto&& b){ return a - b; }>(lhs, rhs);
    }
    
    export
    template<vec_like T, typename U>
    T& operator *=(T& lhs, U const& rhs) {
        return vec_arithmetic<[](auto&& a, auto&& b){ return a * b; }>(lhs, rhs);
    }

    export
    template<vec_like T>
    T operator+(T const& lhs) { return lhs; }

    export
    template<vec_like T>
    T operator-(T const& lhs) { auto r = lhs; r *= -1; return r; }
    
    export
    template<vec_like T>
    T operator+(T const& lhs, T const& rhs) { auto r = lhs; r += rhs; return r; }
    
    export
    template<vec_like T>
    T operator-(T const& lhs, T const& rhs) { auto r = lhs; r -= rhs; return r; }
    
    export
    template<vec_like T, typename U>
    T operator*(T const& lhs, U const& rhs) { auto r = lhs; r *= rhs; return r; }
    
    export
    template<vec_like T, typename U>
    T operator*(U const& lhs, T rhs) { auto r = rhs; r *= lhs; return r; }

    export
    template<vec_like T, vec_like U>
    requires (get_vector_size<T>() == get_vector_size<U>())
    T vector_cast(U const& v) {
        T result;
        using target_type = std::remove_cvref_t<decltype((vector_get_value_helper)(result, 0))>;
        for (std::size_t i = 0; i < get_vector_size<T>(); i++) {
            (vector_get_value_helper)(result, i) = static_cast<target_type>((vector_get_value_helper)(v, i));
        }
        return result;
    }

    export
    template<typename T, size_t N>
    struct vec : std::array<T, N> {
        template<size_t I>
        friend auto& get(std::common_reference_with<vec> auto& self) { return self[I]; }
    };
    export
    template<typename... T>
    vec(T...) -> vec<std::common_type_t<T...>, sizeof...(T)>;

    export
    template<typename T, std::size_t N>
    consteval bool allow_vector_operations(vec<T, N>*) { return true; }

    export
    template<typename T, std::size_t N>
    consteval std::size_t vector_size(vec<T, N>*) { return N; }
    
    
    constexpr auto tuple_visitor_ = []<typename... U>() -> std::tuple<U...> { throw 0; };
    
    export
    template<typename T>
    requires std::is_aggregate_v<T>
    using as_tuple_t = decltype(zpp::bits::visit_members_types<T>(tuple_visitor_));
    
    
    
    export
    template<typename T>
    requires std::is_scoped_enum_v<T>
    constexpr T enum_and(T x, T y) {
        using U = std::underlying_type_t<T>;
        return static_cast<T>(static_cast<U>(x) & static_cast<U>(y));
    }
    
    export
    template<typename T>
    requires std::is_scoped_enum_v<T>
    constexpr T enum_or(T x, T y) {
        using U = std::underlying_type_t<T>;
        return static_cast<T>(static_cast<U>(x) | static_cast<U>(y));
    }
    
    export
    template<typename T>
    requires std::is_scoped_enum_v<T>
    constexpr T enum_xor(T x, T y) {
        using U = std::underlying_type_t<T>;
        return static_cast<T>(static_cast<U>(x) ^ static_cast<U>(y));
    }
    
    export
    template<typename T>
    requires std::is_scoped_enum_v<T>
    constexpr T enum_neg(T x) {
        using U = std::underlying_type_t<T>;
        return static_cast<T>(~static_cast<U>(x));
    }
    
    export 
    enum struct fake_edx_t : int {};
    export constexpr auto fake_edx = fake_edx_t();

    export
    template<typename T>
    T* ptr_to(std::uintptr_t address) {
        return reinterpret_cast<T*>(address);
    }

    export
    template<typename T>
    T& ref_to(std::uintptr_t address) {
        return *ptr_to<T>(address);
    }
    
    export
    template<typename T, typename U>
    requires (sizeof(T) <= sizeof(U))
    T& reinterpret_as(U& r) {
        return reinterpret_cast<T&>(r);
    }
    
    export
    struct dword {

        template<typename T>
        explicit dword(T* v) : ptr_(reinterpret_cast<std::uintptr_t>(v)) {}

        constexpr explicit dword(std::uintptr_t v) : ptr_(v) {}

        constexpr dword(std::nullptr_t) : ptr_(0) {}

        template<typename T>
        explicit operator T&() const { return ref_to<std::remove_reference_t<T>>(ptr_); }

        template<typename T>
        explicit operator T() const = delete;
        
        template<typename T>
        explicit operator T*() const { return ptr_to<std::remove_pointer_t<T>>(ptr_); }

        constexpr explicit operator std::uintptr_t() const { return ptr_; }

        dword operator *() const { return *ptr_to<dword>(ptr_); }

        constexpr dword& operator+=(dword r) { ptr_ += r.ptr_; return *this; }
        constexpr dword& operator-=(dword r) { ptr_ -= r.ptr_; return *this; }
        constexpr dword& operator*=(dword r) { ptr_ *= r.ptr_; return *this; }
        friend constexpr dword operator+(dword l, dword r) { return l += r; }
        friend constexpr dword operator-(dword l, dword r) { return l -= r; }
        friend constexpr dword operator*(dword l, dword r) { return l *= r; }
        friend constexpr std::strong_ordering operator<=>(dword, dword) = default;

    private:
        std::uintptr_t ptr_;
    };

    export
    consteval dword operator ""_dw(unsigned long long v) { return dword(static_cast<std::uintptr_t>(v)); }

    export
    template<typename T>
    T& ref_to(dword address) {
        return static_cast<T&>(address);
    }

    export
    template<typename T>
    T* ptr_to(dword address) {
        return static_cast<T*>(address);
    }

    dword unpack_ptr(dword p) {
        auto a = *(p + 0x4_dw);
        auto d = *(a + 0x8_dw);
        return d + p + 0x4_dw;
    }

    export
    template<typename T>
    struct packed {
        packed() = default;
        explicit packed(dword v) : ptr_(v) {}
        explicit packed(std::uintptr_t v) : ptr_(dword(v)) {}
        packed(std::nullptr_t) {}
        explicit operator bool() const { return ptr_; }
        T* unpack() const { return ptr_ ? &**this : nullptr; }
        void* raw_ptr() const { return ptr_; }
        T* operator ->() const { return ptr_to<T>(unpack_ptr(dword(ptr_))); }
        T& operator *() const { return ref_to<T>(unpack_ptr(dword(ptr_))); }
    private:
        void* ptr_{};
    };


    export
    template<typename Element, size_t N>
    constexpr auto from_array(Element (&&v)[N]) {
        return std::span<Element>(v) | std::views::as_rvalue;
    }

    export
    template<template<typename...> typename Template, typename Element, size_t N>
    constexpr auto init_range(Element (&&v)[N]) {
        return Template<Element>(std::from_range, (from_array)(v));
    }

    export
    template<typename Container, typename Element, size_t N>
    constexpr auto init_range(Element (&&v)[N]) {
        return Container(std::from_range, (from_array)(v));
    }
    
    template<typename T, size_t... Rest>
    struct mdarray_helper;
    
    template<typename T, size_t N, size_t... Rest>
    struct mdarray_helper<T, N, Rest...> {
        using type = std::array<typename mdarray_helper<T, Rest...>::type, N>;
    };
    
    template<typename T, size_t N>
    struct mdarray_helper<T, N> {
        using type = std::array<T, N>;
    };
    
    export
    template<typename T, size_t... Dimensions>
    using mdarray = mdarray_helper<T, Dimensions...>::type;

    export
    template<typename T>
    struct rectangle {
        T x1{}, x2{}, y1{}, y2{};

        friend constexpr bool intersect(rectangle const& a, rectangle const& b) {
            return
                a.x1 <= b.x2 && a.x2 >= b.x1 &&
                a.y2 >= b.y1 && a.y1 <= b.y2;
        }
    };

    export
    template<typename T>
    T& as_lvalue(T&& r) {
        return r;
    }

    export
    struct semaphore_lock {
        explicit semaphore_lock(std::binary_semaphore& sem)
            : sem_(sem)
        {
            sem_.acquire();
        }
        semaphore_lock(semaphore_lock const&) = delete;
        auto& operator=(semaphore_lock const&) = delete;
        ~semaphore_lock() {
            sem_.release();
        }
    private:
        std::binary_semaphore& sem_;
    };

    export
    template<typename T>
    struct thread_safe_ptr {
        explicit thread_safe_ptr(T& ref, std::binary_semaphore& sem)
            : ref_(ref), lk_(sem)
        {}

        thread_safe_ptr(thread_safe_ptr const&) = delete;
        auto& operator =(thread_safe_ptr const&) = delete;

        auto& operator *() const { return ref_; }
        auto* operator ->() const { return &ref_; }
        
        template<size_t I = 0>
        requires (I == 0)
        auto& get() const { return ref_; }
    private:
        T& ref_;
        semaphore_lock lk_;
    };

    export
    template<typename T>
    struct thread_safe {
        template<typename... Args>
        explicit thread_safe(Args&&... args)
            : value_(std::forward<Args>(args)...)
            , sem_(1)
        {}
        
        thread_safe(thread_safe const&) = delete;
        auto& operator =(thread_safe const&) = delete;
        
        auto acquire() {
            return thread_safe_ptr<T>(value_, sem_);
        }
        
        auto acquire() const {
            return thread_safe_ptr<T const>(value_, sem_);
        }
        
    private:
        T value_;
        mutable std::binary_semaphore sem_;
    };

    template<typename T>
    requires !std::is_reference_v<T>
    struct opt_ref {
        using value_type = T&;
        
        constexpr opt_ref() noexcept = default;
        constexpr opt_ref(std::nullopt_t) noexcept : opt_ref() {}
        constexpr opt_ref(T& r) noexcept : ptr_(&r) {}
        constexpr opt_ref(T&& r) = delete;
        constexpr opt_ref(T* p) noexcept : ptr_(p) {}
        constexpr opt_ref(std::optional<std::reference_wrapper<T>> const& r) noexcept : ptr_(r ? r->get() : nullptr) {}

        constexpr bool has_value() const noexcept { return ptr_ != nullptr; }
        constexpr T& value() const { return has_value() ? *ptr_ : throw std::bad_optional_access(); }
        constexpr T& value_or(T& dflt) const noexcept { return has_value() ? *ptr_ : dflt; }
        constexpr T& value_or(T&& dflt) const noexcept = delete;
        constexpr auto and_then(auto&& func) const { return has_value() ? func(*ptr_) : std::nullopt; }
        constexpr void swap(opt_ref& other) noexcept { std::swap(ptr_, other.ptr_); }
        constexpr void reset() noexcept { ptr_ = nullptr; }
        constexpr void emplace(T& r) noexcept { ptr_ = &r; }
        constexpr void emplace(T&& r) noexcept = delete;
        
        constexpr explicit operator bool() const noexcept { return has_value(); }
        constexpr T& operator *() const noexcept { return *ptr_; }
        constexpr T* operator ->() const noexcept { return ptr_; }
        constexpr explicit operator T*() const noexcept { return ptr_; }
        constexpr explicit operator std::optional<std::reference_wrapper<T>>() const noexcept { return ptr_ ? *ptr_ : std::nullopt; }

        friend constexpr bool operator==(opt_ref const& a, T const& b) { return a.has_value() && *a == b; }
    private:
        T* ptr_ = nullptr;
    };

    export
    template<typename T>
    struct opt : std::optional<T> {
        using std::optional<T>::optional;
    };

    export
    template<typename T>
    requires std::is_lvalue_reference_v<T>
    struct opt<T> : opt_ref<std::remove_reference_t<T>> {
        using opt_ref<std::remove_reference_t<T>>::opt_ref;
    };

    export
    template<typename T>
    opt(T) -> opt<std::remove_cvref_t<T>>;

    export
    template<typename T>
    struct refw : std::reference_wrapper<T> {
        using std::reference_wrapper<T>::reference_wrapper;
        constexpr T& operator *() const noexcept { return this->get(); }
        constexpr T* operator ->() const noexcept { return &this->get(); }
    };
    template<typename T>
    refw(T&) -> refw<T>;
    
    template<typename T>
    struct ref_array_iterator {
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        
        refw<T> const* it;
        constexpr ref_array_iterator& operator ++() { ++it; return *this; }
        constexpr ref_array_iterator& operator --() { --it; return *this; }
        constexpr ref_array_iterator operator ++(int) { return {it++}; }
        constexpr ref_array_iterator operator --(int) { return {it--}; }
        friend constexpr ref_array_iterator operator +(ref_array_iterator lhs, difference_type rhs) { return {lhs.it + rhs}; }
        friend constexpr ref_array_iterator operator +(difference_type lhs, ref_array_iterator rhs) { return {lhs + rhs.it}; }
        friend constexpr difference_type operator -(ref_array_iterator lhs, ref_array_iterator rhs) { return lhs.it - rhs.it; }
        friend constexpr ref_array_iterator operator -(ref_array_iterator lhs, difference_type rhs) { return {lhs.it - rhs}; }
        constexpr ref_array_iterator& operator +=(difference_type diff) { it += diff; return *this; }
        constexpr ref_array_iterator& operator -=(difference_type diff) { it -= diff; return *this; }
        constexpr T& operator *() const { return *it; }
        constexpr T& operator [](difference_type index) const { return it[index]; }
        friend constexpr std::strong_ordering operator<=>(ref_array_iterator, ref_array_iterator) = default;
    };
    static_assert(std::random_access_iterator<ref_array_iterator<int>>);
    
    export
    template<typename T, size_t Size>
    requires !std::is_reference_v<T>
    struct ref_array {
        using iterator = ref_array_iterator<T>;
        
        std::array<refw<T>, Size> storage;
        
        constexpr iterator begin() const { return iterator{storage.data()}; }
        constexpr iterator end() const { return iterator{storage.data() + storage.size()}; }
        constexpr size_t size() const { return storage.size(); }
        constexpr T& operator[](size_t index) const { return storage[index]; }
    };
    template<typename T, typename... U>
    ref_array(T&, U&... refs) -> ref_array<T, 1 + sizeof...(refs)>;

    export
    struct string_hash {
        using hash_type = std::hash<std::string_view>;
        using is_transparent = void;
        
        std::size_t operator()(const char* str) const        { return hash_type{}(str); }
        std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
        std::size_t operator()(const std::string& str) const { return hash_type{}(str); }
    };

    export
    template<typename Map, typename Key>
    auto mfind(Map& map, Key const& key) {
        auto iter = map.find(key);
        return iter != map.end()
            ? opt<decltype((iter->second))>(iter->second)
            : std::nullopt;
    }
    
    export
    template<typename... T>
    struct overload : T... {
        using T::operator()...;
        
        template<typename VT>
        constexpr decltype(auto) visit(VT&& variant) const {
            return std::visit(*this, std::forward<VT>(variant));
        }
    };

    export
    template<typename T>
    struct as_variant : T::variant_type, T {
        using T::variant_type::variant_type, T::T;
    };
    
    //==================================================================================================================
    //  METAPROGRAMMING
    //==================================================================================================================
    
    export
    template<typename T>
    using type = T;
    
    export
    template<typename T, typename... TT>
    concept one_of = (std::same_as<T, TT> || ...);
    
    export
    template <class T, template <typename...> class Template> 
    concept specialization_of = requires (std::remove_cvref_t<T> t) {  
        []<typename... Args> 
        requires std::same_as<T, Template<Args...>>
        (Template<Args...>&) {} (t);
    };

    export
    template<typename Func>
    requires std::is_function_v<Func>
    struct function_ref;
    
    export
    template<typename R, typename... Args>
    struct function_ref<R(Args...)> {
        using func_ptr = R(*)(Args...);
    
        template<typename Callable>
        requires std::same_as<std::invoke_result_t<Callable, Args...>, R>
        function_ref(Callable&& callable) {
            func_ = &callable;
            caller_ = [](void* func, Args... args) {
                return (*static_cast<Callable*>(func))(std::forward<Args>(args)...);
            };
        }

        template<typename T>
        requires std::same_as<std::invoke_result_t<T, Args...>, R> && std::convertible_to<T, func_ptr>
        function_ref(T&& callable) {
            func_ = static_cast<func_ptr>(callable);
            caller_ = [](void* func, Args... args) {
                return static_cast<func_ptr>(func)(std::forward<Args>(args)...);
            };
        }
        
        R operator()(Args... args) const {
            return caller_(func_, std::forward<Args>(args)...);
        }
        
    private:
        void* func_;
        type<R(void*, Args...)>* caller_;
    };
    
    //==================================================================================================================
    //  CONTAINERS
    //==================================================================================================================
    
    template<
        std::forward_iterator I,
        std::sentinel_for<I> S,
        class T,
        class Proj = std::identity,
        std::indirect_strict_weak_order<const T*, std::projected<I, Proj>> Comp = std::ranges::less>
    constexpr I binary_find(I first, S last, const T& value, Comp comp = {}, Proj proj = {}) {
        first = std::ranges::lower_bound(first, last, value, comp, proj);
        return first != last && !comp(value, proj(*first)) ? first : last;
    }
    
    export
    template<typename Key, typename Value, size_t Size>
    struct constexpr_map {
        using pair = std::pair<Key, Value>;
        using storage_t = std::array<pair, Size>;
        
        consteval constexpr_map(pair (&&values)[Size])
            : storage_(std::to_array(std::move(values)))
        {
            std::ranges::sort(storage_, {}, extractor);
            for (int i = 1; i < Size; ++i) {
                if (storage_[i - 1].first == storage_[i].first) throw std::invalid_argument("duplicate keys");
            }
        }
        constexpr auto begin() const { return storage_.begin(); }
        constexpr auto end() const { return storage_.end(); }
        constexpr auto size() const { return Size; }
        constexpr auto& operator[](Key const& key) const {
            auto it = find(key);
            return it != end() ? it->second : throw std::out_of_range("key not found");
        }
        constexpr auto find(Key const& key) const {
            return binary_find(begin(), end(), key, {}, extractor);
        }
    private:
        storage_t storage_;

        constexpr static auto& extractor(pair const& p) { return p.first; }
    };

    export
    template<typename Key, typename Value, size_t Size>
    consteval auto make_constexpr_map(std::pair<Key, Value> (&&values)[Size]) {
        return constexpr_map<Key, Value, Size>(std::move(values));
    }

    export
    template<typename Key, size_t Size>
    struct constexpr_set {
        using storage_t = std::array<Key, Size>;
        
        consteval constexpr_set(Key (&&values)[Size])
            : storage_(std::to_array(std::move(values)))
        {
            std::ranges::sort(storage_);
            for (int i = 1; i < Size; ++i) {
                if (storage_[i - 1] == storage_[i]) throw std::invalid_argument("duplicate keys");
            }
        }
        constexpr auto begin() const { return storage_.begin(); }
        constexpr auto end() const { return storage_.end(); }
        constexpr auto size() const { return Size; }
        constexpr auto find(Key const& key) const {
            return binary_find(begin(), end(), key);
        }
        constexpr auto contains(Key const& key) const {
            return find(key) != end();
        }
    private:
        storage_t storage_;
    };

    export
    template<typename Key, size_t Size>
    consteval auto make_constexpr_set(Key (&&values)[Size]) {
        return constexpr_set<Key, Size>(std::move(values));
    }
    
    //==================================================================================================================
    //  STRING
    //==================================================================================================================

    export using external::mpt::basic_zstring_view;
    export using external::mpt::zstring_view;
    export using external::mpt::wzstring_view;
    inline namespace literals {
        export using external::mpt::operator ""_zsv;
    }
    export using external::mpt::operator<<;
    
    
    export
    template<size_t Size>
    struct consteval_string {
        char data[Size];
        consteval consteval_string(char const (&s)[Size]) {
            std::copy(std::begin(s), std::end(s), data);
        }
        constexpr auto begin() const { return std::begin(data); }
        constexpr auto end() const { return std::end(data); }
        constexpr auto size() const { return Size; }
        constexpr auto operator[](size_t index) const { return data[index]; }
    };
    
    export
    struct constexpr_sv {
        template<size_t Size>
        consteval constexpr_sv(char const (&s)[Size]) :
            data_(s),
            size_(Size - 1)
        {
        }
        template<typename T>
        consteval constexpr_sv(T&& sv) {
            auto span = std::span<char const>(sv);
            data_ = span.data();
            size_ = span.size();
        }
        constexpr auto begin() const { return data_; }
        constexpr auto end() const { return data_ + size_; }
        constexpr auto size() const { return size_; }
        constexpr auto operator[](size_t index) const { return data_[index]; }
        constexpr operator std::string_view() const { return std::string_view(data_, size_); }

        char const* data_;
        size_t size_;
    };

    export
    template<constexpr_sv CSV>
    consteval constexpr_sv operator""_csv() {
        return CSV;
    }

    constexpr auto zt_size(auto const& s) {
        return s.size() > 0 && s[s.size() - 1] == '\0' ? s.size() - 1 : s.size();
    };

    export
    template<std::ranges::range auto... S>
    consteval auto concat_sv() {
        constexpr auto size = (zt_size(S) + ... + 0);
        constexpr auto array = [] {
            auto r = std::array<char, size + 1>();
            int index = 0;
            auto f = [&](auto& s) {
                std::copy(s.begin(), s.end(), r.begin() + index);
                index += zt_size(s);
            };
            (f(S), ...);
            return r;
        }();
        return array;
    }

    export
    template<unsigned I>
    consteval auto to_string() {
        constexpr int size = [] {
            unsigned x = I;
            int i = 0;
            while (x > 0) {
                x /= 10;
                i++;
            }
            return std::max(i, 1);
        }();
        constexpr auto array = [] {
            auto r = std::array<char, size + 1>();
            unsigned x = I;
            int i = 0;
            while (x > 0) {
                auto rem = x % 10;
                r[r.size() - i - 2] = '0' + rem;
                x /= 10;
                i++;
            }
            return r;
        }();
        return array;
    }
    
    constexpr auto space_chars = " \n\r\t\v\f"sv;
    
    export
    template<consteval_string Str>
    consteval auto operator ""_bytes() {
        static constexpr auto t = [] {
            bool big = true;
            std::uint8_t byte = 0;
            int len = 0;
            auto result = std::array<std::uint8_t, Str.size() / 2>();
            for (size_t i = 0; i < Str.size(); ++i) {
                char c = Str[i];
                if (c == '\0' && i == Str.size() - 1) break; 
                if (std::ranges::contains(space_chars, c)) continue;
                int repr = 0;
                if (c >= '0' && c <= '9') repr = c - '0';
                else if (c >= 'a' && c <= 'f') repr = c - 'a' + 10;
                else if (c >= 'A' && c <= 'F') repr = c - 'A' + 10;
                else throw std::invalid_argument("invalid char in byte array literal");
                if (big) {
                    byte = repr << 4;
                } else {
                    byte |= repr;
                    result[len] = byte;
                    
                    len++;
                    byte = 0;
                }
                big = !big;
            }
            if (!big) throw std::invalid_argument("odd number of hexadecimal digits");
            return std::tuple(result, len);
        }();
        static constexpr std::array<std::uint8_t, get<1>(t)> result = [] {
            constexpr auto raw_arr = get<0>(t);
            constexpr auto len = get<1>(t);
            auto arr = std::array<std::uint8_t, len>();
            for (int i = 0; i < len; ++i) {
                arr[i] = raw_arr[i];
            }
            return arr;
        }();
        return result;
    }
    
    inline namespace literals {
        using util::operator""_bytes;
    }
    
    export
    template<typename T>
    std::basic_string<T> operator+(std::basic_string<T>&& s, std::basic_string_view<T> const& sv) {
        s += sv;
        return std::move(s);
    }
    
    export
    std::string convert_to_utf8(std::wstring_view utf16_str) {
        // ReSharper disable CppDeprecatedEntity
        auto utf16conv = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>();
        // ReSharper restore CppDeprecatedEntity
        return utf16conv.to_bytes(utf16_str.data(), utf16_str.data() + utf16_str.size());
    }

    export
    using svregex_iterator = std::regex_iterator<std::string_view::const_iterator>;

    export
    struct svmatch_ref {
        explicit svmatch_ref(svregex_iterator const& it)
            : it(it)
        {}

        auto operator[](size_t index) const {
            auto& submatch = (*it)[index];
            return std::string_view(submatch.first, submatch.second);
        }
        auto size() const { return it->size(); }
        auto operator->() const { return &*it; }
    private:
        svregex_iterator const& it;
    };
    
    export
    struct svmatch_results {
        using match_results_t = std::match_results<std::string_view::const_iterator>;
        
        struct iterator {
            match_results_t::const_iterator it;
            auto& operator++() { return ++it; }
            std::string_view operator*() const { return std::string_view(it->first, it->second); }
            friend bool operator==(iterator const&, iterator const&) = default;
        };
        
        explicit svmatch_results() = default;
        
        auto operator[](size_t index) const& {
            auto& submatch = match_results_[index];
            return std::string_view(submatch.first, submatch.second);
        }

        auto& results() & { return match_results_; }
        
        iterator begin() const { return {match_results_.begin()}; }
        iterator end() const { return {match_results_.end()}; }
        size_t size() const { return match_results_.size(); }
        
        int position() const { return match_results_.position(); }
        int length() const { return match_results_.length(); }
    private:
        match_results_t match_results_{};
    };

    export
    struct regex_matches_view {
        struct sentinel {};
        struct iterator {
            svregex_iterator* it;
            auto& operator++() { return ++*it; }
            svmatch_ref operator*() { return svmatch_ref(*it); }
            friend bool operator==(iterator const&, iterator const&) = default;
            bool operator==(sentinel) const { return *it == end_it; }
        };
        
        explicit regex_matches_view(svregex_iterator&& it) : it(std::move(it)) {}
        iterator begin() { return iterator(&it); }
        sentinel end() { return {}; }
    private:
        svregex_iterator it;
        static inline auto const end_it = svregex_iterator();
    };

    export
    auto regex_find_all(
            std::string_view sv,
            std::regex const& rexp,
            std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
        return regex_matches_view(svregex_iterator(sv.begin(), sv.end(), rexp, flags));
    }

    export
    bool regex_search(
            std::string_view sv,
            svmatch_results& match,
            std::regex const& rexp,
            std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
        return std::regex_search(sv.begin(), sv.end(), match.results(), rexp, flags);
    }

    template<typename Element>
    std::basic_string<Element> regex_replace_impl(
        std::basic_string_view<Element> sv,
        std::basic_regex<Element> const& rexp,
        std::basic_string_view<Element> repl,
        std::regex_constants::match_flag_type flags)
    {
        auto result = std::basic_string<Element>();
        std::regex_replace(std::back_inserter(result), sv.data(), sv.data() + sv.size(), rexp, std::basic_string<Element>(repl), flags);
        return result;
    }
    
    template<typename Src>
    using tsv_ = decltype(std::basic_string_view(std::declval<Src>()));
    
    export
    template<typename Src, typename RExp, typename Repl>
    auto regex_replace(
        Src&& sv,
        RExp&& rexp,
        Repl&& repl,
        std::regex_constants::match_flag_type flags = std::regex_constants::match_default)
    requires requires {
        { std::basic_string_view(sv) } -> std::same_as<tsv_<Src>>;
        { std::basic_string_view(repl) } -> std::same_as<tsv_<Src>>;
        { std::basic_regex(rexp) } -> std::same_as<std::basic_regex<typename tsv_<Src>::value_type>>;
    }
    {
        return (regex_replace_impl<tsv_<Src>::value_type>)(sv, rexp, repl, flags);
    }
    
    export
    template<typename Callable>
    requires std::is_invocable_v<Callable, svmatch_ref>
    std::string regex_replace(
            std::string_view sv,
            std::regex const& rexp,
            Callable&& callable,
            std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
        auto result = std::string();
        auto const sv_begin = sv.begin();
        auto last_pos = sv_begin;
        for (auto m : regex_find_all(sv, rexp, flags)) {
            result.append(std::string_view(last_pos, sv_begin + m->position()));
            last_pos = sv_begin + m->position() + m->length();
            result.append(callable(m));
        }
        result.append(std::string_view(last_pos, sv.end()));
        return result;
    }
    
    //==================================================================================================================
    //  FILE SYSTEM
    //==================================================================================================================

    namespace fs {
        
        export
        bool write_all_bytes(
            std::filesystem::path const& file_path,
            auto&& bytes)
        {
            auto span = std::span(bytes);
            auto stream = std::basic_ofstream<typename decltype(span)::value_type>(file_path, std::ios::binary);
            if (!stream.is_open()) {
                return false;
            }
            stream.write(span.data(), span.size());
            return true;
        }

        export
        template<typename T = std::vector<std::byte>>
        opt<T> read_all_bytes(std::filesystem::path const& file_path) {
            using Elem = T::value_type;
            auto file = std::basic_ifstream<Elem>(file_path, std::ios::binary);
            if (!file.is_open()) {
                return std::nullopt;
            }
            auto data = T();
            data.assign(std::istreambuf_iterator<Elem>(file), std::istreambuf_iterator<Elem>());
            return data;
        }
        
    }
    
}

namespace std {
    
    // allow any vec<...> inheritors to use structured bindings
    export
    template<typename T>
    requires (eel::util::allow_vector_operations(static_cast<T*>(nullptr)))
    struct tuple_size<T> {
        static constexpr size_t value = eel::util::vector_size(static_cast<T*>(nullptr));
    };
    export
    template<typename T, size_t I>
    requires (eel::util::allow_vector_operations(static_cast<T*>(nullptr)))
    struct tuple_element<I, T> {
        using type = remove_reference_t<decltype(eel::util::vector_get_value_helper(declval<T>(), I))>;
    };

    
    export
    template<typename T>
    struct tuple_size<eel::util::thread_safe_ptr<T>> {
        static constexpr size_t value = 1;
    };
    export
    template<size_t I, typename T>
    struct tuple_element<I, eel::util::thread_safe_ptr<T>> {
        using type = T&;
    };
    
    
}


