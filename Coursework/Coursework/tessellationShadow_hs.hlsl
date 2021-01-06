//Jamie Haddow tessellationShadow hull Shader
//COMMENTING DONE

//tesselation buffer to decide the edge/inside tesselation
cbuffer TesselationBufferType : register(b0)
{
    float tesselationFactor;
    float3 padding;
};

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
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

//sending data to the next stage
struct OutputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float tessellation : TEXCOORD1;
};



ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;
    
    float3 avgPos = (inputPatch[0].position + inputPatch[1].position + inputPatch[2].position + inputPatch[3].position) / 4;

    float4 distance = distance(cameraPos, avgPos);
    
    float4 tesselationcalculation = clamp(50 * tesselationFactor / distance, 1, 10);
     
    output.edges[0] = output.edges[1] = output.edges[2] = output.edges[3] = tesselationcalculation;
    output.inside[1] = output.inside[0] = tesselationcalculation;

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
    output.normal = patch[pointId].normal;
    
    return output;
}
