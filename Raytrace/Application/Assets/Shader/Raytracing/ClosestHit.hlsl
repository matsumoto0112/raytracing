#ifndef SHADER_RAYTRACING_CLOSESTHIT_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"

struct Material {
    float4 color;
};

ConstantBuffer<Material> l_material : register(b1);

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in MyAttr attr) {
    g_renderTarget[DispatchRaysIndex().xy] = l_material.color;
    //payload.color = float4(1, 0, 0, 1);
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_HLSL