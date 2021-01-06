//Jamie Haddow WaterShader.h
//COMMENTING DONE

#include "BaseShader.h"
#include "DXF.h"
#include "ShaderFunctions.h"

using namespace std;
using namespace DirectX;

class WaterShader : public BaseShader
{
	//structs sending tesselation,time,camera and colour data to the shaders
	struct TesselationBufferType
	{
		float tesselationFactor;
		float displacement;
		XMFLOAT2 tessPadding;
	};

	struct TimeBufferType
	{
		float timer;
		float speed;
		float amplitude;
		float frequency;
	};
	struct CameraBufferType
	{
		XMFLOAT3 camerPos;
		float cameraPadding;
	};

	struct ColourBufferType
	{
		XMFLOAT4 colour1;
		XMFLOAT4 colour2;
	};



public:

	WaterShader(ID3D11Device* device, HWND hwnd);
	~WaterShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap, ID3D11ShaderResourceView* noiseMap, float tesselationFactor, float displacement, float timer, float speed, float amplitude, float frequency, XMFLOAT4 colour1, XMFLOAT4 colour2, XMFLOAT3 camerPos);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* tesselationBuffer;
	ID3D11Buffer* timeBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* colourBuffer;

	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;
};

