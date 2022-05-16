#pragma once
#include "CommonHeaders.h"

namespace Havana::Utils
{
	// NOTE: *IMPORTANT* This utility class is intended for local use only (i.e. within one function).
	//		 Do not keep instances around as member variables!
	class BlobStreamReader
	{
	public:
		DISABLE_COPY_AND_MOVE(BlobStreamReader);
		explicit BlobStreamReader(const u8* buffer)
			: _buffer{buffer}, _position{buffer}
		{
			assert(buffer);
		}

		// This template function is intended to read primitive types (e.g. int, float, bool)
		template<typename T>
		[[nodiscard]] T Read()
		{
			static_assert(std::is_arithmetic_v<T>, "Template argument should be a primitive type.");
			T value{ *((T*)_position) };
			_position += sizeof(T);
			return value;
		}

		// Reads 'length' bytes into 'buffer.' The caller is responsible to allocate enough memory in buffer.
		void Read(u8* buffer, size_t length)
		{
			memcpy(buffer, _position, length);
			_position += length;
		}

		void Skip(size_t offset)
		{
			_position += offset;
		}

		[[nodiscard]] constexpr const u8* const BufferStart() const { return _buffer; }
		[[nodiscard]] constexpr const u8* const Position() const { return _position; }
		[[nodiscard]] constexpr size_t Offset() const { return _position - _buffer; }
	private:
		const u8* const	_buffer;
		const u8*		_position;
	};

	// NOTE: *IMPORTANT* This utility class is intended for local use only (i.e. within one function).
	//		 Do not keep instances around as member variables!
	class BlobStreamWriter
	{
	public:
		DISABLE_COPY_AND_MOVE(BlobStreamWriter);
		explicit BlobStreamWriter(u8* buffer, size_t bufferSize)
			: _buffer{ buffer }, _position{ buffer }, _bufferSize{bufferSize}
		{
			assert(buffer && bufferSize);
		}

		// This template function is intended to write primitive types (e.g. int, float, bool)
		template<typename T>
		void Write(T value)
		{
			static_assert(std::is_arithmetic_v<T>, "Template argument should be a primitive type.");
			assert(&_position[sizeof(T)] <= &_buffer[_bufferSize]);
			*((T*)_position) = value;
			_position += sizeof(T);
		}

		// Writes 'length' chars into 'buffer.'
		void Write(const char* buffer, size_t length)
		{
			assert(&_position[length] <= &_buffer[_bufferSize]);
			memcpy(_position, buffer, length);
			_position += length;
		}

		// Writes 'length' bytes into 'buffer.'
		void Write(const u8* buffer, size_t length)
		{
			assert(&_position[length] <= &_buffer[_bufferSize]);
			memcpy(_position, buffer, length);
			_position += length;
		}

		void Skip(size_t offset)
		{
			assert(&_position[offset] <= &_buffer[_bufferSize]);
			_position += offset;
		}

		[[nodiscard]] constexpr const u8* const BufferStart() const { return _buffer; }
		[[nodiscard]] constexpr const u8* const BufferEnd() const { return &_buffer[_bufferSize]; }
		[[nodiscard]] constexpr const u8* const Position() const { return _position; }
		[[nodiscard]] constexpr size_t Offset() const { return _position - _buffer; }
	private:
		u8* const	_buffer;
		u8*			_position;
		size_t		_bufferSize;
	};
}