
#include "DXF.h"
#include "ShaderFunctions.h"

using namespace std;
using namespace DirectX;
class GeometryShadowShader :    public BaseShader

{

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

	struct ColourBufferType
	{
		XMFLOAT4 colour1;
		XMFLOAT4 colour2;
	};




public:

	GeometryShadowShader(ID3D11Device* device, HWND hwnd);
	~GeometryShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
		ID3D11ShaderResourceView* terrainTexture, ID3D11ShaderResourceView* noisemap, ID3D11ShaderResourceView* windmap, ID3D11ShaderResourceView* depthMap,
		ID3D11ShaderResourceView* depthMap2, ID3D11ShaderResourceView* depthArray1[6], ID3D11ShaderResourceView* depthArray2[6], Light* light[4],
		float terrainTess, float terrainDis, float grassTime, float grassObjects, float grassAmp, float grassFreq, float grassHeight, float grassWidth,
		XMFLOAT4 colour1, XMFLOAT4 colour2, Camera* camera);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* shadowBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* tesselationBuffer;
	ID3D11Buffer* displacementBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* grassBuffer;
	ID3D11Buffer* colourBuffer;

	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;
	ID3D11SamplerState* sampleState3;

};

