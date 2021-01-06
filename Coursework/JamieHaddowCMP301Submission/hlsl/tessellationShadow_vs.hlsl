//Jamie Haddow tessellationShadow vertex Shader
//COMMENTING DONE

//recieving data from the shader.cpp
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
