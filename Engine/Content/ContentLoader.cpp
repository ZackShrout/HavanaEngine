#include "ContentLoader.h"
#include "../Components/Entity.h"
#include "../Components/Transform.h"
#include "../Components/Script.h"
#include "../Graphics/Renderer.h"

#ifndef SHIPPING
#include <fstream>
#include <filesystem>
#ifdef _WIN64
#include <Windows.h>
#elif __linux__
//
#endif

namespace Havana::Content
{
	namespace
	{
		enum ComponentType
		{
			Transform,
			Script,

			Count
		};

		Utils::vector<Entity::Entity> entities;
		Transform::InitInfo transformInfo{};
		Script::InitInfo scriptInfo{};

		bool ReadTransform(const u8*& data, Entity::EntityInfo& info)
		{
			f32 rotation[3];

			assert(!info.transform);

			memcpy(&transformInfo.position[0], data, sizeof(transformInfo.position)); data += sizeof(transformInfo.position);
			memcpy(&rotation[0], data, sizeof(rotation)); data += sizeof(rotation);
			memcpy(&transformInfo.scale[0], data, sizeof(transformInfo.scale)); data += sizeof(transformInfo.scale);

#ifdef _WIN64
			using namespace DirectX;
			
			// convert rotation from a vector3 as used in the editor to quaternion as used in engine
			XMFLOAT3A rot{ &rotation[0] };
			XMVECTOR quat{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot)) };
			XMFLOAT4A rotQuat {};
			XMStoreFloat4A(&rotQuat, quat);
			memcpy(&transformInfo.rotation[0], &rotQuat.x, sizeof(transformInfo.rotation));
#elif __linux__
			// TODO: implement
#endif // _WIN64
			
			info.transform = &transformInfo;

			return true;
		}

		bool ReadScript(const u8*& data, Entity::EntityInfo& info)
		{
			assert(!info.script);
			const u32 nameLength{ *data }; data += sizeof(u32);
			
			if (!nameLength) return false;

			// if a script name is greater than 255 character, something is wrong
			assert(nameLength < 256);

			char scriptName[256];
			memcpy(&scriptName, data, nameLength); data += nameLength;
			// make scriptName a 0-terminated c-string
			scriptName[nameLength] = 0;
			scriptInfo.script_creator = Script::Detail::GetScriptCreatorDll(Script::Detail::string_hash()(scriptName));
			
			info.script = &scriptInfo;
			
			return scriptInfo.script_creator != nullptr;
		}

		using component_reader = bool(*)(const u8*&, Entity::EntityInfo&);
		component_reader componentReaders[]{ ReadTransform, ReadScript };
		static_assert(_countof(componentReaders) == ComponentType::Count);

		bool ReadFile(std::filesystem::path path, std::unique_ptr<u8[]>& data, u64& size)
		{
			if (!std::filesystem::exists(path)) return false;

			size = std::filesystem::file_size(path);
			assert(size);
			if (!size) return false;

			data = std::make_unique<u8[]>(size);
			std::ifstream file{ path, std::ios::in | std::ios::binary };
			if (!file || !file.read((char*)data.get(), size))
			{
				file.close();
				return false;
			}

			file.close();
			return true;
		}
	} // anonymous namespace

	bool LoadGame()
	{
		// read game.bin and create entities
		std::unique_ptr<u8[]> gameData{};
		u64 size{ 0 };
		if (!ReadFile("game.bin", gameData, size)) return false;
		assert(gameData.get());
		const u8* at{ gameData.get() };
		constexpr u32 su32{ sizeof(u32) };
		const u32 numEntities{ *at }; at += su32;
		
		if (!numEntities) return false;

		for (u32 entityIndex{ 0 }; entityIndex < numEntities; entityIndex++)
		{
			Entity::EntityInfo info{};
			const u32 entityType{ *at }; at += su32;
			const u32 numComponents{ *at }; at += su32;
			
			if (!numComponents) return false;

			for (u32 componentIndex{ 0 }; componentIndex < numComponents; componentIndex++)
			{
				const u32 componentType{ *at }; at += su32;
				assert(componentType < ComponentType::Count);
				if (!componentReaders[componentType](at, info)) return false;
			}

			// create entity
			assert(info.transform);
			Entity::Entity entity{ Entity::CreateEntity(info) };
			if (!entity.IsValid()) return false;
			entities.emplace_back(entity);
		}

		assert(at == gameData.get() + size);
		return true;
	}

	void UnloadGame()
	{
		for (auto entity : entities)
		{
			Entity::RemoveEntity(entity.GetID());
		}
	}

	bool LoadEngineShaders(std::unique_ptr<u8[]>& shaders, u64& size)
	{
		auto path = Graphics::GetEngineShadersPath();
		
		return ReadFile(path, shaders, size);
	}
}

#endif // !SHIPPING