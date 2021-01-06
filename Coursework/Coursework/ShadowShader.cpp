//Jamie Haddow shadowShader .cpp
//COMMENTING DONE
#include "shadowshader.h"


ShadowShader::ShadowShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"shadow_vs.cso", L"shadow_ps.cso");
}


ShadowShader::~ShadowShader()
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

void ShadowShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	//helper functions to streamline buffer/sampler creation
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(LightBuffer::MatrixBufferType), &shadowBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(LightBuffer::MatrixBufferType), &lightBuffer, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState2, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState3, renderer);

}


void ShadowShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
									   ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* depthMap2,
	                                   ID3D11ShaderResourceView* depthArray1[6], ID3D11ShaderResourceView* depthArray2[6], Light* lights[4], XMFLOAT3 atten[2])
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBuffer::LightBufferType* lightPtr;
	LightBuffer::MatrixBufferType* shadowPtr;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(world);
	XMMATRIX tview = XMMatrixTranspose(view);
	XMMATRIX tproj = XMMatrixTranspose(projection);
	XMMATRIX tLightViewMatrix1 = XMMatrixTranspose(lights[0]->getViewMatrix());
	XMMATRIX tLightProjectionMatrix1 = XMMatrixTranspose(lights[0]->getOrthoMatrix());
	XMMATRIX tLightViewMatrix2 = XMMatrixTranspose(lights[1]->getViewMatrix());
	XMMATRIX tLightProjectionMatrix2 = XMMatrixTranspose(lights[1]->getOrthoMatrix());

	//sending amtrix data to the vertex shader
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer); //VS 0

	//sending direction and position data to the vertex shader
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
	deviceContext->VSSetConstantBuffers(1, 1, &shadowBuffer); //VS 1

	// Send light data to pixel shader
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
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer); //PS 0

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &depthMap);
	deviceContext->PSSetShaderResources(2, 1, &depthMap2);
	deviceContext->PSSetShaderResources(3, 6, depthArray1);
	deviceContext->PSSetShaderResources(9, 6, depthArray2);

	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleState2);
	deviceContext->PSSetSamplers(2, 1, &sampleState3);
	
	
}

