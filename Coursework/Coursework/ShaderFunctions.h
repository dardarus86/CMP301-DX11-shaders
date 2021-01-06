#pragma once
//Jamie Haddow ShaderFunctions.h
//COMMENTING DONE

#include "DXF.h"
#include "LightBuffer.h"

class ShaderFunctions
{
public:

	static void CreateNewBufferDesc(std::size_t size, ID3D11Buffer** buffer, ID3D11Device* renderer);
	static void CreateWrapSamplerDesc(ID3D11SamplerState** sampler, ID3D11Device* renderer);
	static void CreateNewMirrorSamplerDesc(ID3D11SamplerState** sampler, ID3D11Device* renderer);
	static void CreateNewClampSamplerDesc(ID3D11SamplerState** sampler, ID3D11Device* renderer);

};


