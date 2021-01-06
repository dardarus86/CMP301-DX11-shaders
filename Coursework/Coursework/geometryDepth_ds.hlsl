//Jamie Haddow geometryDepth Domain Shader
//COMMENTING DONE

//passing in height texture used in calculating displacement
Texture2D heightTexture : register(t0);
SamplerState heightSampler : register(s0);

//buffer used in adjusting the height of the map
cbuffer DislacementBufferType : register(b0)
{
    float displacementHeight;
    float3 padding2;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

//receiving data from previous stage
struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float4 depthPosition : TEXCOORD1;
};
//sending data to next stage
struct OutputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float4 depthPosition : TEXCOORD1;
};

//using the quad domain instead of triangles
[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    float3 vertexPosition;
    float3 vertexnormal;
    OutputType output;
      
    //code given by Paul
    float3 v1 = lerp(patch[0].position, patch[1].position, uvwCoord.y);
    float3 v2 = lerp(patch[3].position, patch[2].position, uvwCoord.y);
    vertexPosition = lerp(v1, v2, uvwCoord.x);
    
    float2 t1 = lerp(patch[0].tex, patch[1].tex, uvwCoord.y);
    float2 t2 = lerp(patch[3].tex, patch[2].tex, uvwCoord.y);
    float2 texCoord = lerp(t1, t2, uvwCoord.x);
    
    //using the displacement variable to change the y value
    vertexPosition.y = (heightTexture.SampleLevel(heightSampler, texCoord.xy, 0).r * displacementHeight);

    output.position = float4(vertexPosition, 1.0);
    output.tex = texCoord;
    output.depthPosition = output.position;
    
    return output;
}

