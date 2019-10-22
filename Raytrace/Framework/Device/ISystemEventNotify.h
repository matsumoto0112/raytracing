#pragma once
#include <d3d12.h>

namespace Framework::Device {
    /**
    * @brief �V�X�e���I�ȃC�x���g�ʒm�ΏۃI�u�W�F�N�g
    */
    interface ISystemEventNotify {
        /**
        * @brief ����������
        */
        virtual void onInit() = 0;
        /**
        * @brief �X�V
        */
        virtual void onUpdate() = 0;
        /**
        * @brief �`��
        */
        virtual void onRender() = 0;
        /**
        * @brief �I��������
        */
        virtual void onDestroy() = 0;

        virtual void pushAltEnter() = 0;
    };
} //Framework::Device