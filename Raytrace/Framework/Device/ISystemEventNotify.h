#pragma once
#include <d3d12.h>

namespace Framework::Device {
    /**
    * @brief システム的なイベント通知対象オブジェクト
    */
    interface ISystemEventNotify {
        /**
        * @brief 初期化処理
        */
        virtual void onInit() = 0;
        /**
        * @brief 更新
        */
        virtual void onUpdate() = 0;
        /**
        * @brief 描画
        */
        virtual void onRender() = 0;
        /**
        * @brief 終了時処理
        */
        virtual void onDestroy() = 0;

        virtual void pushAltEnter() = 0;
    };
} //Framework::Device