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

namespace 
{
	/// <summary>
	/// Takes a Transform component from the editor, and
	/// converts it for use in the Engine.
	/// </summary>
	struct transform_component
	{
		f32 position[3];
		f32 rotation[3];
		f32 scale[3];

		transform::init_info to_init_info()
		{
			using namespace DirectX;

			transform::init_info info{};
			memcpy(&info.position[0], &position[0], sizeof(position));
			memcpy(&info.scale[0], &scale[0], sizeof(scale));
			XMFLOAT3A rot{ &rotation[0] };
			XMVECTOR quat{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot)) };
			XMFLOAT4A rot_quat{};
			XMStoreFloat4A(&rot_quat, quat);
			memcpy(&info.rotation[0], &rot_quat.x, sizeof(info.rotation));

			return info;
		}
	};

	/// <summary>
	/// Takes a Script component from the editor, and
	/// converts it for use in the Engine.
	/// </summary>
	struct script_component
	{
		script::detail::script_creator scriptCreator;

		script::init_info to_init_info()
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
	struct game_entity_descriptor
	{
		transform_component transform;
		script_component script;
	};

	game_entity::entity entity_from_id(id::id_type id)
	{
		return game_entity::entity{ game_entity::entity_id{ id } };
	}
} // anonymous namespace

/// <summary>
/// Create a game entity.
/// </summary>
/// <param name="e">- Object that describes the entity being added.</param>
/// <returns>entity_id</returns>
EDITOR_INTERFACE id::id_type
CreateGameEntity(game_entity_descriptor* e)
{
	assert(e);
	game_entity_descriptor descriptor{ *e };
	transform::init_info transform_info{ descriptor.transform.to_init_info() };
	script::init_info script_info{ descriptor.script.to_init_info() };
	game_entity::entity_info entityInfo{ &transform_info, &script_info };

	return game_entity::create(entityInfo).get_id();
}

/// <summary>
/// Removes a game entity.
/// </summary>
/// <param name="e">- Object that describes the entity being added.</param>
EDITOR_INTERFACE void
RemoveGameEntity(id::id_type id)
{
	assert(id::is_valid(id));
	game_entity::remove(game_entity::entity_id{ id });
}

