//Jamie Haddow Depth of Field .cpp
//COMMENTING DONE

#include "DepthOfFieldShader.h"

DepthOfFieldShader::DepthOfFieldShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"depthOfField_vs.cso", L"depthOfField_ps.cso");
}

DepthOfFieldShader::~DepthOfFieldShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix buffer.
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

	// Release the depth buffer.
	if (depthBuffer)
	{
		depthBuffer->Release();
		depthBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void DepthOfFieldShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);
	//helper functions to streamline buffer/sampler creation
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(DepthBufferType), &depthBuffer, renderer);
}

void DepthOfFieldShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* normalSceneTex, ID3D11ShaderResourceView* blurSceneTex, ID3D11ShaderResourceView* depthSceneTex, float nearV, float farV, float range, float offset)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Send matrix data to the vertex shader.
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Send depth data to pixel shader
	DepthBufferType* depthPtr;
	deviceContext->Map(depthBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	depthPtr = (DepthBufferType*)mappedResource.pData;
	depthPtr->range = range;
	depthPtr->nearVal = nearV;
	depthPtr->farVal = farV;
	depthPtr->offsetVal = offset;
	deviceContext->Unmap(depthBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &depthBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &normalSceneTex);
	deviceContext->PSSetShaderResources(1, 1, &blurSceneTex);
	deviceContext->PSSetShaderResources(2, 1, &depthSceneTex);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}
