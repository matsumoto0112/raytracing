#ifndef SHADER_RAYTRACING_LOCAL_H
#define SHADER_RAYTRACING_LOCAL_H

#include "RaytracingTypedef.h"

namespace HitGroupParams {
    namespace LocalRootSignatureParams {
        namespace Constant {
            struct MaterialConstantBuffer {

                Color4 color;
                UINT indexOffset;
                UINT vertexOffset;
#ifndef HLSL
                D3D12_GPU_DESCRIPTOR_HANDLE texture;
#endif
            };
        } //Constant 
    } //LocalRootSignatureParameter 
} //HitGroup 


#endif //! SHADER_RAYTRACING_LOCAL_H