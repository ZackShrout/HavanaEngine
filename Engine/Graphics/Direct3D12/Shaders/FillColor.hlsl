#include "Fractals.hlsli"

struct ShaderConstants
{
	float	Width;
	float	Height;
	uint	Frame;
};

ConstantBuffer<ShaderConstants>	ShaderParams	: register(b1);

float4 FillColorPS(in noperspective float4 Position : SV_Position,
	in noperspective float2 UV : TEXCOORD) : SV_TARGET0
{
	const float2 invDim = float2(1.0f / ShaderParams.Width, 1.0f / ShaderParams.Height);
	const float2 uv = (Position.xy) * invDim;
	float3 color = DrawMandelbrot(uv);

	return float4(color, 1.0f);
}