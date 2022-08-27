#pragma once

#include "CommonHeaders.h"

namespace havana::utl
{
#if USE_STL_VECTOR
	#pragma message("WARNING: using utl::free_list with std::vector results in duplicate calls to class constructor!")
#endif

	template<typename T>
	class free_list
	{
		static_assert(sizeof(T) >= sizeof(u32));
	public:
		free_list() = default;
		explicit free_list(u32 count) { m_array.reserve(count); }
		~free_list() 
		{ 
			assert(!m_size);
#if USE_STL_VECTOR
			memset(m_array.data(), 0, m_array.size() * sizeof(T));
#endif
		}

		template<class... params>
		constexpr u32 add(params&&... p)
		{
			u32 id{ u32_invalid_id };
			if (m_nextFreeIndex == u32_invalid_id)
			{
				id = (u32)m_array.size();
				m_array.emplace_back(std::forward<params>(p)...);
			}
			else
			{
				id = m_nextFreeIndex;
				assert(id < m_array.size() && AlreadyRemoved(id));
				m_nextFreeIndex = *(const u32* const)std::addressof(m_array[id]);
				new (std::addressof(m_array[id])) T(std::forward<params>(p)...);
			}
			++m_size;
			return id;
		}

		constexpr void remove(u32 id)
		{
			assert(id < m_array.size() && !AlreadyRemoved(id));
			T& item{ m_array[id] };
			item.~T();
			DEBUG_OP(memset(std::addressof(m_array[id]), 0xcc, sizeof(T)));
			*(u32* const)std::addressof(m_array[id]) = m_nextFreeIndex;
			m_nextFreeIndex = id;
			--m_size;
		}

		constexpr u32 size() const
		{
			return m_size;
		}

		constexpr u32 capacity() const
		{
			return m_array.size();
		}

		constexpr bool empty() const
		{
			return m_size == 0;
		}

		[[nodiscard]] constexpr T& operator[](u32 id)
		{
			assert(id < m_array.size() && !AlreadyRemoved(id));
			return m_array[id];
		}
		
		[[nodiscard]] constexpr const T& operator[](u32 id) const
		{
			assert(id < m_array.size() && !AlreadyRemoved(id));
			return m_array[id];
		}

	private:
		constexpr bool AlreadyRemoved(u32 id)
		{
			// When sizeof(T) == sizeof(u32) we can't test if the item was already removed
			if constexpr (sizeof(T) > sizeof(u32))
			{
				u32 i{ sizeof(u32) }; // Skip the first 4 bytes
				const u8* const p{ (const u8* const)std::addressof(m_array[id]) };
				while ((p[i] == 0xcc) && (i < sizeof(T))) ++i;
				return i == sizeof(T);
			}
			else
			{
				return true;
			}
		}
#if USE_STL_VECTOR
		utl::vector<T>			m_array;
#else
		utl::vector<T, false>		m_array;
#endif
		u32							m_nextFreeIndex{ u32_invalid_id };
		u32							m_size{ 0 };
	};
}