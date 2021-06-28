#pragma once

#include <iostream>
#include <ctime>
#include "Test.h"
#include "..\Engine\Components\Entity.h"
#include "..\Engine\Components\Transform.h"

using namespace Havana;

class EngineTest : public Test
{
public:
	virtual bool Initialize() override
	{
		srand((u32)time(nullptr));
		return true;
	}
	virtual void Run() override
	{
		do
		{
			for (u32 i{ 0 }; i < 10000; i++)
			{
				CreateRandom();
				RemoveRandom();
				numEntities = (u32)entities.size();
			}
			PrintResults();
		} while (getchar() != 'q');
	}
	virtual void Shutdown() override
	{
	}

private:
	void CreateRandom()
	{
		u32 count = rand() % 20;
		if (entities.empty()) count = 1000;
		Transform::InitInfo transformInfo{};
		Entity::EntityInfo entityInfo { &transformInfo };

		while (count > 0)
		{
			++added;
			Entity::Entity entity{ Entity::CreateEntity(entityInfo) };
			assert(entity.IsValid() && Id::IsValid(entity.GetID()));
			entities.push_back(entity);
			assert(Entity::IsAlive(entity.GetID()));
			--count;
		}
	}

	void RemoveRandom()
	{
		u32 count = rand() % 20;
		if (entities.size() < 1000) return;

		while (count > 0)
		{
			const u32 index{ (u32)rand() % ((u32)entities.size()) };
			const Entity::Entity entity{ entities[index] };
			assert(entity.IsValid() && Id::IsValid(entity.GetID()));
			if (entity.IsValid())
			{
				Entity::RemoveEntity(entity.GetID());
				entities.erase(entities.begin() + index);
				assert(!Entity::IsAlive(entity.GetID()));
				++removed;
			}
			--count;
		}
	}

	void PrintResults()
	{
		std::cout << "Entities added: " << added << std::endl;
		std::cout << "Entities removed: " << removed << std::endl;
	}

	Utils::vector<Entity::Entity> entities;
	u32 added{ 0 };
	u32 removed{ 0 };
	u32 numEntities{ 0 };
};