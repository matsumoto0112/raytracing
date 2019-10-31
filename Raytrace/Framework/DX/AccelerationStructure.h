#pragma once
#include <vector>
#include <d3d12.h>
#include "DX/DXHelper.h"
#include "DX/RaytracingHelper.h"
#include "Math/Vector3.h"
#include "Application/Assets/Shader/RaytracingStructure.h"

namespace Framework::DX {

    struct Buffer {
        D3DBuffer vertexBuffer;
        D3DBuffer indexBuffer;
    };

    /**
    * @class AccelerationStructure
    * @brief ASの管理
    */
    class AccelerationStructure {
        using VertexList = std::vector<Vertex>;
        using IndexList = std::vector<UINT16>;
        using ID = UINT;
    public:
        /**
        * @brief コンストラクタ
        */
        AccelerationStructure();
        /**
        * @brief デストラクタ
        */
        ~AccelerationStructure();

        void addBLASBuffer(ID3D12Device* device, void* indices, size_t indexSize, void* vertices, size_t vertexSize);
        ID buildBLAS(ID3D12Device* nDevice, ID3D12Device5* device, ID3D12GraphicsCommandList5* commandList);
    //private:
        std::vector<Buffer> mBuffers;
        std::vector<Buffer> mBufferStacks;
        std::vector<AccelerationStructureBuffers> mBottomLevelASs;
    };
} //Framework::DX 