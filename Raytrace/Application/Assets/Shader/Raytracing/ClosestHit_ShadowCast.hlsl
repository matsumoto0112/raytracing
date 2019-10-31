#ifndef SHADER_RAYTRACING_CLOSESTHIT_SHADOWCAST_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_SHADOWCAST_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"
#include "Local.h"
#include "Helper.hlsli"

typedef  HitGroupParams::LocalRootSignatureParams::Constant::MaterialConstantBuffer Material;

ConstantBuffer<Material> l_material : register(b1);

[shader("closesthit")]
void ClosestHit_ShadowCast(inout ShadowPayload payload, in MyAttr attr) {
    payload.hit = true;
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_SHADOWCAST_HLSL