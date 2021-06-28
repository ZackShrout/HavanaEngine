#pragma once

// Set these flags to 1 to use STL vector and deque
// Set these flags to 0 to use custom implementeation of those containers
#define USE_STL_VECTOR 1
#define USE_STL_DEQUE 1

#if USE_STL_VECTOR
	#include <vector>
	namespace Havana::Utils
	{
		template<typename T>
		using vector = std::vector<T>;

		template<typename T>
		void EraseUnordered(std::vector<T>& v, size_t index)
		{
			if (v.size() > 1)
			{
				std::iter_swap(v.begin() + index, v.end() - 1);
				v.pop_back();
			}
			else
			{
				v.clear();
			}
		}
	}
#endif

#if USE_STL_DEQUE
	#include <deque>
	namespace Havana::Utils
	{
		template<typename T>
		using deque = std::deque<T>;
	}
#endif

namespace Havana::Utils
{
	// TODO: implement our own containers
}