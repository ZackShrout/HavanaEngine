#pragma once

#include <iostream>
#include <ctime>
#include "Test.h"
#include "../Engine/Components/Entity.h"
#include "../Engine/Components/Transform.h"

using namespace havana;

class EngineTest : public Test
{
public:
#ifdef _WIN64
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
				m_numEntities = (u32)m_entities.size();
			}
			PrintResults();
		} while (getchar() != 'q');
	}
#elif __linux__
	virtual bool Initialize(void* disp) override
	{
		srand((u32)time(nullptr));
		return true;
	}
	
	virtual void Run(void* disp) override
	{
		do
		{
			for (u32 i{ 0 }; i < 10000; i++)
			{
				CreateRandom();
				RemoveRandom();
				m_numEntities = (u32)m_entities.size();
			}
			PrintResults();
		} while (getchar() != 'q');
	}
#endif // _WIN64

	virtual void Shutdown() override
	{
	}

private:
	void CreateRandom()
	{
		u32 count = rand() % 20;
		if (m_entities.empty()) count = 1000;
		Transform::InitInfo transformInfo{};
		Entity::EntityInfo entityInfo { &transformInfo };

		while (count > 0)
		{
			++m_added;
			Entity::Entity entity{ Entity::CreateEntity(entityInfo) };
			assert(entity.is_valid() && id::is_valid(entity.GetID()));
			m_entities.push_back(entity);
			assert(Entity::IsAlive(entity.GetID()));
			--count;
		}
	}

	void RemoveRandom()
	{
		u32 count = rand() % 20;
		if (m_entities.size() < 1000) return;

		while (count > 0)
		{
			const u32 index{ (u32)rand() % ((u32)m_entities.size()) };
			const Entity::Entity entity{ m_entities[index] };
			assert(entity.is_valid() && id::is_valid(entity.GetID()));
			if (entity.is_valid())
			{
				Entity::RemoveEntity(entity.GetID());
				m_entities.erase(m_entities.begin() + index);
				assert(!Entity::IsAlive(entity.GetID()));
				++m_removed;
			}
			--count;
		}
	}

	void PrintResults()
	{
		std::cout << "Entities added: " << m_added << std::endl;
		std::cout << "Entities removed: " << m_removed << std::endl;
	}

	utl::vector<Entity::Entity> m_entities;
	u32 m_added{ 0 };
	u32 m_removed{ 0 };
	u32 m_numEntities{ 0 };
};