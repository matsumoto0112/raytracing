#ifndef SHADER_RAYTRACING_RAYGENSHADER_HLSL
#define SHADER_RAYTRACING_RAYGENSHADER_HLSL

#define HLSL
#include "Helper.hlsli"
#include "Global.hlsli"

[shader("raygeneration")]
void MyRayGenShader() {
    float4 color = float4(0, 0, 0, 0);
    static const uint WIDTH = 3;
    static const uint HEIGHT = 3;
    static const uint SAMPLE_COUNT = WIDTH * HEIGHT;

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            //float2 offset = float2(1.0 / x, 1.0 / y) + 0.5;
            float2 offset = float2(1.0 / WIDTH * x,1.0 / HEIGHT * y);
            Ray ray = generateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld, offset);

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
            color += payload.color;
        }
    }

    color /= SAMPLE_COUNT;
    g_renderTarget[DispatchRaysIndex().xy] = color;
}

#endif //! SHADER_RAYTRACING_RAYGENSHADER_HLSL