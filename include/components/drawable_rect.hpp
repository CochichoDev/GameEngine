#ifndef DRAWABLE_RECT_H
#define DRAWABLE_RECT_H

#include <cstddef>

#include "containers/registry.hpp"

#include "components/transform.hpp"

#include "renderer.hpp"

class RectangleDrawable {
    public:
        explicit RectangleDrawable (size_t transform_idx) : transform_idx (transform_idx) {}

        void build_render_cmd(std::vector<RenderCommand>& cmds, Transform t) {
            cmds.emplace_back(t.value, Vector2D<double>{10, 20}, Vector3D<double>{0, 255, 0});
        }

    public:
        std::size_t transform_idx;
};

template<>
struct ComponentPoolTraits<RectangleDrawable, RenderRegistry> {
        template<typename... Ts>
        static void init(ComponentPool<RectangleDrawable, RenderRegistry>& self, TypeMap<Ts...>& pools) {
            pools.template get<Transform>().subscribe_remove_listener(
                [&](EntityID owner) {
                    self.remove(owner);
                }
            );
            pools.template get<Transform>().subscribe_swap_listener(
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
