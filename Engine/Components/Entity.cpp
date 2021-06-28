#include "Entity.h"
#include "Transform.h"
#include "Sript.h"

namespace Havana::Entity
{
	namespace // anonymous namespace
	{
		Utils::vector <Transform::Component>	transforms;
		Utils::vector <Script::Component>		scripts;
		Utils::vector<Id::generation_type>		generations;
		Utils::deque<entity_id>					freeIDs;
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
		}

		const Entity newEntity{ id };
		const Id::id_type index{ Id::Index(id) };

		// Create transform component
		assert(!transforms[index].IsValid());
		transforms[index] = Transform::Create(*info.transform, newEntity);
		if (!transforms[index].IsValid()) return {};
		
		// Create script component
		if (info.script && info.script->script_creator)
		{
			assert(!scripts[index].IsValid());
			scripts[index] = Script::Create(*info.script, newEntity);
			assert(scripts[index].IsValid());
		}

		return newEntity;
	}

	void RemoveEntity(entity_id id)
	{
		const Id::id_type index{ Id::Index(id) };
		assert(IsAlive(id));
		Transform::Remove(transforms[index]);
		transforms[index] = {};
		freeIDs.push_back(id);
	}

	bool IsAlive(entity_id id)
	{
		assert(Id::IsValid(id));
		const Id::id_type index{ Id::Index(id) };
		assert(index < generations.size());
		assert(generations[index] == Id::Generation(id));
		return (generations[index] == Id::Generation(id) && transforms[index].IsValid());
	}

	// Entity class method implementations
	Transform::Component Entity::Transform() const
	{
		assert(IsAlive(id));
		const Id::id_type index{ Id::Index(id) };
		return transforms[index];
	}

	Script::Component Entity::Script() const
	{
		assert(IsAlive(id));
		const Id::id_type index{ Id::Index(id) };
		return scripts[index];
	}
}