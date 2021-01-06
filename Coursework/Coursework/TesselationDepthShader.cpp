//Jamie Haddow TesselationDpethShader.cpp
//COMMENTING DONE

#include "TesselationDepthShader.h"
//passing in additional steps
TesselationDepthShader::TesselationDepthShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"tessellationDepth_vs.cso", L"tessellationDepth_hs.cso", L"tessellationDepth_ds.cso", L"tessellationDepth_ps.cso");
}


TesselationDepthShader::~TesselationDepthShader()
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

	//Release base shader components
	BaseShader::~BaseShader();
}

void TesselationDepthShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	//helper functions to streamline buffer/sampler creation
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(TesselationBufferType), &tesselationBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(TesselationBufferType), &displacementBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(CameraBufferType), &cameraBuffer, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState, renderer);
	ShaderFunctions::CreateNewMirrorSamplerDesc(&displacementSamplerState, renderer);


}

void TesselationDepthShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	//loading hull and domain shaderss for tesselation
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}


void TesselationDepthShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
												 ID3D11ShaderResourceView* heightMap, XMFLOAT3 camerPos, float tesselationFactor, float displacement)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	XMMATRIX tworld, tview, tproj;
	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);


	///////////////////
	//     MATRIX    //
	///////////////////
	//sending mtrix data to the domain shader
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);

	///////////////////////
	//    TESSELATION    //
	///////////////////////
	//sending tesselation data to the hull shader
	result = deviceContext->Map(tesselationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	TesselationBufferType* tesPtr = (TesselationBufferType*)mappedResource.pData;
	tesPtr->tesselationFactor = tesselationFactor;
	tesPtr->tessPadding = XMFLOAT3(0.0, 0.0, 0.0);
	deviceContext->Unmap(tesselationBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &tesselationBuffer);

	///////////////////////
	//   DISPLACEMENT    //
	///////////////////////
	//sending displacement data to the domain shader
	result = deviceContext->Map(displacementBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	DisplacementBufferType* disPtr = (DisplacementBufferType*)mappedResource.pData;
	disPtr->displacementHeight = displacement;
	disPtr->padding2 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	deviceContext->Unmap(displacementBuffer, 0);
	deviceContext->DSSetConstantBuffers(1, 1, &displacementBuffer);

	/////////////////
	//   CAMERA    //
	/////////////////
	//sending camera data to the hull shader
	result = deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CameraBufferType* cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->camerPos = camerPos;
	cameraPtr->cameraPadding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->HSSetConstantBuffers(1, 1, &cameraBuffer);

	//texture/samplers
	deviceContext->DSSetShaderResources(0, 1, &heightMap);
	deviceContext->DSSetSamplers(0, 1, &displacementSamplerState);

}


