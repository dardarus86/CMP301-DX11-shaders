//Jamie Haddow shadow vertex Shader
//COMMENTING DONE

//recieveing matrix data to calculate position into screenspace

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

//buffer passing in the matrixs for the lights
cbuffer ShadowBuffer : register(b1)
{
    matrix lightViewMatrix[2];
    matrix lightProjectionMatrix[2];
    matrix lightViews[6];
    matrix lightProjections[6];
    matrix lightViews2[6];
    matrix lightProjections2[6];
}
//recieving data from the shader.cpp
struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

//sending data to the next stage
struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 lightViewPos1 : TEXCOORD1;
    float4 lightViewPos2 : TEXCOORD2;
    float3 worldPos : TEXCOORD3;
    float4 pLightViewsPos[6] : TEXCOORD4;
    float4 pLightViewsPos2[6] : TEXCOORD10;
};


OutputType main(InputType input)
{
    OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.worldPos = output.position;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Calculate the position of the vertice as viewed by the light source.
    output.lightViewPos1 = mul(input.position, worldMatrix);
    output.lightViewPos1 = mul(output.lightViewPos1, lightViewMatrix[0]);
    output.lightViewPos1 = mul(output.lightViewPos1, lightProjectionMatrix[0]);
    
    output.lightViewPos2 = mul(input.position, worldMatrix);
    output.lightViewPos2 = mul(output.lightViewPos2, lightViewMatrix[1]);
    output.lightViewPos2 = mul(output.lightViewPos2, lightProjectionMatrix[1]);
    
    for (int i = 0; i < 6; i++)
    {
        output.pLightViewsPos[i] = mul(input.position, worldMatrix);
        output.pLightViewsPos[i] = mul(output.pLightViewsPos[i], lightViews[i]);
        output.pLightViewsPos[i] = mul(output.pLightViewsPos[i], lightProjections[i]);
        
        output.pLightViewsPos2[i] = mul(input.position, worldMatrix);
        output.pLightViewsPos2[i] = mul(output.pLightViewsPos2[i], lightViews2[i]);
        output.pLightViewsPos2[i] = mul(output.pLightViewsPos2[i], lightProjections2[i]);

    }
    
    
    output.worldPos = mul(input.position, worldMatrix).xyz;

    output.tex = input.tex;
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    return output;
}