//Jamie Haddow depth of field pixel Shader
//COMMENTING DONE

//passing in 3 textures. an unblurred scene, a blurred scsene after going through the horizontal and vertical blurs and a camera depth map
Texture2D unblurredSceneTexture : register(t0);
Texture2D blurSceneTexture : register(t1);
Texture2D depthSceneTexture : register(t2);

SamplerState SampleType : register(s0);

cbuffer DepthBuffer : register(b0)
{
    float range;
    //passing these in, they will probably never change but always good as back up
    float nearValue;
    float farValue;
    float offset;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float4 colour = float4(0, 0, 0, 1);
    //getting the texel data of the three textures being passed in. depth being the r value only due to it being a grayscale image
    //getting the center texel colour of the image so the blurring happens on the camera lookat and can then be offset later using the offset variable
    float4 unblurredScene = unblurredSceneTexture.Sample(SampleType, input.tex);
    float4 blurScene = blurSceneTexture.Sample(SampleType, input.tex);
    float depthvalue = depthSceneTexture.Sample(SampleType, input.tex).r;
    float centreTexel = depthSceneTexture.Sample(SampleType, float2(0.5f, 0.5f)).r;

	//from testing, the blur was not being effective until i took the two near and far planes, got the difference between them both and then multiplied to the center and depth values
    //default near = 0.1f, far = 200.0f
    centreTexel *= (nearValue - farValue);
    depthvalue *= (nearValue - farValue);

	// must use a saturate funtion here otherwise the blurvalue can be negative or above 1
    //abs only returning a positive value
    float s = saturate(abs(depthvalue - centreTexel + offset) / range);
      
    //returning a lerp between the two scenes, blurred and unblurred
    colour = lerp(unblurredScene, blurScene, s);
    return colour;

}
