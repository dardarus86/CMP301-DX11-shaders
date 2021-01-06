//Jamie Haddow CMP301 coursework
//COMMENTING DONE

#include "App1.h"

App1::App1()
{
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Initalise scene variables.
	LightInit();
	GeneralInits(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	SceneBox = nullptr;
	pictureFrame = nullptr;
	pictureframe1Plane = nullptr;
	pictureframe2Plane = nullptr;
	pictureframe3Plane = nullptr;
	tesselatedLandscapePlane = nullptr;
	tesselatedWaterPlane = nullptr;
	pointLight1ShereMesh = nullptr;
	pointLight2ShereMesh = nullptr;
	depthMesh = nullptr;
	directionalShadowRenderTexture[dLight1] = nullptr;
	directionalShadowRenderTexture[dLight2] = nullptr;
	cameraDepthRenderTexture = nullptr;
	for (int i = 0; i < 12; i++) { orthoMesh[i] = nullptr; }
	for (int i = 0; i < 6; i++) {pointL1ShadowRenderTexture[i] = nullptr;	}
	for (int i = 0; i < 6; i++) {pointL2ShadowRenderTexture[i] = nullptr;	}
	renderSceneRenderTexture = nullptr;
	horizontalBlurRenderTexture = nullptr;
	verticalBlurRenderTexture = nullptr;
	depthOfFieldRenderTexture = nullptr;
	wireframeSceneRenderTexture = nullptr;
	tesselationShader = nullptr;
	tesselationDepthShader = nullptr;
	tessShadowShader2 = nullptr;
	tessellatedPlaneWithGeometryShader = nullptr;
	horizontalBlurShader = nullptr;
	verticalBlurShader = nullptr;
	depthOfFieldShader = nullptr;
	geometryDepthShader = nullptr;
	waterShader = nullptr;
	textureShader = nullptr;
	shadowShader = nullptr;
	depthShader = nullptr;
	
}

bool App1::frame()
{
	bool result;
	
	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}


	return true;
}

bool App1::render()
{
	LightUpdate();
	pLightRotation += timer->getTime();
	pLightRotation2 += timer->getTime();
	waterTimer += timer->getTime();
	grassTime += timer->getTime();

	//just incase the sceensize does end up getting changed after init
	vertScreenWidth = verticalBlurRenderTexture->getTextureWidth();
	horizScreenWidth = verticalBlurRenderTexture->getTextureWidth();

	camera->update();

	//main function passes
	CameraDepthPass();
	FirstDirectionalShadowPass();
	SecondDirectionalShadowPass();
	FirstPointShadowPass();
	SecondPointShadowPass();
    RenderScene();
	WireFrameRenderScene();
	HorizontalBlurPass();
	VerticalBlurPass();
	SecondHorizontalBlurPass();
	SecondVerticalBlurPass();
	DoFPass();
	finalPass();

	gui();
	renderer->endScene();

	return true;
}

//used to get the camera depth map to be used in later stages
void App1::CameraDepthPass()
{
	cameraDepthRenderTexture->setRenderTarget(renderer->getDeviceContext());
	cameraDepthRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

//BOX AROUND SCENE
	XMMATRIX translate = XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
	SceneBox->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translate, viewMatrix, projectionMatrix);
	depthShader->render(renderer->getDeviceContext(), SceneBox->getIndexCount());

//PICTURE FRAMES
	XMMATRIX pictureFrameTranslate = XMMatrixTranslation(24.0f, 10.0f, 52.0f);
	pictureFrame->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * pictureFrameTranslate, viewMatrix, projectionMatrix);
	depthShader->render(renderer->getDeviceContext(), pictureFrame->getIndexCount());

//TERRAIN
	XMMATRIX terrainTranslation = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	tesselatedLandscapePlane->sendData(renderer->getDeviceContext());
	tesselationDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix* terrainTranslation, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMap()), XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z), terrainTesselationFactor, terrainDisplacementHeight);
	tesselationDepthShader->render(renderer->getDeviceContext(), tesselatedLandscapePlane->getIndexCount());

	tesselatedLandscapePlane->sendData(renderer->getDeviceContext());
	geometryDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMap()),
		textureMgr->getTexture(L"noiseMap"), textureMgr->getTexture(ChangeHeightMap()), textureMgr->getTexture(L"windMap"),
		camera, terrainTesselationFactor, terrainDisplacementHeight, grassTime, numOfGrassObjects, grassAmplitude, grassFrequency, grassHeight, grassWidth);
	geometryDepthShader->render(renderer->getDeviceContext(), tesselatedLandscapePlane->getIndexCount());

//WATER
	renderer->setAlphaBlending(true);
	XMMATRIX waterTranslation = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
	tesselatedWaterPlane->sendData(renderer->getDeviceContext());
	tesselationDepthShader->render(renderer->getDeviceContext(), tesselatedWaterPlane->getIndexCount());
	renderer->setAlphaBlending(false);

	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();

}
//the first direction depth pass using a render texture to store the depth information of all the scene objects
void App1::FirstDirectionalShadowPass()
{
	directionalShadowRenderTexture[dLight1]->setRenderTarget(renderer->getDeviceContext());
	directionalShadowRenderTexture[dLight1]->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);

	lights[dLight1]->generateViewMatrix();
	XMMATRIX viewMatrix = lights[dLight1]->getViewMatrix();
	XMMATRIX projectionMatrix = lights[dLight1]->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

//BOX
	XMMATRIX translate = XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
	SceneBox->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translate, viewMatrix, projectionMatrix);
	depthShader->render(renderer->getDeviceContext(), SceneBox->getIndexCount());

//PICTURE FRAMES
	XMMATRIX pictureFrameTranslate = XMMatrixTranslation(24.0f, 10.0f, 52.0f);
	pictureFrame->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * pictureFrameTranslate, viewMatrix, projectionMatrix);
	depthShader->render(renderer->getDeviceContext(), pictureFrame->getIndexCount());

//TERRAIN
	XMMATRIX terrainTranslation = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	tesselatedLandscapePlane->sendData(renderer->getDeviceContext());
	tesselationDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix* terrainTranslation, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMap()), XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z), terrainTesselationFactor, terrainDisplacementHeight);
	tesselationDepthShader->render(renderer->getDeviceContext(), tesselatedLandscapePlane->getIndexCount());

//WATER
	renderer->setAlphaBlending(true);
	XMMATRIX waterTranslation = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
	tesselatedWaterPlane->sendData(renderer->getDeviceContext());                       
	tesselationDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMap()), XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z), terrainTesselationFactor, terrainDisplacementHeight);
	tesselationDepthShader->render(renderer->getDeviceContext(), tesselatedWaterPlane->getIndexCount());
	renderer->setAlphaBlending(false);

	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}
//the second direction depth pass using a render texture to store the depth information of all the scene objects
void App1::SecondDirectionalShadowPass()
{
	directionalShadowRenderTexture[dLight2]->setRenderTarget(renderer->getDeviceContext());
	directionalShadowRenderTexture[dLight2]->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);

	lights[dLight2]->generateViewMatrix();
	XMMATRIX viewMatrix = lights[dLight2]->getViewMatrix();
	XMMATRIX projectionMatrix = lights[dLight2]->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

//BOX
	XMMATRIX translate = XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
	SceneBox->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translate, viewMatrix, projectionMatrix);
	depthShader->render(renderer->getDeviceContext(), SceneBox->getIndexCount());

