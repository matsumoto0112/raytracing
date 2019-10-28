#include "RootSignature.h"
#include "Libs/d3dx12.h"
#include "Utility/Debug.h"

namespace Framework::DX {
    RootSignature::RootSignature(ID3D12Device* device, const RootSignatureDesc& desc) {
        auto getRangeType = [](DescriptorRangeType::Enum rangeType) {
            static constexpr D3D12_DESCRIPTOR_RANGE_TYPE rangeTypes[] = {
                D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
            };
            return rangeTypes[rangeType];
        };

        auto getVisibilityType = [](ShaderVisibility::Enum visibility) {
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

        auto align = [](UINT bufferSize) {
            return 0;
        };

        std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges(desc.rangeNum);
        for (UINT i = 0; i < desc.rangeNum; i++) {
            ranges[i].Init(getRangeType(desc.ranges[i].rangeType), desc.ranges[i].DescriptorNum, desc.ranges[i].shaderRegister);
        }

        std::vector<CD3DX12_ROOT_PARAMETER> params(desc.paramNum);
        UINT descriptorIndex = 0;
        UINT constantsIndex = 0;
        for (UINT i = 0; i < desc.paramNum; i++) {
            UINT registerNum = desc.params[i].shaderRegister;
            D3D12_SHADER_VISIBILITY visibility = getVisibilityType(desc.params[i].visibility);
            switch (desc.params[i].type) {
                case RootParameterType::DescriptorTable:
                    MY_ASSERTION(descriptorIndex < desc.rangeNum, L"rangeNumの値が小さすぎます");
                    params[i].InitAsDescriptorTable(1, &ranges[descriptorIndex], visibility);
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
                    MY_ASSERTION(constantsIndex < desc.constantsNum, L"constantsNumの値が小さすぎます");
                    params[i].InitAsConstants(align(desc.constans[constantsIndex].bufferSize), registerNum, 0, visibility);
                    constantsIndex++;
                    break;
            }
        }

        std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplers(desc.staticSamplerNum, CD3DX12_STATIC_SAMPLER_DESC(0));
        for (UINT i = 0; i < desc.staticSamplerNum; i++) {
            samplers[i].Filter = desc.staticSamplers[i].filter;
        }

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(params.size(), params.data(), samplers.size(), samplers.data());

        ComPtr<ID3DBlob> blob, error;
        Framework::Utility::throwIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), L"ルートシグネチャ作成失敗");
        Framework::Utility::throwIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
        mRootSignature->SetName(desc.name.c_str());
    }

    RootSignature::~RootSignature() { }

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