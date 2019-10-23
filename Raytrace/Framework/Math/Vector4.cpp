#include "Vector4.h"
#include "Framework/Math/MathUtility.h"

namespace Framework::Math {
    //�萔
    const Vector4 Vector4::ZERO = Vector4(0.0f, 0.0f, 0.0f, 0.0f);

    //�R���X�g���N�^
    Vector4::Vector4(float x, float y, float z, float w)
        :x(x), y(y), z(z), w(w) { }
    //�R���X�g���N�^
    Vector4::Vector4(const Vector4& v)
        : x(v.x), y(v.y), z(v.z), w(v.w) { }
    //���
    Vector4& Vector4::operator=(const Vector4& a) {
        x = a.x; y = a.y; z = a.z; w = a.w;
        return *this;
    }
    //��r
    bool Vector4::operator==(const Vector4& a) const {
        return x == a.x && y == a.y && z == a.z && w == a.w;
    }
    //��r
    bool Vector4::operator!=(const Vector4& a) const {
        return x != a.x || y != a.y || z != a.z || w != a.w;
    }
    //�P��+
    Vector4 Vector4::operator+() const {
        return Vector4(x, y, z, w);
    }
    //�P��-
    Vector4 Vector4::operator-() const {
        return Vector4(-x, -y, -z, -w);
    }
    //���Z
    Vector4 Vector4::operator+(const Vector4& a) const {
        return Vector4(x + a.x, y + a.y, z + a.z, w + a.w);
    }
    //���Z
    Vector4 Vector4::operator-(const Vector4& a) const {
        return Vector4(x - a.x, y - a.y, z - a.z, w - a.w);
    }
    //��Z
    Vector4 Vector4::operator*(float a) const {
        return Vector4(x * a, y * a, z * a, w * a);
    }
    //���Z
    Vector4 Vector4::operator/(float a) const {
        float oneOverA = 1.0f / a;
        return Vector4(x * oneOverA, y * oneOverA, z * oneOverA, w * oneOverA);
    }
    //���Z���
    Vector4& Vector4::operator+=(const Vector4& a) {
        x += a.x; y += a.y; z += a.z; w += a.w;
        return *this;
    }
    //���Z���
    Vector4& Vector4::operator-=(const Vector4& a) {
        x -= a.x; y -= a.y; z -= a.z; w -= a.w;
        return *this;
    }
    //��Z���
    Vector4& Vector4::operator*=(float a) {
        x *= a; y *= a; z *= a; w *= a;
        return *this;
    }
    //���Z���
    Vector4& Vector4::operator/=(float a) {
        float oneOverA = 1.0f / a;
        *this *= oneOverA;
        return *this;
    }
    //�����̓��
    float Vector4::lengthSq() const {
        return x * x + y * y + z * z + w * w;
    }
    //����
    float Vector4::length() const {
        return MathUtil::sqrt(lengthSq());
    }
    //���K������
    void Vector4::normalize() {
        const float lenSq = lengthSq();
        if (lenSq > 0.0f) {
            float oneOverLenSq = 1.0f / MathUtil::sqrt(lenSq);
            x *= oneOverLenSq; y *= oneOverLenSq; z *= oneOverLenSq; w *= oneOverLenSq;
        }
    }
    //���K���x�N�g����Ԃ�
    Vector4 Vector4::getNormal() const {
        float lenSq = lengthSq();
        if (lenSq > 0.0f) {
            float oneOverLenSq = 1.0f / MathUtil::sqrt(lenSq);
            return Vector4(x * oneOverLenSq, y * oneOverLenSq, z * oneOverLenSq, w * oneOverLenSq);
        }
        return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    //����
    float Vector4::dot(const Vector4& a) const {
        return x * a.x + y * a.y + z * a.z + w * a.w;
    }
    //����
    float Vector4::dot(const Vector4& v1, const Vector4& v2) {
        return v1.dot(v2);
    }
    //�N�����v����
    void Vector4::clamp(const Vector4& min, const Vector4& max) {
        x = MathUtil::clamp(x, min.x, max.x);
        y = MathUtil::clamp(y, min.y, max.y);
        z = MathUtil::clamp(z, min.z, max.z);
        w = MathUtil::clamp(w, min.w, max.w);
    }
    //��Z
    Vector4 operator*(float a, const Vector4& v) {
        return v * a;
    }
    //�X�g���[���o��
    std::ostream& operator<<(std::ostream& oss, const Math::Vector4& v) {
        oss << "(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
        return oss;

    }
} //Framework::Math