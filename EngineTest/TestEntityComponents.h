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
	virtual bool initialize() override
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
	virtual bool initialize(void* disp) override
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

	virtual void shutdown() override
	{
	}

private:
	void CreateRandom()
	{
		u32 count = rand() % 20;
		if (m_entities.empty()) count = 1000;
		transform::init_info transformInfo{};
		game_entity::entity_info entityInfo { &transformInfo };

		while (count > 0)
		{
			++m_added;
			game_entity::entity entity{ game_entity::create(entityInfo) };
			assert(entity.is_valid() && id::is_valid(entity.get_id()));
			m_entities.push_back(entity);
			assert(game_entity::is_alive(entity.get_id()));
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
			const game_entity::entity entity{ m_entities[index] };
			assert(entity.is_valid() && id::is_valid(entity.get_id()));
			if (entity.is_valid())
			{
				game_entity::remove(entity.get_id());
				m_entities.erase(m_entities.begin() + index);
				assert(!game_entity::is_alive(entity.get_id()));
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

	utl::vector<game_entity::entity> m_entities;
	u32 m_added{ 0 };
	u32 m_removed{ 0 };
	u32 m_numEntities{ 0 };
};