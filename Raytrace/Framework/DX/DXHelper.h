#pragma once
#include <d3d12.h>
#include <stdexcept>
#include "Libs/d3dx12.h"
#include "Utility/Debug.h"
#include "Utility/Typedef.h"

namespace Framework::DX {
    /**
    * @brief HRESULT��O�N���X
    */
    class HrException : public std::runtime_error {
        inline std::string HrToString(HRESULT hr) {
            char s_str[64] = {};
            sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
            return std::string(s_str);
        }
    public:
        HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) { }
        HRESULT Error() const { return m_hr; }
    private:
        const HRESULT m_hr;
    };

    /**
    * @brief ���s���Ă������O�𓊂���
    */
    inline void throwIfFailed(HRESULT hr) {
        if (FAILED(hr)) {
            throw HrException(hr);
        }
    }

    /**
    * @brief ���s���Ă������O�𓊂���
    */
    inline void throwIfFailed(HRESULT hr, const wchar_t* mes) {
        if (FAILED(hr)) {
            MY_DEBUG_LOG(mes);
            throw HrException(hr);
        }
    }

    /**
    * @brief ���s���Ă������O�𓊂���
    */
    inline void throwIfFalse(bool value) {
        throwIfFailed(value ? S_OK : E_FAIL);
    }

    /**
    * @brief ���s���Ă������O�𓊂���
    */
    inline void throwIfFalse(bool value, const wchar_t* mes) {
        throwIfFailed(value ? S_OK : E_FAIL, mes);
    }

#ifdef _DEBUG
    inline void setName(ID3D12Object* object, LPCWSTR name) {
        object->SetName(name);
    }
    inline void setNameIndexed(ID3D12Object* object, LPCWSTR name, UINT index) {
        static constexpr int MAX_NAME_LENGTH = 50;
        WCHAR fullName[MAX_NAME_LENGTH];
        if (swprintf_s(fullName, L"%s[%u]", name, index) > 0) {
            object->SetName(fullName);
        }
    }
#else
    inline void setName(ID3D12Object*, LPCWSTR) { }
    inline void setNameIndexed(ID3D12Object*, LPCWSTR, UINT) { }
#endif

    inline UINT align(UINT size, UINT alignment) {
        return (size + (alignment - 1)) & ~(alignment - 1);
    }

    inline UINT calculateConstantBufferByteSize(UINT byteSize) {
        return align(byteSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    }

    /**
    * @class GPUUploadBuffer
    * @brief GPU�ɃA�b�v���[�h����o�b�t�@
    */
    class GPUUploadBuffer {
    public:
        /**
        * @brief ���\�[�X�̎擾
        */
        ComPtr<ID3D12Resource> getResource() { return mResource; }
        /**
        * @brief ���\�[�X�̊J��
        */
        virtual void release() { mResource.Reset(); }
    protected:
        /**
        * @brief �R���X�g���N�^
        */
        GPUUploadBuffer() { }
        /**
        * @brief �f�X�g���N�^
        */
        ~GPUUploadBuffer() {
            if (mResource) {
                mResource->Unmap(0, nullptr);
            }
        }

        /**
        * @brief ���\�[�X�̊��蓖��
        */
        void allocate(ID3D12Device* device, UINT bufferSize, LPCWSTR resourceName = nullptr) {
            CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD);

            CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
            throwIfFailed(device->CreateCommittedResource(
                &heapProp,
                D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&mResource)));
            mResource->SetName(resourceName);
        }

        /**
        * @brief �������݉\�ȃ�������Ԃ�
        */
        uint8_t* mapCPUWriteOnly() {
            uint8_t* mapped;
            CD3DX12_RANGE range(0, 0);
            throwIfFailed(mResource->Map(0, &range, reinterpret_cast<void**>(&mapped)));
            return mapped;
        }
    protected:
        ComPtr<ID3D12Resource> mResource; //!< ���\�[�X
    };

    /**
    * @brief �o�b�t�@�̃p�b�`
    */
    struct D3DBuffer {
        ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    };

} //Framework::DX 