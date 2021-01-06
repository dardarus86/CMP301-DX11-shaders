//Jamie Haddow depth of field vertex Shader
//COMMENTING DONE


//recieveing matrix data to calculate position into screenspace
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
//recieving data from the shader .cpp
struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};
//sending data to the next stage
struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

OutputType main(InputType input)
{
    OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	// Set the output texture coordinates
    output.tex = input.tex;

    return output;
}
