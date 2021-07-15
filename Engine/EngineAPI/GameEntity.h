#pragma once
#include "..\Components\ComponentsCommon.h"
#include "TransformComponent.h"
#include "ScriptComponent.h"

namespace Havana
{
	namespace Entity
	{
		DEFINE_TYPED_ID(entity_id);

		class Entity
		{
		public:
			constexpr Entity() : m_id{ Id::INVALID_ID } {}
			constexpr explicit Entity(entity_id id) : m_id{ id } {}
			constexpr entity_id GetID() const { return m_id; }
			constexpr bool IsValid() const { return Id::IsValid(m_id); }
			Transform::Component Transform() const;
			Script::Component Script() const;
		private:
			entity_id m_id;
		};
	} // namespace entity

	namespace Script
	{
		class EntityScript : public Entity::Entity
		{
		public:
			virtual ~EntityScript() = default;
			virtual void BeginPlay() {};
			virtual void Update(float) {};
		protected:
			constexpr explicit EntityScript(Entity entity) : Entity{ entity.GetID() } {};
		};

		namespace Detail
		{
			using script_ptr = std::unique_ptr<EntityScript>;
			using script_creator = script_ptr(*)(Entity::Entity entity);
			using string_hash = std::hash<std::string>;

			u8 RegisterScript(size_t, script_creator);
#ifdef USE_WITH_EDITOR
			extern "C" __declspec(dllexport) // GetScriptCreator needs to be exported to the dll if this is in use with the editor
#endif // USE_WITH_EDITOR
			script_creator GetScriptCreatorDll(size_t tag);

			template<class ScriptClass>
			script_ptr CreateScript(Entity::Entity entity)
			{
				assert(entity.IsValid());
				return std::make_unique<ScriptClass>(entity);
			}
			
#ifdef USE_WITH_EDITOR
			u8 AddScriptName(const char* name);
#define REGISTER_SCRIPT(TYPE)											\
			namespace													\
			{															\
				const u8 register##TYPE									\
					{ Havana::Script::Detail::RegisterScript(			\
					Havana::Script::Detail::string_hash()(#TYPE),		\
					&Havana::Script::Detail::CreateScript<TYPE>) };		\
				const u8 name##TYPE										\
					{  Havana::Script::Detail::AddScriptName(#TYPE) };	\
			}
#else
#define REGISTER_SCRIPT(TYPE)											\
			namespace													\
			{															\
				const u8 register##TYPE									\
					{ Havana::Script::Detail::RegisterScript(			\
					Havana::Script::Detail::string_hash()(#TYPE),		\
					&Havana::Script::Detail::CreateScript<TYPE>) };		\
			}
#endif // USE_WITH_EDITOR
		} // namespace detail
	} // namespace script
}