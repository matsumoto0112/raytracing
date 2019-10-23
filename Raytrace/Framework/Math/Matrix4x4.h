#pragma once

#include <array>
#include <ostream>
#include "Vector2.h"
#include "Vector3.h"

namespace Framework::Math {
    /**
    * @class Matrix4x4
    * @brief 4*4�s��
    */
    class Matrix4x4 {
    public:
        std::array<std::array<float, 4>, 4> m; //!< 4x4�s��
    public:
        static const Matrix4x4 IDENTITY; //!< �P�ʍs��
        static const Matrix4x4 ZERO; //!< �[���s��
    public:
        /**
        * @brief �R���X�g���N�^
        */
        Matrix4x4();
        /**
        * @brief �R���X�g���N�^
        */
        Matrix4x4(float m11, float m12, float m13, float m14,
            float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34,
            float m41, float m42, float m43, float m44);
        /**
        * @brief �R���X�g���N�^
        */
        Matrix4x4(const std::array<std::array<float, 4>, 4>& m);
        /**
        * @brief ������Z�q
        */
        Matrix4x4& operator=(const Matrix4x4& mat);
        /**
        * @brief ���l���Z�q
        */
        bool operator==(const Matrix4x4& mat) const;
        /**
        * @brief ���l���Z�q
        */
        bool operator!=(const Matrix4x4& mat) const;
        /**
        * @brief �P���v���X���Z�q
        */
        Matrix4x4 operator+();
        /**
        * @brief �P���}�C�i�X���Z�q
        */
        Matrix4x4 operator-();
        /**
        * @brief ���Z
        */
        Matrix4x4 operator+(const Matrix4x4& mat) const;
        /**
        * @brief ���Z
        */
        Matrix4x4 operator-(const Matrix4x4& mat) const;
        /**
        * @brief �X�J���[�{
        */
        Matrix4x4 operator*(float k) const;
        /**
        * @brief ��Z
        */
        Matrix4x4 operator*(const Matrix4x4& mat) const;
        /**
        * @brief �x�N�g���~�s��
        */
        friend Vector3 operator*(const Vector3& v, const Matrix4x4& mat);
        /**
        * @brief �X�J���[���Z
        */
        Matrix4x4 operator/(float k) const;
        /**
        * @brief ���Z������Z�q
        */
        Matrix4x4& operator+=(const Matrix4x4& mat);
        /**
        * @brief ���Z������Z�q
        */
        Matrix4x4& operator-=(const Matrix4x4& mat);
        /**
        * @brief ��Z������Z�q
        */
        Matrix4x4& operator*=(float k);
        /**
        * @brief ��Z������Z�q
        */
        Matrix4x4& operator*=(const Matrix4x4& mat);
        /**
        * @brief �x�N�g���~�s��
        */
        friend Vector3& operator*=(Vector3& v, const Matrix4x4& mat);
        /**
        * @brief ���Z������Z�q
        */
        Matrix4x4& operator/=(float k);

