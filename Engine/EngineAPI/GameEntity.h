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
			constexpr Entity() : id{ Id::INVALID_ID } {}
			constexpr explicit Entity(entity_id id) : id{ id } {}
			constexpr entity_id GetID() const { return id; }
			constexpr bool IsValid() const { return Id::IsValid(id); }
			Transform::Component Transform() const;
			Script::Component Script() const;
		private:
			entity_id id;
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

			template<class ScriptClass>
			script_ptr CreateScript(Entity::Entity entity)
			{
				assert(entity.IsValid());
				return std::make_unique<ScriptClass>(entity);
			}
		} // namespace detail

#define REGISTER_SCRIPT(TYPE)										\
		class TYPE;													\
		namespace													\
		{															\
			const u8 register##TYPE									\
				{ Havana::Script::Detail::RegisterScript(			\
				Havana::Script::Detail::string_hash()(#TYPE),		\
				&Havana::Script::Detail::CreateScript<TYPE>) };	\
		}

	} // namespace script
}