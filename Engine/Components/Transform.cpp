#include "Transform.h"
#include "Entity.h"

namespace havana::transform
{
	namespace
	{
		utl::vector<math::m4x4>	to_world;
		utl::vector<math::m4x4>	inv_world;
		utl::vector<math::v4>	rotations;
		utl::vector<math::v3>	orientations;
		utl::vector<math::v3>	positions;
		utl::vector<math::v3>	scales;
		utl::vector<u8>			has_transform;

		void
		calculate_transform_matrices(id::id_type index)
		{
			assert(rotations.size() >= index);
			assert(positions.size() >= index);
			assert(scales.size() >= index);

			using namespace DirectX;
			XMVECTOR r{ XMLoadFloat4(&rotations[index]) };
			XMVECTOR t{ XMLoadFloat3(&positions[index]) };
			XMVECTOR s{ XMLoadFloat3(&scales[index]) };

			XMMATRIX world{ XMMatrixAffineTransformation(s, XMQuaternionIdentity(), r, t) };
			XMStoreFloat4x4(&to_world[index], world);

			// NOTE: (F. Luna) Intro to DirectX 12, section 8.2.2
			world.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			XMMATRIX inverse_world{ XMMatrixInverse(nullptr, world) };
			XMStoreFloat4x4(&inv_world[index], inverse_world);

			has_transform[index] = 1;
		}

		math::v3
		calculate_orientation(math::v4 rotation)
		{
			using namespace DirectX;
			XMVECTOR rotation_quat{ XMLoadFloat4(&rotation) };
			XMVECTOR front{ XMVectorSet(0.f, 0.f, 1.f, 0.f) };
			math::v3 orientation;
			XMStoreFloat3(&orientation, XMVector3Rotate(front, rotation_quat));
			return orientation;
		}
	} // anonymous namespace

	component
	create(init_info info, game_entity::entity entity)
	{
		assert(entity.is_valid());
		const id::id_type entity_index{ id::index(entity.get_id()) };

		// If our entity has filled a hole in the vector of entities, put the
		// transform component into that same slot in the vector of transforms
		if (positions.size() > entity_index)
		{
#ifdef _WIN64
			math::v4 rotation{ info.rotation };
			rotations[entity_index] = rotation;
			orientations[entity_index] = calculate_orientation(rotation);
			positions[entity_index] = math::v3{ info.position };
			scales[entity_index] = math::v3{ info.scale };
			has_transform[entity_index] = 0;
			to_world.emplace_back();
			inv_world.emplace_back();
#elif __linux__
			math::v4 rotation{ info.rotation[0], info.rotation[1], info.rotation[2], info.rotation[3] };
			rotations[entity_index] = rotation;
			orientations[entity_index] = calculate_orientation(rotation);
			positions[entity_index] = math::v3(info.position[0], info.position[1], info.position[2]);
			scales[entity_index] = math::v3(info.scale[0], info.scale[1], info.scale[2]);
			has_transform[entity_index] = 0;
			to_world.emplace_back();
			inv_world.emplace_back();
#endif
		}
		else // If not, place it in the back with our entity
		{
			assert(positions.size() == entity_index);
#ifdef _WIN64
			rotations.emplace_back(info.rotation);
			orientations.emplace_back(calculate_orientation(math::v4{ info.rotation }));
			positions.emplace_back(info.position);
			scales.emplace_back(info.scale);
			has_transform.emplace_back((u8)0);
			to_world.emplace_back();
			inv_world.emplace_back();
#elif __linux__
			positions.emplace_back(math::v3(info.position[0], info.position[1], info.position[2]));
			rotations.emplace_back(math::v4(info.rotation[0], info.rotation[1], info.rotation[2], 
											info.rotation[3]));
			scales.emplace_back(math::v3(info.scale[0], info.scale[1], info.scale[2]));
			has_transform.emplace_back((u8)0);
			to_world.emplace_back();
			inv_world.emplace_back();
#endif
		}

		// NOTE: each entity has a transform component. Therefore, id's for transform components
		//		 are exactly the same as entity ids.
		return component{ transform_id{ entity.get_id() } };
	}

	void
	remove([[maybe_unused]]component c)
	{
		assert(c.is_valid());
	}

	void
	get_transform_matrices(const game_entity::entity_id id, math::m4x4& world, math::m4x4& inverse_world)
	{
		assert(game_entity::entity{ id }.is_valid());

		const id::id_type entity_index{ id::index(id) };
		if (!has_transform[entity_index])
		{
			calculate_transform_matrices(entity_index);
		}

		world = to_world[entity_index];
		inverse_world = inv_world[entity_index];
	}
	
	// Transform class method implementaions
	math::v4
	component::rotation() const
	{
		assert(is_valid());
		return rotations[id::index(_id)];
	}

	math::v3
	component::orientation() const
	{
		assert(is_valid());
		return orientations[id::index(_id)];
	}

	math::v3
	component::position() const
	{
		assert(is_valid());
		return positions[id::index(_id)];
	}

	math::v3
	component::scale() const
	{
		assert(is_valid());
		return scales[id::index(_id)];
	}
}