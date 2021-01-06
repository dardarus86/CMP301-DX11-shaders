//Jamie Haddow water pixel Shader
//COMMENTING DONE

//passing in the textureand height map used for calculating alpha value of the water based on height map
Texture2D texture0 : register(t0);
Texture2D heightMap : register(t1);
SamplerState Sampler0 : register(s0);
SamplerState Sampler2 : register(s1);

//receiving data from previous stage
struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    
};

//buffer passed in for the displacement value
cbuffer TesselationBufferType : register(b0)
{
    float tesselationFactor;
    float displacement;
    float2 tessPadding;
};

//buffer used for changing the waters colour
cbuffer ColourBufferType : register(b1)
{
    float4 colour1;
    float4 colour2;
};

float4 main(InputType input) : SV_TARGET
{
    //terrain starting = 2.0f
    //water starting = 4.0f
    //sampling both the texture and the height map
    float4 textureColour = texture0.Sample(Sampler0, input.tex);
    float heightMapValue = heightMap.Sample(Sampler0, input.tex.xy).r * displacement;
  //getting the distance from the water height and the terrain height
    //float distance = 2.0f - heightMapValue;
 

    textureColour.a = lerp(1.0f, 0.2f, (1.7f - heightMapValue) / 0.2f);
    textureColour.rgb = lerp(colour1.rgb, colour2.rgb, (1.2f - heightMapValue) / 0.2f);
    
    
    return textureColour;
}

