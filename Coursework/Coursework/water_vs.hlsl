//Jamie Haddow water pixel Shader
//COMMENTING DONE

//receiving data from shader.cpp
struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};
//sending data to next stage
struct OutputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

OutputType main(InputType input)
{
    OutputType output;

	 // Pass the vertex position into the hull shader.
    output.position = input.position;
    
    // Pass the input color into the hull shader.
    output.normal = input.normal;
    output.tex = input.tex;
    
    return output;
}
