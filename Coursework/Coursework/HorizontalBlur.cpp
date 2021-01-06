//Jamie Haddow Horizontal Blur .cpp
//COMMENTING DONE


#include "HorizontalBlur.h"


HorizontalBlurShader::HorizontalBlurShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"horizontalBlur_vs.cso", L"horizontalBlur_ps.cso");
}


HorizontalBlurShader::~HorizontalBlurShader()
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
	if (screenSizeBuffer)
	{
		screenSizeBuffer->Release();
		screenSizeBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void HorizontalBlurShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);
	
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(ScreenSizeBufferType), &screenSizeBuffer, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState, renderer);
}


void HorizontalBlurShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture,
										       float width, float strength)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	ScreenSizeBufferType* widthPtr;

	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

//sending matrix data to vertex shader
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);


//sending screen info to pixel shader	
	deviceContext->Map(screenSizeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	widthPtr = (ScreenSizeBufferType*)mappedResource.pData;
	widthPtr->screenWidth = width;
	widthPtr->strength = strength;
	widthPtr->padding = XMFLOAT2(1.0f, 1.f);
	deviceContext->Unmap(screenSizeBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &screenSizeBuffer);

// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}



