//Jamie Haddow water domain Shader
//COMMENTING DONE

Texture2D noiseMap : register(t0);
SamplerState Sampler0 : register(s0);

//recieveing matrix data to calculate position into screenspace
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

//buffer passing in time and variables to be used in the wave creation
cbuffer TimerBuffer : register(b1)
{
    float timerBufferVariable;
    float speedBufferVariable;
    float amplitudeBuferVariable;
    float frequencyBufferVariable;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

//receiving data from previous stage
struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};
//sending data to next stage
struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};



[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    float3 vertexPosition;
    float3 vertexnormal;
    OutputType output;
      
    //provided by paul
    float3 v1 = lerp(patch[0].position, patch[1].position, uvwCoord.y);
    float3 v2 = lerp(patch[3].position, patch[2].position, uvwCoord.y);
    vertexPosition = lerp(v1, v2, uvwCoord.x);
    
    float3 n1 = lerp(patch[0].normal, patch[1].normal, uvwCoord.y);
    float3 n2 = lerp(patch[3].normal, patch[2].normal, uvwCoord.y);
    vertexnormal = lerp(n1, n2, uvwCoord.x);
    
    float2 t1 = lerp(patch[0].tex, patch[1].tex, uvwCoord.y);
    float2 t2 = lerp(patch[3].tex, patch[2].tex, uvwCoord.y);
    float2 texCoord = lerp(t1, t2, uvwCoord.x);
    
    float3 noiseMap1 = noiseMap.SampleLevel(Sampler0, texCoord, 0);
       
    //the wave was originally one sin function and after much testing and using random values, sqrt,sin,cos,tan,abs,saturate, doing a double sin + sqrt looked really nice so i left it
    //as that. I offset the water so the central circle was offset into the corner so it looks like a directional wave
    vertexPosition.y += amplitudeBuferVariable * (sin((sqrt((vertexPosition.x - 50) * (vertexPosition.x - 50) + (vertexPosition.z - 50) * (vertexPosition.z - 50))
														+ (timerBufferVariable * speedBufferVariable) * frequencyBufferVariable)) +
												 sin((sqrt((vertexPosition.z - 50) * (vertexPosition.z - 50))
													+ (timerBufferVariable * speedBufferVariable) * frequencyBufferVariable))) * noiseMap1;
    
    vertexnormal.x = (amplitudeBuferVariable * cos((vertexPosition.x + (timerBufferVariable * speedBufferVariable) * frequencyBufferVariable))) * noiseMap1;
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.normal = mul(vertexnormal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);
    output.tex = texCoord;
   
    return output;
}

