//Jamie Haddow Horizontal Blur Pixel Shader
//base code provided by paul and being extended on
//COMMENTING DONE


Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

//buffer passed in from the shader.cpp storing screen information and an additional weight for testing purposes
cbuffer ScreenSizeBuffer : register(b0)
{
    float screenWidth;
    float additionalWeight;
    float2 padding;
};
//recieveing data from the vertex stage
struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float weight0, weight1, weight2, weight3, weight4;
    float invertweight0, invertweight1, invertweight2, invertweight3, invertweight4;
    float4 colour;

	// Create the weights that each neighbor pixel will contribute to the blur.
    weight0 = 0.382928f;
    weight1 = 0.241732f;
    weight2 = 0.060598f;
    weight3 = 0.005977f;
    weight4 = 0.000229f;
    
    //alternate weighting
    //invertweight0 = 0.02;
    //invertweight1 = 0.04; // 0.08 0.98
    //invertweight2 = 0.05f; // 0.1 0.9
    //invertweight3 = 0.15f; // 0.3 0.8
    //invertweight4 = 0.25f; // 0.5

	// Initialize the colour to black.
    colour = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float texelSize = 1.0f / screenWidth;
    // Add the horizontal pixels to the colour by the specific weight of each.
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * -4.0f, 0.0f)) * weight4 * additionalWeight;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * -3.0f, 0.0f)) * weight3 * additionalWeight;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * -2.0f, 0.0f)) * weight2 * additionalWeight;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * -1.0f, 0.0f)) * weight1 * additionalWeight;
    colour += shaderTexture.Sample(SampleType, input.tex) * weight0 * additionalWeight;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 1.0f, 0.0f)) * weight1 * additionalWeight;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 2.0f, 0.0f)) * weight2 * additionalWeight;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 3.0f, 0.0f)) * weight3 * additionalWeight;
    colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSize * 4.0f, 0.0f)) * weight4 * additionalWeight;

	// Set the alpha channel to one.
    colour.a = 1.0f;

    return colour;
}
