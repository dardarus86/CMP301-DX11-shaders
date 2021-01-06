//Jamie Haddow app1.h
//COMMENTING DONE

// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
#include "TesselatedPlane.h"
#include "PlaneMesh.h"
#include "Model.h"
#include "WaterShader.h"
#include "TesselationShader.h"
#include "TesselationDepthShader.h"
#include "TessShadowShader2.h"
#include "TessellatedPlaneWithGeometry.h"
#include "TesselationgeometryDepthShader.h"
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"
#include "SphereMesh.h"
#include "HorizontalBlur.h"
#include "VerticalBlur.h"
#include "DepthOfFieldShader.h"


class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();



//bools used for ImGui windows
	bool bImguiLandScape;
	bool bImguiGrass;
	bool bImguiPictureframes;
	bool bImguiWater;
	bool bImguiGeometry;
	bool bImguiParticles;
	bool bImguiPointLights;
	bool bImguiDLights;
	bool bImguiBlur;

//consts for array clarification
//direction 0,1
//point     2,3
	const int dLight1 = 0;
	const int dLight2 = 1;
	const int pLight1 = 2;
	const int pLight2 = 3;

//terrain variables
	int heightmapChoice;
	int heightmapTextureChoice;
	float terrainTesselationFactor;
	float terrainDisplacementHeight;

//grass variables
	XMFLOAT4 grassColour1;
	XMFLOAT4 grassColour2;
	float grassTesselationFactor;
	float grassDisplacementHeight;
	float numOfGrassObjects = 1.0;
	float grassTime = 0.0;
	float grassAmplitude = 2.0f;
	float grassFrequency = 0.2f;
	float grassHeight = 40.0f;
	float grassWidth = 40.0f;

//water variables
	float waterTesselationFactor;
	int waterTextureChoice;
	float waterTimer;
	float waterAmplitude;
	float waterSpeed;
	float waterFrequency;
	XMFLOAT4 waterColour1;
	XMFLOAT4 waterColour2;

//pictureframe variables
	float picture1displacement;
	float picture1TessFactor;
	float picture2displacement;
	float picture2TessFactor;
	float picture3displacement;
	float picture3TessFactor;
	XMFLOAT3 pictureframe1Pos;
	XMFLOAT3 pictureframe2Pos;
	XMFLOAT3 pictureframe3Pos;


//Light Variables

	XMFLOAT4 lightAmbient[4];
	XMFLOAT4 lightDiffuse[4];
	XMFLOAT4 lightPosition[4];
	XMFLOAT4 lightDirection[4];
	XMFLOAT3 attenuation[2];
	float pLightRotation;
	float pLightRotation2;
	float pLightradius;
	float pLightFrequency;
	float normalStrength;

//direction array used for point lights
	XMFLOAT3 directions[6] =
	{
		XMFLOAT3(0.0f ,1.0, 0.0f),
		XMFLOAT3(0.0f , -1.0f, 0.0f),
		XMFLOAT3(1,0.0f,0),
		XMFLOAT3(-1,0.0f,0),
		XMFLOAT3(0,0.0f,1),
		XMFLOAT3(0,0.0f,-1)
	};

//shadow variables
	int shadowmapWidth;
	int shadowmapHeight;
	int sceneWidth;
	int sceneHeight;

	//blur variables
	float vertScreenWidth;
	float horizScreenWidth;
	float dofDistance;
	float dofOffset;
	float vertStrength;
	float horizStrength;
	
protected:
	bool render();
	void gui();
	void CameraDepthPass();
	void FirstDirectionalShadowPass();
	void SecondDirectionalShadowPass();
	void FirstPointShadowPass();
	void SecondPointShadowPass();
	void RenderScene();
	void WireFrameRenderScene();
	void HorizontalBlurPass();
	void VerticalBlurPass();
	void SecondHorizontalBlurPass();
	void SecondVerticalBlurPass();
	void DoFPass();
	void finalPass();
	void LightUpdate();
	void LightInit();
	void GeneralInits(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	//imgui functions
	void ImguiLandScape();
	void ImguiGrass();
	void ImguiLandWater();
	void ImguiPictureFrames();
	void ImguiPointLights();
	void ImguiDLights();
	void ImguiBlur();

	//two functions used linked Imgui and the load texture functions inside my terrain shader
	wchar_t* ChangeHeightMap();
	wchar_t* ChangeHeightMapTexture();


private:

	//depth RenderTextures for lights and camera

	RenderTexture* directionalShadowRenderTexture[2];
	RenderTexture* cameraDepthRenderTexture;
	RenderTexture* pointL1ShadowRenderTexture[6];
	RenderTexture* pointL2ShadowRenderTexture[6];
	//rendertextures for postprocessing
	RenderTexture* renderSceneRenderTexture;
	RenderTexture* horizontalBlurRenderTexture;
	RenderTexture* verticalBlurRenderTexture;
	RenderTexture* depthOfFieldRenderTexture;
	//rendertexture to show the scene in wireframe
	RenderTexture* wireframeSceneRenderTexture;

	//blur
	HorizontalBlurShader* horizontalBlurShader;
	VerticalBlurShader* verticalBlurShader;


	//Texture shader
	TextureShader* textureShader;
	//Water
	WaterShader* waterShader;
	//Shadows
	ShadowShader* shadowShader;
	//Depth
	DepthShader* depthShader;
	DepthOfFieldShader* depthOfFieldShader;
	//Tesselated
	TesselatedPlane* tesselatedLandscapePlane;
	TesselatedPlane* tesselatedWaterPlane;
	TesselatedPlane* pictureframe1Plane;
	TesselatedPlane* pictureframe2Plane;
	TesselatedPlane* pictureframe3Plane;
	TessellationShader* tesselationShader;
	TessShadowShader2* tessShadowShader2;
	TesselationDepthShader* tesselationDepthShader;
	TesselationGeometryDepthShader* geometryDepthShader;
	TessellatedPlaneWithGeometry* tessellatedPlaneWithGeometryShader;

	//Lights
	Light* lights[4];

	//sphere meshes used in the wireframerendertexture to show you the position of the point lights
	SphereMesh* pointLight1ShereMesh;
	SphereMesh* pointLight2ShereMesh;

	//OrthoMeshes
	OrthoMesh* orthoMesh[13]; //for visually displaying my 2 directional, 2 point light and camera depth maps
	OrthoMesh* depthMesh; //used for horizontal and vertical blurring

	//Models used in scene
	Model* SceneBox;
	Model* pictureFrame;
	XMFLOAT3 modelPosition;
	XMFLOAT3 pictureFramePosition;

};

#endif