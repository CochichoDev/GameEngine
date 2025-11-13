#ifndef CTIME_TYPEMAP_H
#define CTIME_TYPEMAP_H

#include <type_traits>
#include <functional>

template<typename... Ts>
struct TypeMapNode;

template<typename T, typename... Ts>
struct TypeMapNode<T, Ts...> {
    T value;
    TypeMapNode<Ts...> next;
};

template<>
struct TypeMapNode<> {};

template<typename U, typename T, typename... Ts>
U& get(TypeMapNode<T, Ts...>& node) {
    if constexpr (std::is_same_v<U, T>) return node.value;
    return get<U>(node.next);
}

template<typename U>
U& get(TypeMapNode<>&) {
    static_assert(sizeof(U) == 0, "Type not found in TypeMap");
}

template<typename F>
void for_each(TypeMapNode<>&, F&&) {}

template<typename T, typename... Ts, typename F>
void for_each(TypeMapNode<T, Ts...>& node, F&& func) {
    func(node.value);
    for_each(node.next, std::forward<F>(func));
}

template<typename... Ts>
class TypeMap {
    public:
        template<typename U>
        U& get() {
            return ::get<U>(nodes);
        }

        template<typename F>
        void for_each(F&& func) {
            ::for_each(nodes, std::forward<F>(func));
        }
    
    private:
        TypeMapNode<Ts...> nodes;
};

#endif
