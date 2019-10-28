#pragma once
#include <vector>
#include <d3d12.h>
#include "Libs/d3dx12.h"
#include "Utility/Typedef.h"

namespace Framework::DX {
    /**
    * @namespace ShaderVisibility
    * @brief �V�F�[�_�[�̉�����`
    */
    namespace ShaderVisibility {
        /**
        * @enum Enum
        */
        enum Enum {
            ALL = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL,
            Vertex = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX,
            Hull = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_HULL,
            Domain = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_DOMAIN,
            Geometry = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_GEOMETRY,
            Pixel = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL
        };
    } //ShaderVisibility 

    /**
    * @enum RootParameterType
    * @brief ���[�g�p�����[�^�̎��
    */
    enum class RootParameterType {
        DescriptorTable,
        SRV,
        UAV,
        CBV,
        Constants,
    };

    /**
    * @namespace DescriptorRangeType
    * @brief �f�B�X�N���v�^�����W�̎��
    */
    namespace DescriptorRangeType {
        /**
        * @enum Enum
        */
        enum Enum {
            SRV,
            UAV,
            CBV,
            Sampler,
        };
    } //DescriptorRangeType 

    namespace RootSignatureFlag {
        enum Enum {
            GlobalRootSignature = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_NONE,
            LocalRootSignature = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE,
        };
    } //RootSignatureType 

    /**
    * @brief �f�B�X�N���v�^�����W
    */
    struct DescriptorRange {
        DescriptorRangeType::Enum rangeType; //!< �����W�̎��
        UINT DescriptorNum; //!< ���蓖�Ă�f�B�X�N���v�^�q�[�v�̐�
        UINT shaderRegister; //!< �V�F�[�_�[���W�X�^�[�̊J�n�ԍ�

        /**
        * @brief �R���X�g���N�^
        */
        DescriptorRange()
            :rangeType(DescriptorRangeType::SRV), DescriptorNum(1), shaderRegister(0) { }

        /**
        * @brief ������
        */
        void init(DescriptorRangeType::Enum rangeType, UINT num, UINT shaderRegister) {
            this->rangeType = rangeType;
            this->DescriptorNum = num;
            this->shaderRegister = shaderRegister;
        }
    };

    /**
    * @brief ���[�g�p�����[�^
    */
    struct RootParameterDesc {
        RootParameterType type; //!< �p�����[�^�̎��
        ShaderVisibility::Enum visibility; //!< �V�F�[�_�[�̉���
        UINT shaderRegister; //!< �V�F�[�_�[���W�X�^�[�ԍ�
        /**
        * @brief ������
        */
        void init(RootParameterType paramType, UINT shaderRegister = 0, ShaderVisibility::Enum visibility = ShaderVisibility::Enum::ALL) {
            this->type = paramType;
            this->shaderRegister = shaderRegister;
            this->visibility = visibility;
        }

        /**
        * @brief �f�B�X�N���v�^�q�[�v�Ƃ��ď�����
        */
        void initAsDescriptor() {
            this->type = RootParameterType::DescriptorTable;
            this->shaderRegister = 0;
            this->visibility = ShaderVisibility::Enum::ALL;
        }
    };

    /**
    * @brief �R���X�^���g�f�X�N
    */
    struct ConstantsDesc {
        UINT bufferSize;

        /**
        * @brief �R���X�g���N�^
        */
        ConstantsDesc()
            :bufferSize(0) { }

        /**
        * @brief �R���X�g���N�^
        * @tparam T �g�p����R���X�^���g�o�b�t�@�̌^
        */
        template <class T>
        ConstantsDesc() : bufferSize(sizeof(T)) { }
    };

    /**
    * @brief �X�^�e�B�b�N�T���v���[
    */
    struct StaticSampler {
        D3D12_FILTER filter;
    };

    /**
    * @brief ���[�g�V�O�l�`���f�X�N
    */
    struct RootSignatureDesc {
        std::wstring name = L"";
        RootSignatureFlag::Enum flags = RootSignatureFlag::GlobalRootSignature;
        std::vector<DescriptorRange>* pRanges = nullptr;
        std::vector<ConstantsDesc>* pConstants = nullptr;
        std::vector<RootParameterDesc>* pParams = nullptr;
        std::vector<StaticSampler>* pStaticSamplers = nullptr;

        RootSignatureDesc(RootSignatureFlag::Enum flags, std::vector<DescriptorRange>* ranges,
            std::vector<ConstantsDesc>* constants, std::vector<RootParameterDesc>* params,
            std::vector<StaticSampler>* samplers, const std::wstring& name = L"")
            :name(name), flags(flags), pRanges(ranges), pConstants(constants), pParams(params), pStaticSamplers(samplers) { }
    };


    /**
    * @class RootSignature
    * @brief ���[�g�V�O�l�`��
    */
    class RootSignature {
    public:
        /**
        * @brief �R���X�g���N�^
        */
        RootSignature(ID3D12Device* device, const RootSignatureDesc& rootSignatureDesc);
        /**
        * @brief �f�X�g���N�^
        */
        ~RootSignature();
        /**
        * @brief ���[�g�V�O�l�`�����擾����
        */
        ID3D12RootSignature* getRootSignature() const { return mRootSignature.Get(); }
        /**
        * @brief ���[�g�V�O�l�`�����쐬����
        */
        void create(ID3D12Device* device, const RootSignatureDesc& rootSignatureDesc);
        /**
        * @brief ���[�g�V�O�l�`���̃��Z�b�g
        */
        void reset();
        /**
        * @brief ���[�J�����[�g�V�O�l�`���Ƃ��ēo�^����
        */
        void setLocalRootSignature(CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* local);
        /**
        * @brief �O���[�o�����[�g�V�O�l�`���Ƃ��ēo�^����
        */
        void setGlobalRootSignature(CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* global);
        /**
        * @brief �R���s���[�g�V�F�[�_�[�p���[�g�V�O�l�`���Ƃ��ăZ�b�g����
        */
        void setComputeRootSignature(ID3D12GraphicsCommandList* commandList);
        /**
        * @brief �O���t�B�b�N�X�p���[�g�V�O�l�`���Ƃ��ăZ�b�g����
        */
        void setGraphicsRootSignature(ID3D12GraphicsCommandList* commandList);
    private:
        ComPtr<ID3D12RootSignature> mRootSignature; //!< ���[�g�V�O�l�`��
    };
} //Framework::DX 