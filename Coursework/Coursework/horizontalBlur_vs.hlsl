//Jamie Haddow Horizontal Blur vertex Shader
//base code provided by paul and being extended on
//COMMENTING DONE

//passing in matrix data from the shader.cpp to calculate new screenshape coordinates
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

//recieving position and tex information, no need for normals
struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};
//outputting position and tex information to pixel stage, no need for normals
struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


OutputType main(InputType input)
{
    OutputType output;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = input.tex;

    return output;
}