//PICTURE FRAMES
	XMMATRIX pictureFrameTranslate = XMMatrixTranslation(24.0f, 10.0f, 52.0f);
	pictureFrame->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * pictureFrameTranslate, viewMatrix, projectionMatrix);
	depthShader->render(renderer->getDeviceContext(), pictureFrame->getIndexCount());

//TERRAIN
	XMMATRIX terrainTranslation = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	tesselatedLandscapePlane->sendData(renderer->getDeviceContext());
	tesselationDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix* terrainTranslation, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMap()), XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z), terrainTesselationFactor, terrainDisplacementHeight);
	tesselationDepthShader->render(renderer->getDeviceContext(), tesselatedLandscapePlane->getIndexCount());

//WATER
	renderer->setAlphaBlending(true);
	XMMATRIX waterTranslation = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
	tesselatedWaterPlane->sendData(renderer->getDeviceContext()); 
	tesselationDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMap()), XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z), terrainTesselationFactor, terrainDisplacementHeight);
	tesselationDepthShader->render(renderer->getDeviceContext(), tesselatedWaterPlane->getIndexCount());
	renderer->setAlphaBlending(false);

	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}
//the first point light depth pass using 6 render textures to store the depth information of all the scene objects
void App1::FirstPointShadowPass()
{

	XMFLOAT3 directions[6] = 
	{
		XMFLOAT3(0.0f ,1.0, 0.0f),
		XMFLOAT3(0.0f , -1.0f, 0.0f),
		XMFLOAT3(1,0.0f,0),
		XMFLOAT3(-1,0.0f,0),
		XMFLOAT3(0,0.0f,1),	
		XMFLOAT3(0,0.0f,-1) 
	};

	for (int i = 0; i < 6; i++) 
	{
		pointL1ShadowRenderTexture[i]->setRenderTarget(renderer->getDeviceContext());
		pointL1ShadowRenderTexture[i]->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);
		lights[pLight1]->setDirection(directions[i].x, directions[i].y, directions[i].z);
		lights[pLight1]->generateViewMatrix();

		XMMATRIX viewMatrix = lights[pLight1]->getViewMatrix();
		XMMATRIX projectionMatrix = lights[pLight1]->getProjectionMatrix();
		XMMATRIX worldMatrix = renderer->getWorldMatrix();

//BOX
		XMMATRIX translate = XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
		SceneBox->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translate, viewMatrix, projectionMatrix);
		depthShader->render(renderer->getDeviceContext(), SceneBox->getIndexCount());

//PICTURE FRAMES
		XMMATRIX pictureFrameTranslate = XMMatrixTranslation(24.0f, 10.0f, 52.0f);
		pictureFrame->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * pictureFrameTranslate, viewMatrix, projectionMatrix);
		depthShader->render(renderer->getDeviceContext(), pictureFrame->getIndexCount());

//TERRAIN
		XMMATRIX terrainTranslation = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
		tesselatedLandscapePlane->sendData(renderer->getDeviceContext());
		tesselationDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix* terrainTranslation, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMap()), XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z), terrainTesselationFactor, terrainDisplacementHeight);
		tesselationDepthShader->render(renderer->getDeviceContext(), tesselatedLandscapePlane->getIndexCount());


//WATER
		renderer->setAlphaBlending(true);
		XMMATRIX waterTranslation = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
		tesselatedWaterPlane->sendData(renderer->getDeviceContext()); //     ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap, float tesselationFactor, float timer, float speed, float amplitude, float frequency, XMFLOAT4 colour1, XMFLOAT4 colour2, XMFLOAT3 camerPos);                                         
		tesselationDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMap()), XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z), terrainTesselationFactor, terrainDisplacementHeight);
		tesselationDepthShader->render(renderer->getDeviceContext(), tesselatedWaterPlane->getIndexCount());
		renderer->setAlphaBlending(false);

		renderer->setBackBufferRenderTarget();
		renderer->resetViewport();
	}
}
//the second point light depth pass using 6 render textures to store the depth information of all the scene objects
void App1::SecondPointShadowPass()
{
	XMFLOAT3 directions[6] = {
	XMFLOAT3(0.0f ,1.0, 0.0f),XMFLOAT3(0.0f , -1.0f, 0.0f),	XMFLOAT3(1,0.0f,0),XMFLOAT3(-1,0.0f,0),	XMFLOAT3(0,0.0f,1),	XMFLOAT3(0,0.0f,-1) };

	for (int i = 0; i < 6; i++) 
	{
		pointL2ShadowRenderTexture[i]->setRenderTarget(renderer->getDeviceContext());
		pointL2ShadowRenderTexture[i]->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);
		lights[pLight2]->setDirection(directions[i].x, directions[i].y, directions[i].z);
		lights[pLight2]->generateViewMatrix();

		XMMATRIX viewMatrix = lights[pLight2]->getViewMatrix();
		XMMATRIX projectionMatrix = lights[pLight2]->getProjectionMatrix();
		XMMATRIX worldMatrix = renderer->getWorldMatrix();

//BOX
		XMMATRIX translate = XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
		SceneBox->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translate, viewMatrix, projectionMatrix);
		depthShader->render(renderer->getDeviceContext(), SceneBox->getIndexCount());

//PICTURE FRAMES
		XMMATRIX pictureFrameTranslate = XMMatrixTranslation(24.0f, 10.0f, 52.0f);
		pictureFrame->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * pictureFrameTranslate, viewMatrix, projectionMatrix);
		depthShader->render(renderer->getDeviceContext(), pictureFrame->getIndexCount());

//TERRAIN
		XMMATRIX terrainTranslation = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
		tesselatedLandscapePlane->sendData(renderer->getDeviceContext());
		tesselationDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix* terrainTranslation, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMap()), XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z), terrainTesselationFactor, terrainDisplacementHeight);
		tesselationDepthShader->render(renderer->getDeviceContext(), tesselatedLandscapePlane->getIndexCount());

//WATER
		renderer->setAlphaBlending(true);
		XMMATRIX waterTranslation = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
		tesselatedWaterPlane->sendData(renderer->getDeviceContext());                                      
		tesselationDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMap()), XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z), waterTesselationFactor, terrainDisplacementHeight);
		tesselationDepthShader->render(renderer->getDeviceContext(), tesselatedWaterPlane->getIndexCount());
		renderer->setAlphaBlending(false);

		renderer->setBackBufferRenderTarget();
		renderer->resetViewport();
	}
}
//this function used a render texture to snapshot the sceneto then be used in the depth of field shader later
void App1::RenderScene()
{
	renderSceneRenderTexture->setRenderTarget(renderer->getDeviceContext());
	renderSceneRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.1f, 0.1f, 0.1f, 1.0f);

	//setting the shaderresource view for use in the shaders by asssigning them the data from the point lights render texture
	ID3D11ShaderResourceView* pointLightDirections[6];
	for (int i = 0; i < 6; i++) {pointLightDirections[i] = pointL1ShadowRenderTexture[i]->getShaderResourceView();}
	ID3D11ShaderResourceView* pointLightDirections2[6];
	for (int i = 0; i < 6; i++) {pointLightDirections2[i] = pointL2ShadowRenderTexture[i]->getShaderResourceView();	}

	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	
	XMMATRIX translateBox = XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
	SceneBox->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translateBox, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"),
									  directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(),
								      pointLightDirections, pointLightDirections2, lights,attenuation);
	shadowShader->render(renderer->getDeviceContext(), SceneBox->getIndexCount());
	
