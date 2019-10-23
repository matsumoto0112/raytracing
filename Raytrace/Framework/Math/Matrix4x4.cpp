#include "Matrix4x4.h"
#include "Math/MathUtility.h"

namespace Framework::Math {
    //定数宣言
    const Matrix4x4 Matrix4x4::IDENTITY = Matrix4x4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    const Matrix4x4 Matrix4x4::ZERO = Matrix4x4(
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    );
    //コンストラクタ
    Matrix4x4::Matrix4x4()
        :Matrix4x4(IDENTITY) { }
    //コンストラクタ
    Matrix4x4::Matrix4x4(float m11, float m12, float m13, float m14,
        float m21, float m22, float m23, float m24,
        float m31, float m32, float m33, float m34,
        float m41, float m42, float m43, float m44)
        : m({ {{m11,m12,m13,m14},{m21,m22,m23,m24},{m31,m32,m33,m34},{m41,m42,m43,m44}} }) { }
    //コンストラクタ
    Matrix4x4::Matrix4x4(const std::array<std::array<float, 4>, 4>& m)
        : m(m) { }
    //代入
    Matrix4x4& Matrix4x4::operator=(const Matrix4x4& mat) {
        this->m = mat.m;
        return *this;
    }
    //比較
    bool Matrix4x4::operator==(const Matrix4x4& mat) const {
        return  m == mat.m;
    }
    //比較
    bool Matrix4x4::operator!=(const Matrix4x4& mat) const {
        return m != mat.m;
    }
    //単項+
    Matrix4x4 Matrix4x4::operator+() {
        Matrix4x4 mat(m);
        return mat;
    }
    //単項-
    Matrix4x4 Matrix4x4::operator-() {
        Matrix4x4 mat(m);
        mat *= -1;
        return mat;
    }
    //加算
    Matrix4x4 Matrix4x4::operator+(const Matrix4x4& mat) const {
        Matrix4x4 result(m);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] += mat.m[i][j];
            }
        }
        return result;
    }
    //減算
    Matrix4x4 Matrix4x4::operator-(const Matrix4x4& mat) const {
        Matrix4x4 result(m);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] -= mat.m[i][j];
            }
        }
        return result;
    }
    //乗算
    Matrix4x4 Matrix4x4::operator*(float k) const {
        Matrix4x4 result(m);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] *= k;
            }
        }
        return result;
    }
    //乗算
    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& mat) const {
        Matrix4x4 result;
        result.m[0][0] = m[0][0] * mat.m[0][0] + m[0][1] * mat.m[1][0] + m[0][2] * mat.m[2][0] + m[0][3] * mat.m[3][0];
        result.m[0][1] = m[0][0] * mat.m[0][1] + m[0][1] * mat.m[1][1] + m[0][2] * mat.m[2][1] + m[0][3] * mat.m[3][1];
        result.m[0][2] = m[0][0] * mat.m[0][2] + m[0][1] * mat.m[1][2] + m[0][2] * mat.m[2][2] + m[0][3] * mat.m[3][2];
        result.m[0][3] = m[0][0] * mat.m[0][3] + m[0][1] * mat.m[1][3] + m[0][2] * mat.m[2][3] + m[0][3] * mat.m[3][3];

        result.m[1][0] = m[1][0] * mat.m[0][0] + m[1][1] * mat.m[1][0] + m[1][2] * mat.m[2][0] + m[1][3] * mat.m[3][0];
        result.m[1][1] = m[1][0] * mat.m[0][1] + m[1][1] * mat.m[1][1] + m[1][2] * mat.m[2][1] + m[1][3] * mat.m[3][1];
        result.m[1][2] = m[1][0] * mat.m[0][2] + m[1][1] * mat.m[1][2] + m[1][2] * mat.m[2][2] + m[1][3] * mat.m[3][2];
        result.m[1][3] = m[1][0] * mat.m[0][3] + m[1][1] * mat.m[1][3] + m[1][2] * mat.m[2][3] + m[1][3] * mat.m[3][3];

        result.m[2][0] = m[2][0] * mat.m[0][0] + m[2][1] * mat.m[1][0] + m[2][2] * mat.m[2][0] + m[2][3] * mat.m[3][0];
        result.m[2][1] = m[2][0] * mat.m[0][1] + m[2][1] * mat.m[1][1] + m[2][2] * mat.m[2][1] + m[2][3] * mat.m[3][1];
        result.m[2][2] = m[2][0] * mat.m[0][2] + m[2][1] * mat.m[1][2] + m[2][2] * mat.m[2][2] + m[2][3] * mat.m[3][2];
        result.m[2][3] = m[2][0] * mat.m[0][3] + m[2][1] * mat.m[1][3] + m[2][2] * mat.m[2][3] + m[2][3] * mat.m[3][3];

        result.m[3][0] = m[3][0] * mat.m[0][0] + m[3][1] * mat.m[1][0] + m[3][2] * mat.m[2][0] + m[3][3] * mat.m[3][0];
        result.m[3][1] = m[3][0] * mat.m[0][1] + m[3][1] * mat.m[1][1] + m[3][2] * mat.m[2][1] + m[3][3] * mat.m[3][1];
        result.m[3][2] = m[3][0] * mat.m[0][2] + m[3][1] * mat.m[1][2] + m[3][2] * mat.m[2][2] + m[3][3] * mat.m[3][2];
        result.m[3][3] = m[3][0] * mat.m[0][3] + m[3][1] * mat.m[1][3] + m[3][2] * mat.m[2][3] + m[3][3] * mat.m[3][3];

        return result;
    }
    //除算
    Matrix4x4 Matrix4x4::operator/(float k) const {
        Matrix4x4 mat(m);
        float oneOverK = 1.0f / k;
        return mat * oneOverK;
    }
    //加算代入
    Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& mat) {
        *this = *this + mat;
        return *this;
    }
    //減算代入
    Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& mat) {
        *this = *this - mat;
        return *this;
    }
    //乗算代入
    Matrix4x4& Matrix4x4::operator*=(float k) {
        *this = *this * k;
        return *this;
    }
    //乗算代入
    Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& mat) {
        *this = *this * mat;
        return *this;
    }
    //減算代入
    Matrix4x4& Matrix4x4::operator/=(float k) {
        *this = *this / k;
        return *this;
    }
    //移動行列の作成
    Matrix4x4 Matrix4x4::createTranslate(const Vector3& v) {
        return Matrix4x4
        (
            1.0f, 0, 0, 0,
            0, 1.0f, 0, 0,
            0, 0, 1.0f, 0,
            v.x, v.y, v.z, 1.0f
        );
    }
    //移動行列の作成
    Matrix4x4 Matrix4x4::createTranslate(float x, float y, float z) {
        return Matrix4x4
        (
            1.0f, 0, 0, 0,
            0, 1.0f, 0, 0,
            0, 0, 1.0f, 0,
            x, y, z, 1
        );
    }
    //移動行列でセットアップ
    Matrix4x4& Matrix4x4::setupTransform(const Vector3& v) {
        *this = createTranslate(v);
        return *this;
    }
    //移動行列でセットアップ
    Matrix4x4& Matrix4x4::setupTransform(float x, float y, float z) {
        *this = createTranslate(x, y, z);
        return *this;
    }
    //X軸に回転する回転行列を作成
    Matrix4x4 Matrix4x4::createRotationX(float degree) {
        const float sin = MathUtil::sin(degree);
        const float cos = MathUtil::cos(degree);
        return Matrix4x4
        (
            1.0f, 0, 0, 0,
            0, cos, sin, 0,
            0, -sin, cos, 0,
            0, 0, 0, 1
        );
    }
    //X軸に回転する行列でセットアップ
    Matrix4x4& Matrix4x4::setupRotationX(float degree) {
        *this = createRotationX(degree);
        return *this;
    }
    //Y軸に回転する回転行列を作成
    Matrix4x4 Matrix4x4::createRotationY(float degree) {
        const float sin = MathUtil::sin(degree);
        const float cos = MathUtil::cos(degree);
        return Matrix4x4
        (
            cos, 0, -sin, 0,
            0, 1, 0, 0,
            sin, 0, cos, 0,
            0, 0, 0, 1
        );
    }
    //Y軸に回転する行列でセットアップ
    Matrix4x4& Matrix4x4::setupRotationY(float degree) {
        *this = createRotationY(degree);
        return *this;
    }
    //Z軸に回転する回転行列を作成
    Matrix4x4 Matrix4x4::createRotationZ(float degree) {
        const float sin = MathUtil::sin(degree);
        const float cos = MathUtil::cos(degree);
        return Matrix4x4
        (
            cos, sin, 0, 0,
            -sin, cos, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }
    //Z軸に回転する行列でセットアップ
    Matrix4x4& Matrix4x4::setupRotationZ(float degree) {
        *this = createRotationZ(degree);
        return *this;
    }
    //3軸に回転する回転行列を作成
    Matrix4x4 Matrix4x4::createRotation(const Vector3& r) {
        Matrix4x4 mx(createRotationX(r.x));
        Matrix4x4 my(createRotationY(r.y));
        Matrix4x4 mz(createRotationZ(r.z));
        return mx * my * mz;
    }
    //3軸に回転する行列でセットアップ
    Matrix4x4& Matrix4x4::setupRotation(const Vector3& r) {
        *this = createRotation(r);
        return *this;
    }

    //各軸に平行な等倍スケーリング行列を作成
    Matrix4x4 Matrix4x4::createScale(float s) {
        return Matrix4x4
        (
            s, 0, 0, 0,
            0, s, 0, 0,
            0, 0, s, 0,
            0, 0, 0, 1
        );
    }

    //スケーリング行列を作成
    Matrix4x4 Matrix4x4::createScale(const Vector3& s) {
        return createScale(s.x, s.y, s.z);
    }
    //各軸に平行な等倍スケーリング行列でセットアップ
    Matrix4x4& Matrix4x4::setupScale(float s) {
        *this = createScale(s);
        return *this;
    }
    //スケーリング行列でセットアップ
    Matrix4x4& Matrix4x4::setupScale(const Vector3& s) {
        *this = createScale(s);
        return *this;
    }
    //スケーリング行列を作成
    Matrix4x4 Matrix4x4::createScale(float sx, float sy, float sz) {
        return Matrix4x4
        (
            sx, 0, 0, 0,
            0, sy, 0, 0,
            0, 0, sz, 0,
            0, 0, 0, 1
        );
    }

    Matrix4x4 Matrix4x4::createView(const Vector3& eye, const Vector3& at, const Vector3& up) {
        const Vector3 zaxis = (at - eye).getNormal();
        const Vector3 xaxis = up.cross(zaxis).getNormal();
        const Vector3 yaxis = zaxis.cross(xaxis);
        return  Matrix4x4
        (
            xaxis.x, yaxis.x, zaxis.x, 0.0f,
            xaxis.y, yaxis.y, zaxis.y, 0.0f,
            xaxis.z, yaxis.z, zaxis.z, 0.0f,
            -xaxis.dot(eye), -yaxis.dot(eye), -zaxis.dot(eye), 1.0f
        );
    }

    //ビュー行列でセットアップ
    Matrix4x4& Matrix4x4::setUpView(const Vector3 & eye, const Vector3 & at, const Vector3 & up) {
        *this = createView(eye, at, up);
        return *this;
    }

    //プロジェクション行列を作成
    Matrix4x4 Matrix4x4::createProjection(float fovY, float aspect, float nearZ, float farZ) {
        const float yScale = MathUtil::cot(fovY / 2.0f);
        const float xScale = yScale / aspect;
        float subZ = farZ - nearZ;
        return Matrix4x4
        (
            xScale, 0, 0, 0,
            0, yScale, 0, 0,
            0, 0, farZ / subZ, 1.0f,
            0, 0, -nearZ * farZ / subZ, 0
        );
    }
    //プロジェクション行列でセットアップ
    Matrix4x4& Matrix4x4::setupProjection(float fovY, float aspect, float nearZ, float farZ) {
        *this = createProjection(fovY, aspect, nearZ, farZ);
        return *this;
    }

    //移動量を取得する
    Math::Vector3 Matrix4x4::getTranslate() const {
        return Math::Vector3(m[3][0], m[3][1], m[3][2]);
    }
    //平行投影行列を作成
    Matrix4x4 Matrix4x4::createOrthographic(const Vector2& screenSize) {
        return Matrix4x4
        (
            2 / screenSize.x, 0, 0, 0,
            0, -2 / screenSize.y, 0, 0,
            0, 0, 1.0f, 0,
            -1.0f, 1.0f, 0, 1.0f
        );
    }
    //平行投影行列でセットアップ
    Matrix4x4& Matrix4x4::setUpOrthographic(const Vector2& screenSize) {
        *this = createOrthographic(screenSize);
        return *this;
    }
    //転置行列
    Matrix4x4 Matrix4x4::transpose() const {
        return Matrix4x4(
            m[0][0], m[1][0], m[2][0], m[3][0],
            m[0][1], m[1][1], m[2][1], m[3][1],
            m[0][2], m[1][2], m[2][2], m[3][2],
            m[0][3], m[1][3], m[2][3], m[3][3]
        );
    }
    //行列式
    float Matrix4x4::determinant() const {
        float buf = 0.0f;
        Matrix4x4 m(*this);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (i < j) {
                    buf = m.m[j][i] / m.m[i][i];
                    for (int k = 0; k < 4; k++) {
                        m.m[j][k] -= m.m[i][k] * buf;
                    }
                }
            }
        }
        float det = 1.0f;
        for (int i = 0; i < 4; i++) {
            det *= m.m[i][i];
        }
        return det;
    }
    //逆行列
    Matrix4x4 Matrix4x4::inverse() const {
        Matrix4x4 res = Matrix4x4::IDENTITY;
        Matrix4x4 m(*this);
        float buf = 0.0;
        for (int i = 0; i < 4; i++) {
            buf = 1.0f / m.m[i][i];
            for (int j = 0; j < 4; j++) {
                m.m[i][j] *= buf;
                res.m[i][j] *= buf;
            }
            for (int j = 0; j < 4; j++) {
                if (i != j) {
                    buf = m.m[j][i];
                    for (int k = 0; k < 4; k++) {
                        m.m[j][k] -= m.m[i][k] * buf;
                        res.m[j][k] -= res.m[i][k] * buf;
                    }
                }
            }
        }
        return res;
    }
    //行列の補間
    Matrix4x4 Matrix4x4::lerp(const Matrix4x4& mat1, const Matrix4x4& mat2, float t) {
        return mat1 * (1.0f - t) + mat2 * t;
    }

    //主に法線ベクトル用となるベクトルを回転させる座標変換
    Vector3 Matrix4x4::transformNormal(const Vector3& v) {
        float x = v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0];
        float y = v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1];
        float z = v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2];
        return Vector3(x, y, z);
    }

    //ベクトルとの積をさらにw除算した値を返す
    Vector3 Matrix4x4::multiplyCoord(const Math::Vector3& v, const Math::Matrix4x4& m) {
        Math::Matrix4x4 mat = createTranslate(v) * m;
        Math::Vector3 res(mat.m[3][0] / mat.m[3][3], mat.m[3][1] / mat.m[3][3], mat.m[3][2] / mat.m[3][3]);
        return res;
    }
    //添え字演算子
    //行列の各要素に直接アクセスするため
    std::array<float, 4>& Matrix4x4::operator[](int n) {
        return m[n];
    }
    //ベクトルとの乗算
    Vector3 operator*(const Vector3& v, const Matrix4x4& mat) {
        float x = v.x * mat.m[0][0] + v.y * mat.m[1][0] + v.z * mat.m[2][0] + mat.m[3][0];
        float y = v.x * mat.m[0][1] + v.y * mat.m[1][1] + v.z * mat.m[2][1] + mat.m[3][1];
        float z = v.x * mat.m[0][2] + v.y * mat.m[1][2] + v.z * mat.m[2][2] + mat.m[3][2];
        return Vector3(x, y, z);
    }
    //ベクトルとの乗算代入
    Vector3& operator*=(Vector3& v, const Matrix4x4& mat) {
        v = v * mat;
        return v;
    }
    //出力
    std::ostream& operator<<(std::ostream& os, const Matrix4x4& mat) {
        os << mat.m[0][0] << " " << mat.m[0][1] << " " << mat.m[0][2] << " " << mat.m[0][3] << "\n"
            << mat.m[1][0] << " " << mat.m[1][1] << " " << mat.m[1][2] << " " << mat.m[1][3] << "\n"
            << mat.m[2][0] << " " << mat.m[2][1] << " " << mat.m[2][2] << " " << mat.m[2][3] << "\n"
            << mat.m[3][0] << " " << mat.m[3][1] << " " << mat.m[3][2] << " " << mat.m[3][3] << "\n";
        return os;
    }
} //Framework::Math