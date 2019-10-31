#ifndef SHADER_RAYTRACING_MISSSHADER_SHADOWCAST_HLSL
#define SHADER_RAYTRACING_MISSSHADER_SHADOWCAST_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"

[shader("miss")]
void MissShader_ShadowCast(inout ShadowPayload payload) {
    payload.hit = false;
}

#endif //! SHADER_RAYTRACING_MISSSHADER_SHADOWCAST_HLSL