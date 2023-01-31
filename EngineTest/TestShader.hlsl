#include "Common.hlsli"

struct VertexOut
{
	float4 HomogeneousPosition	: SV_POSITION;
	float3 WorldPosition		: POSITION;
	float3 WorldNormal			: NORMAL;
	float3 WorldTangent			: TANGENT;
	float2 UV					: TEXTURE;
};

struct PixelOut
{
	float4 Color				: SV_TARGET0;
};

ConstantBuffer<GlobalShaderData>				PerFrameBuffer			: register(b0, space0);
ConstantBuffer<PerObjectData>					PerObjectBuffer			: register(b1, space0);
StructuredBuffer<float3>						VertexPositions			: register(t0, space0);

VertexOut TestShaderVS(in uint VertexIdx: SV_VertexID)
{
	VertexOut vsOut;

	vsOut.HomogeneousPosition = 0.f;
	vsOut.WorldPosition = 0.f;
	vsOut.WorldNormal = 0.f;
	vsOut.WorldTangent = 0.f;
	vsOut.UV = 0.f;

	return vsOut;
}

[earlydepthstencil]
PixelOut TestShaderPS(in VertexOut psIn)
{
	PixelOut psOut;
	psOut.Color = 0.f;

	return psOut;
}