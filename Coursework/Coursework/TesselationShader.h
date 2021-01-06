//Jamie Haddow TesselationShader.h
//COMMENTING DONE
#include "DXF.h"
#include "ShaderFunctions.h"

using namespace std;
using namespace DirectX;


class TessellationShader : public BaseShader
{
	//structs used for tessellation,displacement and camera data to send to the shaders
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

	struct CameraBufferType
	{
		XMFLOAT3 camerPos;
		float cameraPadding;
	};





public:

	TessellationShader(ID3D11Device* device, HWND hwnd);
	~TessellationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap, XMFLOAT3 cameraPos, float tesselationFactor, float displacement);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

private:

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* tesselationBuffer;
	ID3D11Buffer* displacementBuffer;
	ID3D11Buffer* cameraBuffer;

	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* displacementSamplerState;
};