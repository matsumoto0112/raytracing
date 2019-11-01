#ifndef SHADER_RAYTRACING_CLOSESTHIT_SHADOWCAST_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_SHADOWCAST_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"
#include "ClosestHit_Local.h"
#include "Helper.hlsli"

[shader("closesthit")]
void ClosestHit_ShadowCast(inout ShadowPayload payload, in MyAttr attr) {
    payload.hit = true;
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_SHADOWCAST_HLSL