#pragma once
#include <array>
#include <d3d12.h>
#include "Utility/Typedef.h"

namespace Framework::Utility {
    /**
    * @class GPUTimer
    * @brief GPU�̎��Ԍv��
    */
    class GPUTimer {
    public:
        /**
        * @brief �R���X�g���N�^
        */
        GPUTimer();
        /**
        * @brief �R���X�g���N�^
        */
        GPUTimer(ID3D12Device* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount);
        /**
        * @brief �f�X�g���N�^
        */
        ~GPUTimer();
        /**
        * @brief �t���[���J�n
        */
        void beginFrame();
        /**
        * @brief �t���[���I��
        */
        void endFrame(ID3D12GraphicsCommandList* commandList);
        /**
        * @brief �v���J�n
        */
        void start(ID3D12GraphicsCommandList* commandList, UINT timerID = 0);
        /**
        * @brief �v���I��
        */
        void stop(ID3D12GraphicsCommandList* commandList, UINT timerID = 0);
        /**
        * @brief ���ώ��Ԃ̃��Z�b�g
        */
        void reset();
        /**
        * @brief �o�ߎ��Ԃ��擾����
        */
        float getElapsedTime(UINT timerID = 0);
        /**
        * @brief ���ώ��Ԃ��擾����
        */
        float getAverageTime(UINT timerID = 0);
        /**
        * @brief �f�o�C�X���������
        */
        void releaseDevice();
        /**
        * @brief �f�o�C�X��o�^����
        */
        void storeDevice(ID3D12Device* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount);
    private:
        static constexpr UINT TIMER_COUNT = 8; //!< �^�C�}�[�̑��g�p�\��
        static constexpr UINT TIMER_SLOT_NUM = 2 * TIMER_COUNT; //!< �^�C�}�[�̎��Ԃ������Ă����� �J�n�ƏI���Ń^�C�}�[�g�p���̓�{�K�v
        ComPtr<ID3D12QueryHeap> mQueryHeap; //!< �f�o�C�X�ւ̃N�G���p�q�[�v
        ComPtr<ID3D12Resource> mBuffer; //!< �v���������Ԃ��擾���邽�߂̃o�b�t�@
        float mGpuFreqInv; //!< GPU�̃^�C���X�^���v�J�E���^�[�̎���
        std::array<float, TIMER_COUNT> mAverages; //!< �o�ߎ��Ԃ̕���
        std::array<UINT64, TIMER_SLOT_NUM> mTimings; //!< �o�ߎ��Ԃ��擾���邽�߂̔z��
        UINT mMaxFrameCount; //!<�o�b�N�o�b�t�@�̖���
    };
} //Framework::Utility 