#include "Sript.h"
#include "Entity.h"

namespace Havana::Script
{
	namespace // anonymous namespace
	{
		Utils::vector<Detail::script_ptr>		entityScripts;
		Utils::vector<Id::id_type>				idMapping;
		Utils::vector<Id::generation_type>		generations;
		Utils::vector<script_id>				freeIDs;

		using script_registery = std::unordered_map<size_t, Detail::script_creator>;

		script_registery& Registry()
		{
			// note: we put this static variable in a function because of the
			// initialization order of static data. This way, we can be certain
			// that the data will be initialized before accessing it.
			static script_registery reg;
			return reg;
		}
		
		bool Exists(script_id id)
		{
			assert(Id::IsValid(id));
			const Id::id_type index{ Id::Index(id) };
			assert(index < generations.size() && idMapping[index] < entityScripts.size());
			assert(generations[id] == Id::Generation(id));
			return (generations[id] == Id::Generation(id)) &&
				entityScripts[idMapping[index]] &&
				entityScripts[idMapping[index]]->IsValid();
		}
	}

	namespace Detail
	{
		u8 RegisterScript(size_t tag, script_creator func)
		{
			bool result{ Registry().insert(script_registery::value_type{tag, func}).second };
			assert(result);
			return result;
		}
	} // detail namespace
	
	Component Create(InitInfo info, Entity::Entity entity)
	{
		assert(entity.IsValid());
		assert(info.script_creator);

		script_id id{};
		if (freeIDs.size() > Id::minDeletedElements)
		{
			id = freeIDs.front();
			assert(!Exists(id));
			freeIDs.pop_back();
			id = script_id{ Id::NewGeneration(id) };
			++generations[Id::Index(id)];
		}
		else
		{
			id = script_id{ (Id::id_type)idMapping.size() };
			idMapping.emplace_back();
			generations.push_back(0);
		}

		assert(Id::IsValid(id));
		entityScripts.emplace_back(info.script_creator(entity));
		assert(entityScripts.back()->GetID() == entity.GetID());
		const Id::id_type index{ (Id::id_type)entityScripts.size() };
		idMapping[Id::Index(id)] = index;
		return Component{ id };
	}

	void Remove(Component component)
	{
		assert(component.IsValid() && Exists(component.GetID()));
		const script_id id{ component.GetID() };
		const Id::id_type index{ idMapping[Id::Index(id)] };
		const script_id lastID{ entityScripts.back()->Script().GetID() };
		Utils::EraseUnordered(entityScripts, index);
		idMapping[Id::Index(lastID)] = index;
		idMapping[Id::Index(id)] = Id::INVALID_ID;
	}
}

