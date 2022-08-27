#include "Script.h"
#include "Entity.h"

namespace havana::Script
{
	namespace // anonymous namespace
	{
		utl::vector<detail::script_ptr>		entityScripts;
		utl::vector<id::id_type>				idMapping;
		utl::vector<id::generation_type>		generations;
		utl::deque<script_id>				freeIDs;

		using script_registry = std::unordered_map<size_t, detail::script_creator>;

		script_registry& Registry()
		{
			// note: we put this static variable in a function because of the
			// initialization order of static data. This way, we can be certain
			// that the data will be initialized before accessing it.
			static script_registry reg;
			return reg;
		}

#ifdef USE_WITH_EDITOR
		utl::vector<std::string>& ScriptNames()
		{
			// note: we put this static variable in a function because of the
			// initialization order of static data. This way, we can be certain
			// that the data will be initialized before accessing it.
			static utl::vector<std::string> names;
			return names;
		}
#endif // USE_WITH_EDITOR


		bool Exists(script_id id)
		{
			assert(id::is_valid(id));
			const id::id_type index{ id::index(id) };
			assert(index < generations.size() && idMapping[index] < entityScripts.size());
			assert(generations[id] == id::generation(id));
			return (generations[id] == id::generation(id)) &&
				entityScripts[idMapping[index]] &&
				entityScripts[idMapping[index]]->is_valid();
		}
	}

	namespace detail
	{
		u8 RegisterScript(size_t tag, script_creator func)
		{
			bool result{ Registry().insert(script_registry::value_type{tag, func}).second };
			assert(result);
			return result;
		}


		script_creator GetScriptCreatorDll(size_t tag)
		{
			auto script = havana::Script::Registry().find(tag);
			assert(script != havana::Script::Registry().end() && script->first == tag);
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
		assert(entity.is_valid());
		assert(info.script_creator);

		script_id id{};
		if (freeIDs.size() > id::mindeleted_elements)
		{
			id = freeIDs.front();
			assert(!Exists(id));
			freeIDs.pop_front();
			id = script_id{ id::new_generation(id) };
			++generations[id::index(id)];
		}
		else
		{
			id = script_id{ (id::id_type)idMapping.size() };
			idMapping.emplace_back();
			generations.push_back(0);
		}

		assert(id::is_valid(id));
		const id::id_type index{ (id::id_type)entityScripts.size() };
		entityScripts.emplace_back(info.script_creator(entity));
		assert(entityScripts.back()->GetID() == entity.GetID());
		idMapping[id::index(id)] = index;
		return Component{ id };
	}

	void Remove(Component component)
	{
		assert(component.is_valid() && Exists(component.GetID()));
		const script_id id{ component.GetID() };
		const id::id_type index{ idMapping[id::index(id)] };
		const script_id lastID{ entityScripts.back()->Script().GetID() };
		utl::EraseUnordered(entityScripts, index);
		idMapping[id::index(lastID)] = index;
		idMapping[id::index(id)] = id::invalid_id;
	}

	void Update(float dt)
	{
		for (auto& ptr : entityScripts)
		{
			ptr->Update(dt);
		}
	}
}

#ifdef USE_WITH_EDITOR

extern "C" __declspec(dllexport)
LPSAFEARRAY GetScriptNamesDll()
{
	const u32 size{ (u32)havana::Script::ScriptNames().size() };
	if (!size) return nullptr; // If there are no script, exit early
	CComSafeArray<BSTR> names(size);
	
	for (u32 i{ 0 }; i < size; i++)
	{
		names.SetAt(i, A2BSTR_EX(havana::Script::ScriptNames()[i].c_str()), false);
	}

	return names.Detach(); // Detach() will move the memory freeing task to the .NET side for Auto GC
}
#endif // USE_WITH_EDITOR


