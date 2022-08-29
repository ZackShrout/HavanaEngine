#pragma once
#include "CommonHeaders.h"

namespace havana::utl
{
//#ifndef _WIN64
//	#include <type_traits>
//
//	// Default case: non-iterator type
//	template<class T, class = void>
//	struct _Is_iterator : std::false_type {};
//
//	// Specialized case: iterator types
//	template<class T>
//	struct _Is_iterator<T, std::void_t< typename std::iterator_traits<T>::difference_type,
//										typename std::iterator_traits<T>::pointer,
//										typename std::iterator_traits<T>::reference,
//										typename std::iterator_traits<T>::value_type,
//										typename std::iterator_traits<T>::iterator_category>> : std::true_type {};
//	
//	template<class T>
//	constexpr bool _Is_iterator_v = _Is_iterator<T>::value;
//#endif // !_WIN64
	
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
	
	//#ifdef _WIN64
	//	template<typename it, typename = std::enable_if_t<std::_Is_iterator_v<it>>>
	//#else
	//	template<typename it, typename = std::enable_if_t<_Is_iterator_v<it>>>
	//#endif // _WIN64
	//	constexpr explicit vector(it first, it last)
	//	{
	//		for (; first != last; first++)
	//		{
	//			emplace_back(*first);
	//		}
	//	}

		// Destructs the vector and its items as specified in the template argument
		~vector() { destroy(); }

		// Copy-constructor. Constructs by copying another vector. The items
		// in the copied vector must be copyable.
		constexpr vector(const vector& o) { *this = o; }
		
		// Move-constructor. Constructs by moving another vector.
		// The original vector will be empty after move.
		constexpr vector(vector&& o) : _capacity{ o._capacity }, _size{ o._size }, _data{ o._data } {o.reset(); }

		// Copy-assignment operator. Clears this vector and copies items
		// from another vector. The items must be copyable.
		constexpr vector& operator=(const vector& o)
		{
			assert(this != std::addressof(o));
			if (this != std::addressof(o))
			{
				clear();
				reserve(o._size);
				for (auto& item : o)
				{
					emplace_back(item);
				}
				assert(_size == o._size);
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
				destroy();
				move(o);
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
			if (_size == _capacity)
			{
				reserve(((_capacity + 1) * 3) >> 1); // reserves 50% more space
			}
			assert(_size < _capacity);

			T* const item{ new (std::addressof(_data[_size])) T(std::forward<params>(p)...) };
			++_size;

			return *item;
		}

		// Resizes the vector and initializes new items with their default value.
		constexpr void resize(u64 new_size)
		{
			static_assert(std::is_default_constructible<T>::value, "Type must be default-constructable.");

			if (new_size > _size)
			{
				reserve(new_size);
				while (_size < new_size)
				{
					emplace_back();
				}
			}
			else if (new_size < _size)
			{
				if constexpr (destruct)
				{
					destruct_range(new_size, _size);
				}

				_size = new_size;
			}

			// Do nothing if new_size == _size
			assert(new_size == _size);
		}

		// Resizes the vector and initializes new items by copying 'value'.
		constexpr void resize(u64 new_size, const T& value)
		{
			static_assert(std::is_copy_constructible<T>::value, "Type must be copy-constructable.");
			if (new_size > _size)
			{
				reserve(new_size);
				while (_size < new_size)
				{
					emplace_back(value);
				}
			}
			else if (new_size < _size)
			{
				if constexpr (destruct)
				{
					destruct_range(new_size, _size);
				}

				_size = new_size;
			}

			// Do nothing if new_size == _size
			assert(new_size == _size);
		}

		// Allocates memory to contain the specified number of items.
		constexpr void reserve(u64 new_capacity)
		{
			if (new_capacity > _capacity)
			{
				// NOTE: realoc() will automatically copy the data in the buffer if a new region of memory is allocated
				void* new_buffer{ realloc(_data, new_capacity * sizeof(T)) };
				assert(new_buffer);
				if (new_buffer)
				{
					_data = static_cast<T*>(new_buffer);
					_capacity = new_capacity;
				}
			}
		}

		// Removes the item at specified index
		constexpr T* const erase(u64 index)
		{
			assert(_data && index < _size);
			return erase(std::addressof(_data[index]));
		}

		// Removes the item at specifies location
		constexpr T* const erase(T* const item)
		{
			assert(_data && item >= std::addressof(_data[0]) && item < std::addressof(_data[_size]));
			if constexpr (destruct) item->~T();
			--_size;
			if (item < std::addressof(_data[_size]))
			{
				memcpy(item, item + 1, (std::addressof(_data[_size]) - item) * sizeof(T));
			}

			return item;
		}

		// Same as erase() but faster because it just copies the last item
		constexpr T* const erase_unordered(u64 index)
		{
			assert(_data && index < _size);
			return erase_unordered(std::addressof(_data[index]));
		}

		// Same as erase() but faster because it just copies the last item 
		constexpr T* const erase_unordered(T* const item)
		{
			assert(_data && item >= std::addressof(_data[0]) && item < std::addressof(_data[_size]));
			if constexpr (destruct) item->~T();
			--_size;
			if (item < std::addressof(_data[_size]))
			{
				memcpy(item, std::addressof(_data[_size]), sizeof(T));
			}

			return item;
		}

		// Clears vector and destructs items as specified in the template argument
		constexpr void clear()
		{
			if constexpr (destruct)
			{
				destruct_range(0, _size);
			}

			_size = 0;
		}

		// Swaps two vectors
		constexpr void swap(vector& o)
		{
			if (this != std::addressof(o))
			{
				auto temp(std::move(o));
				o.move(*this);
				move(temp);
			}
		}

		// Pointer to the start of data, may be null
		[[nodiscard]] constexpr T* data()
		{
			return _data;
		}

		// Pointer to the start of data, may be null
		[[nodiscard]] constexpr T* const data() const
		{
			return _data;
		}

		// Returns true if empty
		[[nodiscard]] constexpr bool empty() const
		{
			return _size == 0;
		}
		
		// Returns number of items in the vector
		[[nodiscard]] constexpr u64 size() const
		{
			return _size;
		}

		// Returns the capacity of the vector
		[[nodiscard]] constexpr u64 capacity() const
		{
			return _capacity;
		}

		// Indexing operator - returns a reference to the item at the specified index.
		[[nodiscard]] constexpr T& operator[](u64 index)
		{
			assert(_data && index < _size);
			return _data[index];
		}

		// Indexing operator - returns a constant reference to the item at the specified index.
		[[nodiscard]] constexpr const T& operator[](u64 index) const
		{
			assert(_data && index < _size);
			return _data[index];
		}

		// Returns a reference to the first element of the array - will cause access violation if called on an empty vector
		[[nodiscard]] constexpr T& front()
		{
			assert(_data && _size);
			return _data[0];
		}

		// Returns a constant reference to the first element of the array - will cause access violaion if called on an empty vector
		[[nodiscard]] constexpr const T& front() const
		{
			assert(_data && _size);
			return _data[0];
		}

		// Returns a reference to the last element of the array - will cause access violaion if called on an empty vector
		[[nodiscard]] constexpr T& back()
		{
			assert(_data && _size);
			return _data[_size - 1];
		}

		// Returns a constant reference to the last element of the array - will cause access violation if called on an empty vector
		[[nodiscard]] constexpr const T& back() const
		{
			assert(_data && _size);
			return _data[_size - 1];
		}

		// Returns a pointer to the first element of the array - return null when vector is empty
		[[nodiscard]] constexpr T* begin()
		{
			return std::addressof(_data[0]);
		}

		// Returns a constant pointer to the first element of the array - return null when vector is empty
		[[nodiscard]] constexpr const T* begin() const
		{
			return std::addressof(_data[0]);
		}

		// Returns a pointer to the last element of the array - return null when vector is empty
		[[nodiscard]] constexpr T* end()
		{
			assert(!(_data == nullptr && _size > 0));
			return std::addressof(_data[_size]);
		}

		// Returns a constant pointer to the last element of the array - return null when vector is empty
		[[nodiscard]] constexpr const T* end() const
		{
			assert(!(_data == nullptr && _size > 0));
			return std::addressof(_data[_size]);
		}

	private:
		constexpr void move(vector& o)
		{
			_capacity = o._capacity;
			_size = o._size;
			_data = o._data;
			o.reset();
		}

		constexpr void reset()
		{
			_capacity = 0;
			_size = 0;
			_data = nullptr;
		}

		constexpr void destruct_range(u64 first, u64 last)
		{
			assert(destruct);
			assert(first <= _size && last <= _size && first <= last);
			if (_data)
			{
				for (; first != last; ++first)
				{
					_data[first].~T();
				}
			}
		}

		constexpr void destroy()
		{
			assert([&] { return _capacity ? _data != nullptr : _data == nullptr; }());
			clear();
			_capacity = 0;
			if (_data) free(_data);
			_data = nullptr;
		}

		u64 _capacity{ 0 };
		u64 _size{ 0 };
		T*	_data{ nullptr };
	};
}