// Mandelbrot fractal constants
#define M_RE_START -2.8f
#define M_RE_END 1.0f
#define M_IM_START -1.5f
#define M_IM_END 1.5f
#define M_MAX_ITERATION 1000

float3 MapColor(float t)
{
	float3 ambient = float3(0.09f, 0.12f, 0.16f);
	return float3(3.0f * t, 5.0f * t, 10.0f * t) * ambient;
}

float2 ComplexSq(float2 c)
{
	return float2(c.x * c.x - c.y * c.y, 2.0f * c.x * c.y);
}

float3 DrawMandelbrot(float2 uv)
{
	const float2 c = float2(M_RE_START + uv.x * (M_RE_END - M_RE_START),
							M_IM_START + uv.y * (M_IM_END - M_IM_START));
	float2 z = 0.0f;
	for (int i = 0; i < M_MAX_ITERATION; i++)
	{
		z = ComplexSq(z) + c;
		const float d = dot(z, z);
		if (d > 4.0f)
		{
			const float t = i + 1 - log(log2(d));
			return MapColor(t / M_MAX_ITERATION);
		}
	}

	return 1.0f;
}