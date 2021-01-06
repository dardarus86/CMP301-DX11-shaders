#include "DXF.h"
#include "ShaderFunctions.h"

using namespace std;
using namespace DirectX;

class TessShadowShader :    public BaseShader
{

private:

	struct TesselationBufferType
	{
		float tesselationFactor;
		XMFLOAT3 tessPadding;
	};

	struct DisplacementBufferType
	{
		float displacementHeight;
		XMFLOAT3 padding2;
	};


public:

	TessShadowShader(ID3D11Device* device, HWND hwnd);
	~TessShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
		ID3D11ShaderResourceView* terrainTexture, ID3D11ShaderResourceView* heightmap, ID3D11ShaderResourceView* depthMap,
		ID3D11ShaderResourceView* depthMap2, ID3D11ShaderResourceView* depthArray1[6], ID3D11ShaderResourceView* depthArray2[6],
		float terrainTess, float terrainDis, Light* lights[4], XMFLOAT3 atten[2]);


private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* shadowBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* tesselationBuffer;
	ID3D11Buffer* displacementBuffer;

	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;
	ID3D11SamplerState* sampleState3;
};

