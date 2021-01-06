//Jamie Haddow water hull Shader
//COMMENTING DONE



struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

//receiving data from previous stage
struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};
//sending data to next stage
struct OutputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

//tesselation buffer to decide the edge/inside tesselation
cbuffer TesselationBufferType : register(b0)
{
    float tesselationFactor;
    float3 padding;
};

//camera buffer passing in the camera position, unused for now but can be used once dynamic tessleation is working
cbuffer CameraBufferType : register(b1)
{
    float3 cameraPos;
    float cameraPadding;
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
 
    
    int modulus = tesselationFactor;
    if (modulus % 2 == 0)
        modulus++;
    output.edges[0] = output.edges[1] = output.edges[2] = output.edges[3] = modulus;

    // Set the tessellation factor for tessallating inside the triangle.
    output.inside[0] = output.inside[1] = modulus;

    return output;
}


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    output.position = patch[pointId].position;
    output.tex = patch[pointId].tex;
    output.normal = patch[pointId].normal;
    
    return output;
}
