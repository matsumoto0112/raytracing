#ifndef SHADER_RAYTRACING_LOCAL_H
#define SHADER_RAYTRACING_LOCAL_H

namespace HitGroupParams {
    namespace LocalRootSignatureParams {
        namespace Constant {
            struct MaterialConstantBuffer {
                UINT indexOffset;
                UINT vertexOffset;
            };
        } //Constant 
    } //LocalRootSignatureParameter 
} //HitGroup 


#endif //! SHADER_RAYTRACING_LOCAL_H