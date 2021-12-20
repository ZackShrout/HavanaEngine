struct VSOutput
{
	noperspective float4 Position : SV_Position;
	noperspective float2 UV : TEXCOORD;
};

VSOutput FullScreenTriangleVS(in uint VertexIdx : SV_VertexID)
{
	VSOutput output;

	float2 tex;
	float2 pos;
	if (VertexIdx == 0)
	{
		tex = float2(0, 0);
		pos = float2(-1, 1);
	}
	else if (VertexIdx == 1)
	{
		tex = float2(0, 2);
		pos = float2(-1, -3);
	}
	else if (VertexIdx == 2)
	{
		tex = float2(2, 0);
		pos = float2(3, 1);
	}

	output.Position = float4(pos, 0, 1);
	output.UV = tex;

	return output;
}