//PICTURE FRAMES
	XMMATRIX pictureFrameTranslate = XMMatrixTranslation(24.0f, 10.0f, 52.0f);
	pictureFrame->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix* pictureFrameTranslate, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"),
								      directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(),
								      pointLightDirections, pointLightDirections2, lights, attenuation);
	shadowShader->render(renderer->getDeviceContext(), pictureFrame->getIndexCount());

//HEIGHTMAP FOR PICTURE 1
	XMMATRIX pictureFrame1planeTranslate = XMMatrixTranslation(pictureframe1Pos.x, pictureframe1Pos.y, pictureframe1Pos.z);
	XMMATRIX pictureFrame1planeRotateX = XMMatrixRotationX(1.57);
	XMMATRIX pictureFrame1planeRotateY = XMMatrixRotationY(3.15);
	XMMATRIX rotations1 = pictureFrame1planeRotateX * pictureFrame1planeRotateY;
	XMMATRIX combined1 = rotations1 * pictureFrame1planeTranslate;
	pictureframe1Plane->sendData(renderer->getDeviceContext());
	tessShadowShader2->setShaderParameters(renderer->getDeviceContext(), worldMatrix* combined1, viewMatrix, projectionMatrix, textureMgr->getTexture(L"paul"), textureMgr->getTexture(L"paulheight"),
										   directionalShadowRenderTexture[dLight1]->getShaderResourceView(),directionalShadowRenderTexture[dLight2]->getShaderResourceView(), pointLightDirections,
										   pointLightDirections2,picture1TessFactor, picture1displacement, lights, attenuation, camera,normalStrength);
	tessShadowShader2->render(renderer->getDeviceContext(), pictureframe1Plane->getIndexCount());

//HEIGHTMAP FOR PICTURE 2
	XMMATRIX pictureFrame2planeTranslate = XMMatrixTranslation(pictureframe2Pos.x, pictureframe2Pos.y, pictureframe2Pos.z);
	XMMATRIX pictureFrame2planeRotateX = XMMatrixRotationX(1.57);
	XMMATRIX pictureFrame2planeRotateY = XMMatrixRotationY(3.15);
	XMMATRIX rotations2 = pictureFrame2planeRotateX * pictureFrame2planeRotateY;
	XMMATRIX combined2 = rotations2 * pictureFrame2planeTranslate;
	pictureframe2Plane->sendData(renderer->getDeviceContext());
	tessShadowShader2->setShaderParameters(renderer->getDeviceContext(), worldMatrix * combined2, viewMatrix, projectionMatrix, textureMgr->getTexture(L"middle"), textureMgr->getTexture(L"middle"),
										   directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(), pointLightDirections,
									       pointLightDirections2, picture2TessFactor, picture2displacement, lights, attenuation,camera, normalStrength);
	tessShadowShader2->render(renderer->getDeviceContext(), pictureframe2Plane->getIndexCount());

//HEIGHTMAP FOR PICTURE 3
	XMMATRIX pictureFrame3planeTranslate = XMMatrixTranslation(pictureframe3Pos.x, pictureframe3Pos.y, pictureframe3Pos.z);
	XMMATRIX pictureFrame3planeRotateX = XMMatrixRotationX(1.57);
	XMMATRIX pictureFrame3planeRotateY = XMMatrixRotationY(3.15);
	XMMATRIX rotations3 = pictureFrame3planeRotateX * pictureFrame3planeRotateY;
	XMMATRIX combined3 = rotations3 * pictureFrame3planeTranslate;
	pictureframe3Plane->sendData(renderer->getDeviceContext());
	tessShadowShader2->setShaderParameters(renderer->getDeviceContext(), worldMatrix * combined3, viewMatrix, projectionMatrix, textureMgr->getTexture(L"jacks"), textureMgr->getTexture(L"jacksheight"),
										   directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(), pointLightDirections,
		                                   pointLightDirections2, picture3TessFactor, picture3displacement, lights, attenuation, camera, normalStrength);
	tessShadowShader2->render(renderer->getDeviceContext(), pictureframe3Plane->getIndexCount());


//TERRAIN
	XMMATRIX terrainTranslation = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	tesselatedLandscapePlane->sendData(renderer->getDeviceContext());
	tessShadowShader2->setShaderParameters(renderer->getDeviceContext(), worldMatrix* terrainTranslation, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMapTexture()), textureMgr->getTexture(ChangeHeightMap()),
										   directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(), pointLightDirections,pointLightDirections2,
										   terrainTesselationFactor, terrainDisplacementHeight, lights, attenuation, camera, normalStrength);
	tessShadowShader2->render(renderer->getDeviceContext(), tesselatedLandscapePlane->getIndexCount());

//GRASS GEOMETRY
	tesselatedLandscapePlane->sendData(renderer->getDeviceContext());
	tessellatedPlaneWithGeometryShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMapTexture()), textureMgr->getTexture(ChangeHeightMap()),
															textureMgr->getTexture(L"noiseMap2"), textureMgr->getTexture(ChangeHeightMap()), textureMgr->getTexture(L"windMap"), textureMgr->getTexture(L"grass"),
															camera, terrainTesselationFactor,terrainDisplacementHeight, grassTime, numOfGrassObjects, grassAmplitude, grassFrequency, grassHeight, grassWidth, 
														    grassColour1, grassColour2);
	tessellatedPlaneWithGeometryShader->render(renderer->getDeviceContext(), tesselatedLandscapePlane->getIndexCount());
	
//WATER
	renderer->setAlphaBlending(true);
	//main water
	XMMATRIX waterTranslation = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
	tesselatedWaterPlane->sendData(renderer->getDeviceContext());                                      
	waterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * waterTranslation, viewMatrix, projectionMatrix, textureMgr->getTexture(L"water"), textureMgr->getTexture(ChangeHeightMap()), textureMgr->getTexture(L"noiseMap2"), waterTesselationFactor, terrainDisplacementHeight, waterTimer, waterSpeed, waterAmplitude, waterFrequency, waterColour1, waterColour2, XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z));
	waterShader->render(renderer->getDeviceContext(), tesselatedWaterPlane->getIndexCount());
	renderer->setAlphaBlending(false);

	renderer->setBackBufferRenderTarget();
}
//visual function using another render texture that then is displayed on an ortho mesh in the top middle of the screen.
void App1::WireFrameRenderScene()
{
	wireframeSceneRenderTexture->setRenderTarget(renderer->getDeviceContext());
	wireframeSceneRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);

	ID3D11ShaderResourceView* pointLightDirections[6];
	for (int i = 0; i < 6; i++) {pointLightDirections[i] = pointL1ShadowRenderTexture[i]->getShaderResourceView();}
	ID3D11ShaderResourceView* pointLightDirections2[6];
	for (int i = 0; i < 6; i++) {pointLightDirections2[i] = pointL2ShadowRenderTexture[i]->getShaderResourceView();}

	renderer->setWireframeMode(true);
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

//BOX
	XMMATRIX translateBox = XMMatrixTranslation(modelPosition.x, modelPosition.y, modelPosition.z);
	SceneBox->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translateBox, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"),
							          directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(),
							          pointLightDirections, pointLightDirections2, lights, attenuation);
	shadowShader->render(renderer->getDeviceContext(), SceneBox->getIndexCount());

//PICTURE FRAMES
	XMMATRIX pictureFrameTranslate = XMMatrixTranslation(24.0f, 10.0f, 52.0f);
	pictureFrame->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * pictureFrameTranslate, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"),
								      directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(),
								      pointLightDirections, pointLightDirections2, lights, attenuation);
	shadowShader->render(renderer->getDeviceContext(), pictureFrame->getIndexCount());

