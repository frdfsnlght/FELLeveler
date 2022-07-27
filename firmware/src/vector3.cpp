#include "vector3.h"

#include <Arduino.h>    // for math functions

void Vector3::cross(Vector3 &lhs, Vector3 &rhs, Vector3 &out) {
    out.x = lhs.y * rhs.z - lhs.z * rhs.y;
    out.y = lhs.z * rhs.x - lhs.x * rhs.z;
    out.z = lhs.x * rhs.y - lhs.y * rhs.x;
}

float Vector3::dot(Vector3 &lhs, Vector3 &rhs) {
    return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
}

void Vector3::projectOnPlane(Vector3 &vector, Vector3 &planeNormal, Vector3 &out) {
    float sqrMag = planeNormal.squareMagnitude();
    if (sqrMag < FloatEpsilon) {
        out.x = vector.x;
        out.y = vector.y;
        out.z = vector.z;
    } else {
        float d = dot(vector, planeNormal);
        out.x = vector.x - planeNormal.x * d / sqrMag;
        out.y = vector.y - planeNormal.y * d / sqrMag;
        out.z = vector.z - planeNormal.z * d / sqrMag;
    }
}

float Vector3::angle(Vector3 &from, Vector3 &to) {
    float denominator = (float)sqrt(from.squareMagnitude() * to.squareMagnitude());
    if (denominator < EpsilonNormalSqrt) return 0.0f;
    float d = fmin(fmax(dot(from, to) / denominator, -1.0f), 1.0f);
    return ((float)acos(d)) * Rad2Deg;
}

float Vector3::signedAngle(Vector3 &from, Vector3 &to, Vector3 &axis) {
    float unsignedAngle = angle(from, to);
    float crossX = from.y * to.z - from.z * to.y;
    float crossY = from.z * to.x - from.x * to.z;
    float crossZ = from.x * to.y - from.y * to.x;
    return copysign(unsignedAngle, axis.x * crossX + axis.y * crossY + axis.z * crossZ);
}

void Vector3::set(float sx, float sy, float sz) {
    x = sx;
    y = sy;
    z = sz;
}

void Vector3::set(Vector3 &v) {
    x = v.x;
    y = v.y;
    z = v.z;
}

float Vector3::magnitude() {
    return sqrt((x * x) + (y * y) + (z * z));
}

float Vector3::squareMagnitude() {
    return (x * x) + (y * y) + (z * z);
}

void Vector3::normalize() {
    float mag = magnitude();
    if (mag > Epsilon) {
        x = x / mag;
        y = y / mag;
        z = z / mag;
    } else
        x = y = z = 0;
}

const String Vector3::toString() {
    String s = "";
    s += '<' + x + ',' + y + ',' + z + '>';
    return s;
}
