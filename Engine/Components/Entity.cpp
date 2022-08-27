#include "Entity.h"
#include "Transform.h"
#include "Script.h"

namespace havana::Entity
{
	namespace // anonymous namespace
	{
		utl::vector <Transform::Component>	transforms;
		utl::vector <Script::Component>		scripts;
		utl::vector<id::generation_type>		generations;
		utl::deque<entity_id>					freeIDs;
	}
	
	Entity CreateEntity(EntityInfo info)
	{
		assert(info.transform); // All entities must have a transform component
		if (!info.transform) return Entity{};

		entity_id id;

		if (freeIDs.size() > id::mindeleted_elements)
		{
			id = freeIDs.front();
			assert(!IsAlive(id));
			freeIDs.pop_front();
			id = entity_id{ id::new_generation(id) };
			++generations[id::index(id)];
		}
		else
		{
			id = entity_id{ (id::id_type)generations.size() };
			generations.push_back(0);

			// Resize components vector to match the amount of entities
			// Using emplace_back() over resize() reduces the amount of memory allocations
			transforms.emplace_back();
			scripts.emplace_back();
		}

		const Entity newEntity{ id };
		const id::id_type index{ id::index(id) };

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
		const id::id_type index{ id::index(id) };
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
		assert(id::is_valid(id));
		const id::id_type index{ id::index(id) };
		assert(index < generations.size());
		return (generations[index] == id::generation(id) && transforms[index].is_valid());
	}

	// Entity class method implementations
	Transform::Component Entity::Transform() const
	{
		assert(IsAlive(m_id));
		const id::id_type index{ id::index(m_id) };
		return transforms[index];
	}

	Script::Component Entity::Script() const
	{
		assert(IsAlive(m_id));
		const id::id_type index{ id::index(m_id) };
		return scripts[index];
	}
}