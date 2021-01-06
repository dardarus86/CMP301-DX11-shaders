//Jamie Haddow LightBuffer.h
//COMMENTING DONE
#pragma once
#include "DXF.h"

//helper class for a general use light buffer that can be used in multiple shaders. Also stores shadow map information
class LightBuffer
{
public:
	struct MatrixBufferType
	{
		XMMATRIX lightViewMatrix[2];
		XMMATRIX lightProjectionMatrix[2];
		XMMATRIX lightViews[6];
		XMMATRIX lightProjections[6];
		XMMATRIX lightViews2[6];
		XMMATRIX lightProjections2[6];
	};

	struct LightBufferType
	{
		XMFLOAT4 ambient[4];
		XMFLOAT4 diffuse[4];
		XMFLOAT4 lightPosition[4];
		XMFLOAT4 lightDirection[4];
		XMFLOAT4 attenuation[2];
	};
};

