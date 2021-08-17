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
		
		template<typename it, typename = std::enable_if_t<std::_Is_iterator_v<it>>>
		constexpr explicit vector(it first, it last)
		{
			for (; first != last; first++)
			{
				emplace_back(*first);
			}
		}

		// Destructs the vector and its items as specified in the template argument
		~vector() { Destroy(); }

		// Copy-constructor. Constructs by copying another vector. The items
		// in the copied vector must be copyable.
		constexpr vector(const vector& o) { *this = o; }
		
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

		// Inserts an item at the end of the vector by copying 'value'
		constexpr void push_back(const T& value)
		{
			emplace_back(value);
		}

		// Inserts an item at the end of the vector by moving 'value'
		constexpr void push_back(const T&& value)
		{
			emplace_back(std::move(value));
		}
		
		// Copy-constructs or move-constructs an item at the end of the vector
		template<typename... params>
		constexpr decltype(auto) emplace_back(params&&... p)
		{
			if (m_size == m_capacity)
			{
				reserve(((m_capacity + 1) * 3) >> 1); // reserves 50% more space
			}
			assert(m_size < m_capacity);

			new (std::addressof(m_data[m_size])) T(std::forward<params>(p)...);
			++m_size;

			return m_data[m_size - 1];
		}

		// Resizes the vector and initializes new items with their default value.
		constexpr void resize(u64 newSize)
		{
			static_assert(std::is_default_constructible_v<T>, "Type must be default-constructable.");

			if (newSize > m_size)
			{
				reserve(newSize);
				while (m_size < newSize)
				{
					emplace_back();
				}
			}
			else if (newSize < m_size)
			{
				if constexpr (destruct)
				{
					DestructRange(newSize, m_size);
				}
			}

			// Do nothing if newSize == m_size
			assert(newSize == m_size);
		}

		// Resizes the vector and initializes new items by copying 'value'.
		constexpr void resize(u64 newSize, const T& value)
		{
			static_assert(std::is_copy_constructible_v<T>, "Type must be copy-constructable.");

			if (newSize > m_size)
			{
				reserve(newSize);
				while (m_size < newSize)
				{
					emplace_back(value);
				}
			}
			else if (newSize < m_size)
			{
				if constexpr (destruct)
				{
					DestructRange(newSize, m_size);
				}
			}

			// Do nothing if newSize == m_size
			assert(newSize == m_size);
		}

		// Allocates memory to contain the specified number of items.
		constexpr void reserve(u64 newCapacity)
		{
			if (newCapacity > m_capacity)
			{
				// NOTE: realoc() will automatically copy the data in the buffer if a new region of memory is allocated
				void* newBuffer{ realloc(m_data, newCapacity * sizeof(T)) };
				assert(newBuffer);
				if (newBuffer)
				{
					m_data = static_cast<T*>(newBuffer);
					m_capacity = newCapacity;
				}
			}
		}

		// Removes the item at specified index
		constexpr T* const erase(u64 index)
		{
			assert(m_data && index < m_size);
			return erase(std::addressof(m_data[index]));
		}

		// Removes the item at specifies location
		constexpr T* const erase(T* const item)
		{
			assert(m_data && item >= std::addressof(m_data[0]) && item < std::addressof(m_data[m_size]));
			if constexpr (destruct) item->~T();
			--m_size;
			if (item < std::addressof(m_data[m_size]))
			{
				memcpy(item, item + 1, (std::addressof(m_data[m_size]) - item) * sizeof(T));
			}

			return item;
		}

		// Same as erase() but faster because it just copies the last item
		constexpr T* const erase_unordered(u64 index)
		{
			assert(m_data && index < m_size);
			return erase_unordered(std::addressof(m_data[index]));
		}

		// Same as erase() but faster because it just copies the last item 
		constexpr T* const erase_unordered(T* const item)
		{
			assert(m_data && item >= std::addressof(m_data[0]) && item < std::addressof(m_data[m_size]));
			if constexpr (destruct) item->~T();
			--m_size;
			if (item < std::addressof(m_data[m_size]))
			{
				memcpy(item, std::addressof(m_data[m_size]), sizeof(T));
			}

			return item;
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

		// Swaps two vectors
		constexpr void swap(vector& o)
		{
			if (this != std::addressof(o))
			{
				auto temp(o);
				o = *this;
				*this = temp;
			}
		}

		// Pointer to the start of data, may be null
		[[nodiscard]] constexpr T* data()
		{
			return m_data;
		}

		// Pointer to the start of data, may be null
		[[nodiscard]] constexpr T* const data() const
		{
			return m_data;
		}

		// Returns true if empty
		[[nodiscard]] constexpr bool empty() const
		{
			return m_size == 0;
		}
		
		// Returns number of items in the vector
		[[nodiscard]] constexpr u64 size() const
		{
			return m_size;
		}

		// Returns the capacity of the vector
		[[nodiscard]] constexpr u64 capacity() const
		{
			return m_capacity;
		}

		// Indexing operator - returns a reference to the item at the specified index.
		[[nodiscard]] constexpr T& operator[](u64 index)
		{
			assert(m_data && index < m_size);
			return m_data[index];
		}

		// Indexing operator - returns a constant reference to the item at the specified index.
		[[nodiscard]] constexpr const T& operator[](u64 index) const
		{
			assert(m_data && index < m_size);
			return m_data[index];
		}

		// Returns a reference to the first element of the array - will cause access violation if called on an empty vector
		[[nodiscard]] constexpr T& front()
		{
			assert(m_data && m_size);
			return m_data[0];
		}

		// Returns a constant reference to the first element of the array - will cause access violaion if called on an empty vector
		[[nodiscard]] constexpr const T& front() const
		{
			assert(m_data && m_size);
			return m_data[0];
		}

		// Returns a reference to the last element of the array - will cause access violaion if called on an empty vector
		[[nodiscard]] constexpr T& back()
		{
			assert(m_data && m_size);
			return m_data[m_size - 1];
		}

		// Returns a constant reference to the last element of the array - will cause access violation if called on an empty vector
		[[nodiscard]] constexpr const T& back() const
		{
			assert(m_data && m_size);
			return m_data[m_size - 1];
		}

		// Returns a pointer to the first element of the array - return null when vector is empty
		[[nodiscard]] constexpr T* begin()
		{
			assert(m_data);
			return std::addressof(m_data[0]);
		}

		// Returns a constant pointer to the first element of the array - return null when vector is empty
		[[nodiscard]] constexpr const T* begin() const
		{
			assert(m_data);
			return std::addressof(m_data[0]);
		}

		// Returns a pointer to the last element of the array - return null when vector is empty
		[[nodiscard]] constexpr T* end()
		{
			assert(m_data);
			return std::addressof(m_data[m_size]);
		}

		// Returns a constant pointer to the last element of the array - return null when vector is empty
		[[nodiscard]] constexpr const T* end() const
		{
			assert(m_data);
			return std::addressof(m_data[m_size]);
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