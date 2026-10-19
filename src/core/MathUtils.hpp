#pragma once

#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

// Global Vector3 Operator Overloads for C++
inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3Add(a, b);
}

inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3Subtract(a, b);
}

inline Vector3 operator*(const Vector3& v, float s) {
    return Vector3Scale(v, s);
}

inline Vector3 operator*(float s, const Vector3& v) {
    return Vector3Scale(v, s);
}

inline Vector3 operator/(const Vector3& v, float s) {
    return { v.x / s, v.y / s, v.z / s };
}

inline Vector3& operator+=(Vector3& a, const Vector3& b) {
    a = a + b;
    return a;
}

inline Vector3& operator-=(Vector3& a, const Vector3& b) {
    a = a - b;
    return a;
}

inline Vector3& operator*=(Vector3& a, float s) {
    a = a * s;
    return a;
}

namespace math {

inline float Length(const Vector3& v) {
    return Vector3Length(v);
}

inline float LengthXZ(const Vector3& v) {
    return std::sqrt(v.x * v.x + v.z * v.z);
}

inline Vector3 Normalize(const Vector3& v) {
    return Vector3Normalize(v);
}

inline Vector3 NormalizeXZ(const Vector3& v) {
    float len = LengthXZ(v);
    if (len < 0.0001f) return { 0.0f, 0.0f, 0.0f };
    return { v.x / len, 0.0f, v.z / len };
}

inline float Dot(const Vector3& a, const Vector3& b) {
    return Vector3DotProduct(a, b);
}

inline Vector3 Cross(const Vector3& a, const Vector3& b) {
    return Vector3CrossProduct(a, b);
}

inline float Distance(const Vector3& a, const Vector3& b) {
    return Vector3Distance(a, b);
}

inline float Lerp(float a, float b, float t) {
    return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
}

inline Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
    return Vector3Lerp(a, b, std::clamp(t, 0.0f, 1.0f));
}

inline float Approach(float current, float target, float maxDelta) {
    if (current < target) return std::min(current + maxDelta, target);
    return std::max(current - maxDelta, target);
}

inline Vector3 ForwardFromYaw(float yawRad) {
    return { -std::sin(yawRad), 0.0f, -std::cos(yawRad) };
}

inline Vector3 RightFromYaw(float yawRad) {
    return { std::cos(yawRad), 0.0f, -std::sin(yawRad) };
}

} // namespace math
