#include "GPUTimer.h"
#include "DX/DXHelper.h"
#include "Math/MathUtility.h"
#include "Utility/Debug.h"

namespace {
    //新しい値に対する現在の平均が与える影響度(0～1)
   //この値が大きいほど平均の値が更新されにくくなる。
   //この値が小さいほど平均の値が更新されやすくなる
    static constexpr float AVERAGE_IMPACT = 0.95f;

    //平均を更新する
    inline float runningAverage(float ave, float value) {
        return Framework::Math::MathUtil::lerp(value, ave, AVERAGE_IMPACT);
    }
}

namespace Framework::Utility {
    //コンストラクタ
    GPUTimer::GPUTimer()
        :mGpuFreqInv(1.0f),
        mAverages{},
        mTimings{},
        mMaxFrameCount(0){ }

    //コンストラクタ
    GPUTimer::GPUTimer(ID3D12Device* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount)
        : mGpuFreqInv(1.0f),
        mAverages{},
        mTimings{} {
        storeDevice(device, commandQueue, maxFrameCount);
    }
    //デストラクタ
    GPUTimer::~GPUTimer() { releaseDevice(); }
    //フレーム開始
    void GPUTimer::beginFrame() {
        //特に処理することはない
    }

    //フレーム終了
    void GPUTimer::endFrame(ID3D12GraphicsCommandList* commandList) {
        static UINT resolveToFrameID = 0;
        //データを取得する
        UINT64 resolveToBaseAddress = resolveToFrameID * TIMER_SLOT_NUM * sizeof(UINT64);
        commandList->ResolveQueryData(mQueryHeap.Get(), D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_TIMESTAMP, 0, TIMER_SLOT_NUM, mBuffer.Get(), resolveToBaseAddress);

        UINT readBackID = (resolveToFrameID + 1) % (mMaxFrameCount + 1);
        SIZE_T readBackOffset = readBackID * TIMER_SLOT_NUM * sizeof(UINT64);

        //バッファからデータを読み出す
        D3D12_RANGE range{ readBackOffset ,readBackOffset + TIMER_SLOT_NUM * sizeof(UINT64) };
        UINT64* timingData;
        DX::throwIfFailed(mBuffer->Map(0, &range, reinterpret_cast<void**>(&timingData)));
        memcpy(mTimings.data(), timingData, TIMER_SLOT_NUM * sizeof(UINT64));
        mBuffer->Unmap(0, nullptr);

        for (UINT i = 0; i < TIMER_COUNT; i++) {
            UINT64 start = mTimings[i * 2];
            UINT64 end = mTimings[i * 2 + 1];
            //なるべく有効桁数を保った状態で演算したい
            float value = float(double(end - start) * mGpuFreqInv);
            mAverages[i] = runningAverage(mAverages[i], value);
        }

        resolveToFrameID = readBackID;
    }

    //計測開始
    void GPUTimer::start(ID3D12GraphicsCommandList* commandList, UINT timerID) {
        MY_ASSERTION(timerID < TIMER_COUNT, L"timerIDの値が不正です");

        commandList->EndQuery(mQueryHeap.Get(), D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_TIMESTAMP, timerID * 2);
    }
    //計測終了
    void GPUTimer::stop(ID3D12GraphicsCommandList* commandList, UINT timerID) {
        MY_ASSERTION(timerID < TIMER_COUNT, L"timerIDの値が不正です");

        commandList->EndQuery(mQueryHeap.Get(), D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_TIMESTAMP, timerID * 2 + 1);
    }
    //平均のリセット
    void GPUTimer::reset() {
        mAverages.fill(0.0f);
    }
    //経過時間の取得
    float GPUTimer::getElapsedTime(UINT timerID) {
        MY_ASSERTION(timerID < TIMER_COUNT, L"timerIDの値が不正です");

        UINT64 start = mTimings[timerID * 2];
        UINT64 end = mTimings[timerID * 2 + 1];
        return float(Math::MathUtil::mymax(0.0, double(end - start))* mGpuFreqInv);
    }

    //平均経過時間の取得
    float GPUTimer::getAverageTime(UINT timerID) {
        MY_ASSERTION(timerID < TIMER_COUNT, L"timerIDの値が不正です");
        return mAverages[timerID];
    }

    //デバイスの解放
    void GPUTimer::releaseDevice() {
        mQueryHeap.Reset();
        mBuffer.Reset();
    }
    //デバイスの登録
    void GPUTimer::storeDevice(ID3D12Device* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount) {
        mMaxFrameCount = maxFrameCount;

        ComPtr<ID3D12InfoQueue> infoQueue;
        //デバッグレイヤーの警告の一部を無効化する
        if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
            D3D12_MESSAGE_ID denyIDs[] =
            {
                D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_GPU_WRITTEN_READBACK_RESOURCE_MAPPED,
            };
            D3D12_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(denyIDs);
            filter.DenyList.pIDList = denyIDs;
            infoQueue->AddStorageFilterEntries(&filter);
            MY_DEBUG_LOG(L"Warning: GPUTimer is disabling an unwanted D3D12 debug layer warning: D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_GPU_WRITTEN_READBACK_RESOURCE_MAPPED.");
        }

        //GPUのタイムスタンプカウンターの周期を取得する
        UINT64 gpuFreq;
        DX::throwIfFailed(commandQueue->GetTimestampFrequency(&gpuFreq));
        //ミリ秒として使う
        mGpuFreqInv = float(1000.0 / double(gpuFreq));

        //時間を計測するためのクエリを発行する
        D3D12_QUERY_HEAP_DESC desc = {};
        desc.Type = D3D12_QUERY_HEAP_TYPE::D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        desc.Count = TIMER_SLOT_NUM;
        DX::throwIfFailed(device->CreateQueryHeap(&desc, IID_PPV_ARGS(&mQueryHeap)));
        mQueryHeap->SetName(L"GPUTimerQuery");

        //計測した時間を格納するためのバッファを確保する
        CD3DX12_HEAP_PROPERTIES prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_READBACK);
        size_t perFrameInstances = mMaxFrameCount + 1;

        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(perFrameInstances * TIMER_SLOT_NUM * sizeof(UINT64));
        DX::throwIfFailed(device->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(mBuffer.ReleaseAndGetAddressOf())));
            //IID_GRAPHICS_PPV_ARGS(mBuffer.ReleaseAndGetAddressOf())));

        mBuffer->SetName(L"GPUTimerBuffer");
    }
} //Framework::Utility 