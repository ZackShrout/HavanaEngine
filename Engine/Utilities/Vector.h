#pragma once
#include "CommonHeaders.h"

namespace Havana::Utils
{
	// A vector class similar to std::vector with basic functionality.
	// The user can specify in the template argument whether they want
	// the element's desctructor to be called when being removed or while
	// clearing/destructing the vector.
	template<typename T, bool destruct = true>
	class vector
	{
	public:
		// Default constructor, doesn't allocate memory.
		vector() = default;
		
		// Constructor resizes the vector and initializes 'count' items.
		constexpr explicit vector(u64 count) { resize(count); }
		
		// Constructor resizes the vector and initializes 'count' items using 'value'.
		constexpr explicit vector(u64 count, const T& value) { resize(count, value); }
		
		~vector() { destroy(); }

		// Copy-constructor. Constructs by copying another vector. The items
		// in the copied vector must be copyable.
		constexpr vector(const vector& o) { *this = 0; }
		
		// Move-constructor. Constructs by moving another vector.
		// The original vector will be empty after move.
		constexpr vector(const vector&& o) : m_capacity{ o.m_capacity }, m_size{ o.m_size }, m_data{ o.m_data } {o.Reset(); }

		// Copy-assignment operator. Clears this vector and copies items
		// from another vector. The items must be copyable.
		constexpr vector& operator=(const vector& o)
		{
			assert(this != std::addressof(o));
			if (this != std::addressof(o))
			{
				clear();
				reserve(o.m_size);
				for (auto& item : o)
				{
					emplace_back(item);
				}
				assert(m_size == o.m_size);
			}

			return *this;
		}

		// Move-assignment operator. Frees all resources in this vector and
		// moves the other vector into this one.
		constexpr vector& operator=(vector&& o)
		{
			assert(this != std::addressof(o));
			if (this != std::addressof(o))
			{
				Destroy();
				Move(o);
			}

			return *this;
		}


		// Clears vector and destructs items as specified in the template argument
		constexpr void clear()
		{
			if constexpr (destruct)
			{
				DestructRange(0, m_size);
			}

			m_size = 0;
		}

	private:
		constexpr void Move(vector& o)
		{
			m_capacity = o.m_capacity;
			m_size = o.m_size;
			m_data = o.m_data;
			o.Reset();
		}

		constexpr void Reset()
		{
			m_capacity = 0;
			m_size = 0;
			m_data = nullptr;
		}

		constexpr void DestructRange(u64 first, u64 last)
		{
			assert(destruct);
			assert(first <= m_size && last <= m_size && first <= last);
			if (m_data)
			{
				for (; first != last; first++)
				{
					m_data[first].~T();
				}
			}
		}

		constexpr void Destroy()
		{
			assert([&] { return m_capacity ? m_data != nullptr : m_data == nullptr; }());
			clear();
			m_capacity = 0;
			if (m_data) free(m_data);
			m_data = nullptr;
		}

		u64 m_capacity{ 0 };
		u64 m_size{ 0 };
		T* m_data{ nullptr };
	};
}