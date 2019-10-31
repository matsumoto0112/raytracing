#ifndef SHADER_RAYTRACING_RAYGENSHADER_HLSL
#define SHADER_RAYTRACING_RAYGENSHADER_HLSL

#define HLSL
#include "Helper.hlsli"
#include "Global.hlsli"

#include "ClosestHit.hlsl"
#include "MissShader.hlsl"

[shader("raygeneration")]
void MyRayGenShader() {
    Ray ray = generateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition, g_sceneCB.projectionToWorld);

    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    rayDesc.TMin = 0.01;
    rayDesc.TMax = 10000.0;

    RayPayload payload = { float4(0,0,0,0),0 };

    TraceRay(g_scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        ~0,
        0,
        1,
        0,
        rayDesc,
        payload);

    g_renderTarget[DispatchRaysIndex().xy] = payload.color;
}

#endif //! SHADER_RAYTRACING_RAYGENSHADER_HLSL