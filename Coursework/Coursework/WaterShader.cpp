//Jamie Haddow WaterShader.h
//COMMENTING DONE

#include "WaterShader.h"

//sending additional stages to shaders
WaterShader::WaterShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"water_vs.cso", L"water_hs.cso", L"water_ds.cso", L"water_ps.cso");
}


WaterShader::~WaterShader()
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

void WaterShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	//helper functions to streamline buffer/sampler creation
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(TesselationBufferType), &tesselationBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(TimeBufferType), &timeBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(CameraBufferType), &cameraBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(ColourBufferType), &colourBuffer, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState2, renderer);

}

void WaterShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Loading hull and domain steps.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}


void WaterShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap, ID3D11ShaderResourceView* noiseMap, float tesselationFactor,float displacement, float timer, float speed, float amplitude, float frequency, XMFLOAT4 colour1, XMFLOAT4 colour2, XMFLOAT3 camerPos)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	MatrixBufferType* dataPtr;
	TimeBufferType* timePtr;
	TesselationBufferType* tesPtr;
	CameraBufferType* cameraPtr;
	ColourBufferType* colourPtr;

	XMMATRIX tworld, tview, tproj;
	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(world);
	tview = XMMatrixTranspose(view);
	tproj = XMMatrixTranspose(projection);

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

	////////////
	//  TIME  //
	////////////
	//sending time data to the domain shader
	deviceContext->Map(timeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	timePtr = (TimeBufferType*)mappedResource.pData;
	timePtr->timer = timer;
	timePtr->speed = speed;
	timePtr->amplitude = amplitude;
	timePtr->frequency = frequency;
	deviceContext->Unmap(timeBuffer, 0);
	deviceContext->DSSetConstantBuffers(1, 1, &timeBuffer);

	///////////////////
	//  TESSELATION  //
	///////////////////
	//sending tessellation data to the hull and pixel shader
	deviceContext->Map(tesselationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	tesPtr = (TesselationBufferType*)mappedResource.pData;
	tesPtr->tesselationFactor = tesselationFactor;
	tesPtr->displacement = displacement;
	tesPtr->tessPadding = XMFLOAT2(0.0, 0.0);
	deviceContext->Unmap(tesselationBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &tesselationBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &tesselationBuffer);

	//////////////
	//  CAMERA  //
	//////////////
	//sending camera data to the hull shader
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->camerPos = camerPos;
	cameraPtr->cameraPadding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->HSSetConstantBuffers(1, 1, &cameraBuffer);  //HULL 1

	//////////////
	//  COLOUR  //
	//////////////
	//sending colour data to the pixel shader
	deviceContext->Map(colourBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	colourPtr = (ColourBufferType*)mappedResource.pData;
	colourPtr->colour1 = colour1;
	colourPtr->colour2 = colour2;
	deviceContext->Unmap(colourBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &colourBuffer);

	//sending texture/samplers to the different shaders
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &heightMap);
	deviceContext->DSSetShaderResources(0, 1, &noiseMap);
	deviceContext->DSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleState2);
}



