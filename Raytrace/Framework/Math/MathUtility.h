#pragma once
#undef max
#undef min
#include <initializer_list>
#include <algorithm>
#include <cmath>

namespace Framework::Math {
    /*! �~������*/
    static constexpr float PI = 3.141592654f;
    /*! 2��*/
    static constexpr float PI2 = PI * 2;

    /*! �v�Z���e�덷*/
    static constexpr float EPSILON = 0.001f;

    /**
    * @class MathUtil
    * @brief ���w�֌W�̃��[�e�B���e�B�N���X
    */
    class MathUtil {
    public:
        /**
        * @brief �N�����v����
        * @tparam t �N�����v����l
        * @tparam min �����l
        * @tparam max ����l
        * @return �N�����v���ꂽ�l
        */
        template<class T>
        static T& clamp(T& t, const T& minValue, const T& maxValue);

        /**
        * @brief �T�C��
        */
        static float sin(float degree) { return std::sinf(toRadian(degree)); }

        /**
        * @brief �R�T�C��
        */
        static float cos(float degree) { return std::cosf(toRadian(degree)); }

        /**
        * @brief �^���W�F���g
        */
        static float tan(float degree) { return std::tanf(toRadian(degree)); }

        /**
        * @brief �A�[�N�^���W�F���g
        * @return �p�x(deg)��Ԃ�
        */
        static float atan2(float y, float x) { return toDegree(std::atan2f(y, x)); }
        /**
        * @brief �A�[�N�T�C��
        * @return �p�x(deg)��Ԃ�
        */
        static float asin(float x) { return toDegree(std::asin(x)); }
        /**
        * @brief �A�[�N�R�T�C��
        * @param �p�x(deg)��Ԃ�
        */
        static float acos(float x) { return toDegree(std::acos(x)); }

        /**
        * @brief ���[�g
        */
        static float sqrt(float a) { return std::sqrtf(a); }

        /**
        * @brief �R�^���W�F���g(cot)
        */
        static float cot(float degree) { return 1.0f / std::tanf(toRadian(degree)); }

        /**
        * @brief �x���烉�W�A���֕ϊ�����
        * @param deg �x
        * @return �ϊ����ꂽ���W�A��
        */
        static float toRadian(float deg) { return deg / 180.0f * PI; }

        /**
        * @brief ���W�A������x�֕ϊ�����
        * @param rad ���W�A��
        * @return �ϊ����ꂽ�x
        */
        static float toDegree(float rad) { return rad / PI * 180.0f; }

        /**
        * @brief ��Βl
        */
        static float abs(float X) { return std::fabsf(X); }

        /**
        * @brief ���
        * @tparam a �J�n�l
        * @tparam b �I���l
        * @tparam t ��Ԓl�i�O�`�P�j
        */
        template <class T>
        static T lerp(const T& a, const T& b, float t);

        /**
        * @brief �ő�l�̎擾
        */
        template <class T>
        static T mymax(const std::initializer_list<T>& param);
        /**
        * @brief �ő�l�̎擾
        */
        template <class T>
        static T mymax(const T& t1, const T& t2);

        /**
        * @brief �ŏ��l�̎擾
        */
        template <class T>
        static T mymin(const std::initializer_list<T>& param);
        /**
        * @brief �ŏ��l�̎擾
        */
        template <class T>
        static T mymin(const T& t1, const T& t2);
    };

    //�N�����v����
    template<class T>
    T& MathUtil::clamp(T& t, const T& minValue, const T& maxValue) {
        if (t < minValue)t = minValue;
        else if (t > maxValue) t = maxValue;
        return t;
    }

    //���
    template<class T>
    inline T MathUtil::lerp(const T& a, const T& b, float t) {
        //0�`1�ɃN�����v
        t = clamp(t, 0.0f, 1.0f);
        return a * (1.0f - t) + b * t;
    }

    //�ő�l�̎擾
    template<class T>
    inline T MathUtil::mymax(const std::initializer_list<T>& param) {
        return std::max(param);
    }

    //�ő�l�̎擾
    template<class T>
    inline T MathUtil::mymax(const T& t1, const T& t2) {
        return std::max(t1, t2);
    }

    //�ŏ��l�̎擾
    template<class T>
    inline T MathUtil::mymin(const std::initializer_list<T>& param) {
        return std::min(param);
    }
    //�ŏ��l�̎擾
    template<class T>
    inline T MathUtil::mymin(const T& t1, const T& t2) {
        return std::min(t1, t2);
    }
} //Framework::Math
