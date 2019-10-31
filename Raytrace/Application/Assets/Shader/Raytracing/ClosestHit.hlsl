#ifndef SHADER_RAYTRACING_CLOSESTHIT_HLSL
#define SHADER_RAYTRACING_CLOSESTHIT_HLSL

#define HLSL
#include "Global.hlsli"
#include "RaytracingCompat.h"

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in MyAttr attr) {
    g_renderTarget[DispatchRaysIndex().xy] = float4(1, 1, 1, 1);
    //payload.color = float4(1, 0, 0, 1);
}
#endif //! SHADER_RAYTRACING_CLOSESTHIT_HLSL