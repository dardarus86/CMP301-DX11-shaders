//Jamie Haddow depth vertex Shader
//base code provided by paul
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
    float3 normal : NORMAL;
};
//sending the data to the pixel stage
struct OutputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
};

OutputType main(InputType input)
{
    OutputType output;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;
	
    return output;
}