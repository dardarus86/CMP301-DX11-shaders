//Jamie Haddow Directional Blur vertex Shader
//no longer used in the final project but left in the hlsl for directionalblur
//COMMENTING DONE
Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

//buffer being pased in to control the angle, the direction and the strength of the blur
cbuffer DirectionBuffer : register(b0)
{
    float angleBO;
    float2 resolutionBO;
    float strength;
};

//recieving input from the vertex shaders output
struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};



float4 main(InputType input) : SV_TARGET
{
    //sampling the current texels cell colour
    float4 textureColor = texture0.Sample(Sampler0, input.tex);

    //calculating the direction of the blur using sin and cos in a float2
    float2 direction = float2(sin(angleBO), cos(angleBO));
    int samples = 20;
    //taking the number of samples to get the delta value
    float delta = 2.0 / float(samples);

    
    //using the above direction float2 and multiplying it by the strength value
    direction = float2(direction.x * strength, direction.y * strength);
    float2 texelSize = 1 / resolutionBO.xy;
    //looping through the delta range and adjusting the texels colour accordingly
    for (float i = -1.0f; i <= 1.0; i += delta)
    {
        textureColor += texture0.Sample(Sampler0, input.tex - texelSize * direction * i);
    }

    return textureColor;
}

