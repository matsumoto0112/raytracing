#ifndef SHADER_RAYTRACING_CLOSESTHIT_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"
#include "Local.h"

typedef  HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer Material;

ConstantBuffer<Material> l_material : register(b1);

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in MyAttr attr) {


    g_renderTarget[DispatchRaysIndex().xy] = g_sceneCB.lightAmbient;
    //payload.color = float4(1, 0, 0, 1);
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_HLSL