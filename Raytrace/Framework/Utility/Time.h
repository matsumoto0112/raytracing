#pragma once
#include <Windows.h>
#include <list>

namespace Framework::Utility {
    /**
    * @class Time
    * @brief 時間管理クラス
    */
    class Time {
    public:
        /**
        * @brief コンストラクタ
        * @param sample サンプリング数
        * @details sampleが大きいほど平均のFPSが安定しやすいがメモリ消費量が多くなる
        */
        Time(UINT sample = 10);
        /**
        * @brief デストラクタ
        */
        ~Time();
        /**
        * @brief 更新
        */
        void update();
        /**
        * @brief FPSを取得する
        */
        double getFPS() const { return mFPS; }
        /**
        * @brief 前フレームからの差分時間秒を取得する
        */
        double getDeltaTime() const { return mDiffTime; }
        /**
        * @brief サンプル数を設定する
        */
        void setSampleCount(UINT sample = 10);
    private:
        /**
        * @brief 差分時間を取得する
        */
        double getCurrentDefTime();
    private:
        LARGE_INTEGER mCounter; //!< パフォーマンスカウンター
        LONGLONG mPrevCount; //!< 前フレームでのカウント
        double mFreq; //!< 1秒あたりのティックカウント
        std::list<double> mDifTimes; //!< 差分時間リスト
        double mSumTimes; //!< 合計差分時間
        UINT mSampleCount; //!< サンプリング数 
        double mFPS; //!< 現在のFPS
        double mDiffTime; //!< 前フレームからの差分時間(秒)
    };
} //Framework::Utility 