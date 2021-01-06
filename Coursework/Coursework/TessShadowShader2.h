//Jamie Haddow tesselation shadow shader.h
//COMMENTING DONE

#include "DXF.h"
#include "ShaderFunctions.h"

class TessShadowShader2 :  public BaseShader
{

private:

	//structs used for tessellation and displacement data
	struct TesselationBufferType
	{
		float tesselationFactor;
		XMFLOAT3 tessPadding;
	};

	struct DisplacementBufferType
	{
		float displacementHeight;
		float strength;
		XMFLOAT2 padding2;
	};

	struct CameraBufferType
	{
		XMFLOAT3 camerPos;
		float cameraPadding;
	};



public:
	TessShadowShader2(ID3D11Device* device, HWND hwnd);
	~TessShadowShader2();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
							 ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightmap, ID3D11ShaderResourceView* depthMap,
						     ID3D11ShaderResourceView* depthMap2, ID3D11ShaderResourceView* depthArray1[6], ID3D11ShaderResourceView* depthArray2[6],
							 float terrainTess, float terrainDis, Light* lights[4], XMFLOAT3 atten[2], Camera* camera, float strength);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* shadowBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* tesselationBuffer;
	ID3D11Buffer* displacementBuffer;
	ID3D11Buffer* cameraBuffer;

	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;
	ID3D11SamplerState* sampleState3;


};

