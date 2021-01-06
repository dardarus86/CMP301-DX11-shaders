//Jamie Haddow TesselationGeometryDepthShader.cpp
//COMMENTING DONE
#include "TesselationgeometryDepthShader.h"

//additional shader steps added here
TesselationGeometryDepthShader::TesselationGeometryDepthShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"geometryDepth_vs.cso", L"geometryDepth_hs.cso", L"geometryDepth_ds.cso", L"geometryDepth_gs.cso", L"geometryDepth_gs.cso");
}


TesselationGeometryDepthShader::~TesselationGeometryDepthShader()
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

void TesselationGeometryDepthShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);
	//helper functions to streamline buffer/sampler creation
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(TesselationBufferType), &tesselationBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(TesselationBufferType), &displacementBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(CameraBufferType), &cameraBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(GrassBufferType), &grassBuffer, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState2, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState3, renderer);



}

void TesselationGeometryDepthShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Loading hull,domain and geometry shaders
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
	loadGeometryShader(gsFilename);
}


void TesselationGeometryDepthShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
														 const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* landheightMap, ID3D11ShaderResourceView* noiseMap,
														 ID3D11ShaderResourceView* heightMapgrass, ID3D11ShaderResourceView* windMap, Camera* camera, float tesselationFactor,
														 float displacement, float grassTime, float grassObjects, float grassAmp, float grassFreq, float grassHeight, float grassWidth)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	MatrixBufferType* dataPtr; // GS 0
	GrassBufferType* grassPtr; // GS 1
	TesselationBufferType* tesPtr;	// HS 0
	CameraBufferType* cameraPtr;    // HS 1
	DisplacementBufferType* disPtr; // DS 0
	
	XMMATRIX tworld, tview, tproj;
	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	///////////////
	//  MATRIX   //
	///////////////
	//sending matrix data to the geometry shader
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->GSSetConstantBuffers(0, 1, &matrixBuffer);

	////////////////////
	//  TESSELATION   //
	////////////////////
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
	deviceContext->DSSetConstantBuffers(0, 1, &displacementBuffer);

	//////////////
	//  CAMERA  //
	//////////////
	//sending camera data to the hull shader
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->cameraPos = XMFLOAT3(camera->getPosition().x, camera->getPosition().y,camera->getPosition().z);
	cameraPtr->cameraPadding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->HSSetConstantBuffers(1, 1, &cameraBuffer);

	//////////////
	//  GRASS  //
	//////////////
	//sending grass data to the geometry shader
	deviceContext->Map(grassBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	grassPtr = (GrassBufferType*)mappedResource.pData;
	grassPtr->numOfGrassObjects = grassObjects;
	grassPtr->amplitude = grassAmp;
	grassPtr->frequency = grassFreq;
	grassPtr->height = grassHeight;
	grassPtr->width = grassWidth;
	grassPtr->padding1 = XMFLOAT3(0.0, 0.0, 0.0);
	deviceContext->Unmap(grassBuffer, 0);
	deviceContext->GSSetConstantBuffers(1, 1, &grassBuffer);

	//texture/samplers

	deviceContext->DSSetShaderResources(0, 1, &landheightMap);
	deviceContext->DSSetSamplers(0, 1, &sampleState);

	deviceContext->GSSetShaderResources(0, 1, &heightMapgrass);
	deviceContext->GSSetShaderResources(1, 1, &windMap);
	deviceContext->GSSetShaderResources(2, 1, &noiseMap);
	deviceContext->GSSetSamplers(0, 1, &sampleState);
	deviceContext->GSSetSamplers(1, 1, &sampleState2);
	deviceContext->GSSetSamplers(2, 1, &sampleState3);

}



