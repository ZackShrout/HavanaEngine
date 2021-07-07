#pragma once

#include "..\Common\CommonHeaders.h"
#include "MathTypes.h"

namespace Havana::Math
{
	template<typename T>
	constexpr T clamp(T value, T min, T max)
	{
		return (value < min) ? min : (value > max) ? max : value;
	}



	
}





