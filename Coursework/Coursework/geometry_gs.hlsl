//Jamie Haddow geometry geometry Shader
//COMMENTING DONE

//passing in 3 textures to be used here. a noise map, a wind map and a height map
Texture2D noiseMap : register(t0);
Texture2D windMap : register(t1);
Texture2D HeightMap : register(t2);
SamplerState Sampler0 : register(s0);
SamplerState Sampler2 : register(s1);
SamplerState Sampler3 : register(s2);

//recieveing matrix data to calculate position into screenspace

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
//leaving this camera buff in as the user can switch to billboarding whenever they want

cbuffer CameraBufferType : register(b1)
{
    float4 cameraPos;
    float grassTime;
    float3 padding;
};

//buffer with variables used in the creation of geometry below
cbuffer GrassBufferType : register(b2)
{
    float NumOfGrassObjectsBO;
    float frequency;
    float amplitude;
    float height;
    float width;
    float3 padding1;
};

//tex coord array 
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
        float3(1.0, 1.0, 0),
        
        float3(-1.0, 1.0, 0),
        float3(1.0, 1.0, 0),
        float3(0.0, 1.0, 0),


    };
};

//recieving data from previous stage
struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

//sending data to next stage
struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 worldPos : TEXCOORD2;
};


[maxvertexcount(15)]
void main(triangle InputType input[3], inout TriangleStream<OutputType> triStream)
{
    
    OutputType output;
    
   
    float grassnumber = NumOfGrassObjectsBO;
    float2 inputvalue = input[0].tex.xy;
    float2 windvalue = input[0].tex.xy;
  
    float heightValue = HeightMap.SampleLevel(Sampler0, inputvalue, 0).r;
    float3 windMapvalue = windMap.SampleLevel(Sampler2, inputvalue + float2(frequency, frequency) * grassTime, 0).rbg;
    float3 startingDirection = noiseMap.SampleLevel(Sampler3, inputvalue, 0).r;
    
    //getting the forward Vector for the Billboard. taken from Frank Luna page 411, 11.2.2 Vertex Structure
    //if you wish to use billboarding, change 'position' to 'right'
        
    float3 forward = normalize(cameraPos.xyz - input[0].position.xyz);
    float3 up = float3(0, 1, 0);
    float3 position = normalize(float3(startingDirection.r, 0, 1 - startingDirection.r));
    //float3 right = cross(forward, up); // for billboarding
        
    float3 windvec;
    //taking in the windmap and using sin to give it wave range

    windvec.xz = sin(windMapvalue.rg * amplitude) * 0.5;
    windvec.y = -0.1f;
        
    int vertCount = 15;
    float3 vertPos[15];
    
        //basic view of the geometry for help create
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
    vertPos[1] = position * 0.2 + input[0].position.xyz;
    vertPos[2] = ((up * 1.0f) * height) + ((position * 0.60f) * width) + input[0].position.xyz + wind;
    //bot left triangle
    vertPos[3] = vertPos[2];
    vertPos[4] = ((up * 1.0f) * height) + ((position * 0.30f) / width) + input[0].position.xyz + wind;
    vertPos[5] = vertPos[0];
    //middle right triangle
    vertPos[6] = vertPos[4];
    vertPos[7] = vertPos[2];
    vertPos[8] = ((up * 3.0f) * height) + ((position * 0.6f) * width) + input[0].position.xyz + wind * 3.0f;
    //middle left triangle  
    vertPos[9] = vertPos[8];
    vertPos[10] = ((up * 3.0f) * height) + ((position * 0.4f) * width) + input[0].position.xyz + wind * 3.0f;
    vertPos[11] = vertPos[4];
    //top triangle
    vertPos[12] = vertPos[10];
    vertPos[13] = vertPos[8];
    vertPos[14] = ((up * 4.0f) * height) + ((position * +0.4f) * width) + input[0].position.xyz + wind * 4.0f;

        //using the heightmap.r value to decide if grass is rendered or not

    if (grassnumber > heightValue)
    {
        for (int i = 0; i < vertCount; i++)
        {
            int stripIndex = 0;
            float4 vposition = float4(vertPos[i] + float3(startingDirection.r , 0, startingDirection.r), 1.0f); //+input[0].position;
            output.worldPos = float4(mul(vposition, worldMatrix).xyz, 1.0f);
            output.position = mul(vposition, worldMatrix);
            output.position = mul(output.position, viewMatrix);
            output.position = mul(output.position, projectionMatrix);
            output.normal = input[stripIndex].normal;
            output.tex = g_positions[i];
            triStream.Append(output);
            stripIndex++;
            if (stripIndex == 2)
            {
                stripIndex = 0;
                triStream.RestartStrip();
            }
        }
    }
}


