#pragma once
#include <vector>
#include <d3d12.h>
#include "Libs/d3dx12.h"
#include "Utility/Typedef.h"

namespace Framework::DX {
    /**
    * @enum ShaderVisibility
    * @brief description
    */
    namespace ShaderVisibility {
        enum Enum {
            ALL = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL,
            Vertex = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX,
            Hull = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_HULL,
            Domain = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_DOMAIN,
            Geometry = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_GEOMETRY,
            Pixel = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL
        };
    } //ShaderVisibility 

    enum class RootParameterType {
        DescriptorTable,
        SRV,
        UAV,
        CBV,
        Constants,
    };
    namespace DescriptorRangeType {
    /**
    * @enum Enum
    * @brief description
    */
        enum Enum {
            SRV,
            UAV,
            CBV,
            Sampler,
        };
    } //DescriptorRangeType 

    struct DescriptorRange {
        DescriptorRangeType::Enum rangeType;
        UINT DescriptorNum;
        UINT shaderRegister;

        void init(DescriptorRangeType::Enum rangeType, UINT num, UINT shaderRegister) {
            this->rangeType = rangeType;
            this->DescriptorNum = num;
            this->shaderRegister = shaderRegister;
        }
    };

    struct RootParameterDesc {
        RootParameterType type;
        ShaderVisibility::Enum visibility;
        UINT shaderRegister;

        void init(RootParameterType paramType, UINT shaderRegister = 0, ShaderVisibility::Enum visibility = ShaderVisibility::Enum::ALL) {
            this->type = paramType;
            this->shaderRegister = shaderRegister;
            this->visibility = visibility;
        }
    };

    struct ConstantsDesc {
        UINT bufferSize;
    };

    struct StaticSampler {
        D3D12_FILTER filter;
    };

    struct RootSignatureDesc {
        std::wstring name = L"";
        UINT rangeNum = 0;
        const DescriptorRange* ranges = nullptr;
        UINT constantsNum = 0;
        const ConstantsDesc* constans = nullptr;
        UINT paramNum = 0;
        const RootParameterDesc* params = nullptr;
        UINT staticSamplerNum = 0;
        const StaticSampler* staticSamplers = nullptr;
    };


    /**
* @class RootSignature
* @brief discription
*/
    class RootSignature {
    public:
        /**
        * @brief コンストラクタ
        */
        RootSignature(ID3D12Device* device, const RootSignatureDesc& rootSignatureDesc);
        /**
        * @brief デストラクタ
        */
        ~RootSignature();

        void setLocalRootSignature(CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* local);
        void setGlobalRootSignature(CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* global);
        void setComputeRootSignature(ID3D12GraphicsCommandList* commandList);
        void setGraphicsRootSignature(ID3D12GraphicsCommandList* commandList);
    private:
        ComPtr<ID3D12RootSignature> mRootSignature;
    };
} //Framework::DX 