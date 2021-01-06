//Jamie Haddow tessellationShadow domain Shader
//COMMENTING DONE

SamplerState heightSampler : register(s0);
Texture2D heightTexture : register(t0);

//recieveing matrix data to calculate position into screenspace

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

//buffer passing in the matrixs for the lights
cbuffer ShadowBuffer : register(b1)
{
    
    matrix lightViewMatrix[2];
    matrix lightProjectionMatrix[2];
    matrix lightViews[6];
    matrix lightProjections[6];
    matrix lightViews2[6];
    matrix lightProjections2[6];
};

//buffer used in adjusting the height of the map
cbuffer DislacementBufferType : register(b2)
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
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float tessellation : TEXCOORD1;
};

//sending data to next stage
struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
    float4 lightViewPos1 : TEXCOORD2;
    float4 lightViewPos2 : TEXCOORD3;
    float4 pLightViewsPos[6] : TEXCOORD4;
    float4 pLightViewsPos2[6] : TEXCOORD10;
};


[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    //code provided by paul
    float3 vertexPosition;
    float3 vertexnormal;
    OutputType output;

    float3 v1 = lerp(patch[0].position, patch[1].position, uvwCoord.y);
    float3 v2 = lerp(patch[3].position, patch[2].position, uvwCoord.y);
    vertexPosition = lerp(v1, v2, uvwCoord.x);
        
    float2 t1 = lerp(patch[0].tex, patch[1].tex, uvwCoord.y);
    float2 t2 = lerp(patch[3].tex, patch[2].tex, uvwCoord.y);
    float2 texCoord = lerp(t1, t2, uvwCoord.x);
    
    vertexPosition.y = (heightTexture.SampleLevel(heightSampler, texCoord.xy, 0).r) * displacementHeight;
    
    float avgTess = (patch[0].tessellation + patch[1].tessellation + patch[2].tessellation + patch[3].tessellation) / 4;
    
    float3 vertexLeft, vertexRight, vertexAbove, vertexBelow;
    
    vertexLeft = vertexRight = vertexAbove = vertexBelow = vertexPosition;
    
    vertexRight.x += 1.0f / avgTess;
    vertexRight.y = heightTexture.SampleLevel(heightSampler, texCoord + float2((1.0f / 25.0f) / avgTess, 0), 0).r;

    vertexLeft.x -= 1.0f / avgTess;
    vertexLeft.y = heightTexture.SampleLevel(heightSampler, texCoord - float2((1.0f / 25.0f) / avgTess, 0), 0).r;

    vertexBelow.z += 1.0f / avgTess;
    vertexBelow.y = heightTexture.SampleLevel(heightSampler, texCoord + float2(0, (1.0f / 25.0f) / avgTess), 0).r;

    vertexAbove.z -= 1.0f / avgTess;
    vertexAbove.y = heightTexture.SampleLevel(heightSampler, texCoord - float2(0, (1.0f / 25.0f) / avgTess), 0).r;
    
    float3 AB = normalize(vertexLeft - vertexRight);
    float3 CD = normalize(vertexAbove - vertexAbove);

    vertexnormal = normalize(cross(AB, CD));
    
    
    //calculating the screenspace for the light views
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.worldPos = mul(float4(vertexPosition, 1.0f), worldMatrix).xyz;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    

    output.lightViewPos1 = mul(float4(vertexPosition, 1.0), worldMatrix);
    output.lightViewPos1 = mul(output.lightViewPos1, lightViewMatrix[0]);
    output.lightViewPos1 = mul(output.lightViewPos1, lightProjectionMatrix[0]);
    
    output.lightViewPos2 = mul(float4(vertexPosition, 1.0), worldMatrix);
    output.lightViewPos2 = mul(output.lightViewPos2, lightViewMatrix[1]);
    output.lightViewPos2 = mul(output.lightViewPos2, lightProjectionMatrix[1]);
    
    for (int i = 0; i < 6; i++)
    {
        output.pLightViewsPos[i] = mul(float4(vertexPosition, 1.0), worldMatrix);
        output.pLightViewsPos[i] = mul(output.pLightViewsPos[i], lightViews[i]);
        output.pLightViewsPos[i] = mul(output.pLightViewsPos[i], lightProjections[i]);
        
        output.pLightViewsPos2[i] = mul(float4(vertexPosition, 1.0), worldMatrix);
        output.pLightViewsPos2[i] = mul(output.pLightViewsPos2[i], lightViews2[i]);
        output.pLightViewsPos2[i] = mul(output.pLightViewsPos2[i], lightProjections2[i]);

    }
    
    output.normal = mul(vertexnormal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);
    output.tex = texCoord;

    return output;
}

