#pragma once
#include <thread>
#include <chrono>
#include <string>

// What test are we performing?
#define TEST_ENTITY_COMPONENTS 0
#define TEST_WINDOW 0
#define TEST_RENDERER 1

class Test
{
public:
#ifdef _WIN64
	virtual bool Initialize() = 0;
	virtual void Run() = 0;
#elif __linux__
	virtual bool Initialize(void* disp) = 0;
	virtual void Run(void* disp) = 0;
#endif
	virtual void Shutdown() = 0;
};

#ifdef _WIN64
#include <Windows.h>
#else
#include <iostream>
#endif // _WIN64

class TimeIt
{
public:
	using clock = std::chrono::steady_clock;
	using time_stamp = std::chrono::steady_clock::time_point;

	void Begin()
	{
		m_start = clock::now();
	}
	
	void End()
	{
		auto dt = clock::now() - m_start;
		m_msAvg += ((float)std::chrono::duration_cast<std::chrono::milliseconds>(dt).count() - m_msAvg) / (float)m_counter;
		++m_counter;

		if (std::chrono::duration_cast<std::chrono::seconds>(clock::now() - m_seconds).count() >= 1)
		{
#ifdef _WIN64
			OutputDebugStringA("Avg. frame (ms): ");
			OutputDebugStringA(std::to_string(m_msAvg).c_str());
			OutputDebugStringA((" " + std::to_string(m_counter)).c_str());
			OutputDebugStringA(" fps");
			OutputDebugStringA("\n");
#else
			std::cout << "Avg. frame (ms): ";
			std::cout << std::to_string(m_msAvg).c_str();
			std::cout << (" " + std::to_string(m_counter)).c_str();
			std::cout << " fps" << std::endl;
#endif // _WIN64
			m_msAvg = 0.0f;
			m_counter = 1;
			m_seconds = clock::now();
		}
	}
private:
	float		m_msAvg{ 0.0f };
	int			m_counter{ 1 };
	time_stamp	m_start;
	time_stamp	m_seconds{ clock::now() };
};