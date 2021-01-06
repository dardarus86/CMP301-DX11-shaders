#include "GeometryShadowShader.h"



GeometryShadowShader::GeometryShadowShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"geometryShadow_vs.cso", L"geometryShadow_hs.cso", L"geometryShadow_ds.cso", L"geometryShadow_gs.cso", L"geometryShadow_ps.cso");
}


GeometryShadowShader::~GeometryShadowShader()
{
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (layout)
	{
		layout->Release();
		layout = 0;
	}
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void GeometryShadowShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(LightBuffer::MatrixBufferType), &shadowBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(LightBuffer::MatrixBufferType), &lightBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(TesselationBufferType), &tesselationBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(DisplacementBufferType), &displacementBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(CameraBufferType), &cameraBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(GrassBufferType), &grassBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(ColourBufferType), &colourBuffer, renderer);
	ShaderFunctions::CreateNewMirrorSamplerDesc(&sampleState, renderer);
	ShaderFunctions::CreateNewMirrorSamplerDesc(&sampleState2, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState3, renderer);
}

void GeometryShadowShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename)
{
	initShader(vsFilename, psFilename);
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
	loadGeometryShader(gsFilename);
}


void GeometryShadowShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
	ID3D11ShaderResourceView* terrainTexture, ID3D11ShaderResourceView* noisemap, ID3D11ShaderResourceView* windmap, ID3D11ShaderResourceView* depthMap,
	ID3D11ShaderResourceView* depthMap2, ID3D11ShaderResourceView* depthArray1[6], ID3D11ShaderResourceView* depthArray2[6], Light* lights[4],
	float terrainTess, float terrainDis, float grassTime, float grassObjects, float grassAmp, float grassFreq, float grassHeight, float grassWidth,
	XMFLOAT4 colour1, XMFLOAT4 colour2, Camera* camera)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBuffer::MatrixBufferType* shadowPtr; //done
	LightBuffer::LightBufferType* lightPtr; //done
	TesselationBufferType* tessPtr; //done
	DisplacementBufferType* dispPtr; //done
	CameraBufferType camPtr; //done
	GrassBufferType* grassPtr;
	ColourBufferType* colourPtr; //done
	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(world);
	XMMATRIX tview = XMMatrixTranspose(view);
	XMMATRIX tproj = XMMatrixTranspose(projection);
	XMMATRIX tLightViewMatrix1 = XMMatrixTranspose(lights[0]->getViewMatrix());
	XMMATRIX tLightProjectionMatrix1 = XMMatrixTranspose(lights[0]->getOrthoMatrix());
	XMMATRIX tLightViewMatrix2 = XMMatrixTranspose(lights[1]->getViewMatrix());
	XMMATRIX tLightProjectionMatrix2 = XMMatrixTranspose(lights[1]->getOrthoMatrix());

	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->GSSetConstantBuffers(0, 1, &matrixBuffer); //GS 0


	///////////////////
	//     SHADOWS   //
	///////////////////
	XMFLOAT3 directions[6] = { XMFLOAT3(0.0f ,1, 0.0f),XMFLOAT3(0.0f , -1.0f, 0.0f),	XMFLOAT3(1,0.0f,0),	XMFLOAT3(-1,0.0f,0),XMFLOAT3(0,0.0f,1),XMFLOAT3(0,0.0f,-1) };

	deviceContext->Map(shadowBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	shadowPtr = (LightBuffer::MatrixBufferType*)mappedResource.pData;
	shadowPtr->lightViewMatrix[0] = tLightViewMatrix1;
	shadowPtr->lightProjectionMatrix[0] = tLightProjectionMatrix1;
	shadowPtr->lightViewMatrix[1] = tLightViewMatrix2;
	shadowPtr->lightProjectionMatrix[1] = tLightProjectionMatrix2;

	for (int i = 0; i < 6; i++)
	{
		lights[3]->setDirection(directions[i].x, directions[i].y, directions[i].z);
		lights[4]->setDirection(directions[i].x, directions[i].y, directions[i].z);
		lights[3]->generateViewMatrix();
		lights[4]->generateViewMatrix();
		XMMATRIX viewMatrix = lights[3]->getViewMatrix();
		XMMATRIX viewMatrix2 = lights[4]->getViewMatrix();
		XMMATRIX tLightViewMatrixPoint = XMMatrixTranspose(viewMatrix);
		XMMATRIX tLightViewMatrixPoint2 = XMMatrixTranspose(viewMatrix2);
		XMMATRIX tLightProjectionMatrixPoint = XMMatrixTranspose(lights[3]->getProjectionMatrix());
		XMMATRIX tLightProjectionMatrixPoint2 = XMMatrixTranspose(lights[4]->getProjectionMatrix());
		shadowPtr->lightViews[i] = tLightViewMatrixPoint;
		shadowPtr->lightProjections[i] = tLightProjectionMatrixPoint;
		shadowPtr->lightViews2[i] = tLightViewMatrixPoint2;
		shadowPtr->lightProjections2[i] = tLightProjectionMatrixPoint2;
	}
	deviceContext->Unmap(shadowBuffer, 0);
	deviceContext->GSSetConstantBuffers(1, 1, &shadowBuffer); //GS 1

	///////////////////
	//     LIGHTS    //
	///////////////////

	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBuffer::LightBufferType*)mappedResource.pData;
	lightPtr->ambient[0] = lights[0]->getAmbientColour();
	lightPtr->diffuse[0] = lights[0]->getDiffuseColour();
	lightPtr->lightDirection[0] = XMFLOAT4(lights[0]->getDirection().x, lights[0]->getDirection().y, lights[0]->getDirection().z, 1);
	lightPtr->lightPosition[0] = XMFLOAT4(lights[0]->getPosition().x, lights[0]->getPosition().y, lights[0]->getPosition().z, 1);

	lightPtr->ambient[1] = lights[1]->getAmbientColour();
	lightPtr->diffuse[1] = lights[1]->getDiffuseColour();
	lightPtr->lightDirection[1] = XMFLOAT4(lights[1]->getDirection().x, lights[1]->getDirection().y, lights[1]->getDirection().z, 1);
	lightPtr->lightPosition[1] = XMFLOAT4(lights[1]->getPosition().x, lights[1]->getPosition().y, lights[1]->getPosition().z, 1);

	lightPtr->ambient[2] = lights[3]->getAmbientColour();
	lightPtr->diffuse[2] = lights[3]->getDiffuseColour();
	lightPtr->lightDirection[2] = XMFLOAT4(lights[3]->getDirection().x, lights[3]->getDirection().y, lights[3]->getDirection().z, 1.0f);
	lightPtr->lightPosition[2] = XMFLOAT4(lights[3]->getPosition().x, lights[3]->getPosition().y, lights[3]->getPosition().z, 1.0f);

	lightPtr->ambient[3] = lights[4]->getAmbientColour();
	lightPtr->diffuse[3] = lights[4]->getDiffuseColour();
	lightPtr->lightDirection[3] = XMFLOAT4(lights[4]->getDirection().x, lights[4]->getDirection().y, lights[4]->getDirection().z, 1.0f);
	lightPtr->lightPosition[3] = XMFLOAT4(lights[4]->getPosition().x, lights[4]->getPosition().y, lights[4]->getPosition().z, 1.0f);
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer); //PS 0

	///////////////////////
	//    TESSELATION    //
	///////////////////////

	deviceContext->Map(tesselationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	tessPtr = (TesselationBufferType*)mappedResource.pData;
	tessPtr->tesselationFactor = terrainTess;
	tessPtr->tessPadding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	deviceContext->Unmap(tesselationBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &tesselationBuffer); //HS 0

	///////////////////////
	//   DISPLACEMENT    //
	///////////////////////

	deviceContext->Map(displacementBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dispPtr = (DisplacementBufferType*)mappedResource.pData;
	dispPtr->displacementHeight = terrainDis;
	dispPtr->padding2 = XMFLOAT3(0.0, 0.0, 0.0);

	deviceContext->Unmap(displacementBuffer, 0);
	deviceContext->DSSetConstantBuffers(1, 1, &displacementBuffer); // DS 2

	////////////////
	//   CAMERA   //
	////////////////

	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CameraBufferType* cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->cameraPos = XMFLOAT4(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z, 1.0f);
	cameraPtr->cameraPadding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->HSSetConstantBuffers(1, 1, &cameraBuffer); //HS 1
	deviceContext->GSSetConstantBuffers(2, 1, &cameraBuffer); //GS 2
	////////////////
	//   GRASS    //
	////////////////

	deviceContext->Map(grassBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	grassPtr = (GrassBufferType*)mappedResource.pData;
	grassPtr->numOfGrassObjects = grassObjects;
	grassPtr->amplitude = grassAmp;
	grassPtr->frequency = grassFreq;
	grassPtr->height = grassHeight;
	grassPtr->width = grassWidth;
	grassPtr->padding1 = XMFLOAT3(0.0, 0.0, 0.0);
	deviceContext->Unmap(grassBuffer, 0);
	deviceContext->GSSetConstantBuffers(3, 1, &grassBuffer); //GS 3

	////////////////
	//   COLOUR   //
	////////////////

	deviceContext->Map(colourBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	colourPtr = (ColourBufferType*)mappedResource.pData;
	colourPtr->colour1 = colour1;
	colourPtr->colour2 = colour2;
	deviceContext->Unmap(colourBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &colourBuffer); // PS 1



	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &terrainTexture);
	deviceContext->PSSetShaderResources(1, 1, &depthMap);
	deviceContext->PSSetShaderResources(2, 1, &depthMap2);
	deviceContext->PSSetShaderResources(3, 6, depthArray1);
	deviceContext->PSSetShaderResources(10, 6, depthArray2);

	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(2, 1, &sampleState2);
	deviceContext->PSSetSamplers(1, 1, &sampleState3);
}


