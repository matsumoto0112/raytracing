#pragma once
#include <string>
#include "Framework/Device/ISystemEventNotify.h"
#include "Framework/DX/DeviceResource.h"
#include "Framework/Window/Window.h"

namespace Framework {
    /**
    * @class Game
    * @brief �Q�[�����N���X
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
        /**
        * @brief �X�V����
        */
        virtual void onUpdate() override;
        /**
        * @brief �`�揈��
        */
        virtual void onRender() override;
        /**
        * @brief �I��������
        */
        virtual void onDestroy() override;
        /**
        * @brief Alt+Enter�������ꂽ
        * @details �V�X�e���L�[�őS��ʉ��̃C�x���g���������Ȃ���΂Ȃ�Ȃ�
        */
        virtual void pushAltEnter() override;
    protected:
        static constexpr UINT FRAME_COUNT = 3;
        UINT mWidth; //!< ��
        UINT mHeight; //!< ����
        std::wstring mTitle; //!< �E�B���h�E�^�C�g��
        std::unique_ptr<Window::Window> mWindow; //!< �E�B���h�E
        std::unique_ptr<DX::DeviceResource> mDeviceResource; //!< �f�o�C�X���\�[�X
    };

} //Framework 