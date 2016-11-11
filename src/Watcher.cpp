#include <thread>
#include "../include/Watcher.h"
#include <sys/time.h>
#include "../include/DataProcessor.h"
#include <chrono>
extern int serverSock;
Watcher::Watcher()
{

}

Watcher::~Watcher()
{

}

Watcher* Watcher::m_instance = nullptr;

Watcher* Watcher::instance()
{
	if (m_instance == nullptr) {
		m_instance = new Watcher();
	}
	return m_instance;
}

void Watcher::Run()
{
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		if (GetTime() - DataProcessor::instance()->GetLastTime().Get() < 10) {
			DataProcessor::instance()->PredictNext(serverSock);
		}
	}

}

long int Watcher::GetTime()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

void Watcher::Notify()
{

}



