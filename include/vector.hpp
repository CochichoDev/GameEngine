#ifndef VECTOR_H
#define VECTOR_H

struct Vector2D {
    double x;
    double y;

    Vector2D operator*(double d) const noexcept {
        return {x*d, y*d};
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

struct Vector3D {
    double x;
    double y;
    double z;

    Vector3D operator*(double d) const noexcept {
        return {x*d, y*d, z*d};
    }
    Vector3D& operator*=(double d) noexcept {
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

    friend Vector3D operator*(double d, const Vector3D& v) noexcept { return {v.x*d, v.y*d, v.z*d}; }
};

#endif
