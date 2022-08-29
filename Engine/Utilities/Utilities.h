#pragma once

// Set these flags to 1 to use STL vector and deque
// Set these flags to 0 to use custom implementeation of those containers
#define USE_STL_VECTOR 0
#define USE_STL_DEQUE 1

#if USE_STL_VECTOR
	#include <vector>
	namespace havana::utl
	{
		template<typename T>
		using vector = std::vector<T>;

		template<typename T>
		void erase_unordered(T& v, size_t index)
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
#else
	#include "Vector.h"
	namespace havana::utl
	{
		template<typename T>
		void erase_unordered(T& v, size_t index)
		{
			v.erase_unordered(index);
		}
	}
#endif

#if USE_STL_DEQUE
	#include <deque>
	namespace havana::utl
	{
		template<typename T>
		using deque = std::deque<T>;
	}
#endif

namespace havana::utl
{
	// TODO: implement our own containers
}

#include "FreeList.h"
#ifndef _WIN64
template < typename T, size_t N >
size_t constexpr _countof(T(&arr)[N])
{
	return std::extent< T[N] >::value;
}
#endif // !_WIN64