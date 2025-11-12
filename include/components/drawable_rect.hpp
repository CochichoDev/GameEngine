#ifndef DRAWABLE_RECT_H
#define DRAWABLE_RECT_H

#include<functional>

#include "renderer.hpp"
#include "vector.hpp"
#include "transform.hpp"
#include "component_pool.hpp"

class Drawable {
    public:
        explicit Drawable (size_t transform_idx) : transform_idx (transform_idx) {}

    public:
        size_t transform_idx;
        std::function<void(std::vector<RenderCommand>&, const Transform&)> push_render_cmd;
};

namespace Drawables {
    
    std::function<void(std::vector<RenderCommand>&, const Transform&)> 
    render_rectangle(Vector2D size, Vector3D color) 
    {
        return [=](std::vector<RenderCommand>& out, const Transform& t) {
            out.push_back(RenderCommand{t.value, size, color});
        };
    }

}

#endif
