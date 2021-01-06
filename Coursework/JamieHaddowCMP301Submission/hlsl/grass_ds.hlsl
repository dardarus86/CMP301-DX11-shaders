Texture2D heightTexture : register(t0);
SamplerState heightSampler : register(s0);



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

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;

};


[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    float4 vertexPosition;
    float3 vertexnormal;
    OutputType output;
    
    //                              3
    //0 left edge               __________
    //1 bottom edge            |          | 
    //2 right edge           0 |          | 2
    //3 top edge               |          | 
    //                         |__________| 
    //                               1
    //

    float4 v1 = lerp(patch[0].position, patch[1].position, uvwCoord.y);
    float4 v2 = lerp(patch[3].position, patch[2].position, uvwCoord.y);
    vertexPosition = lerp(v1, v2, uvwCoord.x);
    
    float3 n1 = lerp(patch[0].normal, patch[1].normal, uvwCoord.y);
    float3 n2 = lerp(patch[3].normal, patch[2].normal, uvwCoord.y);
    vertexnormal = lerp(n1, n2, uvwCoord.x);
    
    float2 t1 = lerp(patch[0].tex, patch[1].tex, uvwCoord.y);
    float2 t2 = lerp(patch[3].tex, patch[2].tex, uvwCoord.y);
    float2 texCoord = lerp(t1, t2, uvwCoord.x);
    
    vertexPosition.y = (heightTexture.SampleLevel(heightSampler, texCoord.xy, 0).r * displacementHeight);
 
    output.position = vertexPosition;
    output.tex = texCoord;
    output.normal = normalize(vertexnormal);
    return output;

}

