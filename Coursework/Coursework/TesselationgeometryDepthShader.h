//Jamie Haddow TesselationGeometryDepthShader.h
//COMMENTING DONE

#include "DXF.h"
#include "ShaderFunctions.h"

using namespace std;
using namespace DirectX;

class TesselationGeometryDepthShader :    public BaseShader
{
	//structs comaining tessellation,displacement,camera and grass information for shaders
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
		XMFLOAT3 cameraPos;
		float cameraPadding;
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





public:

	TesselationGeometryDepthShader(ID3D11Device* device, HWND hwnd);
	~TesselationGeometryDepthShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix,
							 const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* landheightMap, ID3D11ShaderResourceView* noiseMap,
							 ID3D11ShaderResourceView* heightMapgrass, ID3D11ShaderResourceView* windMap, Camera* camera, float tesselationFactor,
							 float displacement, float grassTime, float grassObjects, float grassAmp, float grassFreq, float grassHeight, float grassWidth);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename);

private:

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* tesselationBuffer;
	ID3D11Buffer* displacementBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* grassBuffer;

	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;
	ID3D11SamplerState* sampleState3;
};






