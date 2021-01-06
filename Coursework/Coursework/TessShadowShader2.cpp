//Jamie Haddow tesselation shadow shader.cpp
//COMMENTING DONE

#include "TessShadowShader2.h"

//additional shader stages added
TessShadowShader2::TessShadowShader2(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"tessellationShadow2_vs.cso", L"tessellationShadow2_hs.cso", L"tessellationShadow2_ds.cso", L"tessellationShadow2_ps.cso");
}


TessShadowShader2::~TessShadowShader2()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void TessShadowShader2::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadTextureVertexShader(vsFilename);
	loadPixelShader(psFilename);
	//helper functions to streamline buffer/sampler creation
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(LightBuffer::MatrixBufferType), &shadowBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(LightBuffer::MatrixBufferType), &lightBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(TesselationBufferType), &tesselationBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(DisplacementBufferType), &displacementBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(CameraBufferType), &cameraBuffer, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState2, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState3, renderer);

}

void TessShadowShader2::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	//loading additional shaders hull and domain
	initShader(vsFilename, psFilename);
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);


}


void TessShadowShader2::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
							                ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightmap, ID3D11ShaderResourceView* depthMap,
							                ID3D11ShaderResourceView* depthMap2, ID3D11ShaderResourceView* depthArray1[6], ID3D11ShaderResourceView* depthArray2[6],
							                float terrainTess, float terrainDis, Light* lights[4], XMFLOAT3 atten[2], Camera* camera, float strength)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;      		    //DS0
	LightBuffer::MatrixBufferType* shadowPtr;   //DS1
	TesselationBufferType* tessPtr;				//HS0
	DisplacementBufferType* dispPtr;            //DS2
	LightBuffer::LightBufferType* lightPtr;     //PS0
	CameraBufferType* cameraPtr;


	///////////////////
	//     MATRIX    //
	///////////////////


	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);
	XMMATRIX tLightViewMatrix1 = XMMatrixTranspose(lights[0]->getViewMatrix());
	XMMATRIX tLightProjectionMatrix1 = XMMatrixTranspose(lights[0]->getOrthoMatrix());
	XMMATRIX tLightViewMatrix2 = XMMatrixTranspose(lights[1]->getViewMatrix());
	XMMATRIX tLightProjectionMatrix2 = XMMatrixTranspose(lights[1]->getOrthoMatrix());
	//matrix data sent to the domain shader 
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer); //DS 0

	///////////////////
	//     SHADOWS   //
	///////////////////
	//shadows data sent to the domain shader
	//Dlight
	deviceContext->Map(shadowBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	shadowPtr = (LightBuffer::MatrixBufferType*)mappedResource.pData;
	shadowPtr->lightViewMatrix[0] = tLightViewMatrix1;
	shadowPtr->lightProjectionMatrix[0] = tLightProjectionMatrix1;
	shadowPtr->lightViewMatrix[0] = tLightViewMatrix2;
	shadowPtr->lightProjectionMatrix[1] = tLightProjectionMatrix2;

	//Plight
	XMFLOAT3 directions[6] = { XMFLOAT3(0.0f ,1, 0.0f),XMFLOAT3(0.0f , -1.0f, 0.0f),	XMFLOAT3(1,0.0f,0),	XMFLOAT3(-1,0.0f,0),XMFLOAT3(0,0.0f,1),XMFLOAT3(0,0.0f,-1) };

	for (int i = 0; i < 6; i++)
	{
		lights[2]->setDirection(directions[i].x, directions[i].y, directions[i].z);
		lights[3]->setDirection(directions[i].x, directions[i].y, directions[i].z);
		lights[2]->generateViewMatrix();
		lights[3]->generateViewMatrix();
		XMMATRIX viewMatrix = lights[2]->getViewMatrix();
		XMMATRIX viewMatrix2 = lights[3]->getViewMatrix();
		XMMATRIX tLightViewMatrixPoint = XMMatrixTranspose(viewMatrix);
		XMMATRIX tLightViewMatrixPoint2 = XMMatrixTranspose(viewMatrix2);
		XMMATRIX tLightProjectionMatrixPoint = XMMatrixTranspose(lights[2]->getProjectionMatrix());
		XMMATRIX tLightProjectionMatrixPoint2 = XMMatrixTranspose(lights[3]->getProjectionMatrix());
		shadowPtr->lightViews[i] = tLightViewMatrixPoint;
		shadowPtr->lightProjections[i] = tLightProjectionMatrixPoint;
		shadowPtr->lightViews2[i] = tLightViewMatrixPoint2;
		shadowPtr->lightProjections2[i] = tLightProjectionMatrixPoint2;
	}
	deviceContext->Unmap(shadowBuffer, 0);
	deviceContext->DSSetConstantBuffers(1, 1, &shadowBuffer); //VS 1

	///////////////////////
	//    TESSELATION    //
	///////////////////////
	//tesselation data sent to hull shader
	deviceContext->Map(tesselationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	tessPtr = (TesselationBufferType*)mappedResource.pData;
	tessPtr->tesselationFactor = terrainTess;
	tessPtr->tessPadding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	deviceContext->Unmap(tesselationBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &tesselationBuffer); //HS 0


	///////////////////////
	//   DISPLACEMENT    //
	///////////////////////
	//displacement data sent to domain shader
	deviceContext->Map(displacementBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dispPtr = (DisplacementBufferType*)mappedResource.pData;
	dispPtr->displacementHeight = terrainDis;
	dispPtr->strength = strength;
	dispPtr->padding2 = XMFLOAT2(0.0, 0.0);

	deviceContext->Unmap(displacementBuffer, 0);
	deviceContext->DSSetConstantBuffers(2, 1, &displacementBuffer); // DS 1

	///////////////////
	//     LIGHTS    //
	///////////////////
	//light data sent to pixel shader
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

	lightPtr->ambient[2] = lights[2]->getAmbientColour();
	lightPtr->diffuse[2] = lights[2]->getDiffuseColour();
	lightPtr->lightDirection[2] = XMFLOAT4(lights[2]->getDirection().x, lights[2]->getDirection().y, lights[2]->getDirection().z, 1.0f);
	lightPtr->lightPosition[2] = XMFLOAT4(lights[2]->getPosition().x, lights[2]->getPosition().y, lights[2]->getPosition().z, 1.0f);


	lightPtr->ambient[3] = lights[3]->getAmbientColour();
	lightPtr->diffuse[3] = lights[3]->getDiffuseColour();
	lightPtr->lightDirection[3] = XMFLOAT4(lights[3]->getDirection().x, lights[3]->getDirection().y, lights[3]->getDirection().z, 1.0f);
	lightPtr->lightPosition[3] = XMFLOAT4(lights[3]->getPosition().x, lights[3]->getPosition().y, lights[3]->getPosition().z, 1.0f);
	lightPtr->attenuation[0] = XMFLOAT4(atten[0].x, atten[0].y, atten[0].z, 1.0f);
	lightPtr->attenuation[1] = XMFLOAT4(atten[1].x, atten[1].y, atten[1].z, 1.0f);
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	//////////////
	//  CAMERA  //
	//////////////
	//sending camera data to the hull shader
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->camerPos = XMFLOAT3(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	cameraPtr->cameraPadding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->HSSetConstantBuffers(1, 1, &cameraBuffer);

	// Set shader texture and sampler resource in multiple shaders
	deviceContext->DSSetShaderResources(0, 1, &heightmap);
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &depthMap);
	deviceContext->PSSetShaderResources(2, 1, &depthMap2);
	deviceContext->PSSetShaderResources(3, 6, depthArray1);
	deviceContext->PSSetShaderResources(9, 6, depthArray2);

	deviceContext->DSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleState2);
	deviceContext->PSSetSamplers(2, 1, &sampleState3);

}




