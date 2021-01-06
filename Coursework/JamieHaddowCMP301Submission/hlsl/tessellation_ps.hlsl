//Jamie Haddow tessellation pixel Shader
//COMMENTING DONE

//passing in single texture
Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

//buffer containing all light information

cbuffer LightBuffer : register(b0)
{
    float4 ambient[4];
    float4 diffuse[4];
    float4 lightPosition[4];
    float4 lightDirection[4];
};

//recieving data from the previous stage

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 lightViewPos1 : TEXCOORD1;
    float4 lightViewPos2 : TEXCOORD2;
    float3 worldPos : TEXCOORD3;
    float4 pLightViewsPos[6] : TEXCOORD4;
    float4 pLightViewsPos2[6] : TEXCOORD10;
};

float4 main(InputType input) : SV_TARGET
{
    float4 textureColour = texture0.Sample(Sampler0, input.tex);
 
    return textureColour;

}