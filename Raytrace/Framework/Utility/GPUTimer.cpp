#include "GPUTimer.h"
#include "DX/DXHelper.h"
#include "Math/MathUtility.h"
#include "Utility/Debug.h"

namespace {
    //�V�����l�ɑ΂��錻�݂̕��ς��^����e���x(0�`1)
   //���̒l���傫���قǕ��ς̒l���X�V����ɂ����Ȃ�B
   //���̒l���������قǕ��ς̒l���X�V����₷���Ȃ�
    static constexpr float AVERAGE_IMPACT = 0.95f;

    //���ς��X�V����
    inline float runningAverage(float ave, float value) {
        return Framework::Math::MathUtil::lerp(value, ave, AVERAGE_IMPACT);
    }
}

namespace Framework::Utility {
    //�R���X�g���N�^
    GPUTimer::GPUTimer()
        :mGpuFreqInv(1.0f),
        mAverages{},
        mTimings{},
        mMaxFrameCount(0){ }

    //�R���X�g���N�^
    GPUTimer::GPUTimer(ID3D12Device* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount)
        : mGpuFreqInv(1.0f),
        mAverages{},
        mTimings{} {
        storeDevice(device, commandQueue, maxFrameCount);
    }
    //�f�X�g���N�^
    GPUTimer::~GPUTimer() { releaseDevice(); }
    //�t���[���J�n
    void GPUTimer::beginFrame() {
        //���ɏ������邱�Ƃ͂Ȃ�
    }

    //�t���[���I��
    void GPUTimer::endFrame(ID3D12GraphicsCommandList* commandList) {
        static UINT resolveToFrameID = 0;
        //�f�[�^���擾����
        UINT64 resolveToBaseAddress = resolveToFrameID * TIMER_SLOT_NUM * sizeof(UINT64);
        commandList->ResolveQueryData(mQueryHeap.Get(), D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_TIMESTAMP, 0, TIMER_SLOT_NUM, mBuffer.Get(), resolveToBaseAddress);

        UINT readBackID = (resolveToFrameID + 1) % (mMaxFrameCount + 1);
        SIZE_T readBackOffset = readBackID * TIMER_SLOT_NUM * sizeof(UINT64);

        //�o�b�t�@����f�[�^��ǂݏo��
        D3D12_RANGE range{ readBackOffset ,readBackOffset + TIMER_SLOT_NUM * sizeof(UINT64) };
        UINT64* timingData;
        DX::throwIfFailed(mBuffer->Map(0, &range, reinterpret_cast<void**>(&timingData)));
        memcpy(mTimings.data(), timingData, TIMER_SLOT_NUM * sizeof(UINT64));
        mBuffer->Unmap(0, nullptr);

        for (UINT i = 0; i < TIMER_COUNT; i++) {
            UINT64 start = mTimings[i * 2];
            UINT64 end = mTimings[i * 2 + 1];
            //�Ȃ�ׂ��L��������ۂ�����Ԃŉ��Z������
            float value = float(double(end - start) * mGpuFreqInv);
            mAverages[i] = runningAverage(mAverages[i], value);
        }

        resolveToFrameID = readBackID;
    }

    //�v���J�n
    void GPUTimer::start(ID3D12GraphicsCommandList* commandList, UINT timerID) {
        MY_ASSERTION(timerID < TIMER_COUNT, L"timerID�̒l���s���ł�");

        commandList->EndQuery(mQueryHeap.Get(), D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_TIMESTAMP, timerID * 2);
    }
    //�v���I��
    void GPUTimer::stop(ID3D12GraphicsCommandList* commandList, UINT timerID) {
        MY_ASSERTION(timerID < TIMER_COUNT, L"timerID�̒l���s���ł�");

        commandList->EndQuery(mQueryHeap.Get(), D3D12_QUERY_TYPE::D3D12_QUERY_TYPE_TIMESTAMP, timerID * 2 + 1);
    }
    //���ς̃��Z�b�g
    void GPUTimer::reset() {
        mAverages.fill(0.0f);
    }
    //�o�ߎ��Ԃ̎擾
    float GPUTimer::getElapsedTime(UINT timerID) {
        MY_ASSERTION(timerID < TIMER_COUNT, L"timerID�̒l���s���ł�");

        UINT64 start = mTimings[timerID * 2];
        UINT64 end = mTimings[timerID * 2 + 1];
        return float(Math::MathUtil::mymax(0.0, double(end - start))* mGpuFreqInv);
    }

    //���όo�ߎ��Ԃ̎擾
    float GPUTimer::getAverageTime(UINT timerID) {
        MY_ASSERTION(timerID < TIMER_COUNT, L"timerID�̒l���s���ł�");
        return mAverages[timerID];
    }

    //�f�o�C�X�̉��
    void GPUTimer::releaseDevice() {
        mQueryHeap.Reset();
        mBuffer.Reset();
    }
    //�f�o�C�X�̓o�^
    void GPUTimer::storeDevice(ID3D12Device* device, ID3D12CommandQueue* commandQueue, UINT maxFrameCount) {
        mMaxFrameCount = maxFrameCount;

        ComPtr<ID3D12InfoQueue> infoQueue;
        //�f�o�b�O���C���[�̌x���̈ꕔ�𖳌�������
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

        //GPU�̃^�C���X�^���v�J�E���^�[�̎������擾����
        UINT64 gpuFreq;
        DX::throwIfFailed(commandQueue->GetTimestampFrequency(&gpuFreq));
        //�~���b�Ƃ��Ďg��
        mGpuFreqInv = float(1000.0 / double(gpuFreq));

        //���Ԃ��v�����邽�߂̃N�G���𔭍s����
        D3D12_QUERY_HEAP_DESC desc = {};
        desc.Type = D3D12_QUERY_HEAP_TYPE::D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        desc.Count = TIMER_SLOT_NUM;
        DX::throwIfFailed(device->CreateQueryHeap(&desc, IID_PPV_ARGS(&mQueryHeap)));
        mQueryHeap->SetName(L"GPUTimerQuery");

        //�v���������Ԃ��i�[���邽�߂̃o�b�t�@���m�ۂ���
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