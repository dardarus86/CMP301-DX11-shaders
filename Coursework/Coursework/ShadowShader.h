//Jamie Haddow shadowShader .h
//COMMENTING DONE

#include "DXF.h"
#include "ShaderFunctions.h"

using namespace std;
using namespace DirectX;

class ShadowShader : public BaseShader
{
public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
						     ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* depthMap2,
						     ID3D11ShaderResourceView* depthArray1[6], ID3D11ShaderResourceView* depthArray2[6], Light* lights[4], XMFLOAT3 atten[2]);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* shadowBuffer;
	ID3D11Buffer* lightBuffer;

	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;
	ID3D11SamplerState* sampleState3;

};

