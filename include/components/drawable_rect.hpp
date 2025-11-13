#ifndef DRAWABLE_RECT_H
#define DRAWABLE_RECT_H

#include<functional>

#include "renderer.hpp"
#include "vector.hpp"
#include "transform.hpp"
#include "component_pool.hpp"

class RectangleDrawable {
    public:
        explicit RectangleDrawable (size_t transform_idx) : transform_idx (transform_idx) {}

    public:
        size_t transform_idx;
};

#endif
