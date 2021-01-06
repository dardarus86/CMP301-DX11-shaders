//Jamie Haddow TesselationShader.cpp
//COMMENTING DONE

#include "TessellatedPlaneWithGeometry.h"

TessellatedPlaneWithGeometry::TessellatedPlaneWithGeometry(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"geometry_vs.cso", L"geometry_hs.cso", L"geometry_ds.cso", L"geometry_gs.cso", L"geometry_ps.cso");
}

TessellatedPlaneWithGeometry::~TessellatedPlaneWithGeometry()
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

void TessellatedPlaneWithGeometry::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	//helper functions to streamline buffer/sampler creation
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(TesselationBufferType), &tesselationBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(DisplacementBufferType), &displacementBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(CameraBufferType), &cameraBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(GrassBufferType), &grassBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(ColourBufferType), &colourBuffer, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState2, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState3, renderer);
	ShaderFunctions::CreateNewMirrorSamplerDesc(&displacementSamplerState, renderer);


}

void TessellatedPlaneWithGeometry::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);
	// Loading hull, domain and geometry shaders
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
	loadGeometryShader(gsFilename);
}


void TessellatedPlaneWithGeometry::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, 
													   ID3D11ShaderResourceView* heightMap,ID3D11ShaderResourceView* noiseMap, ID3D11ShaderResourceView* heightMapgrass, ID3D11ShaderResourceView* windMap,
													   ID3D11ShaderResourceView* grass, Camera* camera, float tesselationFactor, float displacement, float grassTime, float grassObjects, float grassAmp, float grassFreq,
													   float grassHeight, float grassWidth, XMFLOAT4 colour1, XMFLOAT4 colour2)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	TesselationBufferType* tesPtr;
	DisplacementBufferType* disPtr;
	CameraBufferType* cameraPtr;
	GrassBufferType* grassPtr;
	ColourBufferType* colourPtr;


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
	deviceContext->GSSetConstantBuffers(0, 1, &matrixBuffer);    //GS 0

	///////////////////
	//  TESSELATION  //
	///////////////////
	//sending tessellation data to the hull shader
	deviceContext->Map(tesselationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	tesPtr = (TesselationBufferType*)mappedResource.pData;
	tesPtr->tesselationFactor = tesselationFactor;
	tesPtr->tessPadding = XMFLOAT3(0.0, 0.0, 0.0);
	deviceContext->Unmap(tesselationBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &tesselationBuffer);  //HS 0

	///////////////////
	//  DISPLACEMENT //
	///////////////////
	//sending displacment data to the domain shader
	deviceContext->Map(displacementBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	disPtr = (DisplacementBufferType*)mappedResource.pData;
	disPtr->displacementHeight = displacement;
	disPtr->padding2 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	deviceContext->Unmap(displacementBuffer, 0);
	deviceContext->DSSetConstantBuffers(0, 1, &displacementBuffer);  //DS1

	//////////////
	//  CAMERA  //
	//////////////
	//sending camera data to the hull and geometry shaders
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->cameraPos = XMFLOAT4(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z, 1.0);
	cameraPtr->grassTime = grassTime;
	cameraPtr->padding = XMFLOAT3(0.0, 0.0, 0.0);
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->HSSetConstantBuffers(1, 1, &cameraBuffer);  //HS1
	deviceContext->GSSetConstantBuffers(1, 1, &cameraBuffer);  //GS1

	//////////////
	//  GRASS   //
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
	deviceContext->GSSetConstantBuffers(2, 1, &grassBuffer);   //GS2

	//////////////
	//  COLOUR  //
	//////////////
	//sending colour data to the pixel shader
	deviceContext->Map(colourBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	colourPtr = (ColourBufferType*)mappedResource.pData;
	colourPtr->colour1 = colour1;
	colourPtr->colour2 = colour2;
	deviceContext->Unmap(colourBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &colourBuffer);  //PS0

	//sending texture/samplers to different shader stages
	deviceContext->DSSetShaderResources(0, 1, &heightMap);
	deviceContext->GSSetShaderResources(0, 1, &noiseMap);
	deviceContext->GSSetShaderResources(1, 1, &windMap);
	deviceContext->GSSetShaderResources(2, 1, &heightMapgrass);
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &noiseMap);
	deviceContext->PSSetShaderResources(2, 1, &grass);
	deviceContext->DSSetSamplers(0, 1, &displacementSamplerState);

	deviceContext->GSSetSamplers(0, 1, &sampleState);
	deviceContext->GSSetSamplers(1, 1, &sampleState2);
	deviceContext->GSSetSamplers(2, 1, &sampleState3);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleState2);
}

void TessellatedPlaneWithGeometry::render(ID3D11DeviceContext* deviceContext, int indexCount)
{
	BaseShader::render(deviceContext, indexCount);
}


