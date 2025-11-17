#ifndef DRAWABLE_RECT_H
#define DRAWABLE_RECT_H

#include <cstddef>

#include "containers/registry.hpp"

#include "components/transform.hpp"

class RectangleDrawable {
    public:
        explicit RectangleDrawable (size_t transform_idx) : transform_idx (transform_idx) {}

    public:
        std::size_t transform_idx;
};

template<>
struct ComponentPoolTraits<RectangleDrawable, RenderRegistry> {
        template<typename... Ts>
        static void init(ComponentPool<RectangleDrawable, RenderRegistry>& self, TypeMap<Ts...>& pools) {
            pools.template get<ComponentPool<Transform, void>>().subscribe_remove_listener(
                [&](EntityID owner) {
                    self.remove(owner);
                }
            );
            pools.template get<ComponentPool<Transform, void>>().subscribe_swap_listener(
                [&](EntityID owner, size_t new_idx) {
                    auto idx = self.find(owner);
                    if (idx.has_value()) {
                        self.entry_at(idx.value()).data.transform_idx = new_idx;
                    }
                }
            );
        }
};

#endif
