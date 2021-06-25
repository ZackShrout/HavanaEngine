#pragma once

class Test
{
public:
	virtual bool Initialize() = 0;
	virtual void Run() = 0;
	virtual void Shutdown() = 0;
};