#pragma once
#include <d3d12.h>
#include "Framework/Utility/StringBuilder.h"

/**
* @def VisualStudio�̏o�͂Ƀ��O�o��
* @param mes �\�����郁�b�Z�[�W
*/
#define MY_DEBUG_LOG(mes) Framework::Utility::Debug::debugLog(__FUNCTION__, __LINE__, mes)

/**
* @def �A�T�[�V����
* @param cond ������
* @param mes cond��false�̎��ɕ\�����郁�b�Z�[�W
*/
#define MY_ASSERTION(cond,mes) Framework::Utility::Debug::assertion(__FUNCTION__, __LINE__, cond, mes)

/**
* @def �G���[�E�B���h�E�̕\��
* @param cond ������
* @param mes cond��false�̎��ɕ\�����郁�b�Z�[�W
* @details �f�o�b�O���ɂ̓��b�Z�[�W�ƂƂ��ɔ��������֐����A�s�ԍ����\�������
*/
#define MY_ERROR_WINDOW(cond,mes) Framework::Utility::Debug::errorWindow(__FUNCTION__, __LINE__, cond, mes)

/**
* @def �����𖞂����Ă�����VisualStudio�̏o�͂Ƀ��O�o��
* @param cond ������
* @param mes �\�����郁�b�Z�[�W
*/
#define MY_DEBUG_LOG_IF(cond,mes) if((cond)) MY_DEBUG_LOG(mes)

#define MY_THROW_IF_FAILED(hr,mes) Framework::Utility::throwIfFailed(hr,mes)

#define MY_THROW_IF_FALSE(cond,mes) Framework::Utility::throwIfFalse(cond,mes)

namespace Framework::Utility {
    /**
    * @brief �f�o�b�O�p�C���^�[�t�F�[�X
    */
    class Debug {
    public:
        /**
        * @brief �A�T�[�V����
        * @param funcName �֐���
        * @param line ���������s
        * @param condition ����
        * @param message �G���[���b�Z�[�W
        */
        static void assertion(const std::string& funcName, int line,
            bool condition, const std::wstring& message);
        /**
        * @brief ���O�o��
        * @param funcName �֐���
        * @param line ���������s
        * @param message �o�͓��e
        */
        static void debugLog(const std::string& funcName, int line, const std::wstring& message);
        /**
        * @brief �G���[�E�B���h�E�̕\��
        * @param funcName �֐���
        * @param line ���������s
        * @param condition ������
        * @param message �G���[���b�Z�[�W
        */
        static void errorWindow(const std::string& funcName, int line,
            bool condition, const std::wstring& message);
        /**
        * @brief �G���[���b�Z�[�W�̍쐬
        * @param funcName �֐���
        * @param line ���������s
        * @param message ���ۂ̃G���[���b�Z�[�W
        * @return [funcName line:message]�̌`���̃��b�Z�[�W��Ԃ�
        */
        static std::wstring errorMessage(const std::string& funcName, int line,
            const std::wstring& message);
    };

    /**
    * @brief HRESULT��O�N���X
    */
    class HrException : public std::runtime_error {
        inline std::string HrToString(HRESULT hr) {
            char s_str[64] = {};
            sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
            return std::string(s_str);
        }
    public:
        HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) { }
        HRESULT Error() const { return m_hr; }
    private:
        const HRESULT m_hr;
    };

    /**
    * @brief ���s���Ă������O�𓊂���
    */
    inline void throwIfFailed(HRESULT hr) {
        if (FAILED(hr)) {
            throw HrException(hr);
        }
    }

    /**
    * @brief ���s���Ă������O�𓊂���
    */
    inline void throwIfFailed(HRESULT hr, const wchar_t* mes) {
        if (FAILED(hr)) {
            MY_DEBUG_LOG(mes);
            throw HrException(hr);
        }
    }

    /**
    * @brief ���s���Ă������O�𓊂���
    */
    inline void throwIfFalse(bool value) {
        throwIfFailed(value ? S_OK : E_FAIL);
    }

    /**
    * @brief ���s���Ă������O�𓊂���
    */
    inline void throwIfFalse(bool value, const wchar_t* mes) {
        throwIfFailed(value ? S_OK : E_FAIL, mes);
    }

} //Framework::Utility