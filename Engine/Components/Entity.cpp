#include "Entity.h"
#include "Transform.h"
#include "Script.h"

namespace havana::Entity
{
	namespace // anonymous namespace
	{
		utl::vector <Transform::Component>	transforms;
		utl::vector <Script::Component>		scripts;
		utl::vector<Id::generation_type>		generations;
		utl::deque<entity_id>					freeIDs;
	}
	
	Entity CreateEntity(EntityInfo info)
	{
		assert(info.transform); // All entities must have a transform component
		if (!info.transform) return Entity{};

		entity_id id;

		if (freeIDs.size() > Id::minDeletedElements)
		{
			id = freeIDs.front();
			assert(!IsAlive(id));
			freeIDs.pop_front();
			id = entity_id{ Id::NewGeneration(id) };
			++generations[Id::Index(id)];
		}
		else
		{
			id = entity_id{ (Id::id_type)generations.size() };
			generations.push_back(0);

			// Resize components vector to match the amount of entities
			// Using emplace_back() over resize() reduces the amount of memory allocations
			transforms.emplace_back();
			scripts.emplace_back();
		}

		const Entity newEntity{ id };
		const Id::id_type index{ Id::Index(id) };

		// Create transform component
		assert(!transforms[index].is_valid());
		transforms[index] = Transform::Create(*info.transform, newEntity);
		if (!transforms[index].is_valid()) return {};
		
		// Create script component
		if (info.script && info.script->script_creator)
		{
			assert(!scripts[index].is_valid());
			scripts[index] = Script::Create(*info.script, newEntity);
			assert(scripts[index].is_valid());
		}

		return newEntity;
	}

	void RemoveEntity(entity_id id)
	{
		const Id::id_type index{ Id::Index(id) };
		assert(IsAlive(id));

		if (scripts[index].is_valid())
		{
			Script::Remove(scripts[index]);
			scripts[index] = {};
		}

		Transform::Remove(transforms[index]);
		transforms[index] = {};
		freeIDs.push_back(id);
	}

	bool IsAlive(entity_id id)
	{
		assert(Id::is_valid(id));
		const Id::id_type index{ Id::Index(id) };
		assert(index < generations.size());
		return (generations[index] == Id::Generation(id) && transforms[index].is_valid());
	}

	// Entity class method implementations
	Transform::Component Entity::Transform() const
	{
		assert(IsAlive(m_id));
		const Id::id_type index{ Id::Index(m_id) };
		return transforms[index];
	}

	Script::Component Entity::Script() const
	{
		assert(IsAlive(m_id));
		const Id::id_type index{ Id::Index(m_id) };
		return scripts[index];
	}
}