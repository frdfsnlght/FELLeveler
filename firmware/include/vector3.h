#ifndef VECTOR3_H
#define VECTOR3_H

#include <Arduino.h>    // for math functions


struct Vector3 {
    float x, y, z;

    static constexpr float Epsilon = 0.00001f;
    static constexpr float FloatEpsilon = 3.4028235E-38f;
    static constexpr float EpsilonNormalSqrt = 1e-15f;
    static constexpr float Rad2Deg = 180.0f / PI;

    Vector3() {
        x = y = z = 0;
    }

    Vector3(float a, float b, float c) {
        x = a;
        y = b;
        z = c;
    }

    float Magnitude() {
        return sqrt((x * x) + (y * y) + (z * z));
    }

    float SquareMagnitude() {
        return (x * x) + (y * y) + (z * z);
    }

    void Normalize() {
        float mag = Magnitude();
        if (mag > Epsilon) {
            x = x / mag;
            y = y / mag;
            z = z / mag;
        } else
            x = y = z = 0;
    }

    static void Cross(Vector3 &lhs, Vector3 &rhs, Vector3 &out) {
        out.x = lhs.y * rhs.z - lhs.z * rhs.y;
        out.y = lhs.z * rhs.x - lhs.x * rhs.z;
        out.z = lhs.x * rhs.y - lhs.y * rhs.x;
    }

    static float Dot(Vector3 &lhs, Vector3 &rhs) {
        return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
    }

    static void ProjectOnPlane(Vector3 &vector, Vector3 &planeNormal, Vector3 &out) {
        float sqrMag = planeNormal.SquareMagnitude();
        if (sqrMag < FloatEpsilon) {
            out.x = vector.x;
            out.y = vector.y;
            out.z = vector.z;
        } else {
            float dot = Dot(vector, planeNormal);
            out.x = vector.x - planeNormal.x * dot / sqrMag;
            out.y = vector.y - planeNormal.y * dot / sqrMag;
            out.z = vector.z - planeNormal.z * dot / sqrMag;
        }
    }

    static float Angle(Vector3 &from, Vector3 &to) {
        float denominator = (float)sqrt(from.SquareMagnitude() * to.SquareMagnitude());
        if (denominator < EpsilonNormalSqrt) return 0.0f;
        float dot = constrain(Dot(from, to) / denominator, -1.0f, 1.0f);
        return ((float)acos(dot)) * Rad2Deg;
    }

    static float SignedAngle(Vector3 &from, Vector3 &to, Vector3 &axis) {
        float unsignedAngle = Angle(from, to);
        float crossX = from.y * to.z - from.z * to.y;
        float crossY = from.z * to.x - from.x * to.z;
        float crossZ = from.x * to.y - from.y * to.x;
        return copysign(unsignedAngle, axis.x * crossX + axis.y * crossY + axis.z * crossZ);
    }

};

#endif
