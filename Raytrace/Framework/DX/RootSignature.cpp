#include "RootSignature.h"
#include "Libs/d3dx12.h"
#include "Utility/Debug.h"

namespace Framework::DX {
    //�R���X�g���N�^
    RootSignature::RootSignature(ID3D12Device* device, const RootSignatureDesc& desc) {
        create(device, desc);
    }

    //�f�X�g���N�^
    RootSignature::~RootSignature() {
        reset();
    }

    //���[�g�V�O�l�`���쐬
    void RootSignature::create(ID3D12Device* device, const RootSignatureDesc& desc) {
        //�Ǝ�DescriptorRangeType��DX12�p�񋓌^�ɕϊ�����
        constexpr auto getRangeType = [](DescriptorRangeType::Enum rangeType) {
            static constexpr D3D12_DESCRIPTOR_RANGE_TYPE rangeTypes[] = {
                D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
            };
            return rangeTypes[rangeType];
        };

        //�Ǝ�ShaderVisibility��DX12�p�񋓌^�ɕϊ�����
        constexpr auto getVisibilityType = [](ShaderVisibility::Enum visibility) {
            static constexpr D3D12_SHADER_VISIBILITY visibilityTypes[] =
            {
                D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL,
                D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX,
                D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_HULL,
                D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_DOMAIN,
                D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_GEOMETRY,
                D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL,
            };
            return visibilityTypes[visibility];
        };

        constexpr auto getRootSignatureFlags = [](RootSignatureFlag::Enum flags) {
            return static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(flags);
        };

        //�R���X�^���g�o�b�t�@�̃o�b�t�@�̃A���C�������g
        constexpr auto align = [](UINT bufferSize) -> UINT {
            return  ((bufferSize - 1) / sizeof(UINT32) + 1);
        };

        const UINT rangeNum = desc.pRanges ? static_cast<UINT>(desc.pRanges->size()) : 0;
        const UINT constantsNum = desc.pConstants ? static_cast<UINT>(desc.pConstants->size()) : 0;
        const UINT paramNum = desc.pParams ? static_cast<UINT>(desc.pParams->size()) : 0;
        const UINT samplerNum = desc.pStaticSamplers ? static_cast<UINT>(desc.pStaticSamplers->size()) : 0;

        //�܂��f�B�X�N���v�^�q�[�v�̃����W���쐬����
        std::vector<CD3DX12_DESCRIPTOR_RANGE> pRanges(rangeNum);
        for (UINT i = 0; i < rangeNum; i++) {
            pRanges[i].Init(getRangeType(desc.pRanges->at(i).rangeType), desc.pRanges->at(i).DescriptorNum, desc.pRanges->at(i).shaderRegister);
        }

        //�p�����[�^��`
        std::vector<CD3DX12_ROOT_PARAMETER> params(paramNum);
        size_t descriptorIndex = 0;
        size_t constantsIndex = 0;
        for (UINT i = 0; i < paramNum; i++) {
            UINT registerNum = desc.pParams->at(i).shaderRegister;
            D3D12_SHADER_VISIBILITY visibility = getVisibilityType(desc.pParams->at(i).visibility);
            switch (desc.pParams->at(i).type) {
                //�f�B�X�N���v�^�q�[�v�̎��̓����W�����Ɋ��蓖�ĂĂ���
                case RootParameterType::DescriptorTable:
                    MY_ASSERTION(descriptorIndex < desc.pRanges->size(), L"rangeNum�̒l�����������܂�");
                    params[i].InitAsDescriptorTable(1, &pRanges[descriptorIndex], visibility);
                    descriptorIndex++;
                    break;
                case RootParameterType::SRV:
                    params[i].InitAsShaderResourceView(registerNum, 0, visibility);
                    break;
                case RootParameterType::UAV:
                    params[i].InitAsUnorderedAccessView(registerNum, 0, visibility);
                    break;
                case RootParameterType::CBV:
                    params[i].InitAsConstantBufferView(registerNum, 0, visibility);
                    break;
                case RootParameterType::Constants:
                    MY_ASSERTION(constantsIndex < constantsNum, L"constantsNum�̒l�����������܂�");
                    params[i].InitAsConstants(align(desc.pConstants->at(constantsIndex).bufferSize), registerNum, 0, visibility);
                    constantsIndex++;
                    break;
            }
        }

        //�X�^�e�B�b�N�T���v���[���쐬����
        std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplers(samplerNum, CD3DX12_STATIC_SAMPLER_DESC(0));
        for (UINT i = 0; i < samplerNum; i++) {
            samplers[i].Filter = desc.pStaticSamplers->at(i).filter;
        }

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(paramNum, params.data(), samplerNum, samplers.data());
        rootSignatureDesc.Flags = getRootSignatureFlags(desc.flags);

        ComPtr<ID3DBlob> blob, error;
        Framework::Utility::throwIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), L"���[�g�V�O�l�`���쐬���s");
        Framework::Utility::throwIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
        mRootSignature->SetName(desc.name.c_str());
    }

    void RootSignature::reset() {
        mRootSignature.Reset();
    }

    void RootSignature::setLocalRootSignature(CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* local) {
        local->SetRootSignature(mRootSignature.Get());
    }

    void RootSignature::setGlobalRootSignature(CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* global) {
        global->SetRootSignature(mRootSignature.Get());
    }

    void RootSignature::setComputeRootSignature(ID3D12GraphicsCommandList* commandList) {
        commandList->SetComputeRootSignature(mRootSignature.Get());
    }

    void RootSignature::setGraphicsRootSignature(ID3D12GraphicsCommandList* commandList) {
        commandList->SetGraphicsRootSignature(mRootSignature.Get());
    }

} //Framework::DX 