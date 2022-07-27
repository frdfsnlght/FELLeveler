#ifndef VECTOR3_H
#define VECTOR3_H

#include <Arduino.h>

class Vector3 {

    public:
    
    float x, y, z;

    static constexpr float Epsilon = 0.00001f;
    static constexpr float FloatEpsilon = 3.4028235E-38f;
    static constexpr float EpsilonNormalSqrt = 1e-15f;
    static constexpr float Rad2Deg = 180.0f / PI;

    static void cross(Vector3 &lhs, Vector3 &rhs, Vector3 &out);
    static float dot(Vector3 &lhs, Vector3 &rhs);
    static void projectOnPlane(Vector3 &vector, Vector3 &planeNormal, Vector3 &out);
    static float angle(Vector3 &from, Vector3 &to);
    static float signedAngle(Vector3 &from, Vector3 &to, Vector3 &axis);

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    void set(float sx, float sy, float sz);
    void set(Vector3 &v);
    float magnitude();
    float squareMagnitude();
    void normalize();

    const String toString();

    friend bool operator==(const Vector3 &lhs, const Vector3 &rhs) {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
    }

};

#endif
