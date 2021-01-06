//Jamie Haddow tessellationShadow2 pixel Shader
//COMMENTING DONE

//passing in the textures for the general texture, both directional textures and the array of 12 textures for both point lights
Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture1 : register(t1);
Texture2D depthMapTexture2 : register(t2);
Texture2D depthArray[6] : register(t3);
Texture2D depthArray2[6] : register(t9);

SamplerState diffuseSampler : register(s0);
SamplerState shadowSampler : register(s1);
SamplerState diffuseSampler2 : register(s2);

//buffer containing all light information
cbuffer LightBuffer : register(b0)
{
    float4 ambient[4];
    float4 diffuse[4];
    float4 lightPosition[4];
    float4 lightDirection[4];
    float4 attenuation[2];
};

//recieving data from the previous stage
struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD1;
    float4 lightViewPos1 : TEXCOORD2;
    float4 lightViewPos2 : TEXCOORD3;
    float4 pLightViewsPos[6] : TEXCOORD4;
    float4 pLightViewsPos2[6] : TEXCOORD10;
};

// function taken from the earlier lab work on lighting
float4 calculateDirectionalLights(InputType input, float3 direction, float4 diffuse)
{
    float intensity = saturate(dot(input.normal, direction));
    return saturate(diffuse * intensity);
}
// function taken from the earlier lab work on lighting
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(lightDirection, normal));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}
// function taken from the earlier lab work on lighting
float CalculateAttenuation(float constantF, float linearF, float quadraticF, float3 dist)
{
    float atten = 1 / (constantF + (linearF * dist) + (quadraticF * pow(dist, 2)));
    return atten;
}
//function given by paul
float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}
//function given by paul
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return true;
    }
    return false;
}
//function to calculate the directional light
float4 calculateDirectionalShadow(InputType input, Texture2D depthTexture, float3 lightDirection, float4 lightViewPos, float4 ambient, float4 diffuse, float4 textureColour)
{
    float depthValue, lightDepthValue;
    float shadowMapBias = 0.0002f;
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);
    float2 pTexCoord = getProjectiveCoords(lightViewPos);

    if (hasDepthData(pTexCoord))
    {
        return textureColour;
    }

    depthValue = depthTexture.Sample(shadowSampler, pTexCoord).r;
    lightDepthValue = lightViewPos.z / lightViewPos.w;
    lightDepthValue -= shadowMapBias;

    if (lightDepthValue < depthValue)
    {
        colour = calculateDirectionalLights(input, -lightDirection, diffuse);
    }
    return saturate(colour + ambient);
}
//function to calculate point lights
//attempted to use the provided functions above but for some reason it would never work, without the helper functions it works fine
float4 CalculatePointShadows(InputType input)
{
    float depthValue1, lightDepthValue1;
    float shadowMapBias = 0.0002f;
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);
    
    for (int i = 0; i < 6; i++)
    {
        float2 pTexCoords = input.pLightViewsPos[i].xy / input.pLightViewsPos[i].w;
        pTexCoords *= float2(0.5, -0.5);
        pTexCoords += float2(0.5f, 0.5f);
        

		// Determine if the projected coordinates are in the 0 to 1 range.  If not don't do lighting.
        if (!(pTexCoords.x < 0.f || pTexCoords.x > 1.f || pTexCoords.y < 0.f || pTexCoords.y > 1.f))
        {
            depthValue1 = depthArray[i].Sample(diffuseSampler, pTexCoords).r;

            lightDepthValue1 = input.pLightViewsPos[i].z / input.pLightViewsPos[i].w;
            lightDepthValue1 -= shadowMapBias;

			// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
            if (lightDepthValue1 < depthValue1)
            {
                 //attempted to use the light vector by using the lights position - the world position however that never worked. using a static vector of 1,1,1 worked
                float3 testVector = (1.0f, 1.0, 1.0f);
                float3 lightVector = (lightPosition[2].xyz - input.worldPos);
                colour += calculateLighting(testVector, input.normal, diffuse[2]);
            }
        }
    }
    
    
    return saturate(colour + ambient[2]);
}
float4 CalculatePointShadows2(InputType input)
{
    float depthValue2, lightDepthValue2;
    float shadowMapBias = 0.0002f;
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);

    for (int j = 0; j < 6; j++)
    {
        float2 pTexCoords2 = input.pLightViewsPos2[j].xy / input.pLightViewsPos2[j].w;
        pTexCoords2 *= float2(0.5, -0.5);
        pTexCoords2 += float2(0.5f, 0.5f);
        

		// Determine if the projected coordinates are in the 0 to 1 range.  If not don't do lighting.
        if (!(pTexCoords2.x < 0.f || pTexCoords2.x > 1.f || pTexCoords2.y < 0.f || pTexCoords2.y > 1.f))
        {
            depthValue2 = depthArray2[j].Sample(diffuseSampler2, pTexCoords2).r;

            lightDepthValue2 = input.pLightViewsPos2[j].z / input.pLightViewsPos2[j].w;
            lightDepthValue2 -= shadowMapBias;

			// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
            if (lightDepthValue2 < depthValue2)
            {
                 //attempted to use the light vector by using the lights position - the world position however that never worked. using a static vector of 1,1,1 worked
                float3 testVector = (1.0f, 1.0, 1.0f);
                float3 lightVector = (lightPosition[3].xyz - input.worldPos);
                colour += calculateLighting(testVector, input.normal, diffuse[3]);
            }
        }
    }
    return saturate(colour + ambient[3]);
}


float4 main(InputType input) : SV_TARGET
{
    float atten1;
    float atten2;
    float3 distance[2];
    float3 lightVector1 = (lightPosition[2].xyz - input.worldPos);
    float3 lightVector2 = (lightPosition[3].xyz - input.worldPos);
    
    distance[0] = length(lightVector1);
    distance[1] = length(lightVector1);
    
    atten1 = CalculateAttenuation(attenuation[0].x, attenuation[0].y, attenuation[0].z, distance[0]);
    atten2 = CalculateAttenuation(attenuation[1].x, attenuation[1].y, attenuation[1].z, distance[1]);
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);
    colour += calculateDirectionalShadow(input, depthMapTexture1, lightDirection[0].xyz, input.lightViewPos1, ambient[0], diffuse[0], textureColour);
    colour += calculateDirectionalShadow(input, depthMapTexture2, lightDirection[1].xyz, input.lightViewPos2, ambient[1], diffuse[1], textureColour);
    colour += atten1 * CalculatePointShadows(input);
    colour += atten2 * CalculatePointShadows2(input);
    return (colour) * textureColour;
   //return float4(input.normal.xyz, 1.0f);
}
