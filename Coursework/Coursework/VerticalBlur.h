// Jamie haddow Vertical blur .h
//COMMENTING DONE
#pragma once

#include "DXF.h"
#include "ShaderFunctions.h"

using namespace std;
using namespace DirectX;

class VerticalBlurShader : public BaseShader
{
private:
	struct ScreenSizeBufferType
	{
		float screenHeight;
		float strength;
		XMFLOAT2 padding;
	};

public:

	VerticalBlurShader(ID3D11Device* device, HWND hwnd);
	~VerticalBlurShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float width,float strength);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* screenSizeBuffer;
};
