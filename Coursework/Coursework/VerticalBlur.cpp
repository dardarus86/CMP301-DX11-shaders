// Jamie Haddow Vertical blur shader .cpp
//COMMENTING DONE
#include "VerticalBlur.h"


VerticalBlurShader::VerticalBlurShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"verticalBlur_vs.cso", L"verticalBlur_ps.cso");
}


VerticalBlurShader::~VerticalBlurShader()
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

	BaseShader::~BaseShader();
}


void VerticalBlurShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{

	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	//helper functions to streamline buffer/sampler creation
	ShaderFunctions::CreateNewBufferDesc(sizeof(MatrixBufferType), &matrixBuffer, renderer);
	ShaderFunctions::CreateNewBufferDesc(sizeof(ScreenSizeBufferType), &screenSizeBuffer, renderer);
	ShaderFunctions::CreateWrapSamplerDesc(&sampleState, renderer);

}


void VerticalBlurShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture,
											 float height,float strength)
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


//sending screen data to pixel shader
	deviceContext->Map(screenSizeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	widthPtr = (ScreenSizeBufferType*)mappedResource.pData;
	widthPtr->screenHeight = height;
	widthPtr->strength = strength;
	widthPtr->padding = XMFLOAT2(1.0f, 1.f);
	deviceContext->Unmap(screenSizeBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &screenSizeBuffer);

// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}