//HEIGHTMAP FOR PICTURE 1
	XMMATRIX pictureFrame1planeTranslate = XMMatrixTranslation(pictureframe1Pos.x, pictureframe1Pos.y, pictureframe1Pos.z);
	XMMATRIX pictureFrame1planeRotateX = XMMatrixRotationX(1.57);
	XMMATRIX pictureFrame1planeRotateY = XMMatrixRotationY(3.15);
	XMMATRIX rotations1 = pictureFrame1planeRotateX * pictureFrame1planeRotateY;
	XMMATRIX combined1 = rotations1 * pictureFrame1planeTranslate;
	pictureframe1Plane->sendData(renderer->getDeviceContext());
	tessShadowShader2->setShaderParameters(renderer->getDeviceContext(), worldMatrix * combined1, viewMatrix, projectionMatrix, textureMgr->getTexture(L"paul"), textureMgr->getTexture(L"paulheight"),
		                                   directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(), pointLightDirections,
		                                   pointLightDirections2, picture1TessFactor, picture1displacement, lights, attenuation, camera, normalStrength);
	tessShadowShader2->render(renderer->getDeviceContext(), pictureframe1Plane->getIndexCount());

//HEIGHTMAP FOR PICTURE 2
	XMMATRIX pictureFrame2planeTranslate = XMMatrixTranslation(pictureframe2Pos.x, pictureframe2Pos.y, pictureframe2Pos.z);
	XMMATRIX pictureFrame2planeRotateX = XMMatrixRotationX(1.57);
	XMMATRIX pictureFrame2planeRotateY = XMMatrixRotationY(3.15);
	XMMATRIX rotations2 = pictureFrame2planeRotateX * pictureFrame2planeRotateY;
	XMMATRIX combined2 = rotations2 * pictureFrame2planeTranslate;
	pictureframe2Plane->sendData(renderer->getDeviceContext());
	tessShadowShader2->setShaderParameters(renderer->getDeviceContext(), worldMatrix * combined2, viewMatrix, projectionMatrix, textureMgr->getTexture(L"middle"), textureMgr->getTexture(L"middle"),
		                                   directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(), pointLightDirections,
		                                   pointLightDirections2, picture2TessFactor, picture2displacement, lights, attenuation, camera, normalStrength);
	tessShadowShader2->render(renderer->getDeviceContext(), pictureframe2Plane->getIndexCount());

//HEIGHTMAP FOR PICTURE 3
	XMMATRIX pictureFrame3planeTranslate = XMMatrixTranslation(pictureframe3Pos.x, pictureframe3Pos.y, pictureframe3Pos.z);
	XMMATRIX pictureFrame3planeRotateX = XMMatrixRotationX(1.57);
	XMMATRIX pictureFrame3planeRotateY = XMMatrixRotationY(3.15);
	XMMATRIX rotations3 = pictureFrame3planeRotateX * pictureFrame3planeRotateY;
	XMMATRIX combined3 = rotations3 * pictureFrame3planeTranslate;
	pictureframe3Plane->sendData(renderer->getDeviceContext());
	tessShadowShader2->setShaderParameters(renderer->getDeviceContext(), worldMatrix * combined3, viewMatrix, projectionMatrix, textureMgr->getTexture(L"jacks"), textureMgr->getTexture(L"jacksheight"),
	                                       directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(), pointLightDirections,
	                                       pointLightDirections2, picture3TessFactor, picture3displacement, lights, attenuation, camera, normalStrength);
	tessShadowShader2->render(renderer->getDeviceContext(), pictureframe3Plane->getIndexCount());

//TERRAIN
	XMMATRIX terrainTranslation = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	tesselatedLandscapePlane->sendData(renderer->getDeviceContext());
	tessShadowShader2->setShaderParameters(renderer->getDeviceContext(), worldMatrix * terrainTranslation, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMapTexture()), textureMgr->getTexture(ChangeHeightMap()),
		                                   directionalShadowRenderTexture[dLight1]->getShaderResourceView(), directionalShadowRenderTexture[dLight2]->getShaderResourceView(), pointLightDirections,
		                                   pointLightDirections2, terrainTesselationFactor, terrainDisplacementHeight, lights, attenuation, camera, normalStrength);
	tessShadowShader2->render(renderer->getDeviceContext(), tesselatedLandscapePlane->getIndexCount());

//GRASS GEOMETRY
	tesselatedLandscapePlane->sendData(renderer->getDeviceContext());
	tessellatedPlaneWithGeometryShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(ChangeHeightMapTexture()), textureMgr->getTexture(ChangeHeightMap()),
		                                textureMgr->getTexture(L"noiseMap2"), textureMgr->getTexture(ChangeHeightMap()), textureMgr->getTexture(L"windMap"), textureMgr->getTexture(L"grass"), camera, terrainTesselationFactor,
		                                terrainDisplacementHeight, grassTime, numOfGrassObjects, grassAmplitude, grassFrequency, grassHeight, grassWidth, grassColour1, grassColour2);
	tessellatedPlaneWithGeometryShader->render(renderer->getDeviceContext(), tesselatedLandscapePlane->getIndexCount());

//WATER
	//renderer->setAlphaBlending(true);
	//XMMATRIX waterTranslation = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
	//tesselatedWaterPlane->sendData(renderer->getDeviceContext()); //     ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap, float tesselationFactor, float timer, float speed, float amplitude, float frequency, XMFLOAT4 colour1, XMFLOAT4 colour2, XMFLOAT3 camerPos);                                         
	//waterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * waterTranslation, viewMatrix, projectionMatrix, textureMgr->getTexture(L"water"), textureMgr->getTexture(ChangeHeightMap()),
	//								 textureMgr->getTexture(L"noiseMap2"), waterTesselationFactor, terrainDisplacementHeight, waterTimer, waterSpeed, waterAmplitude, waterFrequency, waterColour1, waterColour2, XMFLOAT3(camera->getPosition().x,
	//							     camera->getPosition().y, camera->getPosition().z));
	//waterShader->render(renderer->getDeviceContext(), tesselatedWaterPlane->getIndexCount());
	//renderer->setAlphaBlending(false);

