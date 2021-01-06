//Jamie Haddow TesselatedPlane.h
//COMMENTING DONE

#include "BaseMesh.h"

using namespace DirectX;
class TesselatedPlane : public BaseMesh
{
public:
	TesselatedPlane(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution);
	~TesselatedPlane();

	virtual void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

private:
	void initBuffers(ID3D11Device* device);
	int resolution;

};

