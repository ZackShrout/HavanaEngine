#include "Transform.h"

namespace Havana::Transform
{
	namespace // anonymous namespace
	{
		Utils::vector<Math::Vec3> positions;
		Utils::vector<Math::Vec4> rotations;
		Utils::vector<Math::Vec3> scales;
	}

	Component Create(InitInfo info, Entity::Entity entity)
	{
		assert(entity.IsValid());
		const Id::id_type entityIndex{ Id::Index(entity.GetID()) };

		// If our entity has filled a hole in the vector of entities, put the
		// transform component into that same slot in the vector of transforms
		if (positions.size() > entityIndex)
		{
#ifdef _WIN64
			positions[entityIndex] = Math::Vec3(info.position);
			rotations[entityIndex] = Math::Vec4(info.rotation);
			scales[entityIndex] = Math::Vec3(info.scale);
#elif __linux__
			positions[entityIndex] = Math::Vec3(info.position[0], info.position[1], info.position[2]);
			rotations[entityIndex] = Math::Vec4(info.rotation[0], info.rotation[1], info.rotation[2], 
												info.rotation[3]);
			scales[entityIndex] = Math::Vec3(info.scale[0], info.scale[1], info.scale[2]);
#endif
		}
		else // If not, place it in the back with our entity
		{
			assert(positions.size() == entityIndex);
#ifdef _WIN64
			positions.emplace_back(info.position);
			rotations.emplace_back(info.rotation);
			scales.emplace_back(info.scale);
#elif __linux__
			positions.emplace_back(Math::Vec3(info.position[0], info.position[1], info.position[2]));
			rotations.emplace_back(Math::Vec4(info.rotation[0], info.rotation[1], info.rotation[2], 
												info.rotation[3]));
			scales.emplace_back(Math::Vec3(info.scale[0], info.scale[1], info.scale[2]));
#endif
		}

		return Component{ transform_id{ entity.GetID() } };
	}

	void Remove([[maybe_unused]]Component component)
	{
		assert(component.IsValid());
	}

	
	// Transform class method implementaions
	Math::Vec3 Component::Position() const
	{
		assert(IsValid());
		return positions[Id::Index(m_id)];
	}
	
	Math::Vec4 Component::Rotation() const
	{
		assert(IsValid());
		return rotations[Id::Index(m_id)];
	}

	Math::Vec3 Component::Scale() const
	{
		assert(IsValid());
		return scales[Id::Index(m_id)];
	}
}