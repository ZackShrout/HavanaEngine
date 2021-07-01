#include "Script.h"
#include "Entity.h"

namespace Havana::Script
{
	namespace // anonymous namespace
	{
		Utils::vector<Detail::script_ptr>		entityScripts;
		Utils::vector<Id::id_type>				idMapping;
		Utils::vector<Id::generation_type>		generations;
		Utils::vector<script_id>				freeIDs;

		using script_registry = std::unordered_map<size_t, Detail::script_creator>;

		script_registry& Registry()
		{
			// note: we put this static variable in a function because of the
			// initialization order of static data. This way, we can be certain
			// that the data will be initialized before accessing it.
			static script_registry reg;
			return reg;
		}

#ifdef USE_WITH_EDITOR
		Utils::vector<std::string>& ScriptNames()
		{
			// note: we put this static variable in a function because of the
			// initialization order of static data. This way, we can be certain
			// that the data will be initialized before accessing it.
			static Utils::vector<std::string> names;
			return names;
		}
#endif // USE_WITH_EDITOR

		
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
			bool result{ Registry().insert(script_registry::value_type{tag, func}).second };
			assert(result);
			return result;
		}


		script_creator GetScriptCreatorDll(size_t tag)
		{
			auto script = Havana::Script::Registry().find(tag);
			assert(script != Havana::Script::Registry().end() && script->first == tag);
			return script->second;
		}

#ifdef USE_WITH_EDITOR
		u8 AddScriptName(const char* name)
		{
			ScriptNames().emplace_back(name);
			return true;
		}
#endif // USE_WITH_EDITOR
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
		const Id::id_type index{ (Id::id_type)entityScripts.size() };
		entityScripts.emplace_back(info.script_creator(entity));
		assert(entityScripts.back()->GetID() == entity.GetID());
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

#ifdef USE_WITH_EDITOR

extern "C" __declspec(dllexport)
LPSAFEARRAY GetScriptNamesDll()
{
	const u32 size{ (u32)Havana::Script::ScriptNames().size() };
	if (!size) return nullptr; // If there are no script, exit early
	CComSafeArray<BSTR> names(size);
	
	for (u32 i{ 0 }; i < size; i++)
	{
		names.SetAt(i, A2BSTR_EX(Havana::Script::ScriptNames()[i].c_str()), false);
	}

	return names.Detach(); // Detach() will move the memory freeing task to the .NET side for Auto GC
}
#endif // USE_WITH_EDITOR


