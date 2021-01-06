//Jamie Haddow geometry pixel Shader
//COMMENTING DONE

//passing in 3 textures, 1 being unused but left in with potentialy use in the future for additional steps
Texture2D texture0 : register(t0);
Texture2D noiseMap : register(t1);
Texture2D grass : register(t2);
SamplerState Sampler0 : register(s0);
SamplerState Sampler1 : register(s1);

//colour buffer used to give the grass different colours
cbuffer ColourBufferType : register(b0)
{
    float4 colour1;
    float4 colour2;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 depthPosition : TEXCOORD1;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD2;
    
};


float4 main(InputType input) : SV_TARGET
{
    //taking texture 0 which is a plain colour which is overwritten by the two colour floats being passed into the above buffer, a grass texture 
    //i made to give the grass blades some visual variation then lerping between them both
    float4 textureColour = texture0.Sample(Sampler0, input.tex);
    float4 grassTexture = grass.Sample(Sampler1, input.tex);
    textureColour = lerp(colour1, colour2, input.tex.y);
    float4 combined = lerp(textureColour, grassTexture, 0.2);
    
    return combined;
}