//2 SPHERES RESPRESENT POINT LIGHT POSITIONS
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(lights[pLight1]->getPosition().x, lights[pLight1]->getPosition().y, lights[pLight1]->getPosition().z);
	pointLight1ShereMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"ground"));
	textureShader->render(renderer->getDeviceContext(), pointLight1ShereMesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(lights[pLight2]->getPosition().x, lights[pLight2]->getPosition().y, lights[pLight2]->getPosition().z);
	pointLight2ShereMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"ground"));
	textureShader->render(renderer->getDeviceContext(), pointLight2ShereMesh->getIndexCount());
	renderer->setWireframeMode(false);

	renderer->setBackBufferRenderTarget();
}
//horizontal blur pass on the renderscene render texture depth texture
void App1::HorizontalBlurPass()
{
	horizontalBlurRenderTexture->setRenderTarget(renderer->getDeviceContext());
	horizontalBlurRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.02f, 0.02f, 0.02f, 1.0f);

	XMMATRIX worldMatrix, ViewMatrix, orthoMatrix;

	worldMatrix = renderer->getWorldMatrix();
	ViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = horizontalBlurRenderTexture->getOrthoMatrix();

	renderer->setZBuffer(false);
	depthMesh->sendData(renderer->getDeviceContext());
	horizontalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, ViewMatrix, orthoMatrix, renderSceneRenderTexture->getShaderResourceView(), vertScreenWidth, horizStrength);
	horizontalBlurShader->render(renderer->getDeviceContext(), depthMesh->getIndexCount());
	renderer->setZBuffer(true);

	renderer->setBackBufferRenderTarget();
}
//horizontal blur pass on the horizontal render texture depth texture
void App1::VerticalBlurPass()
{

	verticalBlurRenderTexture->setRenderTarget(renderer->getDeviceContext());
	verticalBlurRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.02f, 0.02f, 0.02f, 1.0f);

	XMMATRIX worldMatrix, ViewMatrix, orthoMatrix;
	worldMatrix = renderer->getWorldMatrix();
	ViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = verticalBlurRenderTexture->getOrthoMatrix();

	renderer->setZBuffer(false);
	depthMesh->sendData(renderer->getDeviceContext());
	verticalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, ViewMatrix, orthoMatrix, horizontalBlurRenderTexture->getShaderResourceView(), horizScreenWidth,vertStrength);
	verticalBlurShader->render(renderer->getDeviceContext(), depthMesh->getIndexCount());
	renderer->setZBuffer(true);

	renderer->setBackBufferRenderTarget();
}
//second blur pass to double the blur strength
void App1::SecondHorizontalBlurPass()
{
	horizontalBlurRenderTexture->setRenderTarget(renderer->getDeviceContext());
	horizontalBlurRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.02f, 0.02f, 0.02f, 1.0f);

	XMMATRIX worldMatrix, ViewMatrix, orthoMatrix;

	worldMatrix = renderer->getWorldMatrix();
	ViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = horizontalBlurRenderTexture->getOrthoMatrix();

	renderer->setZBuffer(false);
	depthMesh->sendData(renderer->getDeviceContext());
	horizontalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, ViewMatrix, orthoMatrix, verticalBlurRenderTexture->getShaderResourceView(), vertScreenWidth, horizStrength);
	horizontalBlurShader->render(renderer->getDeviceContext(), depthMesh->getIndexCount());
	renderer->setZBuffer(true);

	renderer->setBackBufferRenderTarget();
}
//second blur pass to double the blur strength
void App1::SecondVerticalBlurPass()
{
	verticalBlurRenderTexture->setRenderTarget(renderer->getDeviceContext());
	verticalBlurRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.02f, 0.02f, 0.02f, 1.0f);

	XMMATRIX worldMatrix, ViewMatrix, orthoMatrix;
	worldMatrix = renderer->getWorldMatrix();
	ViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = verticalBlurRenderTexture->getOrthoMatrix();

	renderer->setZBuffer(false);
	depthMesh->sendData(renderer->getDeviceContext());
	verticalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, ViewMatrix, orthoMatrix, horizontalBlurRenderTexture->getShaderResourceView(), horizScreenWidth, vertStrength);
	verticalBlurShader->render(renderer->getDeviceContext(), depthMesh->getIndexCount());
	renderer->setZBuffer(true);

	renderer->setBackBufferRenderTarget();
}
//previous rendertextures, renderSceneRenderTexture, verticalBlurRenderTexture, cameraDepthRenderTexture and are used in this function inside the depth of field shader
void App1::DoFPass()
{
	depthOfFieldRenderTexture->setRenderTarget(renderer->getDeviceContext());
	depthOfFieldRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.5f, 0.5f, 0.5f, 1.0f);

	XMMATRIX worldMatrix, ViewMatrix, orthoMatrix;
	worldMatrix = renderer->getWorldMatrix();
	ViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = depthOfFieldRenderTexture->getOrthoMatrix();

	renderer->setZBuffer(false);
	depthMesh->sendData(renderer->getDeviceContext());
	depthOfFieldShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, ViewMatrix, orthoMatrix, renderSceneRenderTexture->getShaderResourceView(), verticalBlurRenderTexture->getShaderResourceView(),cameraDepthRenderTexture->getShaderResourceView(), SCREEN_NEAR, SCREEN_DEPTH, dofDistance, dofOffset);
	depthOfFieldShader->render(renderer->getDeviceContext(), depthMesh->getIndexCount());
	renderer->setZBuffer(true);
	renderer->setBackBufferRenderTarget();
}
//final pass used to display the depthOfFieldRenderTexture and also display the 13 orthomeshes for scene information
void App1::finalPass()
{
	renderer->setBackBufferRenderTarget();
	renderer->beginScene(0.1f, 0.1f, 0.1f, 1.0f);

	renderer->setZBuffer(false);
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix(); 
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();
	XMMATRIX worldMatrix1 = renderer->getWorldMatrix();

	depthMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix1, orthoViewMatrix, orthoMatrix, depthOfFieldRenderTexture->getShaderResourceView());
	textureShader->render(renderer->getDeviceContext(), depthMesh->getIndexCount());

//orthoMeshes used at the top of the screen
//ortho 0 and 6 render the directional light depth maps
//ortho 1,2,3,4,5,7,8,9,10,11 are used for the point light direction depth maps minus above as nothing is rendered
//ortho 12 is then used for showing the screen in wireframe
	for (int i = 0; i < 6; i++)
	{
		orthoMesh[i]->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix1, orthoViewMatrix, orthoMatrix, pointL1ShadowRenderTexture[i]->getShaderResourceView());
		textureShader->render(renderer->getDeviceContext(), orthoMesh[i]->getIndexCount());
	}
	for (int i = 6; i < 12; i++)
	{
		orthoMesh[i]->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix1, orthoViewMatrix, orthoMatrix, pointL2ShadowRenderTexture[i - 6]->getShaderResourceView());
		textureShader->render(renderer->getDeviceContext(), orthoMesh[i]->getIndexCount());
	}

	orthoMesh[0]->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix1, orthoViewMatrix, orthoMatrix, directionalShadowRenderTexture[0]->getShaderResourceView());
	textureShader->render(renderer->getDeviceContext(), orthoMesh[0]->getIndexCount());
	orthoMesh[6]->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix1, orthoViewMatrix, orthoMatrix, directionalShadowRenderTexture[1]->getShaderResourceView());
	textureShader->render(renderer->getDeviceContext(), orthoMesh[6]->getIndexCount());

	orthoMesh[12]->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix1, orthoViewMatrix, orthoMatrix, wireframeSceneRenderTexture->getShaderResourceView());
	textureShader->render(renderer->getDeviceContext(), orthoMesh[12]->getIndexCount());
	renderer->setZBuffer(true);
}
//function passed into the render function to be called every frame
void App1::LightUpdate()
{

	lights[dLight1]->setAmbientColour(lightAmbient[dLight1].x, lightAmbient[dLight1].y, lightAmbient[dLight1].z,1.0f);
	lights[dLight1]->setDiffuseColour(lightDiffuse[dLight1].x, lightDiffuse[dLight1].y, lightDiffuse[dLight1].z, 1.0f);
	lights[dLight1]->setDirection(lightDirection[dLight1].x, lightDirection[dLight1].y, lightDirection[dLight1].z);
	lights[dLight1]->setPosition(lightPosition[dLight1].x, lightPosition[dLight1].y, lightPosition[dLight1].z);
	lights[dLight2]->setAmbientColour(lightAmbient[dLight2].x, lightAmbient[dLight2].y, lightAmbient[dLight2].z, 1.0f);
	lights[dLight2]->setDiffuseColour(lightDiffuse[dLight2].x, lightDiffuse[dLight2].y, lightDiffuse[dLight2].z, 1.0f);
	lights[dLight2]->setDirection(lightDirection[dLight2].x, lightDirection[dLight2].y, lightDirection[dLight2].z);
	lights[dLight2]->setPosition(lightPosition[dLight2].x, lightPosition[dLight2].y, lightPosition[dLight2].z);
	//both point lights used the lemniscate of Gerono for movement in a figure 8 which is "x = cos(t)	y = sin(2 * t) / 2;"
	//t being the time being passed in along with a frequency to adjust its speed
	lights[pLight1]->setAmbientColour(lightAmbient[pLight1].x, lightAmbient[pLight1].y, lightAmbient[pLight1].z, 1.0f);
	lights[pLight1]->setDiffuseColour(lightDiffuse[pLight1].x, lightDiffuse[pLight1].y, lightDiffuse[pLight1].z, 1.0f);
	lights[pLight1]->setPosition(lightPosition[pLight1].x + pLightradius * cos(pLightRotation * pLightFrequency), lightPosition[pLight1].y, lightPosition[pLight1].z + pLightradius * sin(2*pLightRotation * pLightFrequency)/2);
	lights[pLight2]->setAmbientColour(lightAmbient[pLight2].x, lightAmbient[pLight2].y, lightAmbient[pLight2].z, 1.0f);
	lights[pLight2]->setDiffuseColour(lightDiffuse[pLight2].x, lightDiffuse[pLight2].y, lightDiffuse[pLight2].z, 1.0f);
	lights[pLight2]->setPosition(lightPosition[pLight2].x + pLightradius * cos(pLightRotation2 * pLightFrequency), lightPosition[pLight2].y, lightPosition[pLight2].z + pLightradius * sin(2*pLightRotation2 * pLightFrequency)/2);

	attenuation[0] = XMFLOAT3(attenuation[0].x, attenuation[0].y, attenuation[0].z);
	attenuation[1] = XMFLOAT3(attenuation[1].x, attenuation[1].y, attenuation[1].z);

	lights[dLight1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 10.0f, 100.0f);
	lights[dLight2]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 10.0f, 100.0f);
	lights[pLight1]->generateProjectionMatrix(5.0f, 100.0f);
	lights[pLight2]->generateProjectionMatrix(5.0f, 100.0f);
}

