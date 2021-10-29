float4 FillColorPS(in noperspective float4 Position : SV_Position,
				   in noperspective float2 UV : TEXCOORD) : SV_TARGET0
{
	return float4(1.0f, 0.0f, 1.0f, 1.0f);
}