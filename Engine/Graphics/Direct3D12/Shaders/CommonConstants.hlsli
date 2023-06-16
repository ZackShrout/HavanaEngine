#if !defined(HAVANA_COMMON_HLSLI) && !defined(__cplusplus)
#error Do not include this header directly in shader files. Only include this file via Common.hlsli.
#endif

// Light types
// NOTE: these need to be in the same order as havana::graphics::light::type enum
static const uint LIGHT_TYPE_DIRECTIONAL_LIGHT	= 0;
static const uint LIGHT_TYPE_POINT_LIGHT		= 1;
static const uint LIGHT_TYPE_SPOT_LIGHT			= 2;