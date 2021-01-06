Texture2D noiseMap : register(t0);
Texture2D windMap : register(t1);
SamplerState Sampler0 : register(s0);
SamplerState Sampler2 : register(s1);




cbuffer CamBuffer : register(b1)
{
    float4 camPosBO;
    float timeBO;

}

//struct GrassBufferType
//{
//    float numOfGrassObjects;
//    float amplitude;
//    float frequency;
//    float height;
//    float width;
//    XMFLOAT3 padding1;
//};

cbuffer GrassBuffer : register(b2)
{
    float numOfGrassObjects;
    float amplitude;
    float frequency;
    float height;
    float width;
    float3 padding1;
}



cbuffer PositionBuffer
{
    static float3 g_positions[15] =
    {
        float3(-1, -1, 0),
        float3(1, -1, 0),
        float3(1.0, 0.0, 0),
        
        float3(1.0, 0.0, 0),
        float3(-1.0, 0.0, 0),
        float3(-1, -1, 0),
        
        float3(-1.0, 0.0, 0),
        float3(1.0, 0.0, 0),
        float3(1.0, 1.0, 0),
        
        float3(-1.0, 1.0, 0),
        float3(-1.0, 0.0, 0),
        float3(-1.0, 1.0, 0),
        
        float3(-1.0, 1.0, 0),
        float3(1.0, 1.0, 0),
        float3(0.0, 1.0, 0),


    };
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;

};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 depthPosition : TEXCOORD1;
    float3 normal : NORMAL;

};

[maxvertexcount(15)]
void main(triangle InputType input[3], inout TriangleStream<OutputType> triStream)
{
    OutputType output;
    


    float grassnumber = numOfGrassObjects;
    float2 inputvalue = input[0].tex.xy;
    float2 windvalue = input[0].tex.xy;
    float scrollSpeed = 2.0f;
    float heightValue = noiseMap.SampleLevel(Sampler0, inputvalue, 0).r;
    float3 windMapvalue = windMap.SampleLevel(Sampler2, inputvalue + float2(0.1f, 0.1f) * timeBO, 0).rbg;
        
    if (grassnumber > heightValue)
    {
    //getting the forward Vector for the Billboard. taken from Frank Luna page 411, 11.2.2 Vertex Structure
        
        float3 forward = normalize(camPosBO.xyz - input[0].position.xyz);
        float3 up = float3(0, 1, 0);
        float3 right = cross(forward, up);
        
        float3 windvec;
        windvec.xz = sin(windMapvalue.rg * amplitude) * 0.5;
        windvec.y = -0.1f;
        
        int vertCount = 15;
        float3 vertPos[15];
        
        /*                         
                                      14
                                     /\
                                    /  \
                                   /    \
                            10,12 /______\ 8,9,13
                                 /        \
                                /          \
                       4,6,11  /____________\2,3,7
                              /              \
                             /                \
                            /                  \
                           /                    \
                      0,5 /______________________\1

        */
        
        
        
    //bot right triangle
        float3 wind = windvec;
        vertPos[0] = input[0].position.xyz;
        vertPos[1] = right + input[0].position.xyz;
        vertPos[2] = (up * 1.0f) + (right * 0.85f) + input[0].position.xyz + wind;
    //bot left triangle
        vertPos[3] = vertPos[2];
        vertPos[4] = (up * 1.0f) + (right * 0.15f) + input[0].position.xyz + wind;
        vertPos[5] = vertPos[0];
    //middle left triangle
        vertPos[6] = vertPos[4];
        vertPos[7] = vertPos[2];
        vertPos[8] = (up * 3.0f) + (right * 0.6f) + input[0].position.xyz + wind * 3.0f;
    //middle right triangle  
        vertPos[9] = vertPos[8];
        vertPos[10] = (up * 3.0f) + (right * 0.4f) + input[0].position.xyz + wind * 3.0f;
        vertPos[11] = vertPos[4];
    //top triangle
        vertPos[12] = vertPos[10];
        vertPos[13] = vertPos[8];
        vertPos[14] = (up * 4.0f) + (right * -0.1f) + input[0].position.xyz + wind * 4.0f;
  
        for (int i = 0; i < vertCount; i++)
        {
            float4 vposition = float4(vertPos[i], 1.0f); //+input[0].position;
            output.tex = g_positions[i];
            output.position = input[0].position;
            float3 tempNormal = input[0].normal;
            output.normal = input[0].normal;
            output.depthPosition = input[0].position;
            triStream.Append(output);
      
            if ((i + 1) % 3 == 0)
            {
                triStream.RestartStrip();
            }
        }
    }
}