        /**
        * @brief ���s�ړ��s��̍쐬
        * @param v �ړ���
        */
        static Matrix4x4 createTranslate(const Vector3& v);
        /**
        * @brief ���s�ړ��s��̍쐬
        * @param _x �ړ���X
        * @param _y �ړ���Y
        * @param z �ړ���Z
        */
        static Matrix4x4 createTranslate(float x, float y, float z);
        /**
        * @brief ���s�ړ��s��̃Z�b�g�A�b�v
        * @param v �ړ���
        */
        Matrix4x4& setupTransform(const Vector3& v);
        /**
        * @brief ���s�ړ��s��̃Z�b�g�A�b�v
        * @param _x �ړ���X
        * @param _y �ړ���Y
        * @param z �ړ���Z
        */
        Matrix4x4& setupTransform(float x, float y, float z);
        /**
        * @brief X����]�s��̍쐬
        * @param degree ��]��(�x)
        */
        static Matrix4x4 createRotationX(float degree);
        /**
        * @brief X����]�s��̃Z�b�g�A�b�v
        * @param degree ��]��(�x)
        */
        Matrix4x4& setupRotationX(float degree);
        /**
        * @brief Y����]�s��̍쐬
        * @param degree ��]��(�x)
        */
        static Matrix4x4 createRotationY(float degree);
        /**
        * @brief Y����]�s��̃Z�b�g�A�b�v
        * @param degree ��]��(�x)
        */
        Matrix4x4& setupRotationY(float degree);
        /**
        * @brief Z����]�s��̍쐬
        * @param degree ��]��(�x)
        */
        static Matrix4x4 createRotationZ(float degree);
        /**
        * @brief Z����]�s��̃Z�b�g�A�b�v
        * @param degree ��]��(�x)
        */
        Matrix4x4& setupRotationZ(float degree);
        /**
        * @brief ��]�s��̍쐬
        * @param r �e���̉�]��(�x)
        */
        static Matrix4x4 createRotation(const Vector3& r);
        /**
        * @brief ��]�s��̃Z�b�g�A�b�v
        * @param r �e���̉�]��(�x)
        */
        Matrix4x4& setupRotation(const Vector3& r);
        /**
        * @brief �g��E�k���s��̍쐬
        * @param s �g��E�k���̑傫��(�e������)
        */
        static Matrix4x4 createScale(float s);
        /**
        * @brief �g��E�k���s��̍쐬
        * @param s �e���̊g��E�k���̑傫��
        */
        static Matrix4x4 createScale(const Vector3& s);
        /**
        * @brief �g��E�k���s��̃Z�b�g�A�b�v
        * @param s �g��E�k���̑傫��(�e������)
        */
        Matrix4x4& setupScale(float s);
        /**
        * @brief �g��E�k���s��̃Z�b�g�A�b�v
        * @param s �e���̊g��E�k���̑傫��
        */
        Matrix4x4& setupScale(const Vector3& s);
        /**
        * @brief �g��E�k���s��̍쐬
        * @param sx x���̊g��E�k���̑傫��
        * @param sy y���̊g��E�k���̑傫��
        * @param sz z���̊g��E�k���̑傫��
        */
        static Matrix4x4 createScale(float sx, float sy, float sz);
        /**
        * @brief �r���[�s��̍쐬
        * @param eye ���_
        * @param at �����_
        * @param up ������̃x�N�g��
        */
        static Matrix4x4 createView(const Vector3& eye, const Vector3& at, const Vector3& up);

        /**
        * @brief �r���[�s��̃Z�b�g�A�b�v
        * @param eye ���_
        * @param at �����_
        * @param up ������̃x�N�g��
        */
        Matrix4x4& setUpView(const Vector3& eye, const Vector3& at, const Vector3& up);

        /**
        * @brief �v���W�F�N�V�����s��̍쐬
        * @param fovY ����p
        * @param aspect �A�X�y�N�g��
        * @param nearZ �ŋߓ_
        * @param farZ �ŉ��_
        */
        static Matrix4x4 createProjection(float fovY, float aspect, float nearZ, float farZ);

        /**
        * @brief �v���W�F�N�V�����s��̃Z�b�g�A�b�v
        * @param fovY ����p
        * @param aspect �A�X�y�N�g��
        * @param nearZ �ŋߓ_
        * @param farZ �ŉ��_
        */
        Matrix4x4& setupProjection(float fovY, float aspect, float nearZ, float farZ);
        /**
        * @brief ���s�ړ������̎擾
        */
        Math::Vector3 getTranslate() const;
        /**
        * @brief �����e�s��̍쐬
        * @param screenSize ��ʂ̑傫��
        */
        static Matrix4x4 createOrthographic(const Vector2& screenSize);
        /**
        * @brief �����e�s��̃Z�b�g�A�b�v
        * @param screenSize ��ʂ̑傫��
        */
        Matrix4x4& setUpOrthographic(const Vector2& screenSize);
        /**
        * @brief �]�u�s����擾����
        */
        Matrix4x4 transpose()const;
        /**
        * @brief �s�񎮂����߂�
        */
        float determinant() const;
        /**
        * @brief �t�s������߂�
        * @param mat ���߂�s��
        */
        Matrix4x4 inverse() const;
        /**
        * @brief �s��̕��
        * @param mat1 �s��1
        * @param mat2 �s��2
        * @param t ��ԌW��
        */
        static Matrix4x4 lerp(const Matrix4x4& mat1, const Matrix4x4& mat2, float t);
        /**
        * @brief �x�N�g���Ƃ̊|���Z(�@���x�N�g���p�ŉ�]�̂�)
        */
        Vector3 transformNormal(const Vector3& v);

        /**
        * @brief �x�N�g���ƍs��̐ς����߁Aw�ŏ��Z���ꂽ�l��Ԃ�
        */
        static Vector3 multiplyCoord(const Math::Vector3& v, const Math::Matrix4x4& m);
        /**
        * @brief �o�͉��Z�q
        */
        friend std::ostream& operator<<(std::ostream& os, const Matrix4x4& mat);
        /**
        * @brief �Y�������Z�q
        */
        std::array<float, 4>& operator[](int n);
    };
} //Framework::Math