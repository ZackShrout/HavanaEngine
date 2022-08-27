#include "Transform.h"

namespace havana::Transform
{
	namespace // anonymous namespace
	{
		utl::vector<math::v3> positions;
		utl::vector<math::v3> orientations;
		utl::vector<math::v4> rotations;
		utl::vector<math::v3> scales;

		math::v3 CalculateOrientation(math::v4 rotation)
		{
			using namespace DirectX;
			XMVECTOR rotationQuat{ XMLoadFloat4(&rotation) };
			XMVECTOR front{ XMVectorSet(0.f, 0.f, 1.f, 0.f) };
			math::v3 orientation;
			XMStoreFloat3(&orientation, XMVector3Rotate(front, rotationQuat));
			return orientation;
		}
	}

	Component Create(InitInfo info, Entity::Entity entity)
	{
		assert(entity.is_valid());
		const id::id_type entityIndex{ id::index(entity.GetID()) };

		// If our entity has filled a hole in the vector of entities, put the
		// transform component into that same slot in the vector of transforms
		if (positions.size() > entityIndex)
		{
#ifdef _WIN64
			math::v4 rotation{ info.rotation };
			rotations[entityIndex] = rotation;
			orientations[entityIndex] = CalculateOrientation(rotation);
			positions[entityIndex] = math::v3{ info.position };
			scales[entityIndex] = math::v3{ info.scale };
#elif __linux__
			math::v4 rotation{ info.rotation[0], info.rotation[1], info.rotation[2], info.rotation[3] };
			rotations[entityIndex] = rotation;
			orientations[entityIndex] = CalculateOrientation(rotation);
			positions[entityIndex] = math::v3(info.position[0], info.position[1], info.position[2]);
			scales[entityIndex] = math::v3(info.scale[0], info.scale[1], info.scale[2]);
#endif
		}
		else // If not, place it in the back with our entity
		{
			assert(positions.size() == entityIndex);
#ifdef _WIN64
			rotations.emplace_back(info.rotation);
			orientations.emplace_back(CalculateOrientation(math::v4{ info.rotation }));
			positions.emplace_back(info.position);
			scales.emplace_back(info.scale);
#elif __linux__
			positions.emplace_back(math::v3(info.position[0], info.position[1], info.position[2]));
			rotations.emplace_back(math::v4(info.rotation[0], info.rotation[1], info.rotation[2], 
												info.rotation[3]));
			scales.emplace_back(math::v3(info.scale[0], info.scale[1], info.scale[2]));
#endif
		}

		return Component{ transform_id{ entity.GetID() } };
	}

	void Remove([[maybe_unused]]Component component)
	{
		assert(component.is_valid());
	}

	
	// Transform class method implementaions
	math::v4 Component::Rotation() const
	{
		assert(is_valid());
		return rotations[id::index(m_id)];
	}

	math::v3 Component::Orientation() const
	{
		assert(is_valid());
		return orientations[id::index(m_id)];
	}

	math::v3 Component::Position() const
	{
		assert(is_valid());
		return positions[id::index(m_id)];
	}


	math::v3 Component::Scale() const
	{
		assert(is_valid());
		return scales[id::index(m_id)];
	}
}