void App1::LightInit()
{
	for (int i = 0; i < 4; i++)
	{
		lights[i] = new Light;
	}
	sceneWidth = 500;
	sceneHeight = 500;
	//directional lights
	lightAmbient[dLight1] = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	lightDiffuse[dLight1] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lightPosition[dLight1] = XMFLOAT4(15, 9, -6, 1.0f);
	lightDirection[dLight1] = XMFLOAT4(0, 0.31, 0.22,1.0f);
	lightAmbient[dLight2] = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	lightDiffuse[dLight2] = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lightPosition[dLight2] = XMFLOAT4(-9, -30, 30, 1.0f);
	lightDirection[dLight2] = XMFLOAT4(0, 0.31,-0.22f, 1.0f);
	lights[dLight1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.0f);
	lights[dLight2]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.0f);

	//point lights
	lightDirection[pLight1] = XMFLOAT4(0,0,0,0);
	lightPosition[pLight1] = XMFLOAT4(11.0f, 14.0f, 21.0f, 1.0f);
	lightAmbient[pLight1] = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	lightDiffuse[pLight1] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	lightDirection[pLight2] = XMFLOAT4(0,0,0,0);
	lightPosition[pLight2] = XMFLOAT4(31.0f, 14.0f, 21.0, 1.0f);
	lightAmbient[pLight2] = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	lightDiffuse[pLight2] = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	
	lights[pLight1]->generateProjectionMatrix(0.01f, 100.0f);
	lights[pLight2]->generateProjectionMatrix(0.01f, 100.0f);

	attenuation[0] = XMFLOAT3(0.4f, 0.02, 0.0f);
	attenuation[1] = XMFLOAT3(0.4f, 0.02, 0.0f);

	pLightRotation = 0;
	pLightRotation2 = 180;
	pLightradius = 20.0;
	pLightFrequency = 1.0;
	normalStrength = 10;
}

void App1::GeneralInits(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
//mesh/objects/models
	modelPosition = XMFLOAT3(25.0f, 0.0f, 25.0f);

	SceneBox = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/models/levelbox.obj");
	pictureFrame = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/models/PictureFrame.obj");
	pictureframe1Plane = new TesselatedPlane(renderer->getDevice(), renderer->getDeviceContext(), 8);
	pictureframe2Plane = new TesselatedPlane(renderer->getDevice(), renderer->getDeviceContext(), 8);
	pictureframe3Plane = new TesselatedPlane(renderer->getDevice(), renderer->getDeviceContext(), 8);
	tesselatedLandscapePlane = new TesselatedPlane(renderer->getDevice(), renderer->getDeviceContext(), 50);
	tesselatedWaterPlane = new TesselatedPlane(renderer->getDevice(), renderer->getDeviceContext(), 50);
	pointLight1ShereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	pointLight2ShereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

    orthoMesh[0] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, -screenWidth / 2.2, screenHeight / 2.28);
    orthoMesh[1] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, -screenWidth / 3.04, screenHeight / 2.28);
    orthoMesh[2] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, -screenWidth / 4.93, screenHeight / 2.28);
    orthoMesh[3] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, -screenWidth / 2.2, screenHeight / 3.2);
    orthoMesh[4] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, -screenWidth / 3.04, screenHeight / 3.2);
    orthoMesh[5] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, -screenWidth / 4.93, screenHeight / 3.2);
    orthoMesh[6] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, screenWidth / 2.2, screenHeight / 2.28);
    orthoMesh[7] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, screenWidth / 3.04, screenHeight / 2.28);
    orthoMesh[8] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, screenWidth / 4.93, screenHeight / 2.28);
    orthoMesh[9] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, screenWidth / 2.2, screenHeight / 3.2);
    orthoMesh[10] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, screenWidth / 3.04, screenHeight / 3.2);
    orthoMesh[11] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 8, screenHeight / 8, screenWidth / 4.93, screenHeight / 3.2);
    orthoMesh[12] = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4.2, screenHeight / 4.2, screenWidth / 200, screenHeight / 2.7);
	depthMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);

//Render textures
	directionalShadowRenderTexture[dLight1] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	directionalShadowRenderTexture[dLight2] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	cameraDepthRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	for (int i = 0; i < 6; i++) {
		pointL1ShadowRenderTexture[i] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	}
	for (int i = 0; i < 6; i++) {
		pointL2ShadowRenderTexture[i] = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	}
	renderSceneRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	horizontalBlurRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	verticalBlurRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	depthOfFieldRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	wireframeSceneRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

//shader inits
	tesselationShader = new TessellationShader(renderer->getDevice(), hwnd);
	tesselationDepthShader = new TesselationDepthShader(renderer->getDevice(), hwnd);
	tessShadowShader2 = new TessShadowShader2(renderer->getDevice(), hwnd);
	tessellatedPlaneWithGeometryShader = new TessellatedPlaneWithGeometry(renderer->getDevice(), hwnd);
	horizontalBlurShader = new HorizontalBlurShader(renderer->getDevice(), hwnd);
	verticalBlurShader = new VerticalBlurShader(renderer->getDevice(), hwnd);
	depthOfFieldShader = new DepthOfFieldShader(renderer->getDevice(), hwnd);
	geometryDepthShader = new TesselationGeometryDepthShader(renderer->getDevice(), hwnd);
	waterShader = new WaterShader(renderer->getDevice(), hwnd);
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);

//ImGui bools
	bImguiLandScape = bImguiPictureframes = bImguiWater = bImguiGeometry = bImguiParticles = bImguiPointLights = bImguiDLights = bImguiBlur = bImguiGrass = false;


//terrain variables
	terrainDisplacementHeight = 20.0f;
	terrainTesselationFactor = 8.7f;
	heightmapChoice = heightmapTextureChoice = 1;
//grass variables
	numOfGrassObjects = 0.446f;
	grassTime = 0.0f;
	grassAmplitude = 0.207f;
	grassFrequency = 0.2f;
	grassHeight = 1.0f;
	grassWidth = 1.5f;
	grassColour1 = XMFLOAT4(0.46f, 0.57f, 0.0f, 1.0f);
	grassColour2 = XMFLOAT4(0.75f, 0.82f, 0.07f, 1.0f);
//water variables
	waterTimer = 1.0f;
	waterTesselationFactor = 0.0f;
	waterAmplitude = 0.116f;
	waterSpeed = 1.0f;
	waterFrequency = 1.0f;
	waterColour1 = XMFLOAT4(0.40f, 0.31f, 1.0f, 1.0f);
	waterColour2 = XMFLOAT4(0.27f, 0.36f, 0.55f, 1.0f);
//pictureframe variable
	picture1displacement = 0.0f;
	picture1TessFactor = 1.0f;
	picture2displacement = 0.0f;
	picture2TessFactor =1.0;
	picture3displacement = 0.0f;
	picture3TessFactor = 1.0f;
	pictureframe1Pos = XMFLOAT3(19.47f, 17.11f, 51.65f);
	pictureframe2Pos = XMFLOAT3(27.57f, 17.09f, 51.68);
	pictureframe3Pos = XMFLOAT3(35.53f, 17.09f, 51.68f);
//blur variables
	vertScreenWidth = verticalBlurRenderTexture->getTextureWidth();
	horizScreenWidth = verticalBlurRenderTexture->getTextureWidth();
	dofDistance = 1.0f;
	dofOffset = 0.0f;
	vertStrength = 1.0;
	horizStrength = 1.0;

//images
//water
	textureMgr->loadTexture(L"water", L"res/water/water.jpg");
//ground texture
	textureMgr->loadTexture(L"ground", L"res/ground/grass.jpg");
	textureMgr->loadTexture(L"ground1", L"res/ground/ground1.png");
	textureMgr->loadTexture(L"ground2", L"res/ground/ground2.png");
	textureMgr->loadTexture(L"ground3", L"res/ground/ground3.png");
	textureMgr->loadTexture(L"ground4", L"res/ground/lowheight1textured.png");
//heightmaps
	textureMgr->loadTexture(L"heightmap1", L"res/heightmap/mountainHeight.png");
	textureMgr->loadTexture(L"heightmap2", L"res/heightmap/ValleysHeight.png");
	textureMgr->loadTexture(L"heightmap3", L"res/heightmap/VolcanoHeight.jpg");
	textureMgr->loadTexture(L"heightmap4", L"res/heightmap/lowheight1.png");
	textureMgr->loadTexture(L"heightmap5", L"res/heightmap/lowheight2.png");
	textureMgr->loadTexture(L"heightmap6", L"res/heightmap/lowheight3.png");
	textureMgr->loadTexture(L"heightmap7", L"res/heightmap/volcano1.png");
//pictureframes
	textureMgr->loadTexture(L"paul", L"res/PaulTexture.png");
	textureMgr->loadTexture(L"paulheight", L"res/heightmap/PaulHeight2.png");
	textureMgr->loadTexture(L"middle", L"res/middle.png");
	textureMgr->loadTexture(L"middleimageheight", L"res/heightmap/middlepicture.jpg");
	textureMgr->loadTexture(L"jacks", L"res/jacks.jpg");
	textureMgr->loadTexture(L"jacksheight", L"res/heightmap/jacks.jpg");
//other
	textureMgr->loadTexture(L"wood", L"res/wood.png");
	textureMgr->loadTexture(L"windMap", L"res/windmap.jpg");
	textureMgr->loadTexture(L"noiseMap", L"res/noise.png");
	textureMgr->loadTexture(L"noiseMap2", L"res/noise2.jpg");
	textureMgr->loadTexture(L"grass", L"res/grass.png");
//camera
	camera->setPosition(-19.0f, 44.0f, 12.0f);
	camera->setRotation(35.0f, 70.0f, 0.0f);
}



void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Checkbox("LandScape", &bImguiLandScape);
	ImGui::Checkbox("Grass", &bImguiGrass);
	ImGui::Checkbox("Water", &bImguiWater);
	ImGui::Checkbox("Pictureframes", &bImguiPictureframes);
	ImGui::Checkbox("Point Lights", &bImguiPointLights);
	ImGui::Checkbox("Directional Lights", &bImguiDLights);
	ImGui::Checkbox("Blur", &bImguiBlur);

	ImGui::Text("CameraX: %.2f", camera->getPosition().x);
	ImGui::Text("CameraY: %.2f", camera->getPosition().y);
	ImGui::Text("CameraZ: %.2f", camera->getPosition().z);


	if (bImguiLandScape)
	{
		ImguiLandScape();
	}

	if (bImguiGrass)
	{
		ImguiGrass();
	}

	if (bImguiPictureframes)
	{
		ImguiPictureFrames();
	}

	if (bImguiWater)
	{
		ImguiLandWater();
	}

	if (bImguiPointLights)
	{
		ImguiPointLights();
	}

	if (bImguiDLights)
	{
		ImguiDLights();
	}

	if (bImguiBlur)
	{
		ImguiBlur();
	}



	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
