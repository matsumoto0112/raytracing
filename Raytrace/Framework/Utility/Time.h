#pragma once
#include <Windows.h>
#include <list>

namespace Framework::Utility {
    /**
    * @class Time
    * @brief ���ԊǗ��N���X
    */
    class Time {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param sample �T���v�����O��
        * @details sample���傫���قǕ��ς�FPS�����肵�₷��������������ʂ������Ȃ�
        */
        Time(UINT sample = 10);
        /**
        * @brief �f�X�g���N�^
        */
        ~Time();
        /**
        * @brief �X�V
        */
        void update();
        /**
        * @brief FPS���擾����
        */
        double getFPS() const { return mFPS; }
        /**
        * @brief �O�t���[������̍������ԕb���擾����
        */
        double getDeltaTime() const { return mDiffTime; }
        /**
        * @brief �T���v������ݒ肷��
        */
        void setSampleCount(UINT sample = 10);
    private:
        /**
        * @brief �������Ԃ��擾����
        */
        double getCurrentDefTime();
    private:
        LARGE_INTEGER mCounter; //!< �p�t�H�[�}���X�J�E���^�[
        LONGLONG mPrevCount; //!< �O�t���[���ł̃J�E���g
        double mFreq; //!< 1�b������̃e�B�b�N�J�E���g
        std::list<double> mDifTimes; //!< �������ԃ��X�g
        double mSumTimes; //!< ���v��������
        UINT mSampleCount; //!< �T���v�����O�� 
        double mFPS; //!< ���݂�FPS
        double mDiffTime; //!< �O�t���[������̍�������(�b)
    };
} //Framework::Utility 