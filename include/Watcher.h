#pragma once
class Watcher
{
public:
	static Watcher* instance();
	void Run();
	void Notify();
	~Watcher();
	long int GetTime();
private:
	Watcher();
	static Watcher* m_instance;
};