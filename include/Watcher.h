#pragma once
#include <thread>
class Watcher
{
public:
	static Watcher* instance();
	
	void Notify();
	~Watcher();
	static long int GetTime();
	std::thread& Thread();
	void Init();
private:
	Watcher();
	static Watcher* m_instance;
	static void Run();
	std::thread m_thread;
};