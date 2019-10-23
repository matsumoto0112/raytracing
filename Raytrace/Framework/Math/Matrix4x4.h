#pragma once

#include <array>
#include <ostream>
#include "Vector2.h"
#include "Vector3.h"

namespace Framework::Math {
    /**
    * @class Matrix4x4
    * @brief 4*4行列
    */
    class Matrix4x4 {
    public:
        std::array<std::array<float, 4>, 4> m; //!< 4x4行列
    public:
        static const Matrix4x4 IDENTITY; //!< 単位行列
        static const Matrix4x4 ZERO; //!< ゼロ行列
    public:
        /**
        * @brief コンストラクタ
        */
        Matrix4x4();
        /**
        * @brief コンストラクタ
        */
        Matrix4x4(float m11, float m12, float m13, float m14,
            float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34,
            float m41, float m42, float m43, float m44);
        /**
        * @brief コンストラクタ
        */
        Matrix4x4(const std::array<std::array<float, 4>, 4>& m);
        /**
        * @brief 代入演算子
        */
        Matrix4x4& operator=(const Matrix4x4& mat);
        /**
        * @brief 等値演算子
        */
        bool operator==(const Matrix4x4& mat) const;
        /**
        * @brief 等値演算子
        */
        bool operator!=(const Matrix4x4& mat) const;
        /**
        * @brief 単項プラス演算子
        */
        Matrix4x4 operator+();
        /**
        * @brief 単項マイナス演算子
        */
        Matrix4x4 operator-();
        /**
        * @brief 加算
        */
        Matrix4x4 operator+(const Matrix4x4& mat) const;
        /**
        * @brief 減算
        */
        Matrix4x4 operator-(const Matrix4x4& mat) const;
        /**
        * @brief スカラー倍
        */
        Matrix4x4 operator*(float k) const;
        /**
        * @brief 乗算
        */
        Matrix4x4 operator*(const Matrix4x4& mat) const;
        /**
        * @brief ベクトル×行列
        */
        friend Vector3 operator*(const Vector3& v, const Matrix4x4& mat);
        /**
        * @brief スカラー除算
        */
        Matrix4x4 operator/(float k) const;
        /**
        * @brief 加算代入演算子
        */
        Matrix4x4& operator+=(const Matrix4x4& mat);
        /**
        * @brief 減算代入演算子
        */
        Matrix4x4& operator-=(const Matrix4x4& mat);
        /**
        * @brief 乗算代入演算子
        */
        Matrix4x4& operator*=(float k);
        /**
        * @brief 乗算代入演算子
        */
        Matrix4x4& operator*=(const Matrix4x4& mat);
        /**
        * @brief ベクトル×行列
        */
        friend Vector3& operator*=(Vector3& v, const Matrix4x4& mat);
        /**
        * @brief 除算代入演算子
        */
        Matrix4x4& operator/=(float k);

        /**
        * @brief 平行移動行列の作成
        * @param v 移動量
        */
        static Matrix4x4 createTranslate(const Vector3& v);
        /**
        * @brief 平行移動行列の作成
        * @param _x 移動量X
        * @param _y 移動量Y
        * @param z 移動量Z
        */
        static Matrix4x4 createTranslate(float x, float y, float z);
        /**
        * @brief 平行移動行列のセットアップ
        * @param v 移動量
        */
        Matrix4x4& setupTransform(const Vector3& v);
        /**
        * @brief 平行移動行列のセットアップ
        * @param _x 移動量X
        * @param _y 移動量Y
        * @param z 移動量Z
        */
        Matrix4x4& setupTransform(float x, float y, float z);
        /**
        * @brief X軸回転行列の作成
        * @param degree 回転量(度)
        */
        static Matrix4x4 createRotationX(float degree);
        /**
        * @brief X軸回転行列のセットアップ
        * @param degree 回転量(度)
        */
        Matrix4x4& setupRotationX(float degree);
        /**
        * @brief Y軸回転行列の作成
        * @param degree 回転量(度)
        */
        static Matrix4x4 createRotationY(float degree);
        /**
        * @brief Y軸回転行列のセットアップ
        * @param degree 回転量(度)
        */
        Matrix4x4& setupRotationY(float degree);
        /**
        * @brief Z軸回転行列の作成
        * @param degree 回転量(度)
        */
        static Matrix4x4 createRotationZ(float degree);
        /**
        * @brief Z軸回転行列のセットアップ
        * @param degree 回転量(度)
        */
        Matrix4x4& setupRotationZ(float degree);
        /**
        * @brief 回転行列の作成
        * @param r 各軸の回転量(度)
        */
        static Matrix4x4 createRotation(const Vector3& r);
        /**
        * @brief 回転行列のセットアップ
        * @param r 各軸の回転量(度)
        */
        Matrix4x4& setupRotation(const Vector3& r);
        /**
        * @brief 拡大・縮小行列の作成
        * @param s 拡大・縮小の大きさ(各軸共通)
        */
        static Matrix4x4 createScale(float s);
        /**
        * @brief 拡大・縮小行列の作成
        * @param s 各軸の拡大・縮小の大きさ
        */
        static Matrix4x4 createScale(const Vector3& s);
        /**
        * @brief 拡大・縮小行列のセットアップ
        * @param s 拡大・縮小の大きさ(各軸共通)
        */
        Matrix4x4& setupScale(float s);
        /**
        * @brief 拡大・縮小行列のセットアップ
        * @param s 各軸の拡大・縮小の大きさ
        */
        Matrix4x4& setupScale(const Vector3& s);
        /**
        * @brief 拡大・縮小行列の作成
        * @param sx x軸の拡大・縮小の大きさ
        * @param sy y軸の拡大・縮小の大きさ
        * @param sz z軸の拡大・縮小の大きさ
        */
        static Matrix4x4 createScale(float sx, float sy, float sz);
        /**
        * @brief ビュー行列の作成
        * @param eye 視点
        * @param at 注視点
        * @param up 上方向のベクトル
        */
        static Matrix4x4 createView(const Vector3& eye, const Vector3& at, const Vector3& up);

        /**
        * @brief ビュー行列のセットアップ
        * @param eye 視点
        * @param at 注視点
        * @param up 上方向のベクトル
        */
        Matrix4x4& setUpView(const Vector3& eye, const Vector3& at, const Vector3& up);

        /**
        * @brief プロジェクション行列の作成
        * @param fovY 視野角
        * @param aspect アスペクト比
        * @param nearZ 最近点
        * @param farZ 最遠点
        */
        static Matrix4x4 createProjection(float fovY, float aspect, float nearZ, float farZ);

        /**
        * @brief プロジェクション行列のセットアップ
        * @param fovY 視野角
        * @param aspect アスペクト比
        * @param nearZ 最近点
        * @param farZ 最遠点
        */
        Matrix4x4& setupProjection(float fovY, float aspect, float nearZ, float farZ);
        /**
        * @brief 平行移動成分の取得
        */
        Math::Vector3 getTranslate() const;
        /**
        * @brief 正投影行列の作成
        * @param screenSize 画面の大きさ
        */
        static Matrix4x4 createOrthographic(const Vector2& screenSize);
        /**
        * @brief 正投影行列のセットアップ
        * @param screenSize 画面の大きさ
        */
        Matrix4x4& setUpOrthographic(const Vector2& screenSize);
        /**
        * @brief 転置行列を取得する
        */
        Matrix4x4 transpose()const;
        /**
        * @brief 行列式を求める
        */
        float determinant() const;
        /**
        * @brief 逆行列を求める
        * @param mat 求める行列
        */
        Matrix4x4 inverse() const;
        /**
        * @brief 行列の補間
        * @param mat1 行列1
        * @param mat2 行列2
        * @param t 補間係数
        */
        static Matrix4x4 lerp(const Matrix4x4& mat1, const Matrix4x4& mat2, float t);
        /**
        * @brief ベクトルとの掛け算(法線ベクトル用で回転のみ)
        */
        Vector3 transformNormal(const Vector3& v);

        /**
        * @brief ベクトルと行列の積を求め、wで除算された値を返す
        */
        static Vector3 multiplyCoord(const Math::Vector3& v, const Math::Matrix4x4& m);
        /**
        * @brief 出力演算子
        */
        friend std::ostream& operator<<(std::ostream& os, const Matrix4x4& mat);
        /**
        * @brief 添え字演算子
        */
        std::array<float, 4>& operator[](int n);
    };
} //Framework::Math