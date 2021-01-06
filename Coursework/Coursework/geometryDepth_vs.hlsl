//Jamie Haddow geometryDepth vertex Shader
//COMMENTING DONE

//recieving data from thr shader.cpp
struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

//sending data to the next stage
struct OutputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float4 depthPosition : TEXCOORD1;
};

OutputType main(InputType input)
{
    OutputType output;

    output.position = input.position;
    output.tex = input.tex;
    output.depthPosition = float4(output.position, 1.0);
	
    return output;
}