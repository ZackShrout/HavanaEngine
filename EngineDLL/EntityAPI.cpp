//******************************************
//Havana Engine API for use in Havana Editor
//		     ***Entities***
//******************************************
#include "Id.h"
#include "CommonHeaders.h"
#include "Components\Entity.h"
#include "Components\Transform.h"
#include "Components\Script.h"
#include "Common.h"

using namespace havana;

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

		transform::init_info ToInitInfo()
		{
			using namespace DirectX;

			transform::init_info info{};
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
		script::detail::script_creator scriptCreator;

		script::init_info ToInitInfo()
		{
			script::init_info info{};
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

	game_entity::entity EntityFromId(id::id_type id)
	{
		return game_entity::entity{ game_entity::entity_id{ id } };
	}
}

/// <summary>
/// Create a game entity.
/// </summary>
/// <param name="e">- Object that describes the entity being added.</param>
/// <returns>EntityID</returns>
EDITOR_INTERFACE
id::id_type CreateGameEntity(GameEntityDescriptor* e)
{
	assert(e);
	GameEntityDescriptor descriptor{ *e };
	transform::init_info transformInfo{ descriptor.transform.ToInitInfo() };
	script::init_info scriptInfo{ descriptor.script.ToInitInfo() };
	game_entity::entity_info entityInfo{ &transformInfo, &scriptInfo };

	return game_entity::create(entityInfo).get_id();
}

/// <summary>
/// Removes a game entity.
/// </summary>
/// <param name="e">- Object that describes the entity being added.</param>
EDITOR_INTERFACE
void RemoveGameEntity(id::id_type id)
{
	assert(id::is_valid(id));
	game_entity::remove(game_entity::entity_id{ id });
}

