#include "../include/DataProcessor.h"
#include <sys/socket.h>
#include "../include/CursorData.h"
#include <sys/time.h>
#include <cstdio>
DataProcessor* DataProcessor::m_instance = nullptr;
DataProcessor* DataProcessor::instance()
{
    if (m_instance == nullptr) {
        m_instance = new DataProcessor();
        

    }
    return m_instance;
}
DataProcessor::DataProcessor()
{
	m_timer = getTime();
}

long int DataProcessor::getTime()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}
void DataProcessor::SendCursorData(int x, int y, int state, int socket)
{
    if (m_queuedData.size() > 50) {
	printf("Clearing queue..\n");
	std::queue<CursorData> empty;
	std::swap(m_queuedData, empty);
    }

    m_queuedData.push(CursorData(x,y,state));

    if (getTime() - m_timer > 5) {
	m_timer = getTime();
	char* msg = m_queuedData.front().ToMsg();
	m_queuedData.pop();
	send(socket, msg, 5, 0);
	delete msg;
    }




}
