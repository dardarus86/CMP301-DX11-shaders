//Jamie Haddow TesselationShader.h
//COMMENTING DONE

#include "DXF.h"
#include "ShaderFunctions.h"

using namespace std;
using namespace DirectX;

class TessellatedPlaneWithGeometry : public BaseShader
{
	//structs passing tesselation,displacement,camera,grass and colour to the shaders
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
		XMFLOAT4 cameraPos;
		float grassTime;
		XMFLOAT3 padding;
	};

	struct GrassBufferType
	{
		float numOfGrassObjects;
		float amplitude;
		float frequency;
		float height;
		float width;
		XMFLOAT3 padding1;
	};

	struct ColourBufferType
	{
		XMFLOAT4 colour1;
		XMFLOAT4 colour2;
	};

public:

	TessellatedPlaneWithGeometry(ID3D11Device* device, HWND hwnd);
	~TessellatedPlaneWithGeometry();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap, ID3D11ShaderResourceView* noiseMap, ID3D11ShaderResourceView* heightMapgrass, ID3D11ShaderResourceView* windMap, ID3D11ShaderResourceView* grass, Camera* camera, float tesselationFactor, float displacement, float grassTime, float grassObjects, float grassAmp, float grassFreq, float grassHeight, float grassWidth, XMFLOAT4 colour1, XMFLOAT4 colour2);
	void render(ID3D11DeviceContext* deviceContext, int vertexCount);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* tesselationBuffer;
	ID3D11Buffer* displacementBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* grassBuffer;
	ID3D11Buffer* colourBuffer;

	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;
	ID3D11SamplerState* sampleState3;
	ID3D11SamplerState* displacementSamplerState;
};

