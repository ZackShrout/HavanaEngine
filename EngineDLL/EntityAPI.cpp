//******************************************
//Havana Engine API for use in Havana Editor
//		     ***Entities***
//******************************************
#include "..\Common\Id.h"
#include "..\Components\Entity.h"
#include "..\Components\Transform.h"
#include "..\Components\Script.h"
#include "Common.h"

using namespace Havana;

namespace // anonymous namespace
{
	/// <summary>
	/// Takes a Transform component from the editor, and
	/// converts it for use in the Engine.
	/// </summary>
	struct TransformComponent
	{
		f32 position[3];
		f32 rotation[3];
		f32 scale[3];

		Transform::InitInfo ToInitInfo()
		{
			using namespace DirectX;

			Transform::InitInfo info{};
			memcpy(&info.position[0], &position[0], sizeof(position));
			memcpy(&info.scale[0], &scale[0], sizeof(scale));
			XMFLOAT3A rot{ &rotation[0] };
			XMVECTOR quat{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot)) };
			XMFLOAT4A rotQuat{};
			XMStoreFloat4A(&rotQuat, quat);
			memcpy(&info.rotation[0], &rotQuat.x, sizeof(info.rotation));

			return info;
		}
	};

	/// <summary>
	/// Takes a Script component from the editor, and
	/// converts it for use in the Engine.
	/// </summary>
	struct ScriptComponent
	{
		Script::Detail::script_creator scriptCreator;

		Script::InitInfo ToInitInfo()
		{
			Script::InitInfo info{};
			info.script_creator = scriptCreator;
			return info;
		}
	};

	/// <summary>
	/// Takes a Game Entity from the editor, and
	/// converts it for use in the Engine.
	/// </summary>
	struct GameEntityDescriptor
	{
		TransformComponent transform;
		ScriptComponent script;
	};

	Entity::Entity EntityFromId(Id::id_type id)
	{
		return Entity::Entity{ Entity::entity_id{ id } };
	}
}

/// <summary>
/// Create a game entity.
/// </summary>
/// <param name="e">- Object that describes the entity being added.</param>
/// <returns>EntityID</returns>
EDITOR_INTERFACE
Id::id_type CreateGameEntity(GameEntityDescriptor* e)
{
	assert(e);
	GameEntityDescriptor descriptor{ *e };
	Transform::InitInfo transformInfo{ descriptor.transform.ToInitInfo() };
	Script::InitInfo scriptInfo{ descriptor.script.ToInitInfo() };
	Entity::EntityInfo entityInfo{ &transformInfo, &scriptInfo };

	return Entity::CreateEntity(entityInfo).GetID();
}

/// <summary>
/// Removes a game entity.
/// </summary>
/// <param name="e">- Object that describes the entity being added.</param>
EDITOR_INTERFACE
void RemoveGameEntity(Id::id_type id)
{
	assert(Id::IsValid(id));
	Entity::RemoveEntity(Entity::entity_id{ id });
}

