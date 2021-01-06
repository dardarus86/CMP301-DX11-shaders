Texture2D noiseMap : register(t0);
Texture2D windMap : register(t1);
Texture2D HeightMap : register(t2);
SamplerState Sampler0 : register(s0);
SamplerState Sampler2 : register(s1);
SamplerState Sampler3 : register(s2);
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

//struct CameraBufferType
//{
//    XMFLOAT4 cameraPos;
//    float grassTime;
//    XMFLOAT3 padding;
//};

cbuffer CameraBufferType : register(b1)
{
    float4 cameraPos;
    float grassTime;
    float3 padding;
};

cbuffer GrassBufferType : register(b2)
{
    float NumOfGrassObjectsBO;
    float frequency;
    float amplitude;
    float height;
    float width;
    float3 padding1;
};

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

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

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
    
    //for (int i = 0; i < 3; i++)
    //{
    //    output.worldPos = float4(mul(input[i].position, worldMatrix).xyz, 1.0f);
    //    output.position = mul(input[i].position, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = input[i].tex;
    //    output.normal = input[i].normal;
    //    triStream.Append(output);
    //}
    //triStream.RestartStrip();
   
    float grassnumber = NumOfGrassObjectsBO;
    float2 inputvalue = input[0].tex.xy;
    float2 windvalue = input[0].tex.xy;
  
    float heightValue = HeightMap.SampleLevel(Sampler0, inputvalue, 0).r;
    float3 windMapvalue = windMap.SampleLevel(Sampler2, inputvalue + float2(frequency, frequency) * grassTime, 0).rbg;
    float3 startingDirection = noiseMap.SampleLevel(Sampler3, inputvalue, 0).r;
    //if (grassnumber > heightValue)
    //{
    //getting the forward Vector for the Billboard. taken from Frank Luna page 411, 11.2.2 Vertex Structure
        
    float3 forward = normalize(cameraPos.xyz - input[0].position.xyz);
        float3 up = float3(0, 1, 0);
    float3 right = normalize(float3(startingDirection.r, 0, 1 - startingDirection.r)); //not actual right but a random direction for grass
        //  float3 right = cross(forward, up); // for billboarding
        
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
    vertPos[1] = right * 0.2 + input[0].position.xyz;
    vertPos[2] = ((up * 1.0f) * height) + ((right * 0.60f) * width) + input[0].position.xyz + wind;
    //bot left triangle
        vertPos[3] = vertPos[2];
    vertPos[4] = ((up * 1.0f) * height) + ((right * 0.30f) / width) + input[0].position.xyz + wind;
        vertPos[5] = vertPos[0];
    //middle left triangle
      vertPos[6] = vertPos[4];
        vertPos[7] = vertPos[2];
    vertPos[8] = ((up * 3.0f) * height) + ((right * 0.6f) * width) + input[0].position.xyz + wind * 3.0f;
    //middle right triangle  
        vertPos[9] = vertPos[8];
    vertPos[10] = ((up * 3.0f) * height) + ((right * 0.4f) * width) + input[0].position.xyz + wind * 3.0f;
        vertPos[11] = vertPos[4];
    //top triangle
        vertPos[12] = vertPos[10];
        vertPos[13] = vertPos[8];
    vertPos[14] = ((up * 4.0f) * height) + ((right * + 0.4f) * width) + input[0].position.xyz + wind * 4.0f;

    if (grassnumber > heightValue)
    {
        for (int i = 0; i < vertCount; i++)
        {
            int stripIndex = 0;
            float4 vposition = float4(vertPos[0], 1.0f); //+input[0].position;
            output.worldPos = float4(mul(vposition, worldMatrix).xyz, 1.0f);
            output.position = mul(vposition, worldMatrix);
            output.position = mul(output.position, viewMatrix);
            output.position = mul(output.position, projectionMatrix);
            output.tex = g_positions[0];
            float3 tempNormal = input[0].normal;
            output.normal = input[0].normal;
            triStream.Append(output);
            stripIndex++;
            if (i + 1 % 3)
            {
                stripIndex = 0;
                triStream.RestartStrip();
            }

        }
    }
    //        float4 vposition = float4(vertPos[0], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[0];
    //    float3 tempNormal = input[0].normal;
    //    output.normal = input[0].normal;
    //    triStream.Append(output);
     
    //    float4 vposition1 = float4(vertPos[1], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition1, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition1, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[1];
    //    float3 tempNormal1 = input[1].normal;
    //    output.normal = input[1].normal;
    //    triStream.Append(output);
    
    //    float4 vposition2 = float4(vertPos[2], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition2, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition2, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[2];
    //    float3 tempNormal2 = input[2].normal;
    //    output.normal = input[2].normal;
    //    triStream.Append(output);
    //    triStream.RestartStrip();
    
    //    float4 vposition3 = float4(vertPos[3], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition3, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition3, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[0];
    //    float3 tempNormal3 = input[0].normal;
    //    output.normal = input[0].normal;
    //    triStream.Append(output);
     
    //    float4 vposition4 = float4(vertPos[4], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition4, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition4, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[1];
    //    float3 tempNormal4 = input[1].normal;
    //    output.normal = input[1].normal;
    //    triStream.Append(output);
    
    //    float4 vposition5 = float4(vertPos[5], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition5, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition5, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[2];
    //    float3 tempNormal5 = input[2].normal;
    //    output.normal = input[2].normal;
    //    triStream.Append(output);
    //    triStream.RestartStrip();
    
    //    float4 vposition6 = float4(vertPos[6], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition6, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition6, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[0];
    //    float3 tempNormal6 = input[0].normal;
    //    output.normal = input[0].normal;
    //    triStream.Append(output);
     
    //    float4 vposition7 = float4(vertPos[7], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition7, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition7, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[1];
    //    float3 tempNormal7 = input[1].normal;
    //    output.normal = input[1].normal;
    //    triStream.Append(output);
    
    //    float4 vposition8 = float4(vertPos[8], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition8, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition8, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[2];
    //    float3 tempNormal8 = input[2].normal;
    //    output.normal = input[2].normal;
    //    triStream.Append(output);
    //    triStream.RestartStrip();
    
    //    float4 vposition9 = float4(vertPos[9], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition9, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition9, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[0];
    //    float3 tempNormal9 = input[0].normal;
    //    output.normal = input[0].normal;
    //    triStream.Append(output);
     
    //    float4 vposition10 = float4(vertPos[10], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition10, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition10, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[1];
    //    float3 tempNormal10 = input[1].normal;
    //    output.normal = input[1].normal;
    //    triStream.Append(output);
    
    //    float4 vposition11 = float4(vertPos[11], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition11, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition11, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[2];
    //    float3 tempNormal11 = input[2].normal;
    //    output.normal = input[2].normal;
    //    triStream.Append(output);
    //    triStream.RestartStrip();
    
    //    float4 vposition12 = float4(vertPos[12], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition12, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition12, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[0];
    //    float3 tempNormal12 = input[0].normal;
    //    output.normal = input[0].normal;
    //    triStream.Append(output);
     
    //    float4 vposition13 = float4(vertPos[13], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition13, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition13, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[1];
    //    float3 tempNormal13 = input[1].normal;
    //    output.normal = input[1].normal;
    //    triStream.Append(output);
    
    //    float4 vposition14 = float4(vertPos[14], 1.0f); //+input[0].position;
    //    output.worldPos = float4(mul(vposition14, worldMatrix).xyz, 1.0f);
    //    output.position = mul(vposition14, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    output.tex = g_positions[2];
    //    float3 tempNormal14 = input[2].normal;
    //    output.normal = input[2].normal;
    //    triStream.Append(output);
    //    triStream.RestartStrip();
    //}
    
    
    
    
    
    
    
    
    
}

