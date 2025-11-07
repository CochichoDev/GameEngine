#ifndef VECTOR_H
#define VECTOR_H

struct Vector2D {
    double x;
    double y;

    Vector2D operator*(double d) const noexcept {
        return {x*d, x*d};
    }
    Vector2D& operator*=(double d) noexcept {
        x *= d;
        y *= d;
        return *this;
    }

    Vector2D operator+(const Vector2D& v) const noexcept {
        return {x+v.x, y+v.y};
    }
    Vector2D& operator+=(const Vector2D& v) noexcept {
        x += v.x;
        y += v.y;

        return *this;
    }

    friend Vector2D operator*(double d, const Vector2D& v) noexcept { return {v.x*d, v.y*d}; }
};

#endif