void App1::ImguiLandScape()
{
	ImGui::Begin("Landscape Window", &bImguiLandScape);
	ImGui::SliderFloat("Tesselation", &terrainTesselationFactor, 1, 20);
	ImGui::SliderFloat("displacement", &terrainDisplacementHeight, -10, 64);
	ImGui::SliderInt("HeightMaps", &heightmapChoice, 0, 6);
	ImGui::SliderInt("Texture", &heightmapTextureChoice, 0, 4);
	ImGui::SliderFloat("Normal strength", &normalStrength, 0, 64);
	ImGui::SliderFloat("grass objects", &numOfGrassObjects, 0, 1);

	ImGui::End();
}
void App1::ImguiGrass()
{
	static float* aColour1[3] = { &grassColour1.x, &grassColour1.y, &grassColour1.z };
	static float* aColour2[3] = { &grassColour2.x, &grassColour2.y, &grassColour2.z };
	ImGui::Begin("Grass Window", &bImguiGrass);
	ImGui::SliderFloat("Speed", &grassAmplitude, 0, 0.4);
	ImGui::SliderFloat("Offset", &grassFrequency, 0.4,1.0);
	ImGui::SliderFloat("Height", &grassHeight, 0.3, 1.2);
	ImGui::SliderFloat("Width", &grassWidth, 0.3, 1.5);
	ImGui::ColorPicker3("Colour 1", *aColour1);
	ImGui::ColorPicker3("Colour 2", *aColour2);

	ImGui::End();

}
void App1::ImguiLandWater()
{
	static float* aColour1[3] = { &waterColour1.x, &waterColour1.y, &waterColour1.z };
	static float* aColour2[3] = { &waterColour2.x, &waterColour2.y, &waterColour2.z };

	ImGui::Begin("Water Window", &bImguiWater);
	ImGui::SliderFloat("Tesselation", &waterTesselationFactor, 1, 64);
	ImGui::SliderFloat("Speed", &waterSpeed, 0, 10);
	ImGui::SliderFloat("Amplitude", &waterAmplitude, 0.0, 0.180);
	ImGui::SliderFloat("Frequency", &waterFrequency, 0, 10);
	ImGui::ColorPicker3("Colour 1", *aColour1);
	ImGui::ColorPicker3("Colour 2", *aColour2);
	ImGui::End();


}
void App1::ImguiPictureFrames()
{
	ImGui::Begin("Picture Frame Window", &bImguiPictureframes);
	ImGui::SliderFloat("Picture 1 Tess", &picture1TessFactor, 1, 10);
	ImGui::SliderFloat("Picture 1 Diss", &picture1displacement, 0, 5);
	ImGui::SliderFloat("Picture 2 Tess", &picture2TessFactor, 1, 10);
	ImGui::SliderFloat("Picture 2 Diss", &picture2displacement, 0, 5);
	ImGui::SliderFloat("Picture 3 Tess", &picture3TessFactor, 1, 10);
	ImGui::SliderFloat("Picture 3 Diss", &picture3displacement, 0, 5);

	ImGui::End();


}
void App1::ImguiPointLights()
{
	ImGui::Begin("Point Light", &bImguiPointLights);
	static float* alight3Position[3] = { &lightPosition[pLight1].x, &lightPosition[pLight1].y, &lightPosition[pLight1].z };
	static float* alight3Ambient[3] = { &lightAmbient[pLight1].x, &lightAmbient[pLight1].y, &lightAmbient[pLight1].z };
	static float* alight3Diffuse[3] = { &lightDiffuse[pLight1].x, &lightDiffuse[pLight1].y, &lightDiffuse[pLight1].z };
	static float* alight4Position[3] = { &lightPosition[pLight2].x, &lightPosition[pLight2].y, &lightPosition[pLight2].z };
	static float* alight4Ambient[3] = { &lightAmbient[pLight2].x, &lightAmbient[pLight2].y, &lightAmbient[pLight2].z };
	static float* alight4Diffuse[3] = { &lightDiffuse[pLight2].x, &lightDiffuse[pLight2].y, &lightDiffuse[pLight2].z };
	static float* aAttenuation1[3] = { &attenuation[0].x, &attenuation[0].y, &attenuation[0].z };
	static float* aAttenuation2[3] = { &attenuation[1].x, &attenuation[1].y, &attenuation[1].z };

	ImGui::SliderFloat3("Position1", *alight3Position, -50.0, 100.0, "%.2f", 1.0f);
	ImGui::SliderFloat3("Position2", *alight4Position, -50.0, 100.0, "%.2f", 1.0f);
	ImGui::SliderFloat3("Attenuation 1", *aAttenuation1, 0.0, 1.0, "%.2f", 1.0f);
	ImGui::SliderFloat3("Attenuation 2", *aAttenuation2, 0.0, 1.0, "%.2f", 1.0f);
	ImGui::SliderFloat("Radius", &pLightradius, 0, 20);
	ImGui::SliderFloat("Frequency", &pLightFrequency, 0, 10);
	ImGui::SliderFloat3("Ambient 1", *alight3Ambient, 0.0, 1.0, "%.2f", 1.0f);
	ImGui::SliderFloat3("Ambient 2", *alight4Ambient, 0.0, 1.0, "%.2f", 1.0f);
	ImGui::ColorPicker3("Diffuse 1", *alight3Diffuse);
	ImGui::ColorPicker3("Diffuse 2", *alight4Diffuse);

	ImGui::End();


}
void App1::ImguiDLights()
{
	ImGui::Begin("DIrectional Light", &bImguiDLights);
	static float* alight1Position[3] = { &lightPosition[dLight1].x, &lightPosition[dLight1].y, &lightPosition[dLight1].z };
	static float* alight1Directional[3] = { &lightDirection[dLight1].x, &lightDirection[dLight1].y, &lightDirection[dLight1].z };
	static float* alight1Ambient[3] = { &lightAmbient[dLight1].x, &lightAmbient[dLight1].y, &lightAmbient[dLight1].z };
	static float* alight1Diffuse[3] = { &lightDiffuse[dLight1].x, &lightDiffuse[dLight1].y, &lightDiffuse[dLight1].z };
	static float* alight2Position[3] = { &lightPosition[dLight2].x, &lightPosition[dLight2].y, &lightPosition[dLight2].z };
	static float* alight2Directional[3] = { &lightDirection[dLight2].x, &lightDirection[pLight2].y, &lightDirection[dLight2].z };
	static float* alight2Ambient[3] = { &lightAmbient[dLight2].x, &lightAmbient[dLight2].y, &lightAmbient[dLight2].z };
	static float* alight2Diffuse[3] = { &lightDiffuse[dLight2].x, &lightDiffuse[dLight2].y, &lightDiffuse[dLight2].z };

	ImGui::SliderFloat3("Position1", *alight1Position, -30, 30, "%.2f", 1.0f);
	ImGui::SliderFloat3("Position2", *alight2Position, -30, 30, "%.2f", 1.0f);
	ImGui::SliderFloat3("Direction1", *alight1Directional, -1, 1, "%.2f", 1.0f);
	ImGui::SliderFloat3("Direction2", *alight2Directional, -1, 1, "%.2f", 1.0f);
	ImGui::SliderFloat("Radius", &pLightradius, 0, 10);
	ImGui::SliderFloat("Frequency", &pLightFrequency, 0, 10);
	ImGui::SliderFloat3("Ambient 1", *alight1Ambient, 0.0, 1.0, "%.2f", 1.0f);
	ImGui::SliderFloat3("Ambient 2", *alight2Ambient, 0.0, 1.0, "%.2f", 1.0f);
	ImGui::ColorPicker3("Diffuse 1", *alight1Diffuse);
	ImGui::ColorPicker3("Diffuse 2", *alight2Diffuse);

	ImGui::End();
}
void App1::ImguiBlur()
{
	ImGui::Begin("Blur", &bImguiBlur);
	ImGui::SliderFloat("Range", &dofDistance, 0.0f, 1.0f);
	ImGui::SliderFloat("Offset", &dofOffset, -0.2, 0.7f);
	ImGui::SliderFloat("horiz Strength", &horizStrength, 0, 1.0f);
	ImGui::SliderFloat("vert Strength", &vertStrength, 0, 1.0f);
	ImGui::End();
}

wchar_t* App1::ChangeHeightMap()
{
	switch (heightmapChoice)
	{
	case 0:
		return L"heightmap1";
	case 1:
		return L"heightmap2";
	case 2:
		return L"heightmap3";
	case 3:
		return L"heightmap4";
	case 4:
		return L"heightmap5";
	case 5:
		return L"heightmap6";
	case 6:
		return L"heightmap7";
	}
}
wchar_t* App1::ChangeHeightMapTexture()
{
	switch (heightmapTextureChoice)
	{
	case 0:
		return L"ground";
	case 1:
		return L"ground1";
	case 2:
		return L"ground2";
	case 3:
		return L"ground3";
	case 4:
		return L"ground4";
	}
}



