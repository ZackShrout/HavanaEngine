#include "Fractals.hlsli"

struct ShaderConstants
{
	float	Width;
	float	Height;
	uint	Frame;
};

ConstantBuffer<ShaderConstants>	ShaderParams	: register(b1);

#define SAMPLES 4

float4 FillColorPS(in noperspective float4 Position : SV_Position,
	in noperspective float2 UV : TEXCOORD) : SV_TARGET0
{
	const float offset = 0.2f;
	const float2 offsets[4] =
	{
		float2(-offset, offset),
		float2(offset, offset),
		float2(offset, -offset),
		float2(-offset, -offset),
	};

	const float2 invDim = float2(1.0f / ShaderParams.Width, 1.0f / ShaderParams.Height);
	float3 color = 0.0f;
	for (int i = 0; i < SAMPLES; i++)
	{
		const float2 uv = (Position.xy + offsets[i]) * invDim;
		//float3 color = DrawMandelbrot(uv);
		color += DrawJuliaSet(uv, ShaderParams.Frame);
	}
	
	return float4(float3(color.z, color.x, 1.0f) * color.x / SAMPLES, 1.0f);
	//return float4(color / SAMPLES, 1.0f);
}