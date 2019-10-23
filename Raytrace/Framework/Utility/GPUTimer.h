#pragma once
#include <array>
#include <d3d12.h>
#include "Utility/Typedef.h"

namespace Framework::Utility {
    /**
    * @class GPUTimer
    * @brief GPUの時間計測
    */
    class GPUTimer {
    public:
        /**
        * @brief コンストラクタ
        */
        GPUTimer();
        /**
        * @brief コンストラクタ
        */
        GPUTimer(ID3D12Device* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount);
        /**
        * @brief デストラクタ
        */
        ~GPUTimer();
        /**
        * @brief フレーム開始
        */
        void beginFrame();
        /**
        * @brief フレーム終了
        */
        void endFrame(ID3D12GraphicsCommandList* commandList);
        /**
        * @brief 計測開始
        */
        void start(ID3D12GraphicsCommandList* commandList, UINT timerID = 0);
        /**
        * @brief 計測終了
        */
        void stop(ID3D12GraphicsCommandList* commandList, UINT timerID = 0);
        /**
        * @brief 平均時間のリセット
        */
        void reset();
        /**
        * @brief 経過時間を取得する
        */
        float getElapsedTime(UINT timerID = 0);
        /**
        * @brief 平均時間を取得する
        */
        float getAverageTime(UINT timerID = 0);
        /**
        * @brief デバイスを解放する
        */
        void releaseDevice();
        /**
        * @brief デバイスを登録する
        */
        void storeDevice(ID3D12Device* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount);
    private:
        static constexpr UINT TIMER_COUNT = 8; //!< タイマーの総使用可能数
        static constexpr UINT TIMER_SLOT_NUM = 2 * TIMER_COUNT; //!< タイマーの時間を持っておく数 開始と終了でタイマー使用数の二倍必要
        ComPtr<ID3D12QueryHeap> mQueryHeap; //!< デバイスへのクエリ用ヒープ
        ComPtr<ID3D12Resource> mBuffer; //!< 計測した時間を取得するためのバッファ
        float mGpuFreqInv; //!< GPUのタイムスタンプカウンターの周期
        std::array<float, TIMER_COUNT> mAverages; //!< 経過時間の平均
        std::array<UINT64, TIMER_SLOT_NUM> mTimings; //!< 経過時間を取得するための配列
        UINT mMaxFrameCount; //!<バックバッファの枚数
    };
} //Framework::Utility 