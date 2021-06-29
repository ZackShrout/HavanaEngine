#pragma once
#include "CommonHeaders.h"

namespace Havana::Id
{
	// Define ID type for ECS system
	using id_type = u32;

	// constants for index and generation bits for ECS system
	namespace Detail
	{
		constexpr u32 GENERATION_BITS{ 10 };
		constexpr u32 INDEX_BITS{ sizeof(id_type) * 8 - GENERATION_BITS };
		constexpr id_type INDEX_MASK{ (id_type{1} << INDEX_BITS) - 1 };
		constexpr id_type GENERATION_MASK{ (id_type{1} << GENERATION_BITS) - 1 };
	}
	constexpr id_type INVALID_ID{ id_type(-1) };
	constexpr u32 minDeletedElements{ 1024 };

	// Define generation type for ECS system
	using generation_type = std::conditional_t<Detail::GENERATION_BITS <= 16, std::conditional_t<Detail::GENERATION_BITS <= 8, u8, u16>, u32>;

	// ASSERTIONS
	static_assert(sizeof(generation_type) * 8 >= Detail::GENERATION_BITS); // generation_type can be no larger than u32
	static_assert(sizeof(id_type) - sizeof(generation_type) > 0); // Enforces id_type larger than generation_type

	// METHODS
	constexpr bool IsValid(id_type id)
	{
		return id != INVALID_ID;
	}

	constexpr id_type Index(id_type id)
	{
		id_type index{ id & Detail::INDEX_MASK };
		assert(index != Detail::INDEX_MASK);
		return index;
	}

	constexpr id_type Generation(id_type id)
	{
		return (id >> Detail::INDEX_BITS) & Detail::GENERATION_MASK;
	}

	constexpr id_type NewGeneration(id_type id)
	{
		const id_type generation{ Id::Generation(id) + 1 };
		assert(generation < (((u64)1 << Detail::GENERATION_BITS) - 1));
		return Index(id) | (generation << Detail::INDEX_BITS);
	}

#if _DEBUG
	namespace internal
	{
		struct IdBase
		{
			constexpr explicit IdBase(id_type id) : id{ id } {}
			constexpr operator id_type() const { return id; }
		private:
			id_type id;
		};
	}
	#define DEFINE_TYPED_ID(name)								\
			struct name final : Id::internal::IdBase			\
			{													\
				constexpr explicit name(Id::id_type id)			\
					: IdBase{ id } {}							\
				constexpr name() : IdBase{ 0 } {}				\
			};
#else
	#define DEFINE_TYPED_ID(name) using name = Id::id_type
#endif
}
