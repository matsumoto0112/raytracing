#pragma once
#include <string>
#include "Framework/Device/ISystemEventNotify.h"
#include "Framework/DX/DeviceResource.h"
#include "Framework/Window/Window.h"

namespace Framework {
    /**
    * @class Game
    * @brief discription
    */
    class Game : public Device::ISystemEventNotify, DX::IDeviceNotify {
    public:
        /**
        * @brief �R���X�g���N�^
        * @param width ��
        * @param height ����
        * @param title �^�C�g��
        */
        Game(UINT width, UINT height, const std::wstring& title);
        /**
        * @brief �f�X�g���N�^
        */
        ~Game();
        /**
        * @brief ���s����
        * @param hInstance �C���X�^���X�n���h��
        * @param nCmdShow �E�B���h�E�̕\�����@
        */
        int run(HINSTANCE hInstance, int nCmdShow);
        /**
        * @brief ����������
        */
        virtual void onInit() override;
        virtual void onUpdate() override;
        virtual void onRender() override;
        /**
        * @brief �I��������
        */
        virtual void onDestroy() override;
        virtual void pushAltEnter() override;
    protected:
        UINT mWidth; //!< ��
        UINT mHeight; //!< ����
        std::wstring mTitle; //!< �E�B���h�E�^�C�g��
        std::unique_ptr<Window::Window> mWindow; //!< �E�B���h�E
        std::unique_ptr<DX::DeviceResource> mDeviceResource;
        static constexpr UINT FRAME_COUNT = 3;
    };

} //Framework 