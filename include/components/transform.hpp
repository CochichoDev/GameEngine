#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "containers/component_pool.hpp"

#include "vector.hpp"

struct Transform {
    Vector2D value;
};

template<>
class ComponentPoolTraits<Transform, void> {
    template<typename... Ts, typename R>
    static void init(ComponentPool<Transform, void>&, TypeMap<Ts...>&) {
        return;
    }
};

#endif
