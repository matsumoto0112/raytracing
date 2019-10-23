#pragma once
#include <string>
#include "Framework/Device/ISystemEventNotify.h"
#include "Framework/DX/DeviceResource.h"
#include "Framework/Window/Window.h"

namespace Framework {
    /**
    * @class Game
    * @brief ゲーム基底クラス
    */
    class Game : public Device::ISystemEventNotify, DX::IDeviceNotify {
    public:
        /**
        * @brief コンストラクタ
        * @param width 幅
        * @param height 高さ
        * @param title タイトル
        */
        Game(UINT width, UINT height, const std::wstring& title);
        /**
        * @brief デストラクタ
        */
        ~Game();
        /**
        * @brief 実行処理
        * @param hInstance インスタンスハンドル
        * @param nCmdShow ウィンドウの表示方法
        */
        int run(HINSTANCE hInstance, int nCmdShow);
        /**
        * @brief 初期化処理
        */
        virtual void onInit() override;
        /**
        * @brief 更新処理
        */
        virtual void onUpdate() override;
        /**
        * @brief 描画処理
        */
        virtual void onRender() override;
        /**
        * @brief 終了時処理
        */
        virtual void onDestroy() override;
        /**
        * @brief Alt+Enterが押された
        * @details システムキーで全画面化のイベントを処理しなければならない
        */
        virtual void pushAltEnter() override;
    protected:
        static constexpr UINT FRAME_COUNT = 3;
        UINT mWidth; //!< 幅
        UINT mHeight; //!< 高さ
        std::wstring mTitle; //!< ウィンドウタイトル
        std::unique_ptr<Window::Window> mWindow; //!< ウィンドウ
        std::unique_ptr<DX::DeviceResource> mDeviceResource; //!< デバイスリソース
    };

} //Framework 