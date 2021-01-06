//Jamie Haddow geometryDepth hull Shader
//COMMENTING DONE

//tesselation buffer to decide the edge/inside tesselation
cbuffer TesselationBufferType : register(b0)
{
    float tesselationFactor;
    float3 padding;
};
//camera buffer passed in for dynamic tesselation that was never created successfully without cracks. left here for future implementation
cbuffer CameraBufferType : register(b1)
{
    float3 cameraPos;
    float cameraPadding;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

//receiving data from the previous stage
struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float4 depthPosition : TEXCOORD1;
};
//sending data to the next stage
struct OutputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float4 depthPosition : TEXCOORD1;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;
   
    //                              1
    //0 left edge               __________
    //1 top edge               |          | 
    //2 right edge           0 |          | 2
    //3 bottom edge            |          | 
    //                         |__________| 
    //                               3
    //

    float3 avgPos = inputPatch[0].position;
    avgPos += inputPatch[1].position;
    avgPos += inputPatch[2].position;
    avgPos += inputPatch[3].position;
    
    avgPos /= 4;
 
    float4 distance1 = distance(cameraPos, avgPos);
     
    output.edges[0] = clamp(100 * tesselationFactor / distance1, 1, 64);
    output.edges[1] = clamp(100 * tesselationFactor / distance1, 1, 64);
    output.edges[2] = clamp(100 * tesselationFactor / distance1, 1, 64);
    output.edges[3] = clamp(100 * tesselationFactor / distance1, 1, 64);

    output.inside[1] = clamp(100 * tesselationFactor / distance1, 1, 64);
    output.inside[0] = clamp(100 * tesselationFactor / distance1, 1, 64);

    return output;
}


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(6)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    output.position = patch[pointId].position;
    output.tex = patch[pointId].tex;
   
    return output;
}
