#pragma once
#include <vector>
#include <d3d12.h>
#include "DX/DescriptorTable.h"
#include "DX/DXHelper.h"

namespace Framework::DX {
    /**
    * @brief �e�N�X�`���f�[�^
    */
    struct TextureData {
        std::vector<BYTE> data; //!< �e�N�X�`���̒��g
        UINT width; //!< �e�N�X�`���̕�
        UINT height; //!< �e�N�X�`���̍���
        UINT textureSizePerPixel; //!< �s�N�Z���P�ʂ̃o�C�g�̑傫��
    };

    /**
    * @class Texture2D
    * @brief �e�N�X�`��2D
    */
    class Texture2D {
    public:
        /**
        * @brief �R���X�g���N�^
        * @device �f�o�C�X
        * @param descriptor ���̃e�N�X�`�����Ǘ�����f�B�X�N���v�^�q�[�v
        * @param textureData �g�p����e�N�X�`���f�[�^
        */
        Texture2D(ID3D12Device* device, DescriptorTable* descriptor, const TextureData& textureData);
        /**
        * @brief �f�X�g���N�^
        */
        ~Texture2D();
        /**
        * @brief CPU�n���h�����擾����
        */
        const D3D12_CPU_DESCRIPTOR_HANDLE& getCPUHandle() const { return mTextureResource.cpuHandle; }
        /**
        * @brief GPU�n���h�����擾����
        */
        const D3D12_GPU_DESCRIPTOR_HANDLE& getGPUHandle() const { return mTextureResource.gpuHandle; }
    private:
        D3DBuffer mTextureResource; //!< �e�N�X�`�����\�[�X
    };

} //Framework::DX 