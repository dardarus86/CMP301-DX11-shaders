//Jamie Haddow TesselationShader.cpp
//COMMENTING DONE
#include "TesselationShader.h"


//additional shader steps being passed here
TessellationShader::TessellationShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"tessellation_vs.cso", L"tessellation_hs.cso", L"tessellation_ds.cso", L"tessellation_ps.cso");
}


TessellationShader::~TessellationShader()
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


void TessellationShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);
	//helper functions to streamline buffer/sampler creation
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(TesselationBufferType), &tesselationBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(DisplacementBufferType), &displacementBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(CameraBufferType), &cameraBuffer, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState, renderer);
	ShaderFunctions::CreateNewMirrorSamplerDesc(&displacementSamplerState, renderer);


}

void TessellationShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Loading the hull and domain shaders
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}


void TessellationShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap, XMFLOAT3 camerPos, float tesselationFactor, float displacement)
{
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	MatrixBufferType* dataPtr;
	TesselationBufferType* tesPtr;
	DisplacementBufferType* disPtr;
	CameraBufferType* cameraPtr;

	XMMATRIX tworld, tview, tproj;
	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	///////////////
	//  MATRIX   //
	///////////////
	//sending matrix data to the domain shader
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);

	///////////////////
	//  TESSELATION  //
	///////////////////
	//sending tessellation data to the hull shader
	deviceContext->Map(tesselationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	tesPtr = (TesselationBufferType*)mappedResource.pData;
	tesPtr->tesselationFactor = tesselationFactor;
	tesPtr->tessPadding = XMFLOAT3(0.0, 0.0, 0.0);
	deviceContext->Unmap(tesselationBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &tesselationBuffer);

	////////////////////
	//  DISPLACEMENT  //
	////////////////////
	//sending displacement data to the domain shader
	deviceContext->Map(displacementBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	disPtr = (DisplacementBufferType*)mappedResource.pData;
	disPtr->displacementHeight = displacement;
	disPtr->padding2 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	deviceContext->Unmap(displacementBuffer, 0);
	deviceContext->DSSetConstantBuffers(2, 1, &displacementBuffer);

	//////////////
	//  CAMERA  //
	//////////////
	//sending camera data to the hull shader
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->camerPos = camerPos;
	cameraPtr->cameraPadding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->HSSetConstantBuffers(1, 1, &cameraBuffer);

	//texture/samplers
	deviceContext->DSSetShaderResources(0, 1, &heightMap);
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->DSSetSamplers(0, 1, &displacementSamplerState);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}


