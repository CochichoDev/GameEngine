#ifndef VECTOR_H
#define VECTOR_H

#include <type_traits>

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Vector2D {
    T x;
    T y;

    Vector2D operator*(T d) const noexcept {
        return {x*d, y*d};
    }
    Vector2D& operator*=(T d) noexcept {
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

    friend Vector2D<T> operator*(T d, const Vector2D<T>& v) noexcept { return {v.x*d, v.y*d}; }
};

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Vector3D {
    T x;
    T y;
    T z;

    Vector3D operator*(T d) const noexcept {
        return {x*d, y*d, z*d};
    }
    Vector3D& operator*=(T d) noexcept {
        x *= d;
        y *= d;
        z *= d;
        return *this;
    }

    Vector3D operator+(const Vector3D& v) const noexcept {
        return {x+v.x, y+v.y, z+v.z};
    }
    Vector3D& operator+=(const Vector3D& v) noexcept {
        x += v.x;
        y += v.y;
        z += v.z;

        return *this;
    }

    friend Vector3D operator*(T d, const Vector3D<T>& v) noexcept { return {v.x*d, v.y*d, v.z*d}; }
};

#endif
