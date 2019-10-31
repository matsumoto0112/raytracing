#ifndef SHADER_RAYTRACING_MISSSHADER_HLSL
#define SHADER_RAYTRACING_MISSSHADER_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"

[shader("miss")]
void MissShader(inout RayPayload payload) {
    //g_renderTarget[DispatchRaysIndex().xy] = float4(1, 0, 0, 1);
    payload.color = float4(0, 1, 0, 1);
}

#endif //! SHADER_RAYTRACING_MISSSHADER_HLSL