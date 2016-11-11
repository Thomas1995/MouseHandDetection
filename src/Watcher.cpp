#include <thread>
#include "../include/Watcher.h"
#include <sys/time.h>
#include "../include/DataProcessor.h"
#include <chrono>
extern int serverSock;
#define WATCHER_SLEEP_TIME 10
#define WATCHER_DIFTIME_FOR_PREDICTION 30
Watcher::Watcher()
{
	std::thread(Run);
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
		std::this_thread::sleep_for(std::chrono::milliseconds(WATCHER_SLEEP_TIME));
		if (GetTime() - DataProcessor::instance()->GetLastTime().Get() < WATCHER_DIFTIME_FOR_PREDICTION) {
			printf("Predicting next move..\n");
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



