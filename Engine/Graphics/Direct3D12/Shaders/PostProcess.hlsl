#include "Common.hlsli"

struct ShaderConstants
{
	uint GPassMainBufferIndex;
};

ConstantBuffer<GlobalShaderData>	GlobalData			: register(b0, space0);
ConstantBuffer<ShaderConstants>		ShaderParams		: register(b1, space0);

// TODO: temporary for visualizing light culling frustums grid
#define TILE_SIZE 16
StructuredBuffer<Frustum>			Frustums			: register(t0, space0);
StructuredBuffer<uint2>				LightGridOpaque		: register(t1, space0);

uint GetGridIndex(float2 posXY, float viewWidth)
{
	const uint2 pos = uint2(posXY);
	const uint tileX = ceil(viewWidth / TILE_SIZE);
	return (pos.x / TILE_SIZE) + (tileX * (pos.y / TILE_SIZE));
}

// Adapted from WickedEngine: https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/shaders/lightCullingCS.hlsl
float4 Heatmap(StructuredBuffer<uint2> buffer, float2 posXY, float blend)
{
	const float w = GlobalData.ViewWidth;
	const uint gridIndex = GetGridIndex(posXY, w);
	const uint numLights = buffer[gridIndex].y;

	const float3 mapTex[] = {
		float3(0, 0, 0),
		float3(0, 0, 1),
		float3(0, 1, 1),
		float3(0, 1, 0),
		float3(1, 1, 0),
		float3(1, 0, 0),
	};
	const uint mapTexLen = 5;
	const uint maxHeat = 40;
	float l = saturate((float)numLights / maxHeat) * mapTexLen;
	float3 a = mapTex[floor(l)];
	float3 b = mapTex[ceil(l)];
	float3 heatmap = lerp(a, b, l - floor(l));

	Texture2D gpassMain = ResourceDescriptorHeap[ShaderParams.GPassMainBufferIndex];
	return float4(lerp(gpassMain[posXY].xyz, heatmap, blend), 1.f);
}

float4 PostProcessPS(in noperspective float4 Position : SV_Position,
	in noperspective float2 UV : TEXCOORD) : SV_TARGET0
{
#if 0 // FRUSTUM VISUALIZATION
	const float w = GlobalData.ViewWidth;
	const uint gridIndex = GetGridIndex(Position.xy, w);
	const Frustum f = Frustums[gridIndex];
	const uint halfTile = TILE_SIZE / 2;
	float3 color = abs(f.Planes[1].Normal);

	if (GetGridIndex(float2(Position.x + halfTile, Position.y), w) == gridIndex && GetGridIndex(float2(Position.x, Position.y + halfTile), w) == gridIndex)
	{
		color = abs(f.Planes[0].Normal);
	}
	else if (GetGridIndex(float2(Position.x + halfTile, Position.y), w) != gridIndex && GetGridIndex(float2(Position.x, Position.y + halfTile), w) == gridIndex)
	{
		color = abs(f.Planes[2].Normal);
	}
	else if (GetGridIndex(float2(Position.x + halfTile, Position.y), w) == gridIndex && GetGridIndex(float2(Position.x, Position.y + halfTile), w) != gridIndex)
	{
		color = abs(f.Planes[3].Normal);
	}

	Texture2D gpassMain = ResourceDescriptorHeap[ShaderParams.GPassMainBufferIndex];
	color = lerp(gpassMain[Position.xy].xyz, color, 0.3f);
	return float4(color, 1.f);
#elif 0 // INDEX VISUALIZATION
	const uint2 pos = uint2(Position.xy);
	const uint tileX = ceil(GlobalData.ViewWidth / TILE_SIZE);
	const uint2 idx = pos / (uint2)TILE_SIZE;

	float c = (idx.x + tileX * idx.y) * 0.00001f;

	if (idx.x % 2 == 0) c += 0.1f;
	if (idx.y % 2 == 0) c += 0.1f;

	return float4((float3)c, 1.f);
#elif 0 // LIGHT GRID OPAQUE
	return Heatmap(LightGridOpaque, Position.xy, 0.75f);
#elif 1 // SCENE
	Texture2D gpassMain = ResourceDescriptorHeap[ShaderParams.GPassMainBufferIndex];
	return float4(gpassMain[Position.xy].xyz, 1.0f);
#endif
}