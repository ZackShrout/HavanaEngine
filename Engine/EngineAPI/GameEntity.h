#pragma once
#include "../Components/ComponentsCommon.h"
#include "TransformComponent.h"
#include "ScriptComponent.h"

namespace havana
{
	namespace game_entity
	{
		DEFINE_TYPED_ID(entity_id);

		class entity
		{
		public:
			constexpr entity() : m_id{ id::invalid_id } {}
			constexpr explicit entity(entity_id id) : m_id{ id } {}
			constexpr entity_id get_id() const { return m_id; }
			constexpr bool is_valid() const { return id::is_valid(m_id); }
			transform::component transform() const;
			script::component script() const;
		private:
			entity_id m_id;
		};
	} // namespace entity

	namespace script
	{
		class EntityScript : public game_entity::entity
		{
		public:
			virtual ~EntityScript() = default;
			virtual void BeginPlay() {};
			virtual void update(float) {};
		protected:
			constexpr explicit EntityScript(entity entity) : entity{ entity.get_id() } {};
		};

		namespace detail
		{
			using script_ptr = std::unique_ptr<EntityScript>;
			using script_creator = script_ptr(*)(game_entity::entity entity);
			using string_hash = std::hash<std::string>;

			u8 register_script(size_t, script_creator);
#ifdef USE_WITH_EDITOR
			extern "C" __declspec(dllexport) // GetScriptCreator needs to be exported to the dll if this is in use with the editor
#endif // USE_WITH_EDITOR
			script_creator get_script_creator(size_t tag);

			template<class ScriptClass>
			script_ptr CreateScript(game_entity::entity entity)
			{
				assert(entity.is_valid());
				return std::make_unique<ScriptClass>(entity);
			}
			
#ifdef USE_WITH_EDITOR
			u8 add_script_name(const char* name);
#define REGISTER_SCRIPT(TYPE)											\
			namespace													\
			{															\
				const u8 register##TYPE									\
					{ havana::script::detail::register_script(			\
					havana::script::detail::string_hash()(#TYPE),		\
					&havana::script::detail::CreateScript<TYPE>) };		\
				const u8 name##TYPE										\
					{  havana::script::detail::add_script_name(#TYPE) };	\
			}
#else
#define REGISTER_SCRIPT(TYPE)											\
			namespace													\
			{															\
				const u8 register##TYPE									\
					{ havana::script::detail::register_script(			\
					havana::script::detail::string_hash()(#TYPE),		\
					&havana::script::detail::CreateScript<TYPE>) };		\
			}
#endif // USE_WITH_EDITOR
		} // namespace detail
	} // namespace script
}