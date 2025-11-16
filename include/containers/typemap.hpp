#ifndef CTIME_TYPEMAP_H
#define CTIME_TYPEMAP_H

#include <stdexcept>
#include <type_traits>

template<typename... Ts>
struct TypeMapNode;

template<typename T, typename... Ts>
struct TypeMapNode<T, Ts...> {
    T value;
    TypeMapNode<Ts...> next;

    template<typename Arg, typename... Args>
    TypeMapNode(Arg a, Args... as) : value (a), next (as...) {}
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

template<std::size_t I = 0, typename T, typename... Ts, typename F>
void for_index(TypeMapNode<T, Ts...>& node, std::size_t idx, F&& func) {
    if (I == idx) {
        func(node.value);
    } else {
        if constexpr (sizeof... (Ts) > 0) {
            for_index<I+1>(node.next, idx, std::forward<F>(func));
        } else {
            throw std::out_of_range("Index is out-of-bounds");
        }
    }
}

template<std::size_t I, typename T, typename... Ts>
auto& get_at(TypeMapNode<T, Ts...>& node) {
    if constexpr (I == 0) {
        return node.value;
    } else {
        static_assert(I <= sizeof... (Ts), "Index out-of bounds");
        return get_at<I-1>(node.next);
    }
}

template<typename U, typename T, typename... Ts>
std::size_t find(TypeMapNode<T, Ts...>& node, std::size_t idx = 0) {
    if constexpr (std::is_same_v<U, T>) {
        return idx;
    } else {
        return find<U>(node.next, idx+1);
    }
}

template<typename U>
std::size_t find(TypeMapNode<>&, std::size_t) {
    static_assert(sizeof(U) == 0, "Type not found in TypeMap");
}

template<typename... Ts>
class TypeMap {
    public:
        template<typename... Args>
        TypeMap(Args&&... args) : nodes (std::forward<Args>(args)...) {}

        template<typename U>
        U& get() {
            return ::get<U>(nodes);
        }

        template<typename F>
        void for_each(F&& func) {
            ::for_each(nodes, std::forward<F>(func));
        }

        template<typename F>
        void for_index(std::size_t idx, F&& func) {
            ::for_index(nodes, idx, std::forward<F>(func));
        }

        template<std::size_t I = 0>
        auto& get_at() {
            return ::get_at<I>(nodes);
        }

        template<typename U>
        std::size_t find() {
            return ::find<U>(nodes);
        }
    
    private:
        TypeMapNode<Ts...> nodes;
};

#endif
