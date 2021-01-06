//Jamie Haddow Depth of Field .h
//COMMENTING DONE

#include "DXF.h"
#include "ShaderFunctions.h"

using namespace std;
using namespace DirectX;
class DepthOfFieldShader :   public BaseShader
{
	//struct passed into shader
	struct DepthBufferType
	{
		float range;
		float nearVal;
		float farVal;
		float offsetVal;
	};
public:
	DepthOfFieldShader(ID3D11Device* device, HWND hwnd);
	~DepthOfFieldShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* normalSceneTex, ID3D11ShaderResourceView* blurSceneTex, ID3D11ShaderResourceView* depthSceneTex,  float nearV, float farV, float range, float offset);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* depthBuffer;
	ID3D11SamplerState* sampleState;
};

