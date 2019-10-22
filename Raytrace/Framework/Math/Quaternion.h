#pragma once
#include <iostream>

namespace Framework {
    namespace Math {

        class Vector3;
        class Matrix4x4;

        /**
        * @class Quaternion
        * @brief �N�H�[�^�j�I��
        */
        class Quaternion {
        public:
            float x, y, z, w;
        public:
            static const Quaternion IDENTITY; //!< �P�ʎl����
        public:
            /**
            * @brief �f�t�H���g�R���X�g���N�^
            */
            Quaternion();
            /**
            * v�e�v�f�ŏ�����
            */
            Quaternion(float x, float y, float z, float w);
            /**
            * @brief �P���l������
            */
            void identity();
            /**
            * @brief  x���ɉ�]�����p�x�ŃZ�b�g�A�b�v
            * @param theta ��]�ʁi�x�j
            */
            void setToRotateAboutX(float degree);
            /**
            * @brief  y���ɉ�]�����p�x�ŃZ�b�g�A�b�v
            * @param theta ��]�ʁi�x�j
            */
            void setToRotateAboutY(float degree);
            /**
            * @brief  z���ɉ�]�����p�x�ŃZ�b�g�A�b�v
            * @param theta ��]�ʁi�x�j
            */
            void setToRotateAboutZ(float degree);
            /**
            * @brief  Axis�����ɉ�]�����p�x�ŃZ�b�g�A�b�v
            * @param theta ��
            * @param theta ��]�ʁi�x�j
            */
            void setToRotateAboutAxis(const Vector3& axis, float degree);
            /**
            * @brief X�����̉�]���쐬
            * @param degree ��]�ʁi�x�j
            */
            static Quaternion createRotateAboutX(float degree);
            /**
            * @brief Y�����̉�]���쐬
            * @param degree ��]�ʁi�x�j
            */
            static Quaternion createRotateAboutY(float degree);
            /**
            * @brief Z�����̉�]���쐬
            * @param degree ��]�ʁi�x�j
            */
            static Quaternion createRotateAboutZ(float degree);
            /**
            * @brief  Axis�����ɉ�]������]���쐬
            * @param theta ��
            * @param theta ��]��(rad)
            */
            static Quaternion createRotateAboutAxis(const Vector3& axis, float degree);

            /**
            * @brief �O��
            * @detail �O�ς̌v�Z���@�𐳎��Ƃ͈Ⴄ�`�Ŏ���
            ���̗��R�ɂ��Ă͎���Ŋw�ԃQ�[���R�c���w���Q��
            */
            Quaternion operator*(const Quaternion& a) const;

            /**
            * @brief �s��ɕϊ�
            */
            Matrix4x4 toMatrix() const;

            /**
            * @brief ����
            */
            float dot(const Quaternion& a) const;
            /**
            * @brief ���K������
            */
            void normalize();
            /**
            * @brief �I�C���[�p���擾
            */
            Vector3 toEular() const;
            /**
            * @brief �I�C���[�p����N�H�[�^�j�I���𐶐�
            */
            static Quaternion fromEular(const Math::Vector3& eular);
            /**
            * @brief �p�x���擾����
            */
            float getRotateAngle() const;
            /**
            * @brief ��]�����擾����
            */
            Vector3 getRotateAxis() const;
            /**
            * @brief �����̎l���������߂�
            */
            static Math::Quaternion conjugate(const Math::Quaternion& q);

            /**
            * @brief �o�͉��Z�q
            */
            friend std::ostream& operator<<(std::ostream& os, const Quaternion& q);
            /**
            * @brief �^�[�Q�b�g�������l�������쐬����
            */
            static Math::Quaternion createLookTarget(const Math::Vector3& v, const Math::Vector3& normal);
            /**
            * @brief �x�N�g���Ƃ̊|���Z
            */
            Math::Vector3 multiply(const Math::Vector3& v);
        };
    } // Math
} //